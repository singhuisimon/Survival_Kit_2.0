/**
 * @file Renderer.cpp
 * @brief Core rendering system for managing the graphics pipeline
 * @details This class encapsulates the entire rendering workflow including
 *          mesh management, shader programs, render passes, framebuffers,
 *          and material systems. It provides a high-level interface for
 *          rendering 3D scenes using OpenGL 4.6.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Graphics/Renderer.h"
#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <GLFW/glfw3.h>

#pragma region NAMESPACE

namespace {

	// Testing values
	constexpr int width = 1280, height = 720;

	inline std::vector<Engine::ShaderProgram> loadShaderPrograms(std::vector<std::pair<std::string, std::string>> shaders) {

		std::vector<Engine::ShaderProgram> shadersStorage;

		for (auto const& file : shaders) {
			// Create the shader files vector with types 
			std::vector<std::pair<GLenum, std::string>> shader_files;
			shader_files.emplace_back(std::make_pair(GL_VERTEX_SHADER, file.first));
			shader_files.emplace_back(std::make_pair(GL_FRAGMENT_SHADER, file.second));

			// Create new shader program
			Engine::ShaderProgram shader_program;

			// Use Graphics_Manager to compile the shader
			if (!shader_program.compileShader(shader_files)) {
				throw std::runtime_error("failed to compile shaders");
			}

			// Insert shader program into vector
			shadersStorage.emplace_back(shader_program);
		}

		return shadersStorage;
	}

	inline void test_load_shaders(std::vector<Engine::ShaderProgram>& shd) {


		
		std::string vertex_obj_path{ Engine::getAssetFilePath("Sources/Shaders/survival_kit_obj.vert") };
		std::string fragment_obj_path{ Engine::getAssetFilePath("Sources/Shaders/survival_kit_obj.frag") };

		std::string vertex_debug_path{ Engine::getAssetFilePath("Sources/Shaders/debug.vert")};
		std::string fragment_debug_path{ Engine::getAssetFilePath("Sources/Shaders/debug.frag")};

		

		// Pair vertex and fragment shader files
		std::vector<std::pair<std::string, std::string>> shader_files{
			std::make_pair(vertex_obj_path, fragment_obj_path),
			std::make_pair(vertex_debug_path, fragment_debug_path)
		};

		shd = loadShaderPrograms(shader_files);
	}

	inline void load_basic_primitives(std::vector<Engine::MeshGL>& ms, std::vector<Engine::MeshData>& md) {

		Engine::MeshData cd = Engine::make_cube();
		Engine::MeshData pd = Engine::make_plane();
		Engine::MeshData sd = Engine::make_sphere();

		md.push_back(cd);
		md.push_back(pd);
		md.push_back(sd);

		Engine::MeshGL c = Engine::upload_mesh_data(cd);
		Engine::MeshGL p = Engine::upload_mesh_data(pd);
		Engine::MeshGL s = Engine::upload_mesh_data(sd);

		ms.push_back(std::move(c));
		ms.push_back(std::move(p));
		ms.push_back(std::move(s));
	}
	
}

namespace Engine {

	namespace {
		int  selected_texture = 0;
		bool textureMode = false;
		bool isPBR = false;
		bool isDebug = false;
		bool isGamma = false;
	}

}

#pragma endregion

namespace Engine {

	Renderer::Renderer(Camera3D& cam, Light& light) : editor_camera(cam), editor_light(light) { }

	// On first load, setup some simple stuff
	void Renderer::setup() {

		// Load OpenGL function pointers with GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
			LOG_ERROR("Renderer::setup() - Failed to load OpenGL, GLAD failed to initialized");
		}
		else {
			LOG_TRACE("Renderer::setup() - GLAD initialized successfuly");
		}

		LOG_INFO("OpenGL initialized");
		LOG_INFO("  Vendor:   ", (const char*)glGetString(GL_VENDOR));
		LOG_INFO("  Renderer: ", (const char*)glGetString(GL_RENDERER));
		LOG_INFO("  Version:  ", (const char*)glGetString(GL_VERSION));
		LOG_INFO("  GLSL:     ", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

		// Temporary functions, used for testing only
		test_load_shaders(m_gl.m_shader_storage);

		// Load a set of basic primitives: Cube, Plane, Sphere
		load_basic_primitives(m_gl.m_mesh_storage, m_gl.m_mesh_data_storage);

		// Create a framebuffer and configure it's settings
		auto fp_fbo = FrameBuffer::create();
		if (fp_fbo.has_value()) {
			m_framebuffers.push_back(std::move(*fp_fbo));
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to create framebuffer!");
		}

		// Allocate storage for a texture on the GPU, this texture will be attached to the framebuffer
		auto fp_tex = Texture::alloc_storage_on_gpu(width, height);
		if (fp_tex.has_value()) {
			m_gl.m_textures.push_back(std::move(*fp_tex));
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to allocate storage on the GPU!");
		}

		// Allocate extra attachments to the framebuffer

		// Testing
		GLuint rboDepth;
		glCreateRenderbuffers(1, &rboDepth);
		glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT24, width, height);
		// end testing

		auto& fpfbo_ = m_framebuffers[0];
		auto& fptex_ = m_gl.m_textures[0];

		fpfbo_.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(fptex_.handle()));
		fpfbo_.attach_depth(GL_DEPTH_ATTACHMENT, rboDepth);

		// Create a render pass for that framebuffer
		RenderPass first_pass
		{
			.pass_name = "First Pass",
			.fbo_handle = 0,
			.shdpgm_handle = 0

			// Leave the rest as default settings
		};

		// Register the pass with the renderer
		m_passes.push_back(first_pass);


#pragma region TEST_TO_SEE_TEXTURE_PASS_TEMP

		{
			RenderPass stub_pass
			{
				.pass_name = "Stub Pass",
				.fbo_handle = 0,
				.shdpgm_handle = 0

			};

			//m_passes.push_back(stub_pass);
		}

#pragma endregion

		if (isDebug) {

		}

		RenderPass debug_pass
		{
			.pass_name = "Debug Pass",
			.fbo_handle = 0,
			.shdpgm_handle = 1,
			.clear_color = false,
			.clear_depth = false,
			.depth_write = false,
			.culling = false,
			.passtype = PassType::DEBUGGING
		};

		//m_passes.push_back(debug_pass);

#if 0
#pragma region TEXTURE_LOAD_TEMP
		{

			// Temporarily load textures 
			for (const auto& entry : std::filesystem::directory_iterator(getAssetFilePath("Textures/"))) {
				if (entry.is_regular_file()) {

					auto path = entry.path();

					if (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".jpeg") {
						auto tex = Texture::load_from_file(path.string(), TextureDesc(false, false, true));
						if (tex && tex->valid()) {
							t_testing_textures.push_back(std::move(*tex));
						}
					}
				}
			}
		}
#pragma endregion
#endif

#pragma region MATERIAL_LOAD_TEMP
		{
			Material mat1 = Material(glm::vec3(0.3f, 0.5f, 0.9f), glm::vec3(0.3f, 0.5f, 0.9f), glm::vec3(0.8f, 0.8f, 0.8f), 100.0f);
			Material mat2 = Material(glm::vec3(0.9f, 0.5f, 0.3f), glm::vec3(0.9f, 0.5f, 0.3f), glm::vec3(0.8f, 0.8f, 0.8f), 100.0f);
			m_gl.t_testing_material.emplace_back(mat1);
			m_gl.t_testing_material.emplace_back(mat2);
		}
#pragma endregion

		LOG_TRACE("Renderer::setup() - Renderer started successfully!");
	}

	void Renderer::beginFrame(RenderPass const& pass) {

		auto& fbo = m_framebuffers[pass.fbo_handle];
		//glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fbo.handle()));

		auto& viewport = pass.view_port;
		glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y), 
				   static_cast<GLsizei>(viewport.z), static_cast<GLsizei>(viewport.w));

		pass.depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(pass.depth_write ? GL_TRUE : GL_FALSE);

		if (pass.culling) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);      
			glFrontFace(GL_CCW);
		}
		else {
			glDisable(GL_CULL_FACE);
		}

		if (pass.blending) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquation(GL_FUNC_ADD);
		}
		else {
			glDisable(GL_BLEND);
		}

		GLbitfield clear_mask = 0;

		if (pass.clear_color) {
			glClearColor(pass.clear_color_value.r,
				pass.clear_color_value.g,
				pass.clear_color_value.b,
				pass.clear_color_value.a);
			clear_mask |= GL_COLOR_BUFFER_BIT;
		}

		if (pass.clear_depth) {
			// If depth writes are disabled *before* clear, enable them just for clearing
			glDepthMask(GL_TRUE);
			clear_mask |= GL_DEPTH_BUFFER_BIT;
		}

		// Finally clear whatever bits were requested
		if (clear_mask != 0) {
			glClear(clear_mask);
		}

		glDepthMask(pass.depth_write ? GL_TRUE : GL_FALSE);

		auto& prog = m_gl.m_shader_storage[pass.shdpgm_handle];
		prog.programUse();
	}

	void Renderer::render_frame(std::span<const DrawItem> draw_items) {

		for (const auto& pass : m_passes) {

			if (!isDebug && (pass.passtype == PassType::DEBUGGING)) { continue; }

			beginFrame(pass);
			draw(pass, draw_items);
			endFrame(pass);
		}
	}

	void Renderer::draw(RenderPass const& pass, std::span<const DrawItem> draw_items) {


		auto& prog = m_gl.m_shader_storage[pass.shdpgm_handle];

		prog.setUniform("V", editor_camera.getLookAt());                // View transform
		prog.setUniform("P", editor_camera.getPerspective());           // Perspective transform

		prog.setUniform("light.position", editor_light.getLightPos());      // Position
		prog.setUniform("light.La", editor_light.getLightAmbient());        // Ambient
		prog.setUniform("light.Ld", editor_light.getLightDiffuse());        // Diffuse
		prog.setUniform("light.Ls", editor_light.getLightSpecular());       // Specular


#pragma region SET_UNIFORM_TEMP
		if (textureMode) {
			glBindTextureUnit(0, static_cast<GLuint>(m_gl.t_testing_textures[selected_texture].handle()));
			prog.setUniform("Texture2D", 0);
			prog.setUniform("isTexture", true);

			if (m_gl.t_testing_textures[selected_texture].is_srgb()) {
				prog.setUniform("isGamma", true);
			}
			else {
				prog.setUniform("isGamma", false);
			}
			
		}
		else {
			prog.setUniform("isTexture", false);
		}

		if (isPBR) {
			prog.setUniform("isPBR", true);
		}
		else {
			prog.setUniform("isPBR", false);
		}

#pragma endregion

		for (const auto& item : draw_items) {
			
			if (pass.passtype == PassType::DEBUGGING) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.f, -1.f);
				glLineWidth(1.0f);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

#pragma region TESTING
			size_t material_handle = static_cast<size_t>(item.m_material_handle);
			Material& test_material = m_gl.t_testing_material[material_handle];
#pragma endregion

			// Temporary transformations
			prog.setUniform("M", item.m_model_to_world_transform); // Model transform
			prog.setUniform("material.Ka", test_material.getMaterialAmbient());
			prog.setUniform("material.Kd", test_material.getMaterialDiffuse());
			prog.setUniform("material.Ks", test_material.getMaterialSpecular());
			prog.setUniform("material.shininess", test_material.getMaterialShininess());

			size_t mesh_handle = static_cast<size_t>(item.m_mesh_handle);
			m_gl.m_mesh_storage[mesh_handle].vao.bind();

			GLenum  primitive  = m_gl.m_mesh_storage[mesh_handle].primitive_type;
			GLsizei draw_count = m_gl.m_mesh_storage[mesh_handle].draw_count;
			GLenum  index_type = m_gl.m_mesh_storage[mesh_handle].index_type;

			glDrawElements(primitive, draw_count, index_type, NULL);
			glBindVertexArray(0);
		}
	}

	void Renderer::endFrame(RenderPass const& pass) {
		auto& prog = m_gl.m_shader_storage[pass.shdpgm_handle];
		prog.programFree();
		glBindTextureUnit(0, 0);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
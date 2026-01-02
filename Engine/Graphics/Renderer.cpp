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
#include "../Asset/ResourceHelpers.h"
#include "../Asset/ResourceManager.h"

// TESTING
#include "../Graphics/stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <GLFW/glfw3.h>

#include "Asset/ResourceData.h"

#pragma region NAMESPACE

namespace {

	// Testing values
	constexpr int width = 1280, height = 720;
	constexpr Engine::u32 NO_HIT = 0xFFFFFFFFu;

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

		std::string vertex_debug_path{ Engine::getAssetFilePath("Sources/Shaders/debug.vert") };
		std::string fragment_debug_path{ Engine::getAssetFilePath("Sources/Shaders/debug.frag") };

		std::string vertex_obj_picking_path{ Engine::getAssetFilePath("Sources/Shaders/object_picking.vert") };
		std::string fragment_obj_picking_path{ Engine::getAssetFilePath("Sources/Shaders/object_picking.frag") };

		std::string vertex_skybox_path{ Engine::getAssetFilePath("Sources/Shaders/skybox.vert") };
		std::string fragment_skybox_path{ Engine::getAssetFilePath("Sources/Shaders/skybox.frag") };

		std::string vertex_hdr_path{ Engine::getAssetFilePath("Sources/Shaders/hdr.vert") };
		std::string fragment_hdr_path{ Engine::getAssetFilePath("Sources/Shaders/hdr.frag") };

		std::string vertex_ui_path{ Engine::getAssetFilePath("Sources/Shaders/ui.vert") };
		std::string fragment_ui_path{ Engine::getAssetFilePath("Sources/Shaders/ui.frag") };

		std::string fragment_bloom_downsample_path{ Engine::getAssetFilePath("Sources/Shaders/bloom_downsample.frag") };
		std::string fragment_bloom_upsample_path{ Engine::getAssetFilePath("Sources/Shaders/bloom_upsample.frag") };

		// Pair vertex and fragment shader files
		std::vector<std::pair<std::string, std::string>> shader_files{
			std::make_pair(vertex_obj_path, fragment_obj_path),
			std::make_pair(vertex_debug_path, fragment_debug_path),
			std::make_pair(vertex_obj_picking_path, fragment_obj_picking_path),
			std::make_pair(vertex_skybox_path, fragment_skybox_path),
			std::make_pair(vertex_hdr_path, fragment_hdr_path),
			std::make_pair(vertex_ui_path, fragment_ui_path),
			std::make_pair(vertex_hdr_path, fragment_bloom_downsample_path),
			std::make_pair(vertex_hdr_path, fragment_bloom_upsample_path)
		};

		shd = loadShaderPrograms(shader_files);
	}

	inline void load_basic_primitives(std::vector<Engine::MeshGL>& ms, std::vector<Engine::MeshData>& md, std::vector<Engine::MeshData2D>& md2d) {

		Engine::MeshData cd = Engine::make_cube();
		Engine::MeshData pd = Engine::make_plane();
		Engine::MeshData sd = Engine::make_sphere();
		Engine::MeshData2D qd = Engine::make_quad();

		md.push_back(cd);
		md.push_back(pd);
		md.push_back(sd);
		md2d.push_back(qd);

		Engine::MeshGL c = Engine::upload_mesh_data(cd);
		Engine::MeshGL p = Engine::upload_mesh_data(pd);
		Engine::MeshGL s = Engine::upload_mesh_data(sd);
		Engine::MeshGL q = Engine::upload_mesh_data2D(qd);

		ms.push_back(std::move(c));
		ms.push_back(std::move(p));
		ms.push_back(std::move(s));
		ms.push_back(std::move(q));
	}

	unsigned int loadCubemap(std::vector<std::string> faces)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return textureID;
	}

	unsigned int loadCubemapHDR(std::vector<std::string> faces)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			// Load as float data instead of unsigned char
			float* data = stbi_loadf(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				// Use HDR internal format
				GLenum internalFormat = (nrChannels == 4) ? GL_RGBA16F : GL_RGB16F;
				GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, internalFormat, width, height, 0, format, GL_FLOAT, data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap HDR tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return textureID;
	}

}

namespace Engine {

	namespace {
		int  selected_texture = 0;
		bool textureMode = false;
		bool isDebug = false;
	}

}

#pragma endregion

namespace Engine {

	Renderer::Renderer(Camera3D& cam) : editor_camera(cam) {}

	// On first load, setup some simple stuff
	void Renderer::setup() {

		// Load OpenGL function pointers with GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			LOG_ERROR("Renderer::setup() - Failed to load OpenGL, GLAD failed to initialized");
		}
		else {
			LOG_TRACE("Renderer::setup() - GLAD initialized successfuly");

			// Load OpenGL information upon successfully load
			LOG_INFO("OpenGL initialized");
			LOG_INFO("  Vendor:   ", (const char*)glGetString(GL_VENDOR));
			LOG_INFO("  Renderer: ", (const char*)glGetString(GL_RENDERER));
			LOG_INFO("  Version:  ", (const char*)glGetString(GL_VERSION));
			LOG_INFO("  GLSL:     ", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		}

		// Temporary functions, used for testing only
		test_load_shaders(m_gl.m_shader_storage);

		// Load a set of basic primitives: Cube, Plane, Sphere
		load_basic_primitives(m_gl.m_mesh_storage, m_gl.m_mesh_data_storage, m_gl.m_mesh_data2d_storage);

		// Load skybox mesh
		MeshData skybox_cube = make_cube();
		m_skybox = upload_mesh_data(skybox_cube);

		// Load skybox textures
		std::vector<std::string> faces = {
				Engine::getAssetFilePath("Sources/Textures/Skybox_Engine_v1_001.png"),
				Engine::getAssetFilePath("Sources/Textures/Skybox_Engine_v1_002.png"),
				Engine::getAssetFilePath("Sources/Textures/Skybox_Engine_v1_003.png"),
				Engine::getAssetFilePath("Sources/Textures/Skybox_Engine_v1_004.png"),
				Engine::getAssetFilePath("Sources/Textures/Skybox_Engine_v1_005.png"),
				Engine::getAssetFilePath("Sources/Textures/Skybox_Engine_v1_006.png")
		};

		m_skybox_texture = loadCubemapHDR(faces);

		// Create a fullscreen quad where the final render output is drawn onto
		MeshData2D quad = make_quad();
		m_fullscreen_quad = upload_mesh_data2D(quad);

#pragma region TESTING LOADING UBO FOR MATERIALS
		// -------- Materials UBO (binding = 1)  --------
		glCreateBuffers(1, &m_materialUBO);
		glNamedBufferData(m_materialUBO, sizeof(MaterialUBO_Std140), nullptr, GL_DYNAMIC_DRAW);

		// Bind the buffer to binding point 1 (match `binding = 1` in GLSL)
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_materialUBO);
		auto bindMaterialBlock = [&](GLuint programID) {
			GLuint blockIndex = glGetUniformBlockIndex(programID, "MaterialBlock");
			if (blockIndex != GL_INVALID_INDEX) {
				glUniformBlockBinding(programID, blockIndex, 1);
			}
			};

		// Call for the object shader(s) that read material
		bindMaterialBlock(m_gl.m_shader_storage[0].getShaderProgramHandle()); // adjust accessor if different
#pragma endregion

#pragma region TESTING LOADING UBO FOR LIGHTINGS
		// -------- Lights UBO (binding = 0)  --------
		glCreateBuffers(1, &m_lightsUBO);
		glNamedBufferData(m_lightsUBO, sizeof(LightsBlockGPU), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_lightsUBO);

		auto bindLightsBlock = [&](GLuint programID) {
			GLuint blockIndex = glGetUniformBlockIndex(programID, "LightsBlock");
			if (blockIndex != GL_INVALID_INDEX) {
				glUniformBlockBinding(programID, blockIndex, 0);
			}
			};

		// Call for the object shader(s) that read light
		bindLightsBlock(m_gl.m_shader_storage[0].getShaderProgramHandle());
#pragma endregion

		// Set default picked ID 
		pickedID = NO_HIT;

		// Set default editor camera toggle
		isEditorCamOn = true;

		// Create a framebuffer for ImGui editor and configure its settings
		auto fp_fbo = FrameBuffer::create();
		if (fp_fbo.has_value()) {
			m_framebuffers.push_back(std::move(*fp_fbo));
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to create framebuffer!");
		}

		// Allocate storage for a texture on the GPU, this texture will be attached to the framebuffer
		auto fp_tex = Texture::alloc_storage_on_gpu(width, height, GL_RGBA16F);
		if (fp_tex.has_value()) {
			m_gl.m_textures.push_back(std::move(*fp_tex));
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to allocate storage on the GPU!");
		}

		// Allocate extra attachments to the framebuffer
		GLuint rboDepth;
		glCreateRenderbuffers(1, &rboDepth);
		glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT24, width, height);
		temp_rbo = rboDepth;

		auto& fpfbo_ = m_framebuffers[0];
		auto& fptex_ = m_gl.m_textures[0];

		// Use depth renderbuffer for attaching to editor
		fpfbo_.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(fptex_.handle()));
		fpfbo_.attach_renderbuffer(GL_DEPTH_ATTACHMENT, rboDepth);

		auto gpu_fbo = FrameBuffer::create();
		if (gpu_fbo.has_value()) {
			m_framebuffers.push_back(std::move(*gpu_fbo));
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to create GPU ID framebuffer!");
		}

		// Create a single channel integer texture to store entity IDs
		auto gpu_tex = Texture::alloc_storage_on_gpu(width, height, GL_R32UI);
		if (gpu_tex.has_value()) {
			m_gl.m_textures.push_back(std::move(*gpu_tex));
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to allocate GPU ID storage on the GPU!");
		}

		// Use the same depth renderbuffer for render pass
		auto& gpufbo_ = m_framebuffers[1];
		auto& gputex_ = m_gl.m_textures[1];
		gpufbo_.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(gputex_.handle()));
		gpufbo_.attach_renderbuffer(GL_DEPTH_ATTACHMENT, rboDepth);

		// Make sure the GPU-ID FBO has correct draw/read buffers and is cleared
		const GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
		gpufbo_.set_draw_buffers(std::span<const GLenum>(bufs, 1));
		gpufbo_.set_read_buffer(GL_COLOR_ATTACHMENT0);

		// Check if fbo is successfully created
		if (gpufbo_.complete()) {
			LOG_INFO("Renderer::setup() - Successfully created GPU ID framebuffer.");
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to created GPU ID framebuffer!");
		}

		// Allocate a framebuffer for the final pass 
		auto finalpass_fbo = FrameBuffer::create();
		if (finalpass_fbo.has_value()) {
			m_framebuffers.push_back(std::move(*finalpass_fbo));
		}
		else {
			LOG_ERROR("Renderer::setup() - Failed to create final pass framebuffer!");
		}

		auto finalpass_tex = Texture::alloc_storage_on_gpu(width, height);
		if (finalpass_tex.has_value()) {
			m_gl.m_textures.push_back(std::move(*finalpass_tex));
		}else {
			LOG_ERROR("Renderer::setup() - Failed to allocate texture for final pass!");
		}

		auto& finalpass_fbo_ = m_framebuffers[2];
		auto& finalpass_tex_ = m_gl.m_textures[2];

		// Use depth renderbuffer for attaching to editor
		finalpass_fbo_.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(finalpass_tex_.handle()));

		if (!finalpass_fbo_.complete()) {
			LOG_ERROR("Renderer::setup() - LDR FBO is incomplete!");
			throw std::runtime_error("");
		}

		// Create bloom mip chain based on initial HDR size
		initBloomMipChain(width, height);

		// Create a render pass for that framebuffer
		RenderPass first_pass
		{
			.pass_name = "First Pass",
			.fbo_handle = 0,
			.shdpgm_handle = 0,
			.auto_aspect = true,
			.depth_test = true,
			.depth_write = true,
			.blending = true,
			.culling = false
		};

		// Register the pass with the renderer
		m_passes.push_back(first_pass);

		RenderPass gpu_id_pass
		{
			.pass_name = "GPU ID",
			.fbo_handle = 1,			// Render into the GPU-ID FBO
			.shdpgm_handle = 2,         // Object_picking shader program
			.auto_aspect = true,
			.clear_color = false,		// Use integer clear below
			.clear_depth = true,		
			.depth_test = true,
			.depth_write = true,		
			.blending = false,
			.culling = true,
			.passtype = PassType::GEOMETRY
		};

		m_passes.push_back(gpu_id_pass);

		RenderPass ui_pass
		{
			.pass_name = "UI Pass",
			.fbo_handle = 0,
			.shdpgm_handle = 5,
			.auto_aspect = true,
			.clear_color = false,
			.clear_depth = false,
			.depth_test = false,
			.depth_write = false,
			.culling = false,
			.passtype = PassType::GEOMETRY
		};

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

		m_finalpass = {
			.pass_name = "Final Pass",
			.fbo_handle = 2,
			.shdpgm_handle = 4,
			.auto_aspect = true,
			.clear_color = false,
			.clear_depth = false,
			.depth_test = false,
			.depth_write = false,
			.culling = false,
			.passtype = PassType::FULLSCREEN
		};

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

		// Decide target framebuffer
		if (pass.passtype == PassType::FULLSCREEN && !isEditorMode) {
			// Enter fullscreen; final composite goes directly to back buffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else {
			// All other passes render into their configured FBOs
			glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fbo.handle()));
		}

		auto& viewport = pass.view_port;
		glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y),
			static_cast<GLsizei>(viewport.z), static_cast<GLsizei>(viewport.w));

		// If this pass targets the GPU-ID FBO (Shader program 2, R32UI), clear with integer clear:
		if (pass.shdpgm_handle == 2) {
			// entt::null == NO_HIT
			fbo.clear_colorui(/*drawbuf index*/0, NO_HIT, 0, 0, 0);
		}

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


	void Renderer::render_frame(std::span<const DrawItem> draw_items, std::span<std::pair<CameraComponent, glm::vec3>> camera_list, std::span<const LightCPU> lights) {

	 	// Render through editor camera
	 	if (isEditorCamOn) {

	 		glm::mat4  cam_view = editor_camera.getLookAt(); 
			glm::vec3& cam_pos  = editor_camera.getCamPos();

	 		for (auto& pass : m_passes) {

				// Skip debugging pass
				if (!isDebug && (pass.passtype == PassType::DEBUGGING)) { continue; }

				// Update pass viewport if allowed
				if (pass.auto_aspect) {
					int vp_w, vp_h;
					glfwGetWindowSize(glfwGetCurrentContext(), &vp_w, &vp_h);

					// Check if viewport needs update
					if ((pass.view_port.z != vp_w && vp_w > 0) || (pass.view_port.w != vp_h && vp_h > 0)) {
						pass.view_port.z = static_cast<float>(vp_w);
						pass.view_port.w = static_cast<float>(vp_h);

						// Resize FBO according to changes
						resizeFBO(pass.fbo_handle, vp_w, vp_h);
					}
				}

				// Get camera perspective transform
				glm::mat4 cam_perspective = editor_camera.getPerspective(pass.view_port.z / pass.view_port.w);

				// If this is the main HDR pass (FBO 0, object shader 0), remember camera matrices
				if (pass.fbo_handle == 0 && pass.shdpgm_handle == 0) {
					m_lastView = cam_view;
					m_lastProj = cam_perspective;
				}

				// Begin drawing frame
				beginFrame(pass);
				draw(pass, draw_items, cam_view, cam_perspective, cam_pos, lights);
				endFrame(pass);

				// Read ID at mouse position for GPU ID pass
				if (pass.shdpgm_handle == 2) {

					// Get mouse position WRT window client
					double mx, my;
					glfwGetCursorPos(glfwGetCurrentContext(), &mx, &my);

					// Check mouse pos if inside viewport panel
					if (mx >= renderEditorVP.tl.x && mx <= (renderEditorVP.tl.x + renderEditorVP.size.x)
						&& my >= renderEditorVP.tl.y && my <= (renderEditorVP.tl.y + renderEditorVP.size.y)) {

						// Normalize mouse position to viewport panel's top left position 
						float u = float(mx - renderEditorVP.tl.x) / renderEditorVP.size.x;
						float v = float(my - renderEditorVP.tl.y) / renderEditorVP.size.y;

						// Map to FBO pixel coords (flip Y because OpenGL images are bottom-up)
						int px = int(u * pass.view_port.z);
						int py = int((1.0f - v) * pass.view_port.w);

						// Read the ID
						u32 id = 0; // (entt::null value)
						auto& idFbo = m_framebuffers[1];
						idFbo.set_read_buffer(GL_COLOR_ATTACHMENT0);
						glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)idFbo.handle());
						glReadPixels(px, py, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &id);
						glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

						// Save Picked ID to somewhere (entt::null will be used as NO_HIT)
						pickedID = id;
					}
				}
			}
		}
		else {
			// For rendering all enabled camera displays
			for (const auto& cam : camera_list) {

				for (auto& pass : m_passes) {

					if (!isDebug && (pass.passtype == PassType::DEBUGGING)) { continue; }

					// Skip GPU-ID pass in fullscreen game mode for efficiency
					if (pass.shdpgm_handle == 2) { continue; }

					// Update pass viewport if allowed
					if (pass.auto_aspect) {
						int vp_w, vp_h;
						glfwGetWindowSize(glfwGetCurrentContext(), &vp_w, &vp_h);

						// Check if viewport needs update
						if ((pass.view_port.z != vp_w && vp_w > 0) || (pass.view_port.w != vp_h && vp_h > 0)) {
							pass.view_port.z = static_cast<float>(vp_w);
							pass.view_port.w = static_cast<float>(vp_h);

							// Resize FBO according to changes
							resizeFBO(pass.fbo_handle, vp_w, vp_h);
						}
					}

					// If this is the main HDR pass (FBO 0, object shader 0), remember camera matrices
					if (pass.fbo_handle == 0 && pass.shdpgm_handle == 0) {
						m_lastView = cam.first.View;
						m_lastProj = (cam.first.Projection == 0) ? cam.first.Persp : cam.first.Ortho;
					}

					glm::mat4 proj = (cam.first.Projection == 0) ? cam.first.Persp : cam.first.Ortho;

					// Begin drawing frame
					beginFrame(pass); // (Future): if cam.TargetTexture != -1, pass it into begin frame for binding to fbo

					// cam.first is the actual underlying camera object
					// cam.second is the camera's position using the transform component
					draw(pass, draw_items, cam.first.View, proj, cam.second, lights);

					endFrame(pass); // (Future): Unbind fbo if TargetTexture is used (May need new PassType to separate editor fbo and TargetTexture fbo)
				}
			}
		}

		renderFinalPass(m_finalpass);
	}

	void Renderer::draw(RenderPass const& pass,
						std::span<const DrawItem> draw_items,
					    const glm::mat4& v,
					    const glm::mat4& p,
						const glm::vec3& cam_pos,
						std::span<const LightCPU> lights) {

		// Get a reference to the shader program being used
		auto& prog = m_gl.m_shader_storage[pass.shdpgm_handle];

		// Compute perspective view transform on CPU before uploading to GPU
		glm::mat4 projection_view = p * v;

		// Upload camera related information
		prog.setUniform("CamPos", cam_pos);
		prog.setUniform("u_ViewProjection", projection_view);

		for (const auto& item : draw_items) {

			// Specific Pass Handling Logic
			if (pass.passtype == PassType::DEBUGGING) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.f, -1.f);
				glLineWidth(1.0f);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			if (pass.shdpgm_handle == 2) {
				u32 pickId = item.m_entity_id;
				prog.setUniform("u_ObjectID", pickId);
			}

			size_t material_handle = static_cast<size_t>(item.m_default_material_handle);
			Material& test_material = m_gl.t_testing_material[material_handle];

#pragma region TESTING UBO FOR MATERIALS
				MaterialUBO_Std140 mubo;
				mubo.Ka = test_material.getMaterialAmbient();
				mubo._pad0 = 0.0f;
				mubo.Kd = test_material.getMaterialDiffuse();
				mubo._pad1 = 0.0f;
				mubo.Ks = test_material.getMaterialSpecular();
				mubo.shininess = test_material.getMaterialShininess();

				// Update UBO (48 bytes)
				glNamedBufferSubData(m_materialUBO, 0, sizeof(MaterialUBO_Std140), &mubo);

				prog.setUniform("isTexture", false);
				prog.setUniform("useNormalMap", false);
#pragma endregion

			 // Check for material resource
			 if (MaterialResource* material_resource = RM.loadResource<MaterialResource>(convertToMaterialGuid(item.m_material_guid)))
			 {
				// New workflow has no ambient lighting
				prog.setUniform("material_.albedo", glm::vec3(material_resource->baseColor[0], 
				 												       material_resource->baseColor[1], 
				 												       material_resource->baseColor[2]));

				prog.setUniform("material_.metallic", material_resource->metallic);
				prog.setUniform("material_.roughness", material_resource->roughness);
				prog.setUniform("material_.ao", material_resource->ambientOcclusion);
				prog.setUniform("material_.opacity", material_resource->opacity);
				prog.setUniform("material_.emissionColor", glm::vec3(material_resource->emissionColor[0],
																			material_resource->emissionColor[1],
																			material_resource->emissionColor[2]));

				prog.setUniform("material_.emissionStrength", material_resource->emissionStrength);

				if (TextureResource* texture_resource = RM.loadResource<TextureResource>(convertToTextureGuid(material_resource->baseMap))) {
				 	glBindTextureUnit(0, static_cast<GLuint>(texture_resource->textureID));
					prog.setUniform("material_.textureOffsetX", material_resource->offset[0]);
					prog.setUniform("material_.textureOffsetY", material_resource->offset[1]);
					prog.setUniform("material_.textureTileX", material_resource->tiling[0]);
					prog.setUniform("material_.textureTileY", material_resource->tiling[1]);
				 	prog.setUniform("Texture2D", 0);
				 	prog.setUniform("isTexture", true);

				}

				if (TextureResource* nm_texture_resource = RM.loadResource<TextureResource>(convertToTextureGuid(material_resource->normalMap)))
				{
					glBindTextureUnit(1, static_cast<GLuint>(nm_texture_resource->textureID));
					prog.setUniform("useNormalMap", true);
				}
			 }
			 else
			 {
				 // New workflow has no ambient lighting
				 prog.setUniform("material_.albedo", glm::vec3(m_defaultMaterial.baseColor[0],
					 m_defaultMaterial.baseColor[1],
					 m_defaultMaterial.baseColor[2]));

				 prog.setUniform("material_.metallic", m_defaultMaterial.metallic);
				 prog.setUniform("material_.roughness", m_defaultMaterial.roughness);
				 prog.setUniform("material_.ao", m_defaultMaterial.ambientOcclusion);
				 prog.setUniform("material_.opacity", m_defaultMaterial.opacity);
				 prog.setUniform("material_.emissionColor", glm::vec3(m_defaultMaterial.emissionColor[0],
					 m_defaultMaterial.emissionColor[1],
					 m_defaultMaterial.emissionColor[2]));

				 prog.setUniform("material_.emissionStrength", m_defaultMaterial.emissionStrength);
			 }

#pragma region TESTING UBO FOR LIGHTING
		// Per-object light culling + upload 
		// Object center/radius (approx): extract translation; radius = heuristic
			glm::vec3 objCenter = glm::vec3(item.m_model_to_world_transform[3]);
			const float objRadius = 1.0f; // Replace with mesh/submesh bounds radius if available

			uint32_t usedLights = buildAndUploadLightsForDraw(objCenter, objRadius, lights);
			(void)usedLights; // block is visible to shader via binding=0
#pragma endregion

			size_t  mesh_handle = static_cast<size_t>(item.m_default_mesh_handle);

			GLenum  primitive = m_gl.m_mesh_storage[mesh_handle].primitive_type;
			GLsizei draw_count = m_gl.m_mesh_storage[mesh_handle].draw_count;
			GLenum  index_type = m_gl.m_mesh_storage[mesh_handle].index_type;


			// Upload model to world transform
			prog.setUniform("u_World", item.m_model_to_world_transform); 

			// Compute the normal matrix and upload it
			glm::mat4 normal_matrix = glm::transpose(glm::inverse(item.m_model_to_world_transform));
			prog.setUniform("u_NormalMatrix", normal_matrix); // Normal matrix

			// Get the underlying mesh resource using its guid
			if (MeshResource* mesh_resource = RM.loadResource<MeshResource>(convertToMeshGuid(item.m_mesh_guid))) {
				glBindVertexArray(mesh_resource->VAO);

				// Get the submesh descriptor
				const auto& submesh = mesh_resource->subMeshes[item.m_submesh_index];

				// Calculate byte offset into the index buffer
				const void* indexOffset = reinterpret_cast<const void*>( submesh.startIndex * sizeof(unsigned int));

				glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, indexOffset);
				glBindVertexArray(0);
			}
			else { // Draws primitives
				m_gl.m_mesh_storage[mesh_handle].vao.bind();
				glDrawElements(primitive, draw_count, index_type, nullptr);
				glBindVertexArray(0);
			}

		}

		prog.programFree();
	}

	void Renderer::endFrame(RenderPass const& pass) {
		//auto& prog = m_gl.m_shader_storage[pass.shdpgm_handle];
		//prog.programFree();
		glBindTextureUnit(0, 0);

		// After the fullscreen pass, ensure we're back on default framebuffer
		if (pass.passtype == PassType::FULLSCREEN) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void Renderer::resizeFBO(u32 handle, int w, int h) {

		// Check if fbo is valid
		if (!m_framebuffers[handle].complete()) return;

		// For all other FBOs
		if (handle == 0) {
			// Allocate storage for a new texture on the GPU
			auto fp_tex_new = Texture::alloc_storage_on_gpu(w, h, GL_RGBA16F);
			if (!fp_tex_new.has_value()) { LOG_ERROR("Renderer::resizeFBO() - Failed to allocate GL_RGBA16F ", w, " ", h, " storage on the GPU!"); }
			else { m_gl.m_textures[handle] = std::move(*fp_tex_new); }

			// Keep bloom mip chain resolution in sync with HDR scene resolution
			resizeBloomMipChain(w, h);
		}
		else if (handle == 1) { // For GPU ID FBO
			// Allocate storage for a new texture on the GPU
			auto fp_tex_new = Texture::alloc_storage_on_gpu(w, h, GL_R32UI);
			if (!fp_tex_new.has_value()) { LOG_ERROR("Renderer::resizeFBO() - Failed to allocate GL_R32UI ", w, " ", h, " storage on the GPU!"); }
			else { m_gl.m_textures[handle] = std::move(*fp_tex_new); }
		}
		else {
			// Allocate storage for a new texture on the GPU
			auto fp_tex_new = Texture::alloc_storage_on_gpu(w, h);
			if (!fp_tex_new.has_value()) { LOG_ERROR("Renderer::resizeFBO() - Failed to allocate RGBA8 ", w, " ", h, " storage on the GPU!"); }
			else { m_gl.m_textures[handle] = std::move(*fp_tex_new); }
		}

		// Update renderbuffer attachement 
		if (!temp_rbo) {
			glCreateRenderbuffers(1, &temp_rbo);
		}
		glNamedRenderbufferStorage(temp_rbo, GL_DEPTH_COMPONENT24, w, h);

		// Get FBO and Texture objects
		auto& fpfbo_ = m_framebuffers[handle];
		auto& fptex_ = m_gl.m_textures[handle];

		// Replace texture and RBO
		fpfbo_.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(fptex_.handle()));
		fpfbo_.attach_renderbuffer(GL_DEPTH_ATTACHMENT, temp_rbo);

		if (handle == 1) {
			// Make sure the GPU-ID FBO has correct draw/read buffers and is cleared
			const GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
			fpfbo_.set_draw_buffers(std::span<const GLenum>(bufs, 1));
			fpfbo_.set_read_buffer(GL_COLOR_ATTACHMENT0);
		}
	}

	// Build a small per-draw light list with simple sphere-sphere test and brightness score
	uint32_t Renderer::buildAndUploadLightsForDraw(const glm::vec3& objCenter, float objRadius,
												   std::span<const LightCPU> sceneLights) {

		// For scoring the lights to sort subsequently
		struct Ref { uint32_t idx; float score; };
		std::vector<Ref> picked; picked.reserve(32);

		auto luminance = [](glm::vec3 c) { return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b; };

		for (uint32_t i = 0; i < sceneLights.size(); ++i) {
			const auto& L = sceneLights[i];

			// Quick reject by type/range
			if (L.type == LIGHT_POINT || L.type == LIGHT_SPOT) {
				// Get distance between object center and light
				float d2 = glm::dot(L.position - objCenter, L.position - objCenter);

				// Compute light limit using its range and object radius; Cull if out of sphere range
				float lim = L.range + objRadius;
				if (d2 > lim * lim) continue;
			}

			// Compute and store base values
			float base = luminance(L.color) * L.intensity;
			float invd2 = 1.0f;
			float angle = 1.0f;

			// For Point and Spot
			if (L.type != LIGHT_DIRECTIONAL) {
				float d = glm::length(L.position - objCenter) - objRadius;
				d = glm::max(d, 1e-3f);
				invd2 = 1.0f / (d * d);
			}

			// For Spot only
			if (L.type == LIGHT_SPOT) {
				glm::vec3 v = glm::normalize(objCenter - L.position);
				float cosT = glm::dot(v, glm::normalize(L.direction));
				angle = glm::clamp((cosT - L.cosOuter) / glm::max(1e-3f, (L.cosInner - L.cosOuter)), 0.0f, 1.0f);
			}

			// Store the final value for the picked light
			picked.push_back({ i, base * invd2 * angle });
		}

		// Keep directionals first (stable)
		std::stable_sort(picked.begin(), picked.end(), [](const Ref& a, const Ref& b) { return a.score > b.score; });

		// cap lights per draw
		const uint32_t MAX_PER_DRAW = 12;
		if (picked.size() > MAX_PER_DRAW) picked.resize(MAX_PER_DRAW);

		// pack to UBO to send to GPU
		m_lightsCPU.ambient_indirect = glm::vec4(0.005f, 0.005f, 0.005f, 1.0f); // a=global indirect multiplier (future)
		m_lightsCPU.count = glm::uvec4((uint32_t)picked.size(), 0, 0, 0);

		// Pack all picked lights into GLSL-friendly std140 UBO pack
		for (uint32_t j = 0; j < (uint32_t)picked.size(); ++j) {

			// Find light from original light list using index
			const auto& L = sceneLights[picked[j].idx];

			// Create std140 GPU UBO pack
			LightGPUStd140 G;
			G.color_intensity = glm::vec4(L.color, L.intensity);								// rgb + intensity
			G.position_range = glm::vec4(L.position, L.range);									// xyz (world) + range
			G.direction_type = glm::vec4(glm::normalize(L.direction), float(L.type));			// xyz (world dir) + type (float)
			G.spot_cos_misc = glm::vec4(L.cosInner, L.cosOuter, L.indirectMultiplier, 0.0f);	// x=cosInner, y=cosOuter, z=indirect, w=unused

			// Store GPU UBO pack
			m_lightsCPU.lights[j] = G;
		}

		// Send LightsBlockGPU over
		glNamedBufferSubData(m_lightsUBO, 0, sizeof(LightsBlockGPU), &m_lightsCPU);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_lightsUBO);
		return (uint32_t)picked.size();
	}

	void Renderer::renderSkyboxHDR()
	{
		// If we don't have any bloom initialized or framebuffers, bail early
		if (m_framebuffers.empty()) return;

		// Bind the HDR scene framebuffer explicitly (FBO 0)
		auto& hdrFbo = m_framebuffers[0];
		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(hdrFbo.handle()));

		// Use the bloom source size (matches HDR resolution)
		glViewport(0, 0,
			static_cast<GLsizei>(m_bloomSrcSize.x),
			static_cast<GLsizei>(m_bloomSrcSize.y));

		// Depth settings for skybox
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);   // Skybox at max depth
		glDepthMask(GL_FALSE);    // Don't write depth

		// Build skybox view-projection: remove translation from last view
		glm::mat4 viewNoTrans = glm::mat4(glm::mat3(m_lastView));
		glm::mat4 skyboxVP = m_lastProj * viewNoTrans;

		// Use skybox shader (index 3)
		size_t skybox_shader_program_idx = 3;
		auto& skybox_prog = m_gl.m_shader_storage[skybox_shader_program_idx];
		skybox_prog.programUse();

		skybox_prog.setUniform("u_SkyboxViewProjection", skyboxVP);

		// Draw skybox cube
		m_skybox.vao.bind();
		glBindTextureUnit(2, m_skybox_texture);
		glDrawElements(m_skybox.primitive_type,
			m_skybox.draw_count,
			m_skybox.index_type,
			nullptr);
		glBindVertexArray(0);

		skybox_prog.programFree();

		// Restore depth state
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
	}


	void Renderer::renderFinalPass(RenderPass& pass) {

		// Update pass viewport if allowed
		if (pass.auto_aspect) {
			int vp_w, vp_h;
			glfwGetWindowSize(glfwGetCurrentContext(), &vp_w, &vp_h);

			// Check if viewport needs update
			if ((pass.view_port.z != vp_w && vp_w > 0) || (pass.view_port.w != vp_h && vp_h > 0)) {
				pass.view_port.z = static_cast<float>(vp_w);
				pass.view_port.w = static_cast<float>(vp_h);

				// Resize FBO (2) for final pass
				resizeFBO(pass.fbo_handle, vp_w, vp_h);

				// Also update bloom source size (HDR scene size)
				m_bloomSrcSize = { vp_w, vp_h };
				resizeBloomMipChain(vp_w, vp_h);
			}
		}

		// Build bloom from HDR scene (texture 0)
		renderBloomDownsamples();
		renderBloomUpsamples(m_bloomFilterRadius);  // 0.005f as default, tweak as needed

		// Render skybox into HDR FBO 0 after bloom is computed  
		renderSkyboxHDR();

		// Final composite: HDR scene + bloom -> LDR
		beginFrame(pass);
		auto& prog = m_gl.m_shader_storage[pass.shdpgm_handle];

		// Bind the initial pass's texture to be sampled (HDR scene color buffer)
		glBindTextureUnit(4, m_gl.m_textures[0].handle());

		// Bloom texture is top mip (A')  
		BloomMip& topMip = m_bloomMips[0]; 
		glBindTextureUnit(5, m_gl.m_textures[topMip.texIndex].handle());

		// Set Uniforms 
		prog.setUniform("exposure", m_exposure);
		prog.setUniform("bloomStrength", m_bloomStrength); 
		prog.setUniform("useBloom", m_bloomOn);

		m_fullscreen_quad.vao.bind();
		glDrawElements(m_fullscreen_quad.primitive_type, m_fullscreen_quad.draw_count, m_fullscreen_quad.index_type, nullptr);
		glBindVertexArray(0);

		
		prog.programFree();
		endFrame(pass);

	}

	void Renderer::initBloomMipChain(int srcWidth, int srcHeight)
	{
		if (m_bloomInitialized) return;

		m_bloomSrcSize = { srcWidth, srcHeight };

		int w = srcWidth;
		int h = srcHeight;

		for (int i = 0; i < BLOOM_MIP_COUNT; ++i) {
			// Downscale by 2 each level
			w = std::max(1, w / 2);
			h = std::max(1, h / 2);

			BloomMip& mip = m_bloomMips[i];
			mip.size = glm::vec2(static_cast<float>(w),
				static_cast<float>(h));
			mip.texelSize = glm::vec2(1.0f / mip.size.x,
				1.0f / mip.size.y);

			// Create FBO
			auto fbo = FrameBuffer::create();
			if (!fbo.has_value()) {
				LOG_ERROR("Bloom: Failed to create FBO for mip {}", i);
				continue;
			}
			m_framebuffers.push_back(std::move(*fbo));
			mip.fboIndex = static_cast<u32>(m_framebuffers.size() - 1);

			// Create texture (HDR RGB; you may use RGBA16F or R11F_G11F_B10F)
			auto tex = Texture::alloc_storage_on_gpu(w, h, GL_RGBA16F);
			if (!tex.has_value()) {
				LOG_ERROR("Bloom: Failed to create texture for mip {} ({}x{})", i, w, h);
				continue;
			}
			m_gl.m_textures.push_back(std::move(*tex));
			mip.texIndex = static_cast<u32>(m_gl.m_textures.size() - 1);

			auto& fb = m_framebuffers[mip.fboIndex];
			auto& texRef = m_gl.m_textures[mip.texIndex];

			fb.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(texRef.handle()));
			const GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
			fb.set_draw_buffers(std::span<const GLenum>(bufs, 1));

			if (!fb.complete()) {
				LOG_ERROR("Bloom: mip {} FBO incomplete!", i);
			}
		}

		m_bloomInitialized = true;
	}

	void Renderer::resizeBloomMipChain(int srcWidth, int srcHeight)
	{
		if (!m_bloomInitialized) {
			initBloomMipChain(srcWidth, srcHeight);
			return;
		}

		m_bloomSrcSize = { srcWidth, srcHeight };

		int w = srcWidth;
		int h = srcHeight;

		for (int i = 0; i < BLOOM_MIP_COUNT; ++i) {
			w = std::max(1, w / 2);
			h = std::max(1, h / 2);

			BloomMip& mip = m_bloomMips[i];
			mip.size = glm::vec2(static_cast<float>(w),
				static_cast<float>(h));
			mip.texelSize = glm::vec2(1.0f / mip.size.x,
				1.0f / mip.size.y);

			auto newTex = Texture::alloc_storage_on_gpu(w, h, GL_RGBA16F);
			if (!newTex.has_value()) {
				LOG_ERROR("Bloom: Failed to reallocate texture for mip {} to {}x{}", i, w, h);
				continue;
			}

			m_gl.m_textures[mip.texIndex] = std::move(*newTex);

			auto& fb = m_framebuffers[mip.fboIndex];
			auto& texRef = m_gl.m_textures[mip.texIndex];

			fb.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(texRef.handle()));
			const GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
			fb.set_draw_buffers(std::span<const GLenum>(bufs, 1));
		}
	}

	void Renderer::renderBloomDownsamples()
	{
		if (!m_bloomInitialized) return;

		auto& mipChain = m_bloomMips;

		auto& downProg = m_gl.m_shader_storage[6]; // bloom_downsample
		downProg.programUse();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);

		// Start with HDR scene texture as source
		glm::vec2 srcRes = glm::vec2(
			static_cast<float>(m_bloomSrcSize.x),
			static_cast<float>(m_bloomSrcSize.y)
		);
		downProg.setUniform("srcResolution", srcRes);
		glBindTextureUnit(4, m_gl.m_textures[0].handle()); // HDR scene

		for (int i = 0; i < BLOOM_MIP_COUNT; ++i) {
			BloomMip& mip = mipChain[i];

			auto& fb = m_framebuffers[mip.fboIndex];
			glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fb.handle()));
			glViewport(0, 0,
				static_cast<GLsizei>(mip.size.x),
				static_cast<GLsizei>(mip.size.y));

			m_fullscreen_quad.vao.bind();
			glDrawElements(m_fullscreen_quad.primitive_type,
				m_fullscreen_quad.draw_count,
				m_fullscreen_quad.index_type,
				nullptr);
			glBindVertexArray(0);

			// Next iteration uses this mip as source
			srcRes = mip.size;
			downProg.setUniform("srcResolution", srcRes);
			glBindTextureUnit(4, m_gl.m_textures[mip.texIndex].handle());
		}

		downProg.programFree();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::renderBloomUpsamples(float filterRadius)
	{
		if (!m_bloomInitialized) return;

		auto& mipChain = m_bloomMips;
		auto& upProg = m_gl.m_shader_storage[7]; // bloom_upsample

		upProg.programUse();
		upProg.setUniform("filterRadius", filterRadius);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);

		// Enable additive blending: dst = dst + src
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		// Work from smallest mip up to mip 0
		for (int i = BLOOM_MIP_COUNT - 1; i > 0; --i) {
			BloomMip& mip = mipChain[i];
			BloomMip& nextMip = mipChain[i - 1];

			// Source: current mip (lower resolution)
			glBindTextureUnit(4, m_gl.m_textures[mip.texIndex].handle());

			// Target: next higher mip
			auto& fb = m_framebuffers[nextMip.fboIndex];
			glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fb.handle()));
			glViewport(0, 0,
				static_cast<GLsizei>(nextMip.size.x),
				static_cast<GLsizei>(nextMip.size.y));

			m_fullscreen_quad.vao.bind();
			glDrawElements(m_fullscreen_quad.primitive_type,
				m_fullscreen_quad.draw_count,
				m_fullscreen_quad.index_type,
				nullptr);
			glBindVertexArray(0);
		}

		glDisable(GL_BLEND);
		upProg.programFree();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


}

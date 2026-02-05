/**
 * @file Renderer.h
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
#pragma once

 // For dynamically allocated array
#include <vector> 

// For OpenGL function pointers
#include <glad/glad.h>

// For graphics related defines and functionality
#include "Graphics/GraphicsLoader.h"

// For Camera3D class
#include "Graphics/Camera.h"

// For Camera component
#include "Component/CameraComponent.h"

// For Font
#include "Font.h"

namespace Engine {

	// Material UBO layout mirrored from GLSL std140
	struct MaterialUBO_Std140 {
		glm::vec3 Ka; float _pad0;   // 16 bytes
		glm::vec3 Kd; float _pad1;   // 16 bytes
		glm::vec3 Ks; float shininess; // 16 bytes
	}; // total = 48 bytes

	// ---------------- Lights UBO (new) ----------------
	enum : uint32_t { LIGHT_DIRECTIONAL = 0u, LIGHT_POINT = 1u, LIGHT_SPOT = 2u };

	// CPU-side light as collected by RenderSystem
	struct LightCPU {
		glm::vec3 color{ 1,1,1 }; float intensity{ 1 };
		glm::vec3 position{ 0 };   float range{ 5 };
		glm::vec3 direction{ 0,-1,0 }; float cosInner{ std::cos(glm::radians(25.f)) };
		float cosOuter{ std::cos(glm::radians(30.f)) };
		float indirectMultiplier{ 1 };
		uint32_t type{ LIGHT_POINT };

		// Shadow info
		// Only directional shadows are implemented for now, but the fields
		// exist for point/spot expansion later.
		uint32_t shadowType{ 0u };      // 0 = No, 1 = Hard, 2 = Soft (matches LightComponent::ShadowType)
		uint32_t shadowResolution{ 1024u };
		float    shadowStrength{ 1.0f };
		float    shadowBias{ 0.005f };   // receiver bias baseline
		float    shadowNearPlane{ 0.2f };
		float    shadowFarPlane{ 50.0f }; // used for (future) cascade splits; currently auto-fit for directional
	};

	// ---------------- Shadows UBO (room for cascades) ----------------
	constexpr uint32_t MAX_SHADOW_CASCADES = 4;
	struct ShadowsBlockGPU {
		glm::mat4 lightViewProj[MAX_SHADOW_CASCADES];
		glm::vec4 cascadeSplits; // view-space split depths (positive). Only x is used for single cascade.
		glm::vec4 shadowParams;  // x=strength, y=bias, z=texelSize (1/res), w=shadowType (0/1/2)
		glm::vec4 lightDirEnabled; // xyz = light dir (world, normalized), w = 1 if enabled
	};

	// std140-tight 64B/light packing (4x vec4)
	struct LightGPUStd140 {
		glm::vec4 color_intensity;   // rgb + intensity
		glm::vec4 position_range;    // xyz + range
		glm::vec4 direction_type;    // xyz + type (as float in .w)
		glm::vec4 spot_cos_misc;     // x=cosInner, y=cosOuter, z=indirect, w=unused
	};

	constexpr uint32_t MAX_LIGHTS = 64; // cap per frame block

	struct LightsBlockGPU {
		glm::vec4 ambient_indirect; // rgb ambient, a = global indirect multiplier
		glm::uvec4 count;            // x = lightCount; yzw unused
		LightGPUStd140 lights[MAX_LIGHTS];
	};

	/**
	 * @brief System responsible for interacting with the graphics layer in order to render game objects.
	 * @details Internally calls OpenGL API calls to the graphics card to perform rendering operations.
	 */
	class Renderer {

	public:
		Renderer(Camera3D& cam);

		/**
		 * @brief Initializes the renderer and sets up required resources
		 */
		void setup();

		/**
		 * @brief Renders a complete frame with the given draw items
		 * @param draw_items Collection of drawable objects to render
		 * @param cameras Collection of active cameras in the scene
		 * @param lights Collection of active lights in the scene
		 */
		void render_frame(std::span<const DrawItem> draw_items, std::span<std::pair<CameraComponent, glm::vec3>> cameras, std::span<const LightCPU> lights);

		/**
		 * @brief Retrieves the OpenGL texture handle for ImGui rendering
		 * @return GLuint handle to the first texture in storage
		 */
		inline GLuint get_imgui_texture() const { return static_cast<GLuint>(m_gl.m_textures[2].handle()); }

		/**
		 * @brief Gets the number of meshes currently stored
		 * @return Size of the mesh storage container
		 */
		inline const size_t mesh_count() const { return m_gl.m_mesh_storage.size(); }

		/**
		 * @brief Provides read-only access to mesh data storage
		 * @return Const reference to the mesh data container
		 */
		inline const std::vector<MeshData>& getMeshDataStorage() { return m_gl.m_mesh_data_storage; }

		/**
		 * @brief Provides read-only access to mesh data 2D storage
		 * @return Const reference to the mesh data 2D container
		 */
		inline const std::vector<MeshData2D>& getMeshData2DStorage() { return m_gl.m_mesh_data2d_storage; }

		/**
		 * @brief Provides read-only access to material storage
		 * @return Const reference to the material container
		 */
		inline const std::vector<Material>& getMaterialStorage() { return m_gl.t_testing_material; }

		/**
		 * @brief Provides read-only access to texture storage
		 * @return Const reference to the texture container
		 */
		inline const std::vector<Texture>& getTextureStorage() { return m_gl.t_testing_textures; }

		/**
		 * @brief Provide a reference to the editor camera
		 * @return Reference to the editor camera
		 */
		inline Camera3D& getEditorCamera() { return editor_camera; }

		/**
		 * @brief Resize a FBO with the given width and height
		 * @param handle The handle of the FBO to resize
		 * @param w The new width 
		 * @param h The new height
		 */
		inline void resizeFBO(u32 handle, int w, int h);

		/**
		 * @brief Provide a reference to the editor viewport data
		 * @return Reference to the editor viewport data
		 */
		inline EditorViewport& getEditorViewport() { return renderEditorVP; }

		/**
		 * @brief Provides read-only access to the picked ID in the editor
		 * @return Const reference to the picked ID
		 */
		inline const u32 getPickedID() const { return pickedID; }

		/**
		 * @brief Provide a reference to the active status of the editor camera
		 * @return Reference to the active status of the editor camera
		 */
		inline bool& getEditorCamToggle() { return isEditorCamOn; }

		/**
		 * @brief Provide a reference to the toggle status of the editor
		 * @return Reference to the toggle status of the editor
		 */
		inline bool& getEditorModeToggle() { return isEditorMode; }

		/**
		 * THIS FUNCTION IS DEPRECATED 
		 * 
		 * @brief  Provides an orthographic projection for the UI.
		 * @return Matrix 4x4 that represents the orthographic projection 
		 *		   for the user interface.
		 * 
		 */
		inline const glm::mat4 getUIProjection() const { return glm::ortho(m_UIPass.view_port.x, m_UIPass.view_port.z, m_UIPass.view_port.w, m_UIPass.view_port.y, -1.f, 1.f); }

		inline bool& getBloomToggle() { return m_bloomOn; }
		inline float& getBloomStrength() { return m_bloomStrength; }
		inline float& getBloomFilterRadius() { return m_bloomFilterRadius; }
		inline float& getExposure() { return m_exposure; }

		inline float& getGlobalBias() { return m_globalBias; }

		/**
		 * @brief  Gets the viewport dimensions of the UI pass.
		 * @return Reference to the UI pass viewport.
		 */
		inline glm::vec4 const& GetUIViewport() { return m_UIPass.view_port; }

		/**
		 * @brief  Gets the projection (orthographic) information of the UI pass.
		 * @return Reference to projection.
		 */
		inline glm::mat4 const& GetUIProjection() { return m_ui_projection; }

	private:

		/**
		 * @brief Maps shader programs loaded to it's index for easy identification.
		 */
		enum class ShaderIndex : size_t 
		{ MAIN			   = 0, 
		  DEBUG_DRAW	   = 1, 
		  OBJECT_PICKING   = 2, 
		  SKYBOX		   = 3, 
		  HDR			   = 4, 
		  UI			   = 5, 
		  BLOOM_DOWNSAMPLE = 6, 
		  BLOOM_UPSAMPLE   = 7,
		  SHADOW		   = 8,
		  FONT			   = 9,
		  TRAIL			   = 10
		};

		/**
		 * @brief Maps frame buffers to it's index for easy identification.
		 */
		enum class FramebufferIndex : size_t 
		{ SCENE		  = 0, 
		  PICKING	  = 1,
		  COMPOSITION = 2 
		};

		/**
		 * @brief Prepares the rendering context for a specific render pass
		 * @param pass The render pass configuration to begin
		 */
		void beginFrame(RenderPass const& pass);

		/**
		 * @brief Executes draw calls for all items in the current render pass
		 * @param pass The active render pass configuration
		 * @param draw_items Collection of objects to draw
		 * @param v View matrix of the camera
		 * @param p Projection matrix of the camera
		 * @param cam_pos Position of the active camera
		 * @param lights Collection of active lights in the scene
		 */
		void draw(RenderPass const& pass, 
		          std::span<const DrawItem> draw_items, 
		          const glm::mat4& v, 
		          const glm::mat4& p, 
		          const glm::vec3& cam_pos, 
		          std::span<const LightCPU> lights);

		/**
		 * @brief Finalizes the render pass and performs cleanup
		 * @param pass The render pass to complete
		 */
		void endFrame(RenderPass const& pass);

		/**
		 * @brief Loads the OpenGL function pointers.
		 */
		void loadGLFunctionPointers();

		/**
		 * @brief Loads shaders from disk.
		 */
		void loadShaders();

		/**
		 * @brief Initializes basic geometry into memory. 
		 *		  Geometry intialized: Cube, Plane, Sphere & Quad.
		 */
		void initBasicGeometry();

		/**
		 * @brief Sets the default state of the renderer on startup.
		 */
		void setDefaultState();

		/**
		 * @brief Sets up the render passes used for multi-pass rendering.
		 */
		void setupPasses();

		/**
		 * @brief Sets up all the necessary framebuffers for rendering and compositing.
		 */
		void setupFramebuffers();

		/**
		 * @brief	  Renders the composition pass.
		 * @param[in] pass
		 *			  The render pass containing render data for rendering the
		 *			  composition pass.
		 */
		void renderFinalPass(RenderPass& pass, std::span<const DrawItem> draw_items, const glm::mat4& view, const glm::mat4& proj, glm::vec3 const& cam_pos);

		/**
		 * @brief	  Renders the UI pass.
		 * @param[in] pass
		 *			  The render pass containing render data for rendering the
		 *			  UI pass.
		 * @param[in] items
		 *			  A view of all available 2D objects.
		 */
		void renderUIPass(RenderPass& pass, std::span<const DrawItem> items);

		/**
		 * @brief Renders the skybox in HDR color range.
		 */
		void renderSkyboxHDR();

		// Helper to build a per-draw light list and upload it (returns count)
		uint32_t buildAndUploadLightsForDraw(const glm::vec3& objCenter, float objRadius,
			std::span<const LightCPU> sceneLights);

		Camera3D& editor_camera;

		// Material UBO handle
		GLuint m_materialUBO = 0;

		// Light UBO handle
		GLuint m_lightsUBO = 0;
		LightsBlockGPU m_lightsCPU{}; // scratch buffer

		// ---------------- Shadows Mapping ----------------
		GLuint m_shadowsUBO = 0;
		ShadowsBlockGPU m_shadowsCPU{};
		GLuint m_shadowFBO = 0;
		GLuint m_shadowDepthTex = 0;
		uint32_t m_shadowMapRes = 1024u;
		//uint32_t m_shadowShaderIndex = 0u;

		float m_globalBias = 0.005;

		void ensureShadowResources(uint32_t res);
		//void renderShadowMap(std::span<const DrawItem> draw_items, std::span<const LightCPU> lights);
		void renderShadowMap(std::span<const DrawItem> draw_items,
			std::span<const LightCPU> lights,
			const glm::mat4& camView,
			const glm::mat4& camProj);

		std::vector<RenderPass>  m_passes;
		std::vector<FrameBuffer> m_framebuffers;

		/*------------------- BLOOM PROPERTIES/DATA ------------------*/
		// Number of bloom mip levels (multi-scale)
		static constexpr int BLOOM_MIP_COUNT = 5;

		struct BloomMip {
			glm::vec2 size{};       // width, height of this mip
			glm::vec2 texelSize{};  // 1.0 / size
			u32 fboIndex = 0;       // index into m_framebuffers
			u32 texIndex = 0;       // index into m_gl.m_textures
		};

		std::array<BloomMip, BLOOM_MIP_COUNT> m_bloomMips{};
		bool  m_bloomInitialized = false;
		bool  m_bloomOn = false;

		// Physically-based bloom strength (mix factor in final pass; reco ~0.03-0.15)
		float m_bloomStrength = 0.01f;  
		float m_bloomFilterRadius = 0.0025f;

		// Cached scene size for bloom
		glm::ivec2 m_bloomSrcSize{ 1280, 720 };

		// Helpers
		void initBloomMipChain(int srcWidth, int srcHeight);
		void resizeBloomMipChain(int srcWidth, int srcHeight);
		void renderBloomDownsamples();
		void renderBloomUpsamples(float filterRadius);
		/*------------------- BLOOM PROPERTIES/DATA ------------------*/

		// Exposure value
		float m_exposure = 1.0;

		MeshGL m_skybox; // Default engine provided skybox
		GLuint m_skybox_texture; // Will need to change

		MaterialResource m_defaultMaterial; // Immutable material, shared across all meshes that DO NOT HAVE material attached to it

		// Temp objects for object picking
		GLuint temp_rbo = 0;				// Used for scene and GPU ID FBO
		EditorViewport renderEditorVP;	// Editor viewport data
		u32 pickedID = 0xFFFFFFFFu;

		// Temp toggle to check if editor camera is enabled
		bool isEditorCamOn = false;
		bool isEditorMode = false;

		// Last camera used for the main HDR pass (FBO 0)
		glm::mat4 m_lastView{};
		glm::mat4 m_lastProj{};

		// Temporary object
		GraphicsLoader m_gl;

		// Fullscreen Quad where the final output of the render is outputted to
		MeshGL m_fullscreen_quad;

		RenderPass m_finalpass;
		RenderPass m_UIPass;

		u32 activeLayer = 1;

		glm::mat4 m_ui_projection;

		// ------------- Font ------------------
		std::unordered_map<std::string, Font> m_fonts; 
		Font m_defaultFont; 
		GLuint m_fontVAO = 0; 
		GLuint m_fontVBO = 0; 
		RenderPass m_textPass; 

		/*
		* @brief Loads a font from compiled .font file 
		* @param filepath path to .font file
		* @return true if loaded successfully
		*/
		
		bool loadFont(const std::string& filepath, const std::string& fontName);
		Font* getFont(const std::string& fontName);

		/**
		 * @brief Renders the text pass
		 * @param pass The render pass containing render data
		 * @param items A view of all available draw items
		 */
		void renderTextPass(RenderPass& pass, std::span<const DrawItem> items);

		/**
		 * @brief Initializes font rendering resources (VAO/VBO)
		 */
		void initFontResources();

		struct TrailVertex {
			glm::vec3 Position;
			glm::vec3 Tangent;   // Direction along trail
			glm::vec2 UV;
			float Width;
			float Age;           // Normalized age [0, 1]
		};

		GLuint m_TrailVAO;
		GLuint m_TrailVBO;
		GLuint m_TrailEBO;

		void InitTrailResources();
		void RenderTrails(std::span<const DrawItem> trailItems, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos);
		void BuildTrailGeometry(const TrailComponent& trail, std::vector<TrailVertex>& vertices, std::vector<u32>& indices);
	};

}
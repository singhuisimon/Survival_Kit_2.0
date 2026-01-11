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
		uint32_t _pad{ 0 };
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

		inline bool& getBloomToggle() { return m_bloomOn; }
		inline float& getBloomStrength() { return m_bloomStrength; }
		inline float& getBloomFilterRadius() { return m_bloomFilterRadius; }
		inline float& getExposure() { return m_exposure; }

	private:
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

		void renderFinalPass(RenderPass& pass);

		void renderUIPass(RenderPass& pass, std::span<const DrawItem> items);

		// Helper to build a per-draw light list and upload it (returns count)
		uint32_t buildAndUploadLightsForDraw(const glm::vec3& objCenter, float objRadius,
			std::span<const LightCPU> sceneLights);

		Camera3D& editor_camera;

		// Material UBO handle
		GLuint m_materialUBO = 0;

		// Light UBO handle
		GLuint m_lightsUBO = 0;
		LightsBlockGPU m_lightsCPU{}; // scratch buffer

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

		// Engine Provided Items1
		MeshGL m_skybox; // Default engine provided skybox
		GLuint m_skybox_texture; // Will need to change
		void renderSkyboxHDR();

		MaterialResource m_defaultMaterial; // Immutable material, shared across all meshes that DO NOT HAVE material attached to it

		// Temp objects for object picking
		GLuint temp_rbo = 0;				// Used for scene and GPU ID FBO
		EditorViewport renderEditorVP;	// Editor viewport data
		u32 pickedID = 0xFFFFFFFFu;

		// Temp toggle to check if editor camera is enabled
		bool isEditorCamOn = true;
		bool isEditorMode = true;

		// Last camera used for the main HDR pass (FBO 0)
		glm::mat4 m_lastView{};
		glm::mat4 m_lastProj{};

		// Temporary object
		GraphicsLoader m_gl;

		// Fullscreen Quad where the final output of the render is outputted to
		MeshGL m_fullscreen_quad;

		RenderPass m_finalpass;
		RenderPass m_UIPass;

		enum class ShaderIndex : size_t { MAIN = 0, DEBUG_DRAW = 1, OBJECT_PICKING = 2, SKYBOX = 3, HDR = 4, UI = 5, BLOOM_DOWNSAMPLE = 6, BLOOM_UPSAMPLE = 7 };
		enum class FramebufferIndex : size_t { SCENE = 0, PICKING = 1, COMPOSITION = 2};
	};

}
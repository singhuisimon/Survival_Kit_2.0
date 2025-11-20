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
		inline GLuint get_imgui_texture() const { return static_cast<GLuint>(m_gl.m_textures[0].handle()); }

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

		// Engine Provided Items
		MeshGL m_skybox; // Default engine provided skybox
		GLuint m_skybox_texture; // Will need to change

		MaterialResource m_defaultMaterial; // Immutable material, shared across all meshes that DO NOT HAVE material attached to it

		// Temp objects for object picking
		GLuint temp_rbo = 0;				// Used for scene and GPU ID FBO
		EditorViewport renderEditorVP;	// Editor viewport data
		u32 pickedID = 0xFFFFFFFFu;

		// Temp toggle to check if editor camera is enabled
		bool isEditorCamOn = true;

		// Temporary object
		GraphicsLoader m_gl;
	};

}
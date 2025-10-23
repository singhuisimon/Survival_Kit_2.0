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

	/**
	 * @brief System responsible for interacting with the graphics layer in order to render game objects.
	 * @details Internally calls OpenGL API calls to the graphics card to perform rendering operations.
	 */
	class Renderer {

	public:
		Renderer(Camera3D& cam, Light& light);

		/**
		 * @brief Initializes the renderer and sets up required resources
		 */
		void setup();

		/**
		 * @brief Renders a complete frame with the given draw items
		 * @param draw_items Collection of drawable objects to render
		 */
		void render_frame(std::span<const DrawItem> draw_items, std::span<const CameraComponent> camera_list);

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

		inline Camera3D& getEditorCamera() { return editor_camera; }

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
		 */
		void draw(RenderPass const& pass, std::span<const DrawItem> draw_items, const glm::mat4 v, const glm::mat4 p);

		/**
		 * @brief Finalizes the render pass and performs cleanup
		 * @param pass The render pass to complete
		 */
		void endFrame(RenderPass const& pass);

		Camera3D& editor_camera;
		Light& editor_light;

		std::vector<RenderPass>  m_passes;
		std::vector<FrameBuffer> m_framebuffers;

		// Temporary object
		GraphicsLoader m_gl;
	};

}
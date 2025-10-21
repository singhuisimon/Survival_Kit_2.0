/**
 * @file MeshData.h
 * @brief CPU and GPU mesh data structures
 * @details Defines containers for mesh geometry data in both CPU memory
 *          (MeshData) and GPU memory (MeshGL). MeshData stores raw vertex
 *          attributes for processing, while MeshGL holds OpenGL buffer
 *          objects for rendering.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

// For dynamically sized arrays
#include <vector>

// For access to vector types
#include <glm/glm.hpp>

// For access to VAO and VBO wrappers
#include "../Graphics/GLResources.h"

namespace Engine {
	
	/**
	 * @brief CPU-side mesh geometry data container
	 * @details Stores vertex attributes in separate arrays (SoA layout).
	 *          Easily extendable with additional attributes (tangents, etc.).
	 *          Used for procedural generation, loading, and CPU-side processing
	 *          before uploading to the GPU.
	 */
	struct MeshData {

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<glm::vec2> texcoords;
		std::vector<uint32_t>  indices;
	};

	/**
	 * @brief GPU-side mesh representation with OpenGL buffer objects
	 * @details Move-only RAII wrapper containing VAO, VBO, and EBO handles.
	 *          Stores all information needed to issue draw calls for a mesh.
	 *          Cannot be copied to prevent unintended GPU resource duplication.
	 */
	struct MeshGL {

		VAO vao{};
		VBO vbo{};
		VBO ebo{};

		GLsizei draw_count = 0;

		GLenum  primitive_type = GL_TRIANGLES;
		GLenum  index_type     = GL_UNSIGNED_INT;

		MeshGL(const MeshGL&) = delete;
		MeshGL& operator=(const MeshGL&) = delete;
		MeshGL() = default;
		~MeshGL() = default;
		MeshGL(MeshGL&&) noexcept = default;
		MeshGL& operator=(MeshGL&&) noexcept = default;
	};
	
	/**
	 * @brief Uploads CPU mesh data to GPU and creates OpenGL buffers
	 * @param mesh The mesh data to upload (VAO, VBO, EBO will be created)
	 * @return MeshGL object containing OpenGL handles and draw information
	 */
	MeshGL   upload_mesh_data(MeshData& mesh);

}

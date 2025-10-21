/**
 * @file GLResources.h
 * @brief RAII Wrappers for GPU resources (OpenGL based).
 * @details Contains the prototype of RAII wrappers for GPU resources like vertex array objects and buffer objects.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

// For OpenGL functionality
#include <glad/glad.h>

namespace Engine {
	
	/**
	 * @brief RAII wrapper for OpenGL buffer object
	 * @details Wraps around an OpenGL buffer object (VBO, EBO, UBO, etc.).
	 *          Provides move semantics and prevents copying.
	 */
	class VBO {

	public:
		/**
		 * @brief Default constructor for buffer.
		 * @details Initializes handle to 0 (invalid/null OpenGL object).
		 */
		VBO();

		/**
		 * @brief Destructor for buffer.
		 * @details Automatically releases OpenGL buffer resources.
		 */
		~VBO();

		// Prevent copy construction and copy assignment
		VBO(const VBO&) = delete;
		VBO& operator=(const VBO&) = delete;

		/**
		 * @brief Move constructor for buffer.
		 * @details Transfers ownership of the OpenGL buffer handle.
		 */
		VBO(VBO&&) noexcept;

		/**
		 * @brief Move assignment operator overload for buffer.
		 * @details Transfers ownership and releases any existing buffer.
		 */
		VBO& operator=(VBO&&) noexcept;

		/**
		 * @brief Creates a new OpenGL buffer object.
		 * @details Deletes any existing buffer before calling glCreateBuffers.
		 *          Does not bind the buffer (uses DSA - Direct State Access).
		 */
		void create();

		/**
		 * @brief Gets the underlying OpenGL handle to the buffer object.
		 * @return The OpenGL buffer object name/ID.
		 */
		GLuint id() const noexcept;

		/**
		 * @brief Initializes buffer object's immutable data store.
		 * @details Creates immutable storage for the buffer using glNamedBufferStorage.
		 *          This allocates GPU memory and optionally initializes it with data.
		 *          Can only be called once per buffer object.
		 * @param size Size of the data store in bytes.
		 * @param data Pointer to initialization data, or nullptr for uninitialized storage.
		 * @param flags Bitfield of GL_*_STORAGE_BIT flags (e.g., GL_DYNAMIC_STORAGE_BIT).
		 */
		void storage(GLsizeiptr size, const void* data, GLbitfield flags);

		/**
		 * @brief Updates a subset of the buffer object's data store.
		 * @details Modifies existing buffer data using glNamedBufferSubData.
		 *          The buffer must have mutable storage or GL_DYNAMIC_STORAGE_BIT flag.
		 * @param offset Byte offset into the buffer where update begins.
		 * @param size Number of bytes to update.
		 * @param data Pointer to the new data.
		 */
		void sub_data(GLintptr offset, GLsizeiptr size, const void* data);

	private:

		/**
		 * @brief Helper function for releasing OpenGL resources.
		 * @details Calls glDeleteBuffers and resets handle to 0.
		 */
		void destroy();

		// Underlying data store
		GLuint handle;

	};

	/**
	 * @brief RAII wrapper for OpenGL vertex array object (VAO).
	 * @details Encapsulates vertex attribute configuration and buffer bindings.
	 *          Provides move semantics and prevents copying.
	 */
	class VAO {

	public:
		/**
		 * @brief Default constructor for vertex array object.
		 * @details Initializes handle to 0 (invalid/null OpenGL object).
		 */
		VAO();

		/**
		 * @brief Destructor for vertex array object.
		 * @details Automatically releases OpenGL VAO resources.
		 */
		~VAO();

		// Disable copy construction and copy assignment
		VAO(const VAO&) = delete;
		VAO& operator=(const VAO&) = delete;

		/**
		 * @brief Move constructor for vertex array object.
		 * @details Transfers ownership of the OpenGL VAO handle.
		 */
		VAO(VAO&&) noexcept;

		/**
		 * @brief Move assignment operator overload for vertex array object.
		 * @details Transfers ownership and releases any existing VAO.
		 */
		VAO& operator=(VAO&&) noexcept;

		/**
		 * @brief Creates a new OpenGL vertex array object.
		 * @details Deletes any existing VAO before calling glCreateVertexArrays.
		 *          Does not bind the VAO (uses DSA - Direct State Access).
		 */
		void create();

		/**
		 * @brief Gets the underlying OpenGL handle to the vertex array object.
		 * @return The OpenGL VAO name/ID.
		 */
		GLuint id() const noexcept;

		/**
		 * @brief Binds this vertex array for subsequent OpenGL operations.
		 * @details Required before draw calls. Uses glBindVertexArray.
		 */
		void bind() const;

		/**
		 * @brief Enables a vertex attribute array using Direct State Access.
		 * @details Calls glEnableVertexArrayAttrib to enable the specified attribute.
		 * @param attrib The vertex attribute index to enable.
		 */
		void enable_attrib(GLuint attrib) const;

		/**
		 * @brief Binds a vertex buffer to a binding point using Direct State Access.
		 * @details Associates a VBO with a binding index for vertex attribute fetching.
		 *          Calls glVertexArrayVertexBuffer.
		 * @param binding The binding point index (0-15 typically).
		 * @param buf The VBO to bind.
		 * @param offset Byte offset into the buffer where vertex data begins.
		 * @param stride Byte offset between consecutive vertices.
		 */
		void bind_vertex_buffer(GLuint brinding, const VBO& buf, GLintptr offset, GLsizei stride) const;
		
		/**
		 * @brief Specifies the format of a vertex attribute using Direct State Access.
		 * @details Defines how vertex data is interpreted (type, component count, etc.).
		 *          Calls glVertexArrayAttribFormat.
		 * @param attrib The vertex attribute index to configure.
		 * @param comps Number of components per vertex (1-4).
		 * @param type Data type of each component (e.g., GL_FLOAT, GL_INT).
		 * @param normalized Whether integer values should be normalized to [0,1] or [-1,1].
		 * @param relativeOffset Byte offset of this attribute relative to the start of each vertex.
		 */
		void attrib_format(GLuint attrib, GLint comps, GLenum type, bool normalized, GLuint relativeOffset) const;

		/**
		 * @brief Associates a vertex attribute with a binding point.
		 * @details Links an attribute index to a binding index set by bind_vertex_buffer.
		 *          Calls glVertexArrayAttribBinding.
		 * @param attrib The vertex attribute index.
		 * @param binding The binding point index to associate with.
		 */
		void attrib_binding(GLuint attrib, GLuint binding) const;

		/**
		 * @brief Binds an element buffer (index buffer) to this VAO.
		 * @details Associates an EBO for indexed drawing. Calls glVertexArrayElementBuffer.
		 * @param ebo The element buffer object to bind.
		 */
		void bind_element_buffer(const VBO& ebo) const;

	private:
		/**
		 * @brief Helper function for releasing OpenGL resources.
		 * @details Calls glDeleteVertexArrays and resets handle to 0.
		 */
		void destroy();

		// Underlying handle to the vertex array object
		GLuint handle;
	};
}

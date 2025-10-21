/**
 * @file GLResources.cpp
 * @brief RAII Wrappers for GPU resources (OpenGL based).
 * @details Contains the definitions of RAII wrappers for GPU resources like vertex array objects and buffer objects.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include <algorithm> // For std::exchange

#include "../Graphics/GLResources.h"

namespace Engine {

#pragma region VBO
	VBO::VBO() : handle(0) { }

	VBO::~VBO() {

		destroy();
	}

	// Move construction
	VBO::VBO(VBO&& other) noexcept : handle(std::exchange(other.handle, 0)) { }

	// Move assignment
	VBO& VBO::operator=(VBO&& other) noexcept {

		// Steal and cleanup
		if (this != &other) {

			if (handle) glDeleteBuffers(1, &handle);
			handle = std::exchange(other.handle, 0);

		}

		return *this;
	}

	void VBO::create() {

		destroy();
		glCreateBuffers(1, &handle);
	}

	GLuint VBO::id() const noexcept {
		return handle;
	}

	void VBO::storage(GLsizeiptr size, const void* data, GLbitfield flags) {
		glNamedBufferStorage(handle, size, data, flags);
	}

	void VBO::sub_data(GLintptr offset, GLsizeiptr size, const void* data) {
		glNamedBufferSubData(handle, offset, size, data);
	}

	void VBO::destroy() {
		if (handle) glDeleteBuffers(1, &handle);
	}

#pragma endregion

#pragma region VAO
	VAO::VAO() : handle(0) {
		
	}

	VAO::~VAO() {
		
		destroy();
	}

	VAO::VAO(VAO&& other) noexcept : handle(std::exchange(other.handle, 0)) { }

	VAO& VAO::operator=(VAO&& other) noexcept {

		if (this != &other) {

			if (handle) glDeleteVertexArrays(1, &handle);
			handle = std::exchange(other.handle, 0);
		}

		return *this;
	}

	void VAO::create() {

		destroy();
		glCreateVertexArrays(1, &handle);
	}

	GLuint VAO::id() const noexcept { return handle; }

	void VAO::bind() const {

		glBindVertexArray(handle);
	}

	void VAO::enable_attrib(GLuint attrib) const {

		glEnableVertexArrayAttrib(handle, attrib);
	}

	void VAO::bind_vertex_buffer(GLuint binding, const VBO& buf, GLintptr offset, GLsizei stride) const {
		
		glVertexArrayVertexBuffer(handle, binding, buf.id(), offset, stride);
	}

	void VAO::attrib_format(GLuint attrib, GLint comps, GLenum type, bool normalized, GLuint relativeOffset) const {
		glVertexArrayAttribFormat(handle, attrib, comps, type, normalized ? GL_TRUE : GL_FALSE, relativeOffset);
	}

	void VAO::attrib_binding(GLuint attrib, GLuint binding) const {
		glVertexArrayAttribBinding(handle, attrib, binding);
	}

	void VAO::bind_element_buffer(const VBO& ebo) const {
		glVertexArrayElementBuffer(handle, ebo.id());
	}

	void VAO::destroy() {

		if(handle) glDeleteVertexArrays(1, &handle);
	}
#pragma endregion
}
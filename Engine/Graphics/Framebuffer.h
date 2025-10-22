/**
 * @file FrameBuffer.h
 * @brief RAII wrapper for OpenGL framebuffer objects
 * @details Provides a type-safe, move-only interface to OpenGL 4.6 framebuffer
 *          objects (FBOs) using Direct State Access (DSA). Supports attaching
 *          color/depth/stencil targets, multi-render targets, blitting, and
 *          pixel readback. Ensures proper resource cleanup via RAII.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include <optional>
#include <array>
#include <span>

#include "Utility/Types.h"

namespace Engine {

	/**
	 * @brief RAII wrapper for OpenGL framebuffer objects
	 * @details Move-only class managing FBO lifetime. Uses DSA (glNamed* functions)
	 *          for state management without binding. Supports render-to-texture,
	 *          multi-render targets (MRT), and offscreen rendering.
	 */
	class FrameBuffer {
	public:
		FrameBuffer() = delete;

		/**
		 * @brief Factory method to create a new framebuffer object
		 * @return Optional containing framebuffer if successful, nullopt on failure
		 */
		inline static std::optional<FrameBuffer> create() {
			GLuint fbo = kInvalid;
			glCreateFramebuffers(1, &fbo);

			if (fbo == 0) {
				return std::nullopt;
			}

			return FrameBuffer(static_cast<u64>(fbo));
		}

		// Move semantics only
		FrameBuffer(FrameBuffer&& o) noexcept { move_from(o); }
		FrameBuffer& operator=(FrameBuffer&& o) noexcept {
			if (this != &o) { destroy(); move_from(o); }
			return *this;
		}

		/**
		 * @brief Destructor that releases GPU framebuffer resources
		 */
		~FrameBuffer() noexcept { destroy(); }

		/**
		 * @brief Gets the underlying OpenGL framebuffer handle
		 * @return 64-bit framebuffer handle (can be cast to GLuint)
		 */
		u64  handle() const noexcept { return m_handle; }

		/**
		 * @brief Checks if the framebuffer has a valid GPU handle
		 * @return True if valid, false if moved-from or failed to create
		 */
		bool valid() const noexcept { return m_handle != kInvalid; }

		/**
		 * @brief Attaches a texture as a color attachment
		 * @param attachment Color attachment point (GL_COLOR_ATTACHMENT0, etc.)
		 * @param tex OpenGL texture handle to attach
		 * @param level Mipmap level to attach (default: 0)
		 */
		inline void attach_color(GLenum attachment, GLuint tex, GLint level = 0) const {
			glNamedFramebufferTexture(gl_id(), attachment, tex, level);
		}

		/**
		 * @brief Attaches a renderbuffer to an attachment point
		 * @param attachment Attachment point (color, depth, stencil)
		 * @param rbo OpenGL renderbuffer handle to attach
		 */
		inline void attach_renderbuffer(GLenum attachment, GLuint rbo) const {
			glNamedFramebufferRenderbuffer(gl_id(), attachment, GL_RENDERBUFFER, rbo);
		}

		/**
		 * @brief Attaches a depth buffer (texture or renderbuffer)
		 * @param texOrRb OpenGL texture or renderbuffer handle
		 * @param isTexture True if attaching texture, false for renderbuffer
		 * @param level Mipmap level if texture (default: 0)
		 */
		inline void attach_depth(GLuint texOrRb, bool isTexture, GLint level = 0) const {
			if (isTexture) glNamedFramebufferTexture(gl_id(), GL_DEPTH_ATTACHMENT, texOrRb, level);
			else glNamedFramebufferTexture(gl_id(), GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texOrRb);
		}

		/**
		 * @brief Attaches a combined depth-stencil buffer
		 * @param texOrRb OpenGL texture or renderbuffer handle
		 * @param isTexture True if attaching texture, false for renderbuffer
		 * @param level Mipmap level if texture (default: 0)
		 */
		inline void attach_depth_stencil(GLuint texOrRb, bool isTexture, GLint level = 0) const {
			if (isTexture) glNamedFramebufferTexture(gl_id(), GL_DEPTH_STENCIL_ATTACHMENT, texOrRb, level);
			else glNamedFramebufferRenderbuffer(gl_id(), GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, level);
		}

		/**
		 * @brief Sets which color attachments to render into (MRT support)
		 * @param bufs Array of color attachment enums (GL_COLOR_ATTACHMENT0, etc.)
		 */
		inline void set_draw_buffers(std::span<const GLenum> bufs) const {
			glNamedFramebufferDrawBuffers(gl_id(), static_cast<GLsizei>(bufs.size()), bufs.data());
		}

		/**
		 * @brief Sets which color attachment to read from
		 * @param buf Color attachment to read (GL_COLOR_ATTACHMENT0, etc.)
		 */

		inline void set_read_buffer(GLenum buf) const { glNamedFramebufferReadBuffer(gl_id(), buf); }

		/**
		 * @brief Checks if the framebuffer is complete and ready for rendering
		 * @return True if framebuffer is complete, false otherwise
		 */
		inline bool complete() const {
			return glCheckNamedFramebufferStatus(gl_id(), GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		/**
		 * @brief Clears a color attachment with integer values
		 * @param drawbuf Color attachment index to clear
		 * @param r Red component (integer)
		 * @param g Green component (integer)
		 * @param b Blue component (integer)
		 * @param a Alpha component (integer)
		 */
		inline void clear_colori(GLint drawbuf, int r, int g, int b, int a) const {
			glClearNamedFramebufferiv(gl_id(), GL_COLOR, drawbuf, std::array<GLint, 4>{r,g,b,a}.data());
		}

		/**
		 * @brief Clears a color attachment with floating-point values
		 * @param drawbuf Color attachment index to clear
		 * @param r Red component (0.0 - 1.0)
		 * @param g Green component (0.0 - 1.0)
		 * @param b Blue component (0.0 - 1.0)
		 * @param a Alpha component (0.0 - 1.0)
		 */
		inline void clear_colorf(GLint drawbuf, float r, float g, float b, float a) const {
			glClearNamedFramebufferfv(gl_id(), GL_COLOR, drawbuf, std::array<float, 4>{r, g, b, a}.data());
		}

		/**
		 * @brief Clears the depth buffer
		 * @param d Depth value to clear to (typically 1.0)
		 */
		inline void clear_depth(float d) const { glClearNamedFramebufferfv(gl_id(), GL_DEPTH, 0, &d); }

		/**
		 * @brief Clears the stencil buffer
		 * @param s Stencil value to clear to (typically 0)
		 */
		inline void clear_stencil(GLint s) const { glClearNamedFramebufferiv(gl_id(), GL_STENCIL, 0, &s); }

		/**
		 * @brief Blits (copies) a region from source to destination framebuffer
		 * @param src Source framebuffer
		 * @param dst Destination framebuffer
		 * @param sx0 Source region left coordinate
		 * @param sy0 Source region bottom coordinate
		 * @param sx1 Source region right coordinate
		 * @param sy1 Source region top coordinate
		 * @param dx0 Destination region left coordinate
		 * @param dy0 Destination region bottom coordinate
		 * @param dx1 Destination region right coordinate
		 * @param dy1 Destination region top coordinate
		 * @param mask Buffer mask (GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, etc.)
		 * @param filter Filtering mode (GL_NEAREST or GL_LINEAR)
		 */
		static void blit(const FrameBuffer& src, const FrameBuffer& dst, 
						 GLint sx0, GLint sy0, GLint sx1, GLint sy1, 
						 GLint dx0, GLint dy0, GLint dx1, GLint dy1, 
						 GLbitfield mask, GLenum filter) {

			glBlitNamedFramebuffer(src.gl_id(), dst.gl_id(), sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter);
		}

		/**
		 * @brief Invalidates framebuffer attachments (optimization hint)
		 * @param attachments Array of attachment enums to invalidate
		 */
		inline void invalidate(std::span<const GLenum> attachments) const {
			glInvalidateNamedFramebufferData(gl_id(), static_cast<GLsizei>(attachments.size()), attachments.data());
		}

		/**
		 * @brief Reads pixel data from the framebuffer to CPU memory
		 * @param x Left coordinate of read region
		 * @param y Bottom coordinate of read region
		 * @param w Width of read region
		 * @param h Height of read region
		 * @param format Pixel format (GL_RGBA, GL_DEPTH_COMPONENT, etc.)
		 * @param type Data type (GL_UNSIGNED_BYTE, GL_FLOAT, etc.)
		 * @param dst Destination buffer (must be large enough)
		 */
		inline void read_pixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, void* dst) const {
			// must have set a read buffer before this if color read.
			glReadnPixels(x, y, w, h, format, type, w * h * 4, dst);
		}

		/**
		 * @brief Sets a debug label for the framebuffer
		 * @param name Debug label string (visible in graphics debuggers)
		 */
		void label(const char* name) const {
			glObjectLabel(GL_FRAMEBUFFER, gl_id(), -1, name);
		}
	 
	private:
		/**
		 * @brief Private constructor used by factory method
		 * @param h OpenGL framebuffer handle
		 */
		explicit FrameBuffer(u64 h) noexcept : m_handle(h) { }

		/**
		 * @brief Releases GPU resources and resets to invalid state
		 */
		void destroy() noexcept {
			if (m_handle != kInvalid) { const GLuint fb = static_cast<GLuint>(m_handle); glDeleteFramebuffers(1, &fb); }
		}

		/**
		 * @brief Transfers ownership from another framebuffer (move helper)
		 * @param o Source framebuffer to move from
		 */
		void move_from(FrameBuffer& o) {
			m_handle = o.m_handle; o.m_handle = kInvalid;
		}

		/**
		 * @brief Converts internal handle to OpenGL GLuint type
		 * @return OpenGL framebuffer ID
		 */
		GLuint gl_id() const { return static_cast<GLuint>(m_handle); }

		// Member variables
		static constexpr u64 kInvalid = 0;
		u64 m_handle = kInvalid;
	};

}


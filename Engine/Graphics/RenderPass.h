/**
 * @file RenderPass.h
 * @brief Render pass configuration and state management
 * @details Defines render pass types and encapsulates all state needed for
 *          a single rendering pass, including framebuffer targets, shader
 *          programs, viewport settings, and OpenGL state flags. Used to
 *          organize complex multi-pass rendering pipelines.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include <span>

#include "../Graphics/DrawItem.h"
#include "../Graphics/ShaderProgram.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Light.h"

namespace Engine {
	
	/**
	 * @brief Categories of render passes for different rendering techniques
	 */
	enum class PassType : uint8_t {GEOMETRY, FULLSCREEN, DEBUGGING};

	/**
	 * @brief Complete configuration for a single render pass
	 * @details Bundles all OpenGL state, targets, and parameters needed to
	 *          execute one stage of the rendering pipeline. Multiple passes
	 *          can be chained together for deferred rendering, post-processing,
	 *          shadow mapping, etc.
	 */
	struct RenderPass {
		std::string     pass_name;
		u32             fbo_handle; 
		u32             shdpgm_handle;
		glm::vec4       clear_color_value = { 1, 1, 1, 1 };
		glm::vec4       view_port = { 0, 0, 1280, 720 };
		bool            clear_color = true;
		bool            clear_depth = true;
		bool            depth_test  = true;
		bool            depth_write = true;
		bool            blending    = false;
		bool            culling     = true;
		PassType        passtype    = PassType::GEOMETRY;
	};
}



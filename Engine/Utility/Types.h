/**
 * @file Types.h
 * @brief Platform-independent type aliases and size validation
 * @details Defines convenient short-hand aliases for fixed-width integer types
 *          and validates OpenGL type sizes at compile-time. Ensures consistent
 *          integer sizes across platforms and verifies OpenGL type compatibility.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include <glad/glad.h>
#include <cstdint>

namespace Engine {

	using u8  = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using i8  = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	static_assert(sizeof(GLubyte) == sizeof(u8), "GLubyte mismatch");
	static_assert(sizeof(GLuint)  == sizeof(u32), "GLuint mismatch");
}
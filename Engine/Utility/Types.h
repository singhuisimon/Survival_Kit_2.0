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
#include <limits>
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

	constexpr u8  u8_max  = std::numeric_limits<u8>::max();
	constexpr u16 u16_max = std::numeric_limits<u16>::max();
	constexpr u32 u32_max = std::numeric_limits<u32>::max();
	constexpr u64 u64_max = std::numeric_limits<u64>::max();

	constexpr i8  i8_max  = std::numeric_limits<i8>::max();
	constexpr i16 i16_max = std::numeric_limits<i16>::max();
	constexpr i32 i32_max = std::numeric_limits<i32>::max();
	constexpr i64 i64_max = std::numeric_limits<i64>::max();

	static_assert(sizeof(GLubyte) == sizeof(u8), "GLubyte mismatch");
	static_assert(sizeof(GLuint)  == sizeof(u32), "GLuint mismatch");
}
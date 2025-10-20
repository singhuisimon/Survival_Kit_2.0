#pragma once
/**
 * @file MathUtils.h
 * @brief Declaration of common math utilities for the game engine.
 * @details Provides common mathematical constants and functions.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#include <cmath>
#include <algorithm> // For min, max, clamp

namespace Engine {

    class MathUtils {
    public:
        // Mathematical constants
        static constexpr float PI = 3.14159265358979323846f;
        static constexpr float TWO_PI = 6.28318530717958647692f;
        static constexpr float HALF_PI = 1.57079632679489661923f;
        static constexpr float QUARTER_PI = 0.78539816339744830962f;
        static constexpr float DEG_TO_RAD = PI / 180.0f;
        static constexpr float RAD_TO_DEG = 180.0f / PI;
        static constexpr float EPSILON = 0.000001f;

        // Angle conversion functions
        static float toRadians(float degrees);
        static float toDegrees(float radians);

        // Utility functions
        static float clamp(float value, float min, float max);
        static int clamp(int value, int min, int max);
        static float lerp(float a, float b, float t);
        static float inverseLerp(float a, float b, float value);
        static float smoothStep(float a, float b, float t);
        static bool approximatelyEqual(float a, float b, float epsilon = EPSILON);

        // Random number functions
        static void seedRandom(unsigned int seed);
        static float random();                                  // Random [0.0, 1.0)
        static float random(float min, float max);              // Random [min, max)
        static int randomInt(int min, int max);                 // Random integer [min, max]

        // Wave functions
        static float sin(float radians);
        static float cos(float radians);
        static float tan(float radians);
        static float sinDeg(float degrees);
        static float cosDeg(float degrees);
        static float tanDeg(float degrees);
    };

} // end of namespace gam300

#endif // __MATH_UTILS_H__
/**
 * @file MathUtils.cpp
 * @brief Implementation of common math utilities for the game engine.
 * @details Contains implementations for all functions declared in MathUtils.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "MathUtils.h"
#include <random>
#include <ctime>

namespace Engine {

    // Static random engine
    static std::mt19937 randomEngine;
    static bool hasBeenSeeded = false;

    // Angle conversion
    float MathUtils::toRadians(float degrees) {
        return degrees * DEG_TO_RAD;
    }

    float MathUtils::toDegrees(float radians) {
        return radians * RAD_TO_DEG;
    }

    // Clamping functions
    float MathUtils::clamp(float value, float min, float max) {
        return std::max(min, std::min(max, value));
    }

    int MathUtils::clamp(int value, int min, int max) {
        return std::max(min, std::min(max, value));
    }

    // Linear interpolation
    float MathUtils::lerp(float a, float b, float t) {
        t = clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

    // Inverse linear interpolation
    float MathUtils::inverseLerp(float a, float b, float value) {
        if (std::abs(a - b) < EPSILON) {
            return 0.0f;
        }
        return clamp((value - a) / (b - a), 0.0f, 1.0f);
    }

    // Smooth step interpolation
    float MathUtils::smoothStep(float a, float b, float t) {
        t = clamp(t, 0.0f, 1.0f);
        t = t * t * (3.0f - 2.0f * t); // Smooth step formula
        return a + (b - a) * t;
    }

    // Approximate equality
    bool MathUtils::approximatelyEqual(float a, float b, float epsilon) {
        return std::abs(a - b) < epsilon;
    }

    // Seed the random number generator
    void MathUtils::seedRandom(unsigned int seed) {
        randomEngine.seed(seed);
        hasBeenSeeded = true;
    }

    // Random [0.0, 1.0)
    float MathUtils::random() {
        if (!hasBeenSeeded) {
            seedRandom(static_cast<unsigned int>(std::time(nullptr)));
        }

        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        return distribution(randomEngine);
    }

    // Random [min, max)
    float MathUtils::random(float min, float max) {
        if (!hasBeenSeeded) {
            seedRandom(static_cast<unsigned int>(std::time(nullptr)));
        }

        std::uniform_real_distribution<float> distribution(min, max);
        return distribution(randomEngine);
    }

    // Random integer [min, max]
    int MathUtils::randomInt(int min, int max) {
        if (!hasBeenSeeded) {
            seedRandom(static_cast<unsigned int>(std::time(nullptr)));
        }

        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(randomEngine);
    }
}
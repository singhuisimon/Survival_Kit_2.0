/**
 * @file LightComponent.h
 * @brief Light component - directional / point / spot light properties
 * @author Chua Wen Bin Kenny
 * @date 03 November 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include <cstdint>
#include <glm/glm.hpp>

 // Types for u32
#include "../Utility/Types.h"

// Resource types for xresource::instance_guid
#include "../Asset/ResourceTypes.h"

namespace Engine {

    // Must match the values used in Renderer.h (LIGHT_DIRECTIONAL=0, LIGHT_POINT=1, LIGHT_SPOT=2)
    enum class LightType : uint32_t {
        Directional = 0u,
        Point = 1u,
        Spot = 2u
    };

    // For future expansion; currently only Realtime is used
    enum class LightMode : uint32_t {
        Realtime = 0u,
        // Baked = 1u, // reserved
    };

    struct LightComponent
    {
        //// --------- Identity ---------
        //xresource::instance_guid ComponentGUID{ xresource::instance_guid::GenerateGUIDCopy() };

        // --------- On/Off ---------
        bool Enabled = true;

        // --------- Type / Mode ---------
        LightType Type = LightType::Point;      // default to Point (common for scene lights)
        LightMode Mode = LightMode::Realtime;   // realtime only for now

        // --------- Radiometric properties ---------
        // Use linear RGB (NOT sRGB) here 
        glm::vec3 Color = glm::vec3(1.0f);      // white
        float     Intensity = 1.0f;             // default 1.0; Directional defaults to 0.5 

        // --------- Range / cone (used for point & spot) ---------
        // Range is the "influence radius" used both for CPU culling and shader falloff.
        float     Range = 5.0f;                 // only point/spot; ignored for directional
        float     SpotAngleDeg = 30.0f;         // outer cone angle in degrees; inner = 0.85 * outer (in RenderSystem);

        // --------- Indirect / future GI scaling ---------
        float     IndirectMultiplier = 1.0f;    // reserved for ambient/IBL/lightmaps scaling

        // --------- Reserved / alignment (keep struct compact & consistent) ---------
        // (Not strictly necessary here, but good to leave room for flags like CastShadows, CookieIndex, etc.)
        uint32_t  _Reserved0 = 0u;

        // Default constructor
        LightComponent() = default;

        // Construct with type; sets a sensible default intensity per type
        explicit LightComponent(LightType type)
            : Type(type)
        {
            // Unity-like default intensities:
            // - Directional: 0.5 (no distance falloff; easy to blow out otherwise)
            // - Point/Spot : 1.0
            Intensity = (type == LightType::Directional) ? 0.5f : 1.0f;
        }

        // Fully-specified constructor with optional parameters
        LightComponent(LightType type,
            const glm::vec3& color = glm::vec3(1.0f),
            float intensity = -1.0f,          // -1 uses type default
            float range = 5.0f,
            float spotAngleDeg = 30.0f,
            float indirectMult = 1.0f,
            LightMode mode = LightMode::Realtime,
            bool  enabled = true)
            : Enabled(enabled),
            Type(type),
            Mode(mode),
            Color(color),
            Intensity(intensity < 0.0f ? ((type == LightType::Directional) ? 0.5f : 1.0f) : intensity),
            Range(range),
            SpotAngleDeg(spotAngleDeg),
            IndirectMultiplier(indirectMult)
        {
        }

        // Convenience mutators that maintain sensible defaults

        void SetType(LightType t, bool resetIntensityToDefault = true) {
            Type = t;
            if (resetIntensityToDefault) {
                Intensity = (t == LightType::Directional) ? 0.5f : 1.0f;
            }
        }

        void SetColorLinear(const glm::vec3& c) { Color = c; }

        void SetIntensity(float i) { Intensity = i; }

        // For point/spot only (safe to set anyway; directional ignores Range)
        void SetRange(float r) { Range = r; }

        // For spot only (safe to set anyway; non-spot ignores SpotAngleDeg)
        void SetSpotAngleDeg(float deg) { SpotAngleDeg = deg; }

        void SetIndirectMultiplier(float m) { IndirectMultiplier = m; }
    };

} // namespace Engine

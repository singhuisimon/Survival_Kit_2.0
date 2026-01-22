/**
 * @file LightComponent.h
 * @brief   Component that stores Light-related properties
 * @details The data stored will be used by the graphics pipeline to   
 *          produce the desired lighting in the scene
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
#include "../Serialization/ComponentRegistry.h"

namespace Engine {

    // Must match the values used in Renderer.h (LIGHT_DIRECTIONAL=0, LIGHT_POINT=1, LIGHT_SPOT=2)
    enum class LightType : u32 {
        Directional = 0u,
        Point = 1u,
        Spot = 2u
    };

    // For future expansion; currently only Realtime is used
    enum class LightMode : u32 {
        Realtime = 0u,
        // Baked = 1u, // reserved
    };

    // Shadow type
    enum class ShadowType : u32 {
        No = 0u,
        Hard = 1u,
        Soft = 2u
    };

    struct LightComponent
    {
        static constexpr ComponentTypeID TypeID = ComponentTypeID::Light;
        static constexpr const char* TypeName = "LightComponent";

        xresource::instance_guid ComponentGUID;
        // --------- On/Off ---------
        bool Enabled = true;

        // --------- Type / Mode ---------
        LightType Type = LightType::Point;      // Default to Point 
        LightMode Mode = LightMode::Realtime;   // Realtime only for now

        // --------- Radiometric properties ---------
        // Use linear RGB (NOT sRGB) here 
        glm::vec3 Color = glm::vec3(1.0f);      // White
        float     Intensity = 1.0f;             // Default 1.0; Directional defaults to 0.5 

        // --------- Range / cone (used for point & spot) ---------
        // Range is the "influence radius" used both for CPU culling and shader falloff.
        float     Range = 5.0f;                 // Only point/spot; ignored for directional
        float     SpotAngleDeg = 30.0f;         // Outer cone angle in degrees; inner = 0.85 * outer (in RenderSystem);

        // --------- Indirect / future GI scaling ---------
        float     IndirectMultiplier = 1.0f;    // Reserved for ambient/IBL/lightmaps scaling

        // --------- Shadows  ---------
        ShadowType TypeShadow = ShadowType::No;
        u32 Resolution = 1024; // Default 1024
        float Strength = 1.0f; // 0.0 to 1.0; Default 1.0
        float Bias = 0.005;
        // float NormalBias = 0.05;
        float NearPlane = 0.2; // Clamped to 0.1 or 1% of light's range, whichever is lower. Set to 0.2 by default

        // Default constructor
        LightComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
        {
        }

        // Construct with type; sets a sensible default intensity per type
        explicit LightComponent(LightType type)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()),
            Type(type)
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
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()),
            Enabled(enabled),
            Type(type),
            Mode(mode),
            Color(color),
            Intensity(intensity < 0.0f ? ((type == LightType::Directional) ? 0.5f : 1.0f) : intensity),
            Range(range),
            SpotAngleDeg(spotAngleDeg),
            IndirectMultiplier(indirectMult)
        {
        }

        /**
        * @brief Set lighting type with defaults for certain lights
        * @param t The type to set 
        * @param resetIntensityToDefault To set type-specific intensity defaults
        */
        void SetLightType(LightType t, bool resetIntensityToDefault = true) {
            Type = t;
            if (resetIntensityToDefault) {
                Intensity = (t == LightType::Directional) ? 0.5f : 1.0f;
            }
        }

        /**
        * @brief Set lighting color
        * @param c The color value to set
        */
        void SetColorLinear(const glm::vec3& c) { Color = c; }

        /**
        * @brief Set lighting intensity
        * @param i The intensity value to set
        */
        void SetIntensity(float i) { Intensity = i; }

        /**
        * @brief Set lighting range (For point/spot light only; directional ignores Range)
        * @param r The range value to set
        */
        void SetRange(float r) { Range = r; }

        /**
        * @brief Set lighting range (For spot light only; non-spot ignores SpotAngleDeg)
        * @param deg The degree value to set
        */
        void SetSpotAngleDeg(float deg) { SpotAngleDeg = deg; }

        /**
        * @brief Set indirect multiplier 
        * @param m The multiplier value to set
        */
        void SetIndirectMultiplier(float m) { IndirectMultiplier = m; }



        void SetShadowType(ShadowType t) { TypeShadow = t; }
    };

} // namespace Engine

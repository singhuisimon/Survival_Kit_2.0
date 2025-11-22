/**
 * @file AnimatorComponent.h
 * @brief   Component that stores Camera-related properties
 * @details The data stored will be used by the graphics pipeline to
 *          create the desired view of the scene
 * @author Chua Wen Bin Kenny
 * @date 20 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

//#include <glm/glm.hpp>                  // Core types: vec, mat
//#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective, translate/scale/rotate
//#include <glm/common.hpp>               // glm::clamp
//#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr

 // Math Utility
#include "../Utility/MathUtils.h"

// Types for u32
#include "../Utility/Types.h"

// Resource types for xresource::instance_guid
#include "../Asset/ResourceTypes.h"

namespace Engine {

    /**
     * @brief Animator component
     */
    struct AnimatorComponent {

        // Toggles
        bool playing = true;
        bool respectClipLoop = true;   // you can override clip.loop if needed

        // Controller handle
        u32 controller = 0;

        // Runtime state
        u32 currentClipIndex = 0;
        float currentTime = 0.0f;   // seconds in current clip
        float playbackSpeed = 1.0f;
    };

} // namespace Engine



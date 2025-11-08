/**
 * @file ListenerComponent.h
 * @brief Component to mark an entity as an FMOD 3D Listener
 * @author Amanda Leow Boon Suan (100%)
 * @date 23/10/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#include <entt/entt.hpp>

namespace Engine {

    /**
     * @brief Marks an entity as an FMOD 3D Listener (usually 1 per scene)
     */
    struct ListenerComponent {
        
        bool Active; // Is this the primary active listener?

        ListenerComponent() : Active(true){}

    };

} //namespace Engine
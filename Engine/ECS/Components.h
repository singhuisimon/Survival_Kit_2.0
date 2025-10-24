#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fmod.hpp>
#include "Utility/Types.h"

// Components
#include "Component/TransformComponent.h"
#include "Component/MeshRendererComponent.h"
#include "Component/CameraComponent.h"
#include "Component/AudioComponent.h"
#include "Component/ListenerComponent.h"
#include "Component/ReverbZoneComponent.h"
#include "Component/AIComponent.h"

namespace Engine {

    /**
     * IMPORTANT: All components are POD (Plain Old Data) structs
     * - NO methods (except simple getters that do calculations)
     * - NO virtual functions
     * - NO inheritance
     * - Just data that systems operate on
     */

     /**
      * @brief Gives an entity a human-readable name
      */
    struct TagComponent {
        std::string Tag;

        // Default constructor
        TagComponent() : Tag("Entity") {}

        // Constructor with name
        TagComponent(const std::string& tag) : Tag(tag) {}

        // Copy constructor
        TagComponent(const TagComponent& other) : Tag(other.Tag) {}

        // Move constructor
        TagComponent(TagComponent&& other) noexcept : Tag(std::move(other.Tag)) {}

        // Assignment operator
        TagComponent& operator=(const TagComponent& other) {
            if (this != &other) {
                Tag = other.Tag;
            }
            return *this;
        }

        // Move assignment
        TagComponent& operator=(TagComponent&& other) noexcept {
            if (this != &other) {
                Tag = std::move(other.Tag);
            }
            return *this;
        }
    };

    /**
     * @brief Rigidbody component (for Jolt Physics)
     */
    struct RigidbodyComponent {
        float Mass;
        bool IsKinematic;
        bool UseGravity;
        glm::vec3 Velocity;

        // Default constructor
        RigidbodyComponent()
            : Mass(1.0f)
            , IsKinematic(false)
            , UseGravity(true)
            , Velocity(0.0f, 0.0f, 0.0f) {
        }
    };

} // namespace Engine
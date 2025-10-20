#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "../Scripting/ScriptComponent.h"  // Add at the top with other includes
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
     * @brief Transform component - position, rotation, scale
     */
    struct TransformComponent {
        glm::vec3 Position;
        glm::vec3 Rotation; // Euler angles in degrees
        glm::vec3 Scale;

        // Default constructor
        TransformComponent()
            : Position(0.0f, 0.0f, 0.0f)
            , Rotation(0.0f, 0.0f, 0.0f)
            , Scale(1.0f, 1.0f, 1.0f) {
        }

        // Constructor with position
        TransformComponent(const glm::vec3& position)
            : Position(position)
            , Rotation(0.0f, 0.0f, 0.0f)
            , Scale(1.0f, 1.0f, 1.0f) {
        }

        /**
         * @brief Calculate transformation matrix
         * @note This is OK - it's just a calculation, not game logic
         */
        glm::mat4 GetTransform() const {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1, 0, 0))
                * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0, 1, 0))
                * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0, 0, 1));

            return glm::translate(glm::mat4(1.0f), Position)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    /**
     * @brief Camera component
     */
    struct CameraComponent {
        float FOV;
        float NearClip;
        float FarClip;
        bool Primary; // Is this the main camera?

        // Default constructor
        CameraComponent()
            : FOV(45.0f)
            , NearClip(0.1f)
            , FarClip(1000.0f)
            , Primary(true) {
        }

        glm::mat4 GetProjection(float aspectRatio) const {
            return glm::perspective(glm::radians(FOV), aspectRatio, NearClip, FarClip);
        }
    };

    /**
     * @brief Mesh renderer component (for future rendering system)
     */
    struct MeshRendererComponent {
        bool Visible;

        // Default constructor
        MeshRendererComponent()
            : Visible(true) {
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
#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine {

    /**
     * @brief Transform component - position, rotation, scale
     */
    struct TransformComponent {
        glm::vec3 Position;
        glm::quat Rotation; // Rotation stored as a quaternion instead of euler angles -> to avoid gimbal lock
        glm::vec3 Scale;

        // Cache the matrices
        glm::mat4 LocalTransform;
        glm::mat4 WorldTransform;

        // Dirty flags
        bool IsDirty;

        // Parent-Child
        entt::entity Parent;
        std::vector<entt::entity> Children;

        // Default constructor
        TransformComponent()
            : Position(0.0f, 0.0f, 0.0f)
            , Rotation(1, 0, 0, 0)
            , Scale(1.0f, 1.0f, 1.0f)
            , LocalTransform(1.0f)
            , WorldTransform(1.0f)
            , IsDirty(true)
            , Parent(entt::null){
        }

        // Constructor with position
        TransformComponent(const glm::vec3& position)
            : Position(position)
            , Rotation(1, 0, 0, 0)
            , Scale(1.0f, 1.0f, 1.0f)
            , LocalTransform(1.0f)
            , WorldTransform(1.0f)
            , IsDirty(true) 
            , Parent(entt::null) {
        }

        void SetPosition(glm::vec3 const& pos) {
            Position = pos;
            IsDirty = true;
        }

        void SetScale(glm::vec3 const& scl) {
            Scale = scl;
            IsDirty = true;
        }

        void SetRotation(glm::vec3 const& eulerAngles) {
            glm::vec3 eulerRadians = glm::radians(eulerAngles);
            Rotation = glm::quat(eulerRadians);
            IsDirty = true;
        }
    };

}
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include "Utility/Types.h"

// Components
#include "../Component/TransformComponent.h"

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

    // Kenny: Don't think we need a camera component, just need primary camera for the game, 
    //        and additional secondary camera(s) if needed in the game
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
        bool Visible;           // Determine if sent to draw call
        bool ShadowReceive;     // For future expansion (WIP)
        bool ShadowCast;        // For future expansion (WIP)
        bool GlobalIlluminate;  // Require further expansion; for now true means it receives light from a light object
        u32 MeshType;           // Mesh that the object uses (primitive/custom)
        u32 Material;           // Material handle
        u32 Texture;            // Texture handle (0 means no texture, actual textures start from 1)

        // Default constructor
        MeshRendererComponent()
            : Visible(true), 
              ShadowReceive(false), 
              ShadowCast(false), 
              GlobalIlluminate(true), 
              MeshType(0), 
              Material(0),
              Texture(0)  {
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
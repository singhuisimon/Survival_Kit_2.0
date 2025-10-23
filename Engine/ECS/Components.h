//#pragma once
//
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//#include <string>
//#include "Utility/Types.h"
//#include <vector>
//#include "../xresource_guid/include/xresource_guid.h"
//
//// Components
//#include "../Component/TransformComponent.h"
//
//namespace Engine {
//
//    /**
//     * IMPORTANT: All components are POD (Plain Old Data) structs
//     * - NO methods (except simple getters that do calculations)
//     * - NO virtual functions
//     * - NO inheritance
//     * - Just data that systems operate on
//     */
//
//     /**
//      * @brief Gives an entity a human-readable name
//      */
//    struct TagComponent {
//        xresource::instance_guid ComponentGUID;
//        std::string Tag;
//
//        // Default constructor
//        TagComponent() : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()), Tag("Entity") {}
//
//        // Constructor with name
//        TagComponent(const std::string& tag)
//            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()), Tag(tag) {
//        }
//
//        // Copy constructor
//        TagComponent(const TagComponent& other)
//            : ComponentGUID(other.ComponentGUID), Tag(other.Tag) {
//        }
//
//        // Move constructor
//        TagComponent(TagComponent&& other) noexcept
//            : ComponentGUID(other.ComponentGUID), Tag(std::move(other.Tag)) {
//        }
//
//        // Assignment operator
//        TagComponent& operator=(const TagComponent& other) {
//            if (this != &other) {
//                ComponentGUID = other.ComponentGUID;
//                Tag = other.Tag;
//            }
//            return *this;
//        }
//
//        // Move assignment
//        TagComponent& operator=(TagComponent&& other) noexcept {
//            if (this != &other) {
//                ComponentGUID = other.ComponentGUID;
//                Tag = std::move(other.Tag);
//            }
//            return *this;
//        }
//    };
//
//    /**
//     * @brief Transform component - position, rotation, scale
//     */
//    struct TransformComponent {
//        xresource::instance_guid ComponentGUID;
//        glm::vec3 Position;
//        glm::vec3 Rotation; // Euler angles in degrees
//        glm::vec3 Scale;
//
//        // Default constructor
//        TransformComponent()
//            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
//            , Position(0.0f, 0.0f, 0.0f)
//            , Rotation(0.0f, 0.0f, 0.0f)
//            , Scale(1.0f, 1.0f, 1.0f) {
//        }
//
//        // Constructor with position
//        TransformComponent(const glm::vec3& position)
//            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
//            , Position(position)
//            , Rotation(0.0f, 0.0f, 0.0f)
//            , Scale(1.0f, 1.0f, 1.0f) {
//        }
//
//        /**
//         * @brief Calculate transformation matrix
//         * @note This is OK - it's just a calculation, not game logic
//         */
//        glm::mat4 GetTransform() const {
//            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1, 0, 0))
//                * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0, 1, 0))
//                * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0, 0, 1));
//
//            return glm::translate(glm::mat4(1.0f), Position)
//                * rotation
//                * glm::scale(glm::mat4(1.0f), Scale);
//        }
//    };
//
//    /**
//     * @brief Camera component
//     */
//    struct CameraComponent {
//        xresource::instance_guid ComponentGUID;
//        float FOV;
//        float NearClip;
//        float FarClip;
//        bool Primary; // Is this the main camera?
//
//        // Default constructor
//        CameraComponent()
//            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
//            , FOV(45.0f)
//            , NearClip(0.1f)
//            , FarClip(1000.0f)
//            , Primary(true) {
//        }
//
//        glm::mat4 GetProjection(float aspectRatio) const {
//            return glm::perspective(glm::radians(FOV), aspectRatio, NearClip, FarClip);
//        }
//    };
//
//    /**
//     * @brief Mesh renderer component (for future rendering system)
//     */
//    struct MeshRendererComponent {
//        bool Visible;           // Determine if sent to draw call
//        bool ShadowReceive;     // For future expansion (WIP)
//        bool ShadowCast;        // For future expansion (WIP)
//        bool GlobalIlluminate;  // Require further expansion; for now true means it receives light from a light object
//        u32 MeshType;           // Mesh that the object uses (primitive/custom)
//        u32 Material;           // Material handle
//        u32 Texture;            // Texture handle (0 means no texture, actual textures start from 1)
//
//        // Default constructor
//        MeshRendererComponent()
//            : Visible(true), 
//              ShadowReceive(false), 
//              ShadowCast(false), 
//              GlobalIlluminate(true), 
//              MeshType(0), 
//              Material(0),
//              Texture(0)  {
//        }
//    };
//
//    /**
//     * @brief Rigidbody component (for Jolt Physics)
//     */
//    struct RigidbodyComponent {
//        xresource::instance_guid ComponentGUID;
//        float Mass;
//        bool IsKinematic;
//        bool UseGravity;
//        glm::vec3 Velocity;
//
//        // Default constructor
//        RigidbodyComponent()
//            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
//            , Mass(1.0f)
//            , IsKinematic(false)
//            , UseGravity(true)
//            , Velocity(0.0f, 0.0f, 0.0f) {
//        }
//    };
//
//    /**
//     * @brief Prefab component - tracks prefab instances and their overrides
//     * @note This is an invisible component that marks an entity as a prefab instance
//     */
//    struct PrefabComponent {
//        xresource::instance_guid ComponentGUID;
//
//        // Reference to the prefab resource
//        xresource::instance_guid PrefabGUID;
//
//        // Track added components (components not in original prefab)
//        std::vector<xresource::instance_guid> AddedComponents;
//
//        // Track deleted components (components removed from prefab)
//        std::vector<xresource::instance_guid> DeletedComponents;
//
//        // Track overridden properties
//        struct OverriddenProperty {
//            xresource::instance_guid ComponentGUID;  // Which component?
//            std::string PropertyPath;                // Which property?
//            std::string Value;                       // Serialized value
//        };
//        std::vector<OverriddenProperty> OverriddenProperties;
//
//        // Default constructor
//        PrefabComponent()
//            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
//            , PrefabGUID(xresource::instance_guid{}) {
//        }
//
//        // Constructor with prefab GUID
//        PrefabComponent(xresource::instance_guid prefabGuid)
//            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
//            , PrefabGUID(prefabGuid) {
//        }
//    };
//
//} // namespace Engine
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fmod.hpp>
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

    enum class AudioType {MASTER, SFX, BGM, UI};
    enum class PlayState {PLAY, PAUSE, STOP};

    struct AudioComponent {
		std::string AudioFilePath;  // Path to the audio asset
        AudioType Type;             // SFX or BGM
        PlayState State;            // Play, Pause, Stop
        float Volume;               // Volume (0-1.0)
        float Pitch;                // Pitch ()
        bool Loop;                  // Indicate if audio should be in loop
        bool Mute;                  // Mute flag
        bool Reverb;                // Affected by Reverb
        bool Is3D;                  // Enable 3D Spatialization
        float MinDistance;          // 3D attentuation min
        float MaxDistance;          // 3D attentuation max

        FMOD::Channel* Channel;
		std::string PreviousPath;   // To track changes in audio file
        PlayState PreviousState;    // To track changes in state

        AudioComponent()
            : AudioFilePath("")
            , Type(AudioType::SFX)
            , State(PlayState::STOP)
            , Volume(1.0f)
            , Pitch(1.0f)
            , Loop(false)
            , Mute(false)
            , Reverb(false)
            , Is3D(true)
            , MinDistance(1.0f)
            , MaxDistance(100.0f)
            , Channel (nullptr)
            , PreviousPath("") 
            , PreviousState(PlayState::STOP){
		}
    };

    struct ListenerComponent {
        bool Active;

        ListenerComponent()
            : Active(true) {
		}
    };

} // namespace Engine

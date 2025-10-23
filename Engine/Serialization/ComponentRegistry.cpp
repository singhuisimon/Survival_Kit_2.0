/**
 * @file ComponentRegistry.cpp
 * @brief Implementation of component registration for serialization
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "ComponentRegistry.h"
#include "ReflectionRegistry.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/CameraComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/RigidbodyComponent.h"
#include "../Component/PrefabComponent.h"
#include "../Utility/Logger.h"

 // Required for quaternion to Euler conversion
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Engine {

    void ComponentRegistry::RegisterAllComponents() {
        LOG_INFO("Registering component reflection metadata...");

        // Register TagComponent
        {
            auto& meta = REGISTER_COMPONENT(TagComponent);
            meta.AddProperty<TagComponent, std::string>(
                "Tag",
                PropertyType::String,
                [](const TagComponent& c) { return c.Tag; },
                [](TagComponent& c, const std::string& v) { c.Tag = v; }
            );
        }

        // Register TransformComponent
        {
            auto& meta = REGISTER_COMPONENT(TransformComponent);
            meta.AddProperty<TransformComponent, glm::vec3>(
                "Position",
                PropertyType::Vec3,
                [](const TransformComponent& c) { return c.Position; },
                [](TransformComponent& c, const glm::vec3& v) { c.SetPosition(v); }
            );
            meta.AddProperty<TransformComponent, glm::vec3>(
                "Rotation",
                PropertyType::Vec3,
                [](const TransformComponent& c) {
                    // Convert quaternion to Euler angles (in degrees) for serialization
                    return glm::degrees(glm::eulerAngles(c.Rotation));
                },
                [](TransformComponent& c, const glm::vec3& v) {
                    // Convert Euler angles to quaternion
                    c.SetRotation(v);
                }
            );
            meta.AddProperty<TransformComponent, glm::vec3>(
                "Scale",
                PropertyType::Vec3,
                [](const TransformComponent& c) { return c.Scale; },
                [](TransformComponent& c, const glm::vec3& v) { c.SetScale(v); }
            );
        }

        // Register CameraComponent
        {
            auto& meta = REGISTER_COMPONENT(CameraComponent);
            meta.AddProperty<CameraComponent, float>(
                "FOV",
                PropertyType::Float,
                [](const CameraComponent& c) { return c.FOV; },
                [](CameraComponent& c, const float& v) { c.FOV = v; }
            );
            meta.AddProperty<CameraComponent, float>(
                "NearClip",
                PropertyType::Float,
                [](const CameraComponent& c) { return c.NearClip; },
                [](CameraComponent& c, const float& v) { c.NearClip = v; }
            );
            meta.AddProperty<CameraComponent, float>(
                "FarClip",
                PropertyType::Float,
                [](const CameraComponent& c) { return c.FarClip; },
                [](CameraComponent& c, const float& v) { c.FarClip = v; }
            );
            meta.AddProperty<CameraComponent, bool>(
                "Primary",
                PropertyType::Bool,
                [](const CameraComponent& c) { return c.Primary; },
                [](CameraComponent& c, const bool& v) { c.Primary = v; }
            );
        }

        // Register MeshRendererComponent
        {
            auto& meta = REGISTER_COMPONENT(MeshRendererComponent);
            meta.AddProperty<MeshRendererComponent, bool>(
                "Visible",
                PropertyType::Bool,
                [](const MeshRendererComponent& c) { return c.Visible; },
                [](MeshRendererComponent& c, const bool& v) { c.Visible = v; }
            );
        }

        // Register RigidbodyComponent
        {
            auto& meta = REGISTER_COMPONENT(RigidbodyComponent);
            meta.AddProperty<RigidbodyComponent, float>(
                "Mass",
                PropertyType::Float,
                [](const RigidbodyComponent& c) { return c.Mass; },
                [](RigidbodyComponent& c, const float& v) { c.Mass = v; }
            );
            meta.AddProperty<RigidbodyComponent, bool>(
                "IsKinematic",
                PropertyType::Bool,
                [](const RigidbodyComponent& c) { return c.IsKinematic; },
                [](RigidbodyComponent& c, const bool& v) { c.IsKinematic = v; }
            );
            meta.AddProperty<RigidbodyComponent, bool>(
                "UseGravity",
                PropertyType::Bool,
                [](const RigidbodyComponent& c) { return c.UseGravity; },
                [](RigidbodyComponent& c, const bool& v) { c.UseGravity = v; }
            );
            meta.AddProperty<RigidbodyComponent, glm::vec3>(
                "Velocity",
                PropertyType::Vec3,
                [](const RigidbodyComponent& c) { return c.Velocity; },
                [](RigidbodyComponent& c, const glm::vec3& v) { c.Velocity = v; }
            );
        }

        //Register AudioComponent
        {
            auto& meta = REGISTER_COMPONENT(AudioComponent);

            meta.AddProperty<AudioComponent, std::string>(
                "Filepath",
                PropertyType::String,
                [](const AudioComponent& c) { return c.AudioFilePath; },
                [](AudioComponent& c, const std::string& v) { c.AudioFilePath = v; }
            );
            meta.AddProperty<AudioComponent, AudioType>(
                "Type",
                PropertyType::Int,
                [](const AudioComponent& c) { return c.Type; },
                [](AudioComponent& c, const AudioType& v) { c.Type = v; }
            );
            meta.AddProperty<AudioComponent, PlayState>(
                "State",
                PropertyType::Int,
                [](const AudioComponent& c) { return c.State; },
                [](AudioComponent& c, const PlayState& v) { c.State = v; }
            );
            meta.AddProperty<AudioComponent, float>(
                "Volume",
                PropertyType::Float,
                [](const AudioComponent& c) { return c.Volume; },
                [](AudioComponent& c, const float& v) { c.Volume = v; }
            );
            meta.AddProperty<AudioComponent, float>(
                "Pitch",
                PropertyType::Float,
                [](const AudioComponent& c) { return c.Pitch; },
                [](AudioComponent& c, const float& v) { c.Pitch = v; }
            );
            meta.AddProperty<AudioComponent, bool>(
                "Loop",
                PropertyType::Bool,
                [](const AudioComponent& c) { return c.Loop; },
                [](AudioComponent& c, const bool& v) { c.Loop = v; }
            );
            meta.AddProperty<AudioComponent, bool>(
                "Mute",
                PropertyType::Bool,
                [](const AudioComponent& c) { return c.Mute; },
                [](AudioComponent& c, const bool& v) { c.Mute = v; }
            );
            meta.AddProperty<AudioComponent, float>(
                "ReverbProperties",
                PropertyType::Float,
                [](const AudioComponent& c) { return c.ReverbProperties; },
                [](AudioComponent& c, const float& v) { c.ReverbProperties = v; }
            );
            meta.AddProperty<AudioComponent, bool>(
                "Is3D",
                PropertyType::Bool,
                [](const AudioComponent& c) { return c.Is3D; },
                [](AudioComponent& c, const bool& v) { c.Is3D = v; }
            );
            meta.AddProperty<AudioComponent, float>(
                "MinDistance",
                PropertyType::Float,
                [](const AudioComponent& c) { return c.MinDistance; },
                [](AudioComponent& c, const float& v) { c.MinDistance = v; }
            );
            meta.AddProperty<AudioComponent, float>(
                "MaxDistance",
                PropertyType::Float,
                [](const AudioComponent& c) { return c.MaxDistance; },
                [](AudioComponent& c, const float& v) { c.MaxDistance = v; }
            );
        }

        //Register ListenerComponenet
        {
            auto& meta = REGISTER_COMPONENT(ListenerComponent);

            meta.AddProperty<ListenerComponent, bool>(
                "Active",
                PropertyType::Bool,
                [](const ListenerComponent& c) { return c.Active; },
                [](ListenerComponent& c, const bool& v) { c.Active = v; }
            );
        }

        // Register PrefabComponent
        {
            auto& meta = REGISTER_COMPONENT(PrefabComponent);
            // PrefabComponent properties are managed internally
            // No user-editable properties exposed in inspector by default
        }

        LOG_INFO("Component reflection registration complete");
        LOG_INFO("  - Registered 7 component types");
    }

} // namespace Engine
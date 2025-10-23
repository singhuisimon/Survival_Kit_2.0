#include "ComponentRegistry.h"
#include "ReflectionRegistry.h"
#include "../ECS/Components.h"
#include "../Utility/Logger.h"

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
                [](TransformComponent& c, const glm::vec3& v) { c.Position = v; }
            );
            meta.AddProperty<TransformComponent, glm::quat>(
                "Rotation",
                PropertyType::Quat,
                [](const TransformComponent& c) { return c.Rotation; },
                [](TransformComponent& c, const glm::quat& v) { c.Rotation = v; }
            );
            meta.AddProperty<TransformComponent, glm::vec3>(
                "Scale",
                PropertyType::Vec3,
                [](const TransformComponent& c) { return c.Scale; },
                [](TransformComponent& c, const glm::vec3& v) { c.Scale = v; }
            );
        }

        // Register CameraComponent
        {
            auto& meta = REGISTER_COMPONENT(CameraComponent);
            meta.AddProperty<CameraComponent, bool>(
                "Enabled",
                PropertyType::Bool,
                [](const CameraComponent& c) { return c.Enabled; },
                [](CameraComponent& c, const bool& v) { c.Enabled = v; }
            );
            meta.AddProperty<CameraComponent, bool>(
                "autoAspect",
                PropertyType::Bool,
                [](const CameraComponent& c) { return c.autoAspect; },
                [](CameraComponent& c, const bool& v) { c.autoAspect = v; }
            );
            meta.AddProperty<CameraComponent, bool>(
                "isDirty",
                PropertyType::Bool,
                [](const CameraComponent& c) { return c.isDirty; },
                [](CameraComponent& c, const bool& v) { c.isDirty = v; }
            );
            meta.AddProperty<CameraComponent, u32>(
                "Depth",
                PropertyType::U32,
                [](const CameraComponent& c) { return c.Depth; },
                [](CameraComponent& c, const u32& v) { c.Depth = v; }
            );
            meta.AddProperty<CameraComponent, float>(
                "Aspect",
                PropertyType::Float,
                [](const CameraComponent& c) { return c.Aspect; },
                [](CameraComponent& c, const float& v) { c.Aspect = v; }
            );
            meta.AddProperty<CameraComponent, float>(
                "FOV",
                PropertyType::Float,
                [](const CameraComponent& c) { return c.FOV; },
                [](CameraComponent& c, const float& v) { c.FOV = v; }
            );
            meta.AddProperty<CameraComponent, float>(
                "NearPlane",
                PropertyType::Float,
                [](const CameraComponent& c) { return c.NearPlane; },
                [](CameraComponent& c, const float& v) { c.NearPlane = v; }
            );
            meta.AddProperty<CameraComponent, float>(
                "FarPlane",
                PropertyType::Float,
                [](const CameraComponent& c) { return c.FarPlane; },
                [](CameraComponent& c, const float& v) { c.FarPlane = v; }
            );
            meta.AddProperty<CameraComponent, glm::vec3>(
                "Target",
                PropertyType::Vec3,
                [](const CameraComponent& c) { return c.Target; },
                [](CameraComponent& c, const glm::vec3& v) { c.Target = v; }
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

            meta.AddProperty<MeshRendererComponent, bool>(
                "ShadowReceive",
                PropertyType::Bool,
                [](const MeshRendererComponent& c) { return c.ShadowReceive; },
                [](MeshRendererComponent& c, const bool& v) { c.ShadowReceive = v; }
            );

            meta.AddProperty<MeshRendererComponent, bool>(
                "ShadowCast",
                PropertyType::Bool,
                [](const MeshRendererComponent& c) { return c.ShadowCast; },
                [](MeshRendererComponent& c, const bool& v) { c.ShadowCast = v; }
            );

            meta.AddProperty<MeshRendererComponent, bool>(
                "GlobalIlluminate",
                PropertyType::Bool,
                [](const MeshRendererComponent& c) { return c.GlobalIlluminate; },
                [](MeshRendererComponent& c, const bool& v) { c.GlobalIlluminate = v; }
            );

            meta.AddProperty<MeshRendererComponent, u32>(
                "MeshType",
                PropertyType::U32,
                [](const MeshRendererComponent& c) { return c.MeshType; },
                [](MeshRendererComponent& c, const u32& v) { c.MeshType = v; }
            );

            meta.AddProperty<MeshRendererComponent, u32>(
                "Material",
                PropertyType::U32,
                [](const MeshRendererComponent& c) { return c.Material; },
                [](MeshRendererComponent& c, const u32& v) { c.Material = v; }
            );

            meta.AddProperty<MeshRendererComponent, u32>(
                "Texture",
                PropertyType::U32,
                [](const MeshRendererComponent& c) { return c.Texture; },
                [](MeshRendererComponent& c, const u32& v) { c.Texture = v; }
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

        //Register ReverbComponent
        {
            auto& meta = REGISTER_COMPONENT(ReverbZoneComponent);

            meta.AddProperty<ReverbZoneComponent, ReverbPreset>(
                "Preset",
                PropertyType::Int,
                [](const ReverbZoneComponent& c) { return c.Preset; },
                [](ReverbZoneComponent& c, const ReverbPreset& v) { c.Preset = v; }
            );
            meta.AddProperty<ReverbZoneComponent, float>(
                "MinDistance",
                PropertyType::Float,
                [](const ReverbZoneComponent& c) { return c.MinDistance; },
                [](ReverbZoneComponent& c, const float& v) { c.MinDistance = v; }
            );
            meta.AddProperty<ReverbZoneComponent, float>(
                "MaxDistance",
                PropertyType::Float,
                [](const ReverbZoneComponent& c) { return c.MaxDistance; },
                [](ReverbZoneComponent& c, const float& v) { c.MaxDistance = v; }
            );
            meta.AddProperty<ReverbZoneComponent, float>(
                "DecayTime",
                PropertyType::Float,
                [](const ReverbZoneComponent& c) { return c.DecayTime; },
                [](ReverbZoneComponent& c, const float& v) { c.DecayTime = v; }
            );
            meta.AddProperty<ReverbZoneComponent, float>(
                "HfDecayRatio",
                PropertyType::Float,
                [](const ReverbZoneComponent& c) { return c.HfDecayRatio; },
                [](ReverbZoneComponent& c, const float& v) { c.HfDecayRatio = v; }
            );
            meta.AddProperty<ReverbZoneComponent, float>(
                "Diffusion",
                PropertyType::Float,
                [](const ReverbZoneComponent& c) { return c.Diffusion; },
                [](ReverbZoneComponent& c, const float& v) { c.Diffusion = v; }
            );
            meta.AddProperty<ReverbZoneComponent, float>(
                "Density",
                PropertyType::Float,
                [](const ReverbZoneComponent& c) { return c.Density; },
                [](ReverbZoneComponent& c, const float& v) { c.Density = v; }
            );
            meta.AddProperty<ReverbZoneComponent, float>(
                "WetLevel",
                PropertyType::Float,
                [](const ReverbZoneComponent& c) { return c.WetLevel; },
                [](ReverbZoneComponent& c, const float& v) { c.WetLevel = v; }
            );
        }

        LOG_INFO("Component reflection registration complete");
        LOG_INFO("  - Registered 7 component types");
    }

} // namespace Engine
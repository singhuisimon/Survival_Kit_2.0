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
                [](MeshRendererComponent& c, const bool& v) { c.MeshType = v; }
            );

            meta.AddProperty<MeshRendererComponent, u32>(
                "Material",
                PropertyType::U32,
                [](const MeshRendererComponent& c) { return c.Material; },
                [](MeshRendererComponent& c, const bool& v) { c.Material = v; }
            );

            meta.AddProperty<MeshRendererComponent, u32>(
                "Texture",
                PropertyType::U32,
                [](const MeshRendererComponent& c) { return c.Texture; },
                [](MeshRendererComponent& c, const bool& v) { c.Texture = v; }
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

        // Register PrefabComponent
        {
            auto& meta = REGISTER_COMPONENT(PrefabComponent);
            // PrefabComponent properties are managed internally
            // No user-editable properties exposed in inspector by default
        }

        LOG_INFO("Component reflection registration complete");
        LOG_INFO("  - Registered 6 component types");
    }

} // namespace Engine
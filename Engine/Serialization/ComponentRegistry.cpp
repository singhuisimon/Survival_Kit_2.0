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
#include "../Component/AudioComponent.h"
#include "../Component/ListenerComponent.h"
#include "../Component/ReverbZoneComponent.h"
#include "../Component/BehaviourTreeComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/LightComponent.h"
#include "../Utility/Logger.h"
#include "../Component/ScriptComponent.h"
#include "../Component/AnimatorComponent.h"
#include "../Component/SpriteRendererComponent.h"
#include "../Component/TextComponent.h"

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

        //script 
     // Register ScriptComponent
        {
            auto& meta = REGISTER_COMPONENT(ScriptComponent);
            meta.AddProperty<ScriptComponent, std::string>(
                "ScriptClassName",
                PropertyType::String,
                [](const ScriptComponent& c) -> std::string { return c.ScriptClassName; },
                [](ScriptComponent& c, const std::string& v) { c.ScriptClassName = v; }
            );
            meta.AddProperty<ScriptComponent, bool>(
                "Started",
                PropertyType::Bool,
                [](const ScriptComponent& c) -> bool { return c.Started; },
                [](ScriptComponent& c, bool v) { c.Started = v; }
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
                "Projection",
                PropertyType::Bool,
                [](const CameraComponent& c) { return c.Projection; },
                [](CameraComponent& c, const bool& v) { c.Projection = v; }
            );
            meta.AddProperty<CameraComponent, bool>(
                "autoAspect",
                PropertyType::Bool,
                [](const CameraComponent& c) { return c.autoAspect; },
                [](CameraComponent& c, const bool& v) { c.autoAspect = v; }
            );
            meta.AddProperty<CameraComponent, glm::vec2>(
                "Size",
                PropertyType::Vec2,
                [](const CameraComponent& c) { return c.Size; },
                [](CameraComponent& c, const glm::vec2& v) { c.Size = v; }
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

            meta.AddProperty<MeshRendererComponent, u64>(
				"MeshGuid",
                PropertyType::U64,                
                [](const MeshRendererComponent& c) { return static_cast<u64>(c.MeshGuid.m_Value); },
				[](MeshRendererComponent& c, const u64& v) { c.MeshGuid = xresource::instance_guid{v}; }
            );

            meta.AddProperty<MeshRendererComponent, u64>(
				"MaterialGuid",
                PropertyType::U64,
				[](const MeshRendererComponent& c) { return static_cast<u64>(c.MaterialGuid.m_Value); },
				[](MeshRendererComponent& c, const u64& v) { c.MaterialGuid = xresource::instance_guid{ v }; }
            );

            meta.AddProperty<MeshRendererComponent, u64>(
				"TextureGuid",
				PropertyType::U64,
				[](const MeshRendererComponent& c) { return static_cast<u64>(c.TextureGuid.m_Value); },
				[](MeshRendererComponent& c, const u64& v) { c.TextureGuid = xresource::instance_guid{ v }; }
            );

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

            //meta.AddProperty<MeshRendererComponent, bool>(
            //    "ShadowCast",
            //    PropertyType::Bool,
            //    [](const MeshRendererComponent& c) { return c.ShadowCast; },
            //    [](MeshRendererComponent& c, const bool& v) { c.ShadowCast = v; }
            //);

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

            meta.AddProperty<MeshRendererComponent, u32>(
                "SubmeshIndex",
                PropertyType::U32,
                [](const MeshRendererComponent& c) { return c.SubmeshIndex; },
				[](MeshRendererComponent& c, const u32& v) { c.SubmeshIndex = v; }
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
            meta.AddProperty<RigidbodyComponent, glm::vec3>(
                "AngularVelocity",
                PropertyType::Vec3,
                [](const RigidbodyComponent& c) { return c.AngularVelocity; },
                [](RigidbodyComponent& c, const glm::vec3& v) { c.AngularVelocity = v; }
            );
            meta.AddProperty<RigidbodyComponent, float>(
                "LinearDamping",
                PropertyType::Float,
                [](const RigidbodyComponent& c) { return c.LinearDamping; },
                [](RigidbodyComponent& c, const float& v) { c.LinearDamping = v; }
            );
            meta.AddProperty<RigidbodyComponent, float>(
                "AngularDamping",
                PropertyType::Float,
                [](const RigidbodyComponent& c) { return c.AngularDamping; },
                [](RigidbodyComponent& c, const float& v) { c.AngularDamping = v; }
            );
            meta.AddProperty<RigidbodyComponent, float>(
                "AngularDamping",
                PropertyType::Float,
                [](const RigidbodyComponent& c) { return c.Restitution; },
                [](RigidbodyComponent& c, const float& v) { c.Restitution = v; }
            );
            meta.AddProperty<RigidbodyComponent, ColliderType>(
                "CollideType",
                PropertyType::Int,
                [](const RigidbodyComponent& c) { return c.Shape; },
                [](RigidbodyComponent& c, const ColliderType& v) { c.Shape = v; }
            );
            meta.AddProperty<RigidbodyComponent, glm::vec3>(
                "BoxHalfExtents",
                PropertyType::Vec3,
                [](const RigidbodyComponent& c) { return c.BoxHalfExtents; },
                [](RigidbodyComponent& c, const glm::vec3& v) { c.BoxHalfExtents = v; }
            );
            meta.AddProperty<RigidbodyComponent, float>(
                "SphereRadius",
                PropertyType::Float,
                [](const RigidbodyComponent& c) { return c.SphereRadius; },
                [](RigidbodyComponent& c, const float& v) { c.SphereRadius = v; }
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
            meta.AddProperty<AudioComponent, AudioRolloffMode>(
                "RolloffMode",
                PropertyType::Int,  // Enum stored as int
                [](const AudioComponent& c) { return c.RolloffMode; },
                [](AudioComponent& c, const AudioRolloffMode& v) { c.RolloffMode = v; }
            );
            meta.AddProperty<AudioComponent, float>(
                "DopplerLevel",
                PropertyType::Float,
                [](const AudioComponent& c) { return c.DopplerLevel; },
                [](AudioComponent& c, const float& v) { c.DopplerLevel = v; }
            );
            meta.AddProperty<AudioComponent, float>(
                "Pan2D",
                PropertyType::Float,
                [](const AudioComponent& c) { return c.Pan2D; },
                [](AudioComponent& c, const float& v) { c.Pan2D = v; }
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
            (void)meta;
            // PrefabComponent properties are managed internally
            // No user-editable properties exposed in inspector by default
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

        {
            // Register BehaviourTreeComponent
            auto& meta = REGISTER_COMPONENT(BehaviourTreeComponent);

            // Active property
            meta.AddProperty<BehaviourTreeComponent, bool>(
                "Active",
                PropertyType::Bool,
                [](const BehaviourTreeComponent& c) { return c.Active; },
                [](BehaviourTreeComponent& c, const bool& v) { c.Active = v; }
            );

            // ResetOnComplete property
            meta.AddProperty<BehaviourTreeComponent, bool>(
                "ResetOnComplete",
                PropertyType::Bool,
                [](const BehaviourTreeComponent& c) { return c.ResetOnComplete; },
                [](BehaviourTreeComponent& c, const bool& v) { c.ResetOnComplete = v; }
            );

            meta.AddProperty<BehaviourTreeComponent, std::string>(
                "TreeAssetPath",
                PropertyType::String,
                [](const BehaviourTreeComponent& c) {
                    return c.TreeAssetPath;
                },
                [](BehaviourTreeComponent& c, const std::string& v) {
                    c.TreeAssetPath = v;
                }
            );
        }

        {
	        auto& meta = REGISTER_COMPONENT(ParticleComponent);

            meta.AddProperty<ParticleComponent, glm::vec3>(
                "InitialVelocity",
                PropertyType::Vec3,
                [](const ParticleComponent& c) { return c.InitialVelocity; },
                [](ParticleComponent& c, const glm::vec3& v) { c.InitialVelocity = v; }
            );

            meta.AddProperty<ParticleComponent, glm::vec4>(
            "ColorMin",
                PropertyType::Vec4,
                [](const ParticleComponent& c) { return c.ColorMin; },
                [](ParticleComponent& c, const glm::vec4& v) { c.ColorMin = v; }
                );

            meta.AddProperty<ParticleComponent, glm::vec4>(
                "ColorMax",
                PropertyType::Vec4,
                [](const ParticleComponent& c) { return c.ColorMax; },
                [](ParticleComponent& c, const glm::vec4& v) { c.ColorMax = v; }
            );

            meta.AddProperty<ParticleComponent, u32>(
                "MaxParticles",
                PropertyType::U32,
                [](const ParticleComponent& c) { return c.MaxParticles; },
                [](ParticleComponent& c, const u32& v) { c.MaxParticles = v; }
            );

            meta.AddProperty<ParticleComponent, u32>(
                "ParticleType",
                PropertyType::U32,
                [](const ParticleComponent& c) { return c.ParticleType; },
                [](ParticleComponent& c, const u32& v) { c.ParticleType = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "EmissionRate",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.EmissionRate; },
                [](ParticleComponent& c, const float& v) { c.EmissionRate = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "ParticleLifetime",
                PropertyType::Float,
				[](const ParticleComponent& c) { return c.ParticleLifetime; },
				[](ParticleComponent& c, const float& v) { c.ParticleLifetime = v; }
                );

            meta.AddProperty<ParticleComponent, float>(
                "EmissionAccumulator",
				PropertyType::Float,
                [](const ParticleComponent& c) { return c.EmissionAccumulator; },
				[](ParticleComponent& c, const float& v) { c.EmissionAccumulator = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "ParticleSize",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.ParticleSize; },
                [](ParticleComponent& c, const float& v) { c.ParticleSize = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "VelocityRandomness",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.VelocityRandomness; },
                [](ParticleComponent& c, const float& v) { c.VelocityRandomness = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "LifetimeRandomness",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.LifetimeRandomness; },
                [](ParticleComponent& c, const float& v) { c.LifetimeRandomness = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "SpreadAngle",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.SpreadAngle; },
                [](ParticleComponent& c, const float& v) { c.SpreadAngle = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "MinSpeed",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.MinSpeed; },
                [](ParticleComponent& c, const float& v) { c.MinSpeed = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "MaxSpeed",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.MaxSpeed; },
                [](ParticleComponent& c, const float& v) { c.MaxSpeed = v; }
            );

            meta.AddProperty<ParticleComponent, float>(
                "RotationSpeed",
                PropertyType::Float,
                [](const ParticleComponent& c) { return c.RotationSpeed; },
                [](ParticleComponent& c, const float& v) { c.RotationSpeed = v; }
            );

            meta.AddProperty<ParticleComponent, bool>(
                "RandomizeRotation",
                PropertyType::Bool,
                [](const ParticleComponent& c) { return c.RandomizeRotation; },
                [](ParticleComponent& c, const bool& v) { c.RandomizeRotation = v; }
            );

            meta.AddProperty<ParticleComponent, bool>(
                "Loop",
                PropertyType::Bool,
                [](const ParticleComponent& c) { return c.Loop; },
				[](ParticleComponent& c, const bool& v) { c.Loop = v; }
                );

            meta.AddProperty<ParticleComponent, bool>(
                "Active",
                PropertyType::Bool,
                [](const ParticleComponent& c) { return c.Active; },
                [](ParticleComponent& c, const bool& v) { c.Active = v; }
            );
        }

        // Register LightComponent
        {
            auto& meta = REGISTER_COMPONENT(LightComponent);

            meta.AddProperty<LightComponent, bool>(
                "Enabled",
                PropertyType::Bool,
                [](const LightComponent& c) { return c.Enabled; },
                [](LightComponent& c, const bool& v) { c.Enabled = v; }
            );
            // Enums as ints for editor/serialization
            meta.AddProperty<LightComponent, LightType>(
                "Type",
                PropertyType::Int,
                [](const LightComponent& c) { return c.Type; },
                [](LightComponent& c, const LightType& v) { c.Type = v; }
            );
            meta.AddProperty<LightComponent, LightMode>(
                "Mode",
                PropertyType::Int,
                [](const LightComponent& c) { return c.Mode; },
                [](LightComponent& c, const LightMode& v) { c.Mode = v; }
            );
            meta.AddProperty<LightComponent, glm::vec3>(
                "Color",
                PropertyType::Vec3,
                [](const LightComponent& c) { return c.Color; },
                [](LightComponent& c, const glm::vec3& v) { c.Color = v; }
            );
            meta.AddProperty<LightComponent, float>(
                "Intensity",
                PropertyType::Float,
                [](const LightComponent& c) { return c.Intensity; },
                [](LightComponent& c, const float& v) { c.Intensity = v; }
            );
            meta.AddProperty<LightComponent, float>(
                "Range",
                PropertyType::Float,
                [](const LightComponent& c) { return c.Range; },
                [](LightComponent& c, const float& v) { c.Range = v; }
            );
            meta.AddProperty<LightComponent, float>(
                "SpotAngleDeg",
                PropertyType::Float,
                [](const LightComponent& c) { return c.SpotAngleDeg; },
                [](LightComponent& c, const float& v) { c.SpotAngleDeg = v; }
            );
            meta.AddProperty<LightComponent, float>(
                "IndirectMultiplier",
                PropertyType::Float,
                [](const LightComponent& c) { return c.IndirectMultiplier; },
                [](LightComponent& c, const float& v) { c.IndirectMultiplier = v; }
            );
        }

        // Register AnimatorComponent
        {
            auto& meta = REGISTER_COMPONENT(AnimatorComponent);

            meta.AddProperty<AnimatorComponent, bool>(
                "playing",
                PropertyType::Bool,
                [](const AnimatorComponent& c) { return c.playing; },
                [](AnimatorComponent& c, const bool& v) { c.playing = v; }
            );
            // Enums as ints for editor/serialization
            meta.AddProperty<AnimatorComponent, bool>(
                "respectClipLoop",
                PropertyType::Bool,
                [](const AnimatorComponent& c) { return c.respectClipLoop; },
                [](AnimatorComponent& c, const bool& v) { c.respectClipLoop = v; }
            );
            meta.AddProperty<AnimatorComponent, u32>(
                "controller",
                PropertyType::U32,
                [](const AnimatorComponent& c) { return c.controller; },
                [](AnimatorComponent& c, const u32& v) { c.controller = v; }
            );
            meta.AddProperty<AnimatorComponent, u32>(
                "currentClipIndex",
                PropertyType::U32,
                [](const AnimatorComponent& c) { return c.currentClipIndex; },
                [](AnimatorComponent& c, const u32& v) { c.currentClipIndex = v; }
            );
            meta.AddProperty<AnimatorComponent, float>(
                "currentTime",
                PropertyType::Float,
                [](const AnimatorComponent& c) { return c.currentTime; },
                [](AnimatorComponent& c, const float& v) { c.currentTime = v; }
            );
            meta.AddProperty<AnimatorComponent, float>(
                "playbackSpeed",
                PropertyType::Float,
                [](const AnimatorComponent& c) { return c.playbackSpeed; },
                [](AnimatorComponent& c, const float& v) { c.playbackSpeed = v; }
            );
        }

        // Register SpriteRenderer Component
        {
            auto& meta = REGISTER_COMPONENT(SpriteRendererComponent);

            meta.AddProperty<SpriteRendererComponent, u64>(
                "TextureGuid",
                PropertyType::U64, 
                [](const SpriteRendererComponent& c) {return static_cast<u64>(c.TextureGuid.m_Value); },
                [](SpriteRendererComponent& c, const xresource::instance_guid& v) { c.TextureGuid = v; });

			meta.AddProperty<SpriteRendererComponent, glm::vec4>(
                "Color", 
                PropertyType::Vec4,
                [](const SpriteRendererComponent& c) { return c.Color; },
                [](SpriteRendererComponent& c, const glm::vec4& v) { c.Color = v; });

            meta.AddProperty<SpriteRendererComponent, u32>(
                "Quad",
                PropertyType::U32,
                [](const SpriteRendererComponent& c) { return c.Quad; },
                [](SpriteRendererComponent& c, const u32& v) { c.Quad = v; });

            meta.AddProperty<SpriteRendererComponent, u32>(
                "SpriteLayer",
                PropertyType::U32,
                [](const SpriteRendererComponent& c) { return c.SpriteLayer; },
				[](SpriteRendererComponent& c, const u32& v) { c.SpriteLayer = v; });

            meta.AddProperty<SpriteRendererComponent, bool>(
                "IsActive", 
                PropertyType::Bool,
                [](const SpriteRendererComponent& c) {return c.IsActive; },
                [](SpriteRendererComponent& c, const bool& v) {c.IsActive = v; });

            meta.AddProperty<SpriteRendererComponent, bool>(
                "IsVisible",
                PropertyType::Bool,
                [](const SpriteRendererComponent& c) {return c.IsVisible; },
                [](SpriteRendererComponent& c, const bool& v) {c.IsVisible = v; });
        }

        // Register Text Component
        {
            auto& meta = REGISTER_COMPONENT(TextComponent);

            meta.AddProperty<TextComponent, std::string>(
                "Text",
                PropertyType::String,
                [](const TextComponent& c) { return c.text; },
                [](TextComponent& c, const std::string& v) { c.setText(v); }
            );

            meta.AddProperty<TextComponent, float>(
                "FontSize",
                PropertyType::Float,
                [](const TextComponent& c) { return c.fontSize;  },
                [](TextComponent& c, const float& v) { return c.setFontSize(v);  }
            );

            meta.AddProperty<TextComponent, glm::vec4>(
                "Color",
                PropertyType::Vec4,
                [](const TextComponent& c) {
                    return glm::vec4(c.color[0], c.color[1], c.color[2], c.color[3]);
                },
                [](TextComponent& c, const glm::vec4& v) {
                    c.setColor(v);
                }
            );

            meta.AddProperty<TextComponent, u32>(
                "Alignment",
                PropertyType::U32,
                [](const TextComponent& c) { return static_cast<u32>(c.align); },
                [](TextComponent& c, const u32& v) {
                    c.setAlignment(static_cast<TextAlignment>(v));
                }
            );

            meta.AddProperty<TextComponent, float>(
                "LineSpacing",
                PropertyType::Float,
                [](const TextComponent& c) { return c.lineSpacing; },
                [](TextComponent& c, const float& v) { c.lineSpacing = v; }
            );

            meta.AddProperty<TextComponent, float>(
                "LetterSpacing",
                PropertyType::Float,
                [](const TextComponent& c) { return c.letterSpacing; },
                [](TextComponent& c, const float& v) { c.setLetterSpacing(v); }
            );

            meta.AddProperty<TextComponent, float>(
                "MaxWidth",
                PropertyType::Float,
                [](const TextComponent& c) { return c.maxWidth; },
                [](TextComponent& c, const float& v) { c.maxWidth = v; }
            );
        }

        LOG_INFO("Component reflection registration complete");
        LOG_INFO("  - Registered 11 component types");
    }

} // namespace Engine
/**
 * @file PrefabInstantiator.cpp
 * @brief Implementation of PrefabInstantiator
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "PrefabInstantiator.h"
#include "../Prefab/PrefabRegistry.h"
#include "../Component/PrefabComponent.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/CameraComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/RigidbodyComponent.h"
#include "../Component/AudioComponent.h"
#include "../Component/ListenerComponent.h"
#include "../Component/ReverbZoneComponent.h"
#include "../Component/BehaviourTreeComponent.h"
#include "../Component/ScriptComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/LightComponent.h"
#include "../Utility/Logger.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace Engine {

    Entity PrefabInstantiator::InstantiateEntityPrefab(
        Scene* scene,
        xresource::instance_guid prefabGUID,
        entt::entity entityId) {

        if (!scene) {
            LOG_ERROR("PrefabInstantiator: Cannot instantiate into null scene");
            return Entity();
        }

        // Get prefab from registry
        auto prefab = PrefabRegistry::Get().GetPrefab(prefabGUID);
        if (!prefab) {
            LOG_ERROR("PrefabInstantiator: Prefab not found in registry (GUID: 0x",
                std::hex, prefabGUID.m_Value, std::dec, ")");
            return Entity();
        }

        if (prefab->GetType() != PrefabType::Entity) {
            LOG_ERROR("PrefabInstantiator: Prefab is not an entity prefab");
            return Entity();
        }

        // Deserialize entity from prefab data
        Entity entity = DeserializeEntity(scene, prefab->GetEntityData(), entityId);

        if (!entity) {
            LOG_ERROR("PrefabInstantiator: Failed to deserialize entity from prefab");
            return Entity();
        }

        // Add PrefabComponent to mark this as a prefab instance
        entity.AddComponent<PrefabComponent>(prefabGUID);

        LOG_INFO("PrefabInstantiator: Instantiated entity prefab '", prefab->GetName(),
            "' (Entity ID: ", static_cast<uint32_t>(entity), ")");

        return entity;
    }

    Entity PrefabInstantiator::InstantiateScenePrefab(
        Scene* scene,
        xresource::instance_guid prefabGUID) {

        if (!scene) {
            LOG_ERROR("PrefabInstantiator: Cannot instantiate into null scene");
            return Entity();
        }

        // Get prefab from registry
        auto prefab = PrefabRegistry::Get().GetPrefab(prefabGUID);
        if (!prefab) {
            LOG_ERROR("PrefabInstantiator: Prefab not found in registry (GUID: 0x",
                std::hex, prefabGUID.m_Value, std::dec, ")");
            return Entity();
        }

        if (prefab->GetType() != PrefabType::Scene) {
            LOG_ERROR("PrefabInstantiator: Prefab is not a scene prefab");
            return Entity();
        }

        // Parse scene data
        ::rapidjson::Document doc;
        doc.Parse(prefab->GetSceneData().c_str());

        if (doc.HasParseError() || !doc.HasMember("Entities")) {
            LOG_ERROR("PrefabInstantiator: Invalid scene prefab data");
            return Entity();
        }

        const ::rapidjson::Value& entitiesArray = doc["Entities"];
        Entity rootEntity;

        // Instantiate each entity
        for (::rapidjson::SizeType i = 0; i < entitiesArray.Size(); i++) {
            const ::rapidjson::Value& entityObj = entitiesArray[i];

            // Convert entity object to JSON string
            ::rapidjson::StringBuffer buffer;
            ::rapidjson::Writer<::rapidjson::StringBuffer> writer(buffer);
            entityObj.Accept(writer);
            std::string entityJson = buffer.GetString();

            // Deserialize entity
            Entity entity = DeserializeEntity(scene, entityJson);
            if (entity) {
                // Add PrefabComponent
                entity.AddComponent<PrefabComponent>(prefabGUID);

                // Track root entity (first entity)
                if (i == 0) {
                    rootEntity = entity;
                }
            }
        }

        LOG_INFO("PrefabInstantiator: Instantiated scene prefab '", prefab->GetName(),
            "' (Root Entity ID: ", static_cast<uint32_t>(rootEntity), ")");

        return rootEntity;
    }

    void PrefabInstantiator::ApplyOverrides(Entity entity, Scene* scene) {
        if (!entity.HasComponent<PrefabComponent>()) {
            LOG_WARNING("PrefabInstantiator: Entity does not have PrefabComponent");
            return;
        }

        const auto& prefabComp = entity.GetComponent<PrefabComponent>();

        // Apply property overrides
        for (const auto& override : prefabComp.OverriddenProperties) {
            // Find component by GUID and apply override
            // This is a simplified version - full implementation would need
            // to iterate through all components and match by GUID
            LOG_DEBUG("PrefabInstantiator: Applying override - Property: ",
                override.PropertyPath, ", Value: ", override.Value);
        }

        // Handle added components
        // (Components not in original prefab - already added during instantiation)

        // Handle deleted components
        for (const auto& deletedGUID : prefabComp.DeletedComponents) {
            // Remove component with matching GUID
            LOG_DEBUG("PrefabInstantiator: Removing deleted component (GUID: 0x",
                std::hex, deletedGUID.m_Value, std::dec, ")");
        }
    }

    Entity PrefabInstantiator::DeserializeEntity(
        Scene* scene,
        const std::string& entityJson,
        entt::entity entityId) {

        ::rapidjson::Document doc;
        doc.Parse(entityJson.c_str());

        if (doc.HasParseError()) {
            LOG_ERROR("PrefabInstantiator: JSON parse error");
            return Entity();
        }

        // Create entity with specific ID or auto-generate
        Entity entity;
        if (entityId != entt::null) {
            // Create entity with specific ID
            auto& registry = scene->GetRegistry();
            entity = Entity(registry.create(entityId), &registry);
        }
        else {
            // Auto-generate entity ID
            entity = scene->CreateEntity();
        }

        // Deserialize components
        if (doc.HasMember("Components") && doc["Components"].IsArray()) {
            const ::rapidjson::Value& componentsArray = doc["Components"];

            for (::rapidjson::SizeType i = 0; i < componentsArray.Size(); i++) {
                const ::rapidjson::Value& componentObj = componentsArray[i];

                if (!componentObj.HasMember("Type") || !componentObj.HasMember("Properties")) {
                    continue;
                }

                std::string componentType = componentObj["Type"].GetString();
                const ::rapidjson::Value& properties = componentObj["Properties"];

                AddComponentFromJson(entity, componentType, properties);
            }
        }

        return entity;
    }

    template<typename ValueType>
    void PrefabInstantiator::AddComponentFromJson(
        Entity entity,
        const std::string& componentType,
        const ValueType& properties) {

        if (componentType == "TagComponent") {
            auto& comp = entity.AddComponent<TagComponent>();

            if (properties.HasMember("ComponentGUID")) {
                uint64_t guidValue = std::stoull(properties["ComponentGUID"].GetString());
                comp.ComponentGUID = xresource::instance_guid{ guidValue };
            }
            if (properties.HasMember("Tag")) {
                comp.Tag = properties["Tag"].GetString();
            }
        }
        else if (componentType == "TransformComponent") {
            auto& comp = entity.AddComponent<TransformComponent>();

            if (properties.HasMember("Position") && properties["Position"].IsArray() && properties["Position"].Size() >= 3) {
                const auto& pos = properties["Position"];
                comp.Position = glm::vec3(pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat());
            }

            if (properties.HasMember("Rotation") && properties["Rotation"].IsArray() && properties["Rotation"].Size() >= 4) {
                const auto& rot = properties["Rotation"];
                comp.Rotation = glm::quat(rot[3].GetFloat(), rot[0].GetFloat(), rot[1].GetFloat(), rot[2].GetFloat());
            }
            if (properties.HasMember("Scale") && properties["Scale"].IsArray()) {
                const auto& scale = properties["Scale"];
                comp.Scale = glm::vec3(scale[0].GetFloat(), scale[1].GetFloat(), scale[2].GetFloat());
            }
            if (properties.HasMember("Parent")) {
                comp.Parent = properties["Parent"].GetUint();
            }
            if (properties.HasMember("Children") && properties["Children"].IsArray()) {
                comp.Children.clear();
                const auto& childrenArray = properties["Children"];

                for (rapidjson::SizeType i = 0; i < childrenArray.Size(); ++i)
                    comp.Children.push_back(childrenArray[i].GetUint());
            }
        }
        else if (componentType == "CameraComponent") {
            auto& comp = entity.AddComponent<CameraComponent>();

            if (properties.HasMember("ComponentGUID")) {
                uint64_t guidValue = std::stoull(properties["ComponentGUID"].GetString());
                comp.ComponentGUID = xresource::instance_guid{ guidValue };
            }
            if (properties.HasMember("FOV")) {
                comp.FOV = properties["FOV"].GetFloat();
            }
            if (properties.HasMember("NearPlane")) {
                comp.NearPlane = properties["NearPlane"].GetFloat();
            }
            if (properties.HasMember("FarPlane")) {
                comp.FarPlane = properties["FarPlane"].GetFloat();
            }
            if (properties.HasMember("Primary")) {
                comp.Primary = properties["Primary"].GetBool();
            }
        }
        else if (componentType == "MeshRendererComponent") {
            auto& comp = entity.AddComponent<MeshRendererComponent>();

            if (properties.HasMember("MeshGuid")) {
                uint64_t guidValue = properties["MeshGuid"].GetUint64();
                comp.MeshGuid = xresource::instance_guid{ guidValue };
            }
            if (properties.HasMember("MaterialGuid")) {
                uint64_t guidValue = properties["MaterialGuid"].GetUint64();
                comp.MaterialGuid = xresource::instance_guid{ guidValue };
            }
            if (properties.HasMember("TextureGuid")) {
                uint64_t guidValue = properties["TextureGuid"].GetUint64();
                comp.TextureGuid = xresource::instance_guid{ guidValue };
            }
            if (properties.HasMember("Visible")) {
                comp.Visible = properties["Visible"].GetBool();
            }
            if (properties.HasMember("MeshType")) {
                comp.MeshType = properties["MeshType"].GetUint();
            }
            if (properties.HasMember("Material")) {
                comp.Material = properties["Material"].GetUint();
            }
            if (properties.HasMember("Texture")) {
                comp.Texture = properties["Texture"].GetUint();
            }
        }
        else if (componentType == "RigidbodyComponent") {
            auto& comp = entity.AddComponent<RigidbodyComponent>();

            if (properties.HasMember("ComponentGUID")) {
                uint64_t guidValue = std::stoull(properties["ComponentGUID"].GetString());
                comp.ComponentGUID = xresource::instance_guid{ guidValue };
            }
            if (properties.HasMember("Mass")) {
                comp.Mass = properties["Mass"].GetFloat();
            }
            if (properties.HasMember("IsKinematic")) {
                comp.IsKinematic = properties["IsKinematic"].GetBool();
            }
            if (properties.HasMember("UseGravity")) {
                comp.UseGravity = properties["UseGravity"].GetBool();
            }
            if (properties.HasMember("Velocity") && properties["Velocity"].IsArray()) {
                const auto& vel = properties["Velocity"];
                comp.Velocity = glm::vec3(vel[0].GetFloat(), vel[1].GetFloat(), vel[2].GetFloat());
            }
        }
        else if (componentType == "AudioComponent") {
            auto& comp = entity.AddComponent<AudioComponent>();

            if (properties.HasMember("AudioFilePath")) {
                comp.AudioFilePath = properties["AudioFilePath"].GetString();
            }
            if (properties.HasMember("Type")) {
                comp.Type = static_cast<AudioType>(properties["Type"].GetInt());
            }
            if (properties.HasMember("State")) {
                comp.State = static_cast<PlayState>(properties["State"].GetInt());
            }
            if (properties.HasMember("Volume")) {
                comp.Volume = properties["Volume"].GetFloat();
            }
            if (properties.HasMember("Pitch")) {
                comp.Pitch = properties["Pitch"].GetFloat();
            }
            if (properties.HasMember("Loop")) {
                comp.Loop = properties["Loop"].GetBool();
            }
            if (properties.HasMember("Mute")) {
                comp.Mute = properties["Mute"].GetBool();
            }
            if (properties.HasMember("Is3D")) {
                comp.Is3D = properties["Is3D"].GetBool();
            }
            if (properties.HasMember("MinDistance")) {
                comp.MinDistance = properties["MinDistance"].GetFloat();
            }
            if (properties.HasMember("MaxDistance")) {
                comp.MaxDistance = properties["MaxDistance"].GetFloat();
            }
            if (properties.HasMember("ReverbProperties")) {
                comp.ReverbProperties = properties["ReverbProperties"].GetFloat();
            }

            // Runtime fields are NOT deserialized (Channel, IsDirty, PreviousPath)
            // They will be initialized to their default values
        }
        else if (componentType == "ListenerComponent") {
            auto& comp = entity.AddComponent<ListenerComponent>();

            if (properties.HasMember("Active")) {
                comp.Active = properties["Active"].GetBool();
            }
        }
        else if (componentType == "ReverbComponent") {
            auto& comp = entity.AddComponent<ReverbZoneComponent>();

            if (properties.HasMember("Preset")) {
                comp.Preset = static_cast<ReverbPreset>(properties["Preset"].GetInt());
            }
            if (properties.HasMember("MinDistance")) {
                comp.MinDistance = properties["MinDistance"].GetFloat();
            }
            if (properties.HasMember("MaxDistance")) {
                comp.MaxDistance = properties["MaxDistance"].GetFloat();
            }
            if (properties.HasMember("DecayTime")) {
                comp.DecayTime = properties["DecayTime"].GetFloat();
            }
            if (properties.HasMember("HfDecayRatio")) {
                comp.HfDecayRatio = properties["HfDecayRatio"].GetFloat();
            }
            if (properties.HasMember("Diffusion")) {
                comp.Diffusion = properties["Diffusion"].GetFloat();
            }
            if (properties.HasMember("Density")) {
                comp.Density = properties["Density"].GetFloat();
            }
            if (properties.HasMember("WetLevel")) {
                comp.WetLevel = properties["WetLevel"].GetFloat();
            }

            // Runtime fields are NOT deserialized (ReverbZone, IsDirty)
            // They will be initialized to their default values
        }

        else if (componentType == "BehaviourTreeComponent") {
            auto& comp = entity.AddComponent<BehaviourTreeComponent>();

            if (properties.HasMember("Active"))
                comp.Active = properties["Active"].GetBool();
            if (properties.HasMember("ResetOnComplete"))
                comp.ResetOnComplete = properties["ResetOnComplete"].GetBool();
            if (properties.HasMember("TreeAssetPath"))
                comp.TreeAssetPath = properties["TreeAssetPath"].GetString();

            LOG_INFO("[PrefabInstantiator] Added BehaviourTreeComponent from prefab with path: ", comp.TreeAssetPath);


            comp.TreeInstance = nullptr; // runtime-only
        }

        else if (componentType == "ParticleComponent") {
            auto& emitter = entity.AddComponent<ParticleComponent>();

            // Initial Velocity
            if (properties.HasMember("Initial Velocity") && properties["Initial Velocity"].IsArray()) {
                const auto& velArray = properties["Initial Velocity"].GetArray();
                if (velArray.Size() >= 3) {
                    emitter.InitialVelocity.x = velArray[0].GetFloat();
                    emitter.InitialVelocity.y = velArray[1].GetFloat();
                    emitter.InitialVelocity.z = velArray[2].GetFloat();
                }
            }

            // Color Min
            if (properties.HasMember("Color Min") && properties["Color Min"].IsArray()) {
                const auto& minColorArray = properties["Color Min"].GetArray();
                if (minColorArray.Size() >= 3) {
                    emitter.ColorMin.x = minColorArray[0].GetFloat();
                    emitter.ColorMin.y = minColorArray[1].GetFloat();
                    emitter.ColorMin.z = minColorArray[2].GetFloat();
                }
            }

            // Color Max
            if (properties.HasMember("Color Max") && properties["Color Max"].IsArray()) {
                const auto& maxColorArray = properties["Color Max"].GetArray();
                if (maxColorArray.Size() >= 3) {
                    emitter.ColorMax.x = maxColorArray[0].GetFloat();
                    emitter.ColorMax.y = maxColorArray[1].GetFloat();
                    emitter.ColorMax.z = maxColorArray[2].GetFloat();
                }
            }

            // Numeric properties
            if (properties.HasMember("Max Particles"))
                emitter.MaxParticles = properties["Max Particles"].GetUint();
            if (properties.HasMember("Particle Type"))
                emitter.ParticleType = properties["Particle Type"].GetUint();
            if (properties.HasMember("Emission Rate"))
                emitter.EmissionRate = properties["Emission Rate"].GetFloat();
            if (properties.HasMember("Particle Lifetime"))
                emitter.ParticleLifetime = properties["Particle Lifetime"].GetFloat();
            if (properties.HasMember("Emission Accumulator"))
                emitter.EmissionAccumulator = properties["Emission Accumulator"].GetFloat();
            if (properties.HasMember("Particle Size"))
                emitter.ParticleSize = properties["Particle Size"].GetFloat();

            // Randomization parameters
            if (properties.HasMember("Velocity Randomness"))
                emitter.VelocityRandomness = properties["Velocity Randomness"].GetFloat();
            if (properties.HasMember("Lifetime Randomness"))
                emitter.LifetimeRandomness = properties["Lifetime Randomness"].GetFloat();
            if (properties.HasMember("Spread Angle"))
                emitter.SpreadAngle = properties["Spread Angle"].GetFloat();
            if (properties.HasMember("Min Speed"))
                emitter.MinSpeed = properties["Min Speed"].GetFloat();
            if (properties.HasMember("Max Speed"))
                emitter.MaxSpeed = properties["Max Speed"].GetFloat();
            if (properties.HasMember("Rotation Speed"))
                emitter.RotationSpeed = properties["Rotation Speed"].GetFloat();

            // Boolean parameters
            if (properties.HasMember("Randomize Rotation"))
                emitter.RandomizeRotation = properties["Randomize Rotation"].GetBool();
            if (properties.HasMember("Loop"))
                emitter.Loop = properties["Loop"].GetBool();
            if (properties.HasMember("Active"))
                emitter.Active = properties["Active"].GetBool();

            emitter.Particles.clear();
            emitter.EmissionAccumulator = 0.0f;
        }

        else if (componentType == "ScriptComponent") {
            auto& comp = entity.AddComponent<ScriptComponent>();

            if (properties.HasMember("ScriptClassName")) {
                comp.ScriptClassName = properties["ScriptClassName"].GetString();
            }

            // ScriptInstance and Started will be initialized by ScriptSystem at runtime
            LOG_DEBUG("PrefabInstantiator: Added ScriptComponent with class '",
                comp.ScriptClassName, "'");
        }

        else if (componentType == "LightComponent") {
            auto& comp = entity.AddComponent<LightComponent>();

            if (properties.HasMember("Enabled")) {
                comp.Enabled = properties["Enabled"].GetBool();
            }
            if (properties.HasMember("Type")) {
                comp.Type = static_cast<LightType>(properties["Type"].GetUint());
            }
            //if (properties.HasMember("Mode")) {
            //    comp.Mode = static_cast<LightMode>(properties["Mode"].GetUint());
            //}
            if (properties.HasMember("Color") && properties["Color"].IsArray()) {
                const auto& col = properties["Color"];
                comp.Color = glm::vec3(col[0].GetFloat(), col[1].GetFloat(), col[2].GetFloat());
            }
            if (properties.HasMember("Intensity")) {
                comp.Intensity = properties["Intensity"].GetFloat();
            }
            if (properties.HasMember("Range")) {
                comp.Range = properties["Range"].GetFloat();
            }
            if (properties.HasMember("SpotAngleDeg")) {
                comp.SpotAngleDeg = properties["SpotAngleDeg"].GetFloat();
            }
            if (properties.HasMember("IndirectMultiplier")) {
                comp.IndirectMultiplier = properties["IndirectMultiplier"].GetFloat();
            }
        }
    }



    // Explicit template instantiation for rapidjson::Value
    template void PrefabInstantiator::AddComponentFromJson<::rapidjson::Value>(
        Entity entity,
        const std::string& componentType,
        const ::rapidjson::Value& properties
    );


} // namespace Engine
/**
 * @file PrefabSerializer.cpp
 * @brief Implementation of PrefabSerializer
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "PrefabSerializer.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/CameraComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/RigidbodyComponent.h"
#include "../Component/AudioComponent.h"
#include "../Component/ListenerComponent.h"
#include "../Component/ReverbZoneComponent.h"
#include "../Component/BehaviourTreeComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/ScriptComponent.h"
#include "../Component/LightComponent.h"
#include "../Component/AnimatorComponent.h"
#include "../Component/SpriteRendererComponent.h"
#include "../Utility/Logger.h"
#include "../Asset/AssetManager.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <filesystem> 

namespace Engine {

    std::shared_ptr<Prefab> PrefabSerializer::CreateEntityPrefab(Entity entity, const std::string& name) {

        if (!entity) {
            LOG_ERROR("PrefabSerializer: Cannot create prefab from invalid entity");
            return nullptr;
        }

#if 1 // to check if entity has children, if got, create a scene prefab instead and also show the sub entities in prefab file
        if (entity.HasComponent<TransformComponent>()) {
            const auto& transform = entity.GetComponent<TransformComponent>();
            if (!transform.Children.empty()) {
                LOG_INFO("PrefabSerializer: Entity has children, creating Scene prefab instead");
                return CreateEntityWithChildrenPrefab(entity, name);
            }
        }

#endif
        auto prefab = std::make_shared<Prefab>(PrefabType::Entity);
        prefab->SetName(name);

     
        //entt::registry* registry = nullptr;

        std::string entityData = SerializeEntity(entity, entity);
        prefab->SetEntityData(entityData);

        LOG_INFO("PrefabSerializer: Created entity prefab '", name, "'");
        return prefab;
    }

    std::shared_ptr<Prefab> PrefabSerializer::CreateScenePrefab(
        Scene* scene,
        const std::vector<Entity>& entities,
        const std::string& name) {

        if (!scene) {
            LOG_ERROR("PrefabSerializer: Cannot create scene prefab from null scene");
            return nullptr;
        }

        if (entities.empty()) {
            LOG_ERROR("PrefabSerializer: Cannot create scene prefab with no entities");
            return nullptr;
        }

        auto prefab = std::make_shared<Prefab>(PrefabType::Scene);
        prefab->SetName(name);

        // Serialize all entities
        std::string sceneData = SerializeEntities(entities, scene->GetRegistry());
        prefab->SetSceneData(sceneData);

        // Set root entity (first entity in the list)
        if (entities[0]) {
            uint32_t entityID = static_cast<uint32_t>(entities[0]);
            prefab->SetRootEntityGUID(xresource::instance_guid{ static_cast<uint64_t>(entityID) });
        }

        LOG_INFO("PrefabSerializer: Created scene prefab '", name, "' with ", entities.size(), " entities");
        return prefab;
    }

    bool PrefabSerializer::SavePrefabToFile(const Prefab& prefab, const std::string& filepath) {
        LOG_INFO("PrefabSerializer: Saving prefab to ", filepath);

        // Create parent directory if it doesn't exist
        std::filesystem::path filePath(filepath);
        std::filesystem::path parentDir = filePath.parent_path();

        if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
            try {
                std::filesystem::create_directories(parentDir);
                LOG_INFO("PrefabSerializer: Created directory ", parentDir.string());
            }
            catch (const std::filesystem::filesystem_error& e) {
                LOG_ERROR("PrefabSerializer: Failed to create directory: ", e.what());
                return false;
            }
        }

        std::string jsonString = SerializePrefabToString(prefab);

        std::ofstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("PrefabSerializer: Failed to open file for writing: ", filepath);
            return false;
        }

        file << jsonString;
        file.close();

        LOG_INFO("PrefabSerializer: Prefab saved successfully");
        return true;
    }

    std::shared_ptr<Prefab> PrefabSerializer::LoadPrefabFromFile(const std::string& filepath) {
        //LOG_INFO("PrefabSerializer: Loading prefab from ", filepath);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("PrefabSerializer: Failed to open file for reading: ", filepath);
            return nullptr;
        }

        std::string jsonString((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        auto prefab = DeserializePrefabFromString(jsonString);
        if (prefab) {
            prefab->SetSourcePath(filepath);
            //LOG_INFO("PrefabSerializer: Prefab loaded successfully");
        }

        return prefab;
    }

    std::string PrefabSerializer::SerializePrefabToString(const Prefab& prefab) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Prefab metadata
        doc.AddMember("PrefabVersion", "1.0", allocator);
        doc.AddMember("Name", rapidjson::Value(prefab.GetName().c_str(), allocator), allocator);
        doc.AddMember("GUID", rapidjson::Value(std::to_string(prefab.GetGUID().m_Value).c_str(), allocator), allocator);

        // Prefab type
        std::string typeStr = (prefab.GetType() == PrefabType::Entity) ? "Entity" : "Scene";
        doc.AddMember("Type", rapidjson::Value(typeStr.c_str(), allocator), allocator);

        // Serialized data
        if (prefab.GetType() == PrefabType::Entity) {
            doc.AddMember("EntityData", rapidjson::Value(prefab.GetEntityData().c_str(), allocator), allocator);
        }
        else {
            doc.AddMember("SceneData", rapidjson::Value(prefab.GetSceneData().c_str(), allocator), allocator);
            doc.AddMember("RootEntityGUID",
                rapidjson::Value(std::to_string(prefab.GetRootEntityGUID().m_Value).c_str(), allocator),
                allocator);
        }

        // Convert to string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    std::shared_ptr<Prefab> PrefabSerializer::DeserializePrefabFromString(const std::string& jsonString) {
        rapidjson::Document doc;
        doc.Parse(jsonString.c_str());
        if (doc.HasParseError()) {
            LOG_ERROR("PrefabSerializer: JSON parse error");
            return nullptr;
        }

        // Read prefab type
        if (!doc.HasMember("Type")) {
            LOG_ERROR("PrefabSerializer: Missing Type field");
            return nullptr;
        }
        std::string typeStr = doc["Type"].GetString();
        PrefabType type = (typeStr == "Entity") ? PrefabType::Entity : PrefabType::Scene;

        // Create prefab (constructor still generates temp GUID)
        auto prefab = std::make_shared<Prefab>(type);

        // Overwrite GUID from JSON
        if (doc.HasMember("GUID")) {
            uint64_t guidValue = std::stoull(doc["GUID"].GetString());
            prefab->SetGUID(xresource::instance_guid{ guidValue });
        }

        // Metadata
        if (doc.HasMember("Name")) prefab->SetName(doc["Name"].GetString());

        // Entity/Scene data
        if (type == PrefabType::Entity && doc.HasMember("EntityData")) {
            prefab->SetEntityData(doc["EntityData"].GetString());
        }
        else if (type == PrefabType::Scene) {
            if (doc.HasMember("SceneData")) prefab->SetSceneData(doc["SceneData"].GetString());
            if (doc.HasMember("RootEntityGUID")) {
                uint64_t rootGuid = std::stoull(doc["RootEntityGUID"].GetString());
                prefab->SetRootEntityGUID(xresource::instance_guid{ rootGuid });
            }
        }

        return prefab;
    }

    std::string PrefabSerializer::SerializeEntity(Entity entity, Entity /* dummyEntity */) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();


        // Entity ID
        doc.AddMember("ID", static_cast<uint32_t>(entity), allocator);

        // Components array
        rapidjson::Value componentsArray(rapidjson::kArrayType);

        // Serialize TagComponent
        if (entity.HasComponent<TagComponent>()) {
            const auto& tag = entity.GetComponent<TagComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "TagComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("ComponentGUID",
                rapidjson::Value(std::to_string(tag.ComponentGUID.m_Value).c_str(), allocator), allocator);
            propertiesObj.AddMember("Tag", rapidjson::Value(tag.Tag.c_str(), allocator), allocator);
            componentObj.AddMember("Properties", propertiesObj, allocator);

            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize TransformComponent
        if (entity.HasComponent<TransformComponent>()) {
            const auto& transform = entity.GetComponent<TransformComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "TransformComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);

            rapidjson::Value posArray(rapidjson::kArrayType);
            posArray.PushBack(transform.Position.x, allocator);
            posArray.PushBack(transform.Position.y, allocator);
            posArray.PushBack(transform.Position.z, allocator);
            propertiesObj.AddMember("Position", posArray, allocator);

            rapidjson::Value rotArray(rapidjson::kArrayType);
            rotArray.PushBack(transform.Rotation.x, allocator);
            rotArray.PushBack(transform.Rotation.y, allocator);
            rotArray.PushBack(transform.Rotation.z, allocator);
            rotArray.PushBack(transform.Rotation.w, allocator);
            propertiesObj.AddMember("Rotation", rotArray, allocator);

            rapidjson::Value scaleArray(rapidjson::kArrayType);
            scaleArray.PushBack(transform.Scale.x, allocator);
            scaleArray.PushBack(transform.Scale.y, allocator);
            scaleArray.PushBack(transform.Scale.z, allocator);
            propertiesObj.AddMember("Scale", scaleArray, allocator);

            propertiesObj.AddMember("Parent", transform.Parent, allocator);

            rapidjson::Value childrenArray(rapidjson::kArrayType);
            for (const auto& child : transform.Children) {
                childrenArray.PushBack(child, allocator);
            }
            propertiesObj.AddMember("Children", childrenArray, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize CameraComponent
        if (entity.HasComponent<CameraComponent>()) {
            const auto& camera = entity.GetComponent<CameraComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "CameraComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("ComponentGUID",
                rapidjson::Value(std::to_string(camera.ComponentGUID.m_Value).c_str(), allocator), allocator);
            propertiesObj.AddMember("Primary", camera.Primary, allocator);
            propertiesObj.AddMember("Projection", camera.Projection, allocator);
            propertiesObj.AddMember("FOV", camera.FOV, allocator);
            propertiesObj.AddMember("NearPlane", camera.NearPlane, allocator);
            propertiesObj.AddMember("FarPlane", camera.FarPlane, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize MeshRendererComponent
        if (entity.HasComponent<MeshRendererComponent>()) {
            const auto& mesh = entity.GetComponent<MeshRendererComponent>();
            std::string meshFilename = AM.getNameFromGuid(mesh.MeshGuid);
            std::string materialFilename = AM.getNameFromGuid(mesh.MaterialGuid);
            std::string textureFilename = AM.getNameFromGuid(mesh.TextureGuid);

            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "MeshRendererComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("Mesh",
                rapidjson::Value(meshFilename.empty() ? "" : meshFilename.c_str(), allocator),
                allocator);
            propertiesObj.AddMember("Material",
                rapidjson::Value(materialFilename.empty() ? "" : materialFilename.c_str(), allocator),
                allocator);
            propertiesObj.AddMember("Texture",
                rapidjson::Value(textureFilename.empty() ? "" : textureFilename.c_str(), allocator),
                allocator);

            //propertiesObj.AddMember("MeshGuid",
            //    rapidjson::Value(std::to_string(mesh.MeshGuid.m_Value).c_str(), allocator), allocator);

            propertiesObj.AddMember("Visible", mesh.Visible, allocator);

            propertiesObj.AddMember("MeshType", mesh.MeshType, allocator);
            propertiesObj.AddMember("Material", mesh.Material, allocator);
            propertiesObj.AddMember("Texture", mesh.Texture, allocator);
            propertiesObj.AddMember("SubmeshIndex", mesh.SubmeshIndex, allocator);
            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize RigidbodyComponent
        if (entity.HasComponent<RigidbodyComponent>()) {
            const auto& rb = entity.GetComponent<RigidbodyComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "RigidbodyComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("ComponentGUID",
                rapidjson::Value(std::to_string(rb.ComponentGUID.m_Value).c_str(), allocator), allocator);
            propertiesObj.AddMember("Mass", rb.Mass, allocator);
            propertiesObj.AddMember("IsKinematic", rb.IsKinematic, allocator);
            propertiesObj.AddMember("UseGravity", rb.UseGravity, allocator);

            rapidjson::Value velArray(rapidjson::kArrayType);
            velArray.PushBack(rb.Velocity.x, allocator);
            velArray.PushBack(rb.Velocity.y, allocator);
            velArray.PushBack(rb.Velocity.z, allocator);
            propertiesObj.AddMember("Velocity", velArray, allocator);

            rapidjson::Value angVel(rapidjson::kArrayType);
            angVel.PushBack(rb.AngularVelocity.x, allocator);
            angVel.PushBack(rb.AngularVelocity.y, allocator);
            angVel.PushBack(rb.AngularVelocity.z, allocator);
            propertiesObj.AddMember("AngularVelocity", angVel, allocator);

            propertiesObj.AddMember("LinearDamping", rb.LinearDamping, allocator);
            propertiesObj.AddMember("AngularDamping", rb.AngularDamping, allocator);
            propertiesObj.AddMember("Restitution", rb.Restitution, allocator);

            propertiesObj.AddMember("CollideType", static_cast<int>(rb.Shape), allocator);

            rapidjson::Value boxHalfExtent(rapidjson::kArrayType);
            boxHalfExtent.PushBack(rb.BoxHalfExtents.x, allocator);
            boxHalfExtent.PushBack(rb.BoxHalfExtents.y, allocator);
            boxHalfExtent.PushBack(rb.BoxHalfExtents.z, allocator);
            propertiesObj.AddMember("BoxHalfExtents", boxHalfExtent, allocator);

            propertiesObj.AddMember("SphereRadius", rb.SphereRadius, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize AudioComponent
        if (entity.HasComponent<AudioComponent>()) {
            const auto& audio = entity.GetComponent<AudioComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "AudioComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("AudioFilePath",
                rapidjson::Value(audio.AudioFilePath.c_str(), allocator), allocator);
            propertiesObj.AddMember("Type", static_cast<int>(audio.Type), allocator);
            propertiesObj.AddMember("State", static_cast<int>(audio.State), allocator);
            propertiesObj.AddMember("Volume", audio.Volume, allocator);
            propertiesObj.AddMember("Pitch", audio.Pitch, allocator);
            propertiesObj.AddMember("Loop", audio.Loop, allocator);
            propertiesObj.AddMember("Mute", audio.Mute, allocator);
            propertiesObj.AddMember("Is3D", audio.Is3D, allocator);
            propertiesObj.AddMember("MinDistance", audio.MinDistance, allocator);
            propertiesObj.AddMember("MaxDistance", audio.MaxDistance, allocator);
            propertiesObj.AddMember("ReverbProperties", audio.ReverbProperties, allocator);
            propertiesObj.AddMember("RolloffMode", static_cast<int>(audio.RolloffMode), allocator);
            propertiesObj.AddMember("DopplerLevel", audio.DopplerLevel, allocator);
            propertiesObj.AddMember("Pan2D", audio.Pan2D, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize ListenerComponent
        if (entity.HasComponent<ListenerComponent>()) {
            const auto& listener = entity.GetComponent<ListenerComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "ListenerComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("Active", listener.Active, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize ReverbComponent
        if (entity.HasComponent<ReverbZoneComponent>()) {
            const auto& reverb = entity.GetComponent<ReverbZoneComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "ReverbComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("Preset", static_cast<int>(reverb.Preset), allocator);
            propertiesObj.AddMember("MinDistance", reverb.MinDistance, allocator);
            propertiesObj.AddMember("MaxDistance", reverb.MaxDistance, allocator);
            propertiesObj.AddMember("DecayTime", reverb.DecayTime, allocator);
            propertiesObj.AddMember("HfDecayRatio", reverb.HfDecayRatio, allocator);
            propertiesObj.AddMember("Diffusion", reverb.Diffusion, allocator);
            propertiesObj.AddMember("Density", reverb.Density, allocator);
            propertiesObj.AddMember("WetLevel", reverb.WetLevel, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        if (entity.HasComponent<BehaviourTreeComponent>()) {
            const auto& bt = entity.GetComponent<BehaviourTreeComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "BehaviourTreeComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("Active", bt.Active, allocator);
            propertiesObj.AddMember("ResetOnComplete", bt.ResetOnComplete, allocator);
            propertiesObj.AddMember("TreeAssetPath",
                rapidjson::Value(bt.TreeAssetPath.c_str(), allocator), allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);

            LOG_INFO("[PrefabSerializer] Serializing BehaviourTreeComponent with path: ", bt.TreeAssetPath);

            componentsArray.PushBack(componentObj, allocator);
        }

        if (entity.HasComponent<ParticleComponent>()) {
            const auto& emitter = entity.GetComponent<ParticleComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "ParticleComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);

            // Initial Velocity
            rapidjson::Value velArray(rapidjson::kArrayType);
            velArray.PushBack(emitter.InitialVelocity.x, allocator);
            velArray.PushBack(emitter.InitialVelocity.y, allocator);
            velArray.PushBack(emitter.InitialVelocity.z, allocator);
            propertiesObj.AddMember("Initial Velocity", velArray, allocator);

            // Min Color
            rapidjson::Value minColorArray(rapidjson::kArrayType);
            minColorArray.PushBack(emitter.ColorMin.x, allocator);
            minColorArray.PushBack(emitter.ColorMin.y, allocator);
            minColorArray.PushBack(emitter.ColorMin.z, allocator);
            propertiesObj.AddMember("Color Min", minColorArray, allocator);

            // Max Color
            rapidjson::Value maxColorArray(rapidjson::kArrayType);
            maxColorArray.PushBack(emitter.ColorMax.x, allocator);
            maxColorArray.PushBack(emitter.ColorMax.y, allocator);
            maxColorArray.PushBack(emitter.ColorMax.z, allocator);
            propertiesObj.AddMember("Color Max", maxColorArray, allocator);

            // Max ParScriptComponentticles
            propertiesObj.AddMember("Max Particles", emitter.MaxParticles, allocator);

            // Particle Type
            propertiesObj.AddMember("Particle Type", emitter.ParticleType, allocator);

            // Emission Rate
            propertiesObj.AddMember("Emission Rate", emitter.EmissionRate, allocator);

            // Particle Lifetime
            propertiesObj.AddMember("Particle Lifetime", emitter.ParticleLifetime, allocator);

            // Emission Accumulator
            propertiesObj.AddMember("Emission Accumulator", emitter.EmissionAccumulator, allocator);

            // Particle Size
            propertiesObj.AddMember("Particle Size", emitter.ParticleSize, allocator);

            // Randomization parameters
            propertiesObj.AddMember("Velocity Randomness", emitter.VelocityRandomness, allocator);
            propertiesObj.AddMember("Lifetime Randomness", emitter.LifetimeRandomness, allocator);
            propertiesObj.AddMember("Spread Angle", emitter.SpreadAngle, allocator);
            propertiesObj.AddMember("Min Speed", emitter.MinSpeed, allocator);
            propertiesObj.AddMember("Max Speed", emitter.MaxSpeed, allocator);
            propertiesObj.AddMember("Rotation Speed", emitter.RotationSpeed, allocator);

            // Boolean parameters
            propertiesObj.AddMember("Randomize Rotation", emitter.RandomizeRotation, allocator);
            propertiesObj.AddMember("Loop", emitter.Loop, allocator);
            propertiesObj.AddMember("Active", emitter.Active, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize ScriptComponent
        if (entity.HasComponent<ScriptComponent>()) {
            const auto& script = entity.GetComponent<ScriptComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "ScriptComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("ScriptClassName",
                rapidjson::Value(script.ScriptClassName.c_str(), allocator), allocator);



            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize LightComponent
        if (entity.HasComponent<LightComponent>()) {
            const auto& light = entity.GetComponent<LightComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "LightComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("Enabled", light.Enabled, allocator);
            propertiesObj.AddMember("Type", static_cast<uint32_t>(light.Type), allocator);
            //propertiesObj.AddMember("Mode", static_cast<uint32_t>(light.Mode), allocator);

            rapidjson::Value colorArray(rapidjson::kArrayType);
            colorArray.PushBack(light.Color.x, allocator);
            colorArray.PushBack(light.Color.y, allocator);
            colorArray.PushBack(light.Color.z, allocator);
            propertiesObj.AddMember("Color", colorArray, allocator);

            propertiesObj.AddMember("Intensity", light.Intensity, allocator);
            propertiesObj.AddMember("Range", light.Range, allocator);
            propertiesObj.AddMember("SpotAngleDeg", light.SpotAngleDeg, allocator);
            propertiesObj.AddMember("IndirectMultiplier", light.IndirectMultiplier, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize AnimatorComponent
        if (entity.HasComponent<AnimatorComponent>()) {
            const auto& animator = entity.GetComponent<AnimatorComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "AnimatorComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("playing", animator.playing, allocator);
            propertiesObj.AddMember("respectClipLoop", animator.respectClipLoop, allocator);
            propertiesObj.AddMember("controller", animator.controller, allocator);
            propertiesObj.AddMember("currentClipIndex", animator.currentClipIndex, allocator);
            propertiesObj.AddMember("currentTime", animator.currentTime, allocator);
            propertiesObj.AddMember("playbackSpeed", animator.playbackSpeed, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize SpriteRendererComponent
        if (entity.HasComponent<SpriteRendererComponent>())
        {
            LOG_TRACE(" - Serializing SpriteRendererComponent");
            auto& SpriteRenderer = entity.GetComponent<SpriteRendererComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "SpriteRendererComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);

            std::string textureFilename = AM.getNameFromGuid(SpriteRenderer.TextureGuid);

            propertiesObj.AddMember("Texture",
                rapidjson::Value(textureFilename.empty() ? "" : textureFilename.c_str(), allocator),
                allocator);

            rapidjson::Value colorArr(rapidjson::kArrayType);
            colorArr.PushBack(SpriteRenderer.Color.r, allocator);
            colorArr.PushBack(SpriteRenderer.Color.g, allocator);
            colorArr.PushBack(SpriteRenderer.Color.b, allocator);
            colorArr.PushBack(SpriteRenderer.Color.a, allocator);

            propertiesObj.AddMember("Color", colorArr, allocator);
            propertiesObj.AddMember("Quad", SpriteRenderer.Quad, allocator);
            propertiesObj.AddMember("Sprite Layer", SpriteRenderer.SpriteLayer, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        doc.AddMember("Components", componentsArray, allocator);

        // Convert to string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    std::string PrefabSerializer::SerializeEntities(const std::vector<Entity>& entities, entt::registry& registry) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();


        rapidjson::Value entitiesArray(rapidjson::kArrayType);

#if 1 // original code bfr modified
        for (const auto& entity : entities) {
            std::string entityJson = SerializeEntity(entity, entity);

            rapidjson::Document entityDoc;
            entityDoc.Parse(entityJson.c_str());

            rapidjson::Value entityValue;
            entityValue.CopyFrom(entityDoc, allocator);
            entitiesArray.PushBack(entityValue, allocator);
        }
#endif 

        doc.AddMember("Entities", entitiesArray, allocator);

        // Convert to string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

#if 1 // to show sub entities in prefab file 26/11 
    std::shared_ptr<Prefab> PrefabSerializer::CreateEntityWithChildrenPrefab(Entity rootEntity, const std::string& name) {
        if (!rootEntity) {
            LOG_ERROR("PrefabSerializer: Cannot create prefab from invalid entity");
            return nullptr;
        }

        entt::registry* registry = rootEntity.GetRegistry();
        if (!registry) {
            LOG_ERROR("PrefabSerializer: Entity has no valid registry");
            return nullptr;
        }

        // Collect all entities in the hierarchy
        std::vector<Entity> allEntities;
        std::stack<Entity> toProcess;
        toProcess.push(rootEntity);

        while (!toProcess.empty()) {
            Entity current = toProcess.top();
            toProcess.pop();
            allEntities.push_back(current);

            if (current.HasComponent<TransformComponent>()) {
                const auto& transform = current.GetComponent<TransformComponent>();
                for (u32 childId : transform.Children) {
                    Entity childEntity(static_cast<entt::entity>(childId), registry);
                    toProcess.push(childEntity);
                }
            }
        }

        LOG_INFO("PrefabSerializer: Collected ", allEntities.size(), " entities in hierarchy");

        // Create Scene prefab with all entities
        auto prefab = std::make_shared<Prefab>(PrefabType::Scene);
        prefab->SetName(name);

        // Serialize all entities
        std::string sceneData = SerializeEntities(allEntities, *registry);
        prefab->SetSceneData(sceneData);

        // Set root entity
        uint32_t entityID = static_cast<uint32_t>(rootEntity);
        prefab->SetRootEntityGUID(xresource::instance_guid{ static_cast<uint64_t>(entityID) });

        LOG_INFO("PrefabSerializer: Created scene prefab '", name, "' with ", allEntities.size(), " entities");
        return prefab;
    }
#endif

} // namespace Engine
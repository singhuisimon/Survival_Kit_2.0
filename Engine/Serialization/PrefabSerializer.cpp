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
#include "../Utility/Logger.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>

namespace Engine {

    std::shared_ptr<Prefab> PrefabSerializer::CreateEntityPrefab(Entity entity, const std::string& name) {
        if (!entity) {
            LOG_ERROR("PrefabSerializer: Cannot create prefab from invalid entity");
            return nullptr;
        }

        auto prefab = std::make_shared<Prefab>(PrefabType::Entity);
        prefab->SetName(name);

        // Get the entity's registry - need to access via the entity's internal pointer
        // This is a workaround since Entity doesn't expose the registry directly
        entt::registry* registry = nullptr;

        // We'll use the entity to get components, which implicitly uses its registry
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
        LOG_INFO("PrefabSerializer: Loading prefab from ", filepath);

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
            LOG_INFO("PrefabSerializer: Prefab loaded successfully");
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

        auto prefab = std::make_shared<Prefab>(type);

        // Read metadata
        if (doc.HasMember("Name")) {
            prefab->SetName(doc["Name"].GetString());
        }

        if (doc.HasMember("GUID")) {
            uint64_t guidValue = std::stoull(doc["GUID"].GetString());
            prefab->SetGUID(xresource::instance_guid{ guidValue });
        }

        // Read data
        if (type == PrefabType::Entity) {
            if (doc.HasMember("EntityData")) {
                prefab->SetEntityData(doc["EntityData"].GetString());
            }
        }
        else {
            if (doc.HasMember("SceneData")) {
                prefab->SetSceneData(doc["SceneData"].GetString());
            }
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
            propertiesObj.AddMember("FOV", camera.FOV, allocator);
            propertiesObj.AddMember("NearClip", camera.NearClip, allocator);
            propertiesObj.AddMember("FarClip", camera.FarClip, allocator);
            propertiesObj.AddMember("Primary", camera.Primary, allocator);

            componentObj.AddMember("Properties", propertiesObj, allocator);
            componentsArray.PushBack(componentObj, allocator);
        }

        // Serialize MeshRendererComponent
        if (entity.HasComponent<MeshRendererComponent>()) {
            const auto& mesh = entity.GetComponent<MeshRendererComponent>();
            rapidjson::Value componentObj(rapidjson::kObjectType);
            componentObj.AddMember("Type", "MeshRendererComponent", allocator);

            rapidjson::Value propertiesObj(rapidjson::kObjectType);
            propertiesObj.AddMember("ComponentGUID",
                rapidjson::Value(std::to_string(mesh.ComponentGUID.m_Value).c_str(), allocator), allocator);
            propertiesObj.AddMember("Visible", mesh.Visible, allocator);

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

        for (const auto& entity : entities) {
            std::string entityJson = SerializeEntity(entity, entity);

            rapidjson::Document entityDoc;
            entityDoc.Parse(entityJson.c_str());

            rapidjson::Value entityValue;
            entityValue.CopyFrom(entityDoc, allocator);
            entitiesArray.PushBack(entityValue, allocator);
        }

        doc.AddMember("Entities", entitiesArray, allocator);

        // Convert to string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

} // namespace Engine
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
#include "../Utility/Logger.h"
#include "../Asset/AssetManager.h"
#include "../Serialization/ComponentSerializer.h"
#include "../Prefab/PrefabRegistry.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <filesystem> 

namespace Engine {

    bool PrefabSerializer::SerializePrefab(const Prefab& prefab, const std::string& filepath)
    {
        LOG_INFO("--- Start of PrefabSerializer::SerializePrefab ---- ");
        if (!prefab.IsValid()) {
            LOG_ERROR("Cannot serialize invalid prefab");
            return false;
        }

        LOG_INFO("PrefabSerializer: Saving prefab '", prefab.name, "' to ", filepath);
        
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        doc.AddMember("Prefab Name", rapidjson::Value(prefab.name.c_str(), allocator), allocator);
        doc.AddMember("Prefab GUID", prefab.guid.m_Value, allocator);
        doc.AddMember("version", prefab.version, allocator);

        rapidjson::Value entitiesArray(rapidjson::kArrayType);
        for (const auto& entity : prefab.entities) {
            rapidjson::Value entityObj(rapidjson::kObjectType);
            entityObj.AddMember("Name", rapidjson::Value(entity.name.c_str(), allocator), allocator);
            entityObj.AddMember("LocalID", entity.localID, allocator);
            entityObj.AddMember("ParentLocalID", entity.parentLocalID, allocator);

            // Components array
            rapidjson::Value componentsArray(rapidjson::kArrayType);
            for (const auto& comp : entity.components) {
                rapidjson::Value compObj(rapidjson::kObjectType);
                compObj.AddMember("type", static_cast<u32>(comp.type), allocator);
                compObj.AddMember("typeName", rapidjson::Value(comp.typeName.c_str(), allocator), allocator);

                // Parse and embed JSON data
                if (!comp.serializedData.empty()) {
                    std::string jsonStr(comp.serializedData.begin(), comp.serializedData.end());

                    rapidjson::Document compDoc;
                    compDoc.Parse(jsonStr.c_str());
                    if (!compDoc.HasParseError() && compDoc.IsObject()) {
                        rapidjson::Value compDataCopy(compDoc, allocator);
                        compObj.AddMember("data", compDataCopy, allocator);
                    }
                }

                componentsArray.PushBack(compObj, allocator);
            }
            entityObj.AddMember("components", componentsArray, allocator);
            entitiesArray.PushBack(entityObj, allocator);
           
        }

        doc.AddMember("entities", entitiesArray, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        // write json file
        std::ofstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file: ", filepath);
            return false;
        }

        file << buffer.GetString();
        file.close();

        LOG_INFO("Prefab serialized successfully to: ", filepath.c_str());
        LOG_INFO("--- End of PrefabSerializer::SerializePrefab ---- ");
        return true;
    }

    bool PrefabSerializer::SerializeEntityToPrefabFile(Entity rootEntity, const std::string& prefabName,
        const std::string& filePath) {

        if (!rootEntity) {
            LOG_ERROR("Cannot serialize invalid entity to prefab file");
            return false;
        }

        Prefab prefab;
        if (!CreatePrefabFromEntity(rootEntity, prefab, prefabName)) {
            LOG_ERROR("Failed to create prefab from entity");
            return false;
        }

        // Create directory if it doesn't exist
        std::filesystem::path pathObj(filePath);
        std::filesystem::create_directories(pathObj.parent_path());

        // Serialize to file
        if (!SerializePrefab(prefab, filePath)) {
            LOG_ERROR("Failed to save prefab to file: ", filePath.c_str());
            return false;
        }
       
        LOG_INFO("Successfully created and saved prefab.");
        return true;
    }
    bool PrefabSerializer::CreatePrefabFromEntity(Entity rootEntity, Prefab& outPrefab, const std::string& prefabName) 
    {
        LOG_INFO("=== Start of PrefabSerializer::CreatePrefabFromEntity ===");
        if (!rootEntity) {
            LOG_ERROR("Cannot create prefab from invalid/null entity");
            return false;
        }

        if (!rootEntity.HasComponent<TagComponent>()) {
            LOG_ERROR("Entity must have TagComponent to create prefab");
            return false;
        }

        std::string entityName = rootEntity.GetComponent<TagComponent>().Tag;

        outPrefab.Clear();
        outPrefab.name = prefabName;
        outPrefab.version = 1;

        if (outPrefab.guid == 0) {
            outPrefab.guid = xresource::instance_guid::GenerateGUIDCopy();
        }
        LOG_DEBUG("Prefab GUID: ", outPrefab.guid.m_Value);
        u32 nextLocalID = 1;

        SerializeEntityHierarchy(rootEntity, outPrefab, nextLocalID, 0);
        outPrefab.m_IsValid = !outPrefab.entities.empty();
        return outPrefab.m_IsValid;
    }
  
    void PrefabSerializer::SerializeEntityHierarchy(Entity entity, Prefab& prefab, u32& nextLocalID, u32 parentLocalID) {
        PrefabEntityData entityData;

        // Get entity name from TagComponent
        if (entity.HasComponent<TagComponent>()) {
            entityData.name = entity.GetComponent<TagComponent>().Tag;
        }
        else {
            entityData.name = std::to_string(static_cast<u32>(entity));
        }

        entityData.localID = nextLocalID++;
        entityData.parentLocalID = parentLocalID;

        // Serialize all components (except PrefabComponent - it's added on instantiation)
        if (entity.HasComponent<TransformComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Transform));
        }

        if (entity.HasComponent<TagComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Tag));
        }

        if (entity.HasComponent<MeshRendererComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::MeshRenderer));
        }

        if (entity.HasComponent<CameraComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Camera));
        }

        if (entity.HasComponent<RigidbodyComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::RigidBody));
        }

        if (entity.HasComponent<AudioComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Audio));
        }

        if (entity.HasComponent<ListenerComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Listerner));
        }

        if (entity.HasComponent<ReverbZoneComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::ReverbZone));
        }

        if (entity.HasComponent<BehaviourTreeComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::BehaviourTree));
        }

        if (entity.HasComponent<ParticleComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::ParticleSystem));
        }

        if (entity.HasComponent<ScriptComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Script));
        }

        if (entity.HasComponent<LightComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Light));
        }

        if (entity.HasComponent<AnimatorComponent>()) {
            entityData.components.push_back(SerializeEntityComponent(entity, ComponentTypeID::Animator));
        }

        prefab.entities.push_back(entityData);

        u32 currentLocalID = entityData.localID;

        // Recursively serialize children using TransformComponent
        if (entity.HasComponent<TransformComponent>()) {
            auto& transform = entity.GetComponent<TransformComponent>();
            entt::registry* registry = entity.GetRegistry();

            for (u32 childID : transform.Children) {
                entt::entity childHandle = static_cast<entt::entity>(childID);

                // Check if child entity is valid
                if (registry->valid(childHandle)) {
                    Entity childEntity(childHandle, registry);
                    SerializeEntityHierarchy(childEntity, prefab, nextLocalID, currentLocalID);
                }
            }
        }
    }

    PrefabComponentData PrefabSerializer::SerializeEntityComponent(Entity entity, ComponentTypeID type) {
        PrefabComponentData data;
        data.type = type;
        data.typeName = ComponentSerializer::GetComponentTypeName(type);

        // Use ComponentSerializer
        std::string jsonStr = ComponentSerializer::SerializeComponent(entity, type);
        data.serializedData.assign(jsonStr.begin(), jsonStr.end());

        return data;
    }

    bool PrefabSerializer::DeserializePrefab(const std::string& filepath, Prefab& outPrefab) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open prefab file: ", filepath.c_str());
            return false;
        }

        std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        rapidjson::Document doc;
        doc.Parse(json.c_str());

        if (doc.HasParseError() || !doc.IsObject()) {
            LOG_ERROR("Failed to parse prefab JSON");
            return false;
        }

        outPrefab.Clear();

        // Read metadata
        if (doc.HasMember("Prefab Name")) outPrefab.name = doc["Prefab Name"].GetString();
        if (doc.HasMember("Prefab GUID")) outPrefab.guid = doc["Prefab GUID"].GetUint64();
        if (doc.HasMember("version")) outPrefab.version = doc["version"].GetUint();

        // Read entities
        if (doc.HasMember("entities") && doc["entities"].IsArray()) {
            const rapidjson::Value& entitiesArray = doc["entities"];

            for (rapidjson::SizeType i = 0; i < entitiesArray.Size(); i++) {
                const rapidjson::Value& entityObj = entitiesArray[i];

                PrefabEntityData entityData;
                if (entityObj.HasMember("name")) entityData.name = entityObj["name"].GetString();
                if (entityObj.HasMember("localID")) entityData.localID = entityObj["localID"].GetUint64();
                if (entityObj.HasMember("parentLocalID")) entityData.parentLocalID = entityObj["parentLocalID"].GetUint64();

                // Read components
                if (entityObj.HasMember("components") && entityObj["components"].IsArray()) {
                    const rapidjson::Value& componentsArray = entityObj["components"];

                    for (rapidjson::SizeType j = 0; j < componentsArray.Size(); j++) {
                        const rapidjson::Value& compObj = componentsArray[j];

                        PrefabComponentData compData;
                        if (compObj.HasMember("type")) {
                            compData.type = static_cast<ComponentTypeID>(compObj["type"].GetUint());
                        }
                        if (compObj.HasMember("typeName")) {
                            compData.typeName = compObj["typeName"].GetString();
                        }

                        // Read component data
                        if (compObj.HasMember("data")) {
                            rapidjson::StringBuffer buffer;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                            compObj["data"].Accept(writer);

                            std::string jsonStr = buffer.GetString();
                            compData.serializedData.assign(jsonStr.begin(), jsonStr.end());
                        }

                        entityData.components.push_back(compData);
                    }
                }

                outPrefab.entities.push_back(entityData);
            }
        }

        outPrefab.m_IsValid = !outPrefab.entities.empty();
        if (outPrefab.m_IsValid) {
          /*  if (outPrefab.guid == 0) {
                outPrefab.guid = xresource::instance_guid::GenerateGUIDCopy();
                LOG_WARNING("Generated GUID for prefab: ", outPrefab.guid.m_Value);
            }*/

            if (!PrefabRegistry::Get().IsPrefabRegistered(outPrefab.guid)) {
                PrefabRegistry::Get().RegisterPrefab(
                    outPrefab.guid,
                    filepath,
                    outPrefab.name
                );
                LOG_INFO("Registered: ", outPrefab.name, " (GUID : ", outPrefab.guid.m_Value, ")");
            }
        }
        LOG_INFO("Prefab deserialized successfully from: ", filepath.c_str());
        return outPrefab.m_IsValid;
    }


}
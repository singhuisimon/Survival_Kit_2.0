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
#include "../Component/AnimatorComponent.h"

#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"

#include "../Asset/AssetManager.h"

#include "../Serialization/ComponentSerializer.h"
#include "../Serialization/PrefabSerializer.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <filesystem>

namespace Engine
{
   

    Entity PrefabInstantiator::InstantiatePrefabFromFile(Scene* scene, const std::string& filepath, Entity parent) {
        if (!scene) {
            LOG_ERROR("Cannot instantiate prefab into null scene");
            return Entity{};
        }

        Prefab prefab;

        if (!PrefabSerializer::DeserializePrefab(filepath, prefab)) {
            LOG_ERROR("Failed to load prefab from file: ", filepath.c_str());
            return Entity{};
        }
        if (!PrefabRegistry::Get().IsPrefabRegistered(prefab.guid)) {
            PrefabRegistry::Get().RegisterPrefab(prefab.guid, filepath, prefab.name);
            LOG_INFO("Registered prefab from file: ", filepath.c_str());
        }
        return InstantiatePrefab(scene, prefab, parent);
    }

    Entity PrefabInstantiator::InstantiatePrefab(Scene* scene, const Prefab& prefab, Entity parent) {
        LOG_INFO("===== Start of PrefabInstantiator::INSTANTIATING PREFAB =====");
        if (!scene || !prefab.IsValid()) {
            LOG_ERROR("Cannot instantiate invalid prefab into null scene");
            return Entity{};
        }
        if (!PrefabRegistry::Get().IsPrefabRegistered(prefab.guid)) {
            //LOG_WARNING("Prefab not yet registered: ", prefab.name.c_str());
            std::string prefabPath = "Resources/Prefabs/" + prefab.name + ".prefab";
            PrefabRegistry::Get().RegisterPrefab(prefab.guid, prefabPath, prefab.name);
            LOG_INFO("Auto-registered prefab: ", prefab.name.c_str());
        }

        const PrefabEntityData* rootData = prefab.GetRootEntity();
        if (!rootData) {
            LOG_ERROR("Prefab has no root entity");
            return Entity{};
        }

       
        // LOG_INFO("Prefab: ", prefab.name);
        LOG_INFO("Root entity in prefab: ", rootData->name);
        // LOG_INFO("Total components: ", rootData->components.size());

        std::unordered_map<u64, Entity> localIDToEntity;

        // Recursively instantiate the hierarchy
        Entity rootEntity = InstantiateEntity(scene, *rootData, prefab, parent, localIDToEntity);

        if (!rootEntity) {
            LOG_ERROR("Failed to instantiate prefab root entity");
            return Entity{};
        }
        RebuildPrefabHierarchy(prefab, localIDToEntity, scene);
        PrefabComponent* prefabComp = nullptr;
        if (rootEntity.HasComponent<PrefabComponent>()) {
            prefabComp = &rootEntity.GetComponent<PrefabComponent>();
        }
        else {
            prefabComp = &rootEntity.AddComponent<PrefabComponent>();
        }

        prefabComp->PrefabAssetGuid = prefab.guid;
        prefabComp->prefabName = prefab.name;
        prefabComp->prefabVersion = prefab.version;
        //prefabComp->isPrefabRoot = true;


        // FIXED: Store original data AFTER all entities and components are fully instantiated
        StoreOriginalComponentDataForAllEntities(scene, prefab, localIDToEntity);

        LOG_INFO("Verifying stored data...");
        for (const auto& override : prefabComp->componentOverrides) {
            std::string componentName = ComponentSerializer::GetComponentTypeName(override.componentType);
            if (!override.originalComponentJSON.empty()) {
                LOG_INFO(componentName, " has original data stored");
            }
            else {
                LOG_WARNING(componentName, " has NO original data!");
            }
        }

        // Store child entity IDs for prefab tracking
        for (const auto& [localID, entity] : localIDToEntity) {
            if (entity != rootEntity) {
                prefabComp->childEntityIDs.push_back(static_cast<u32>(entity.GetHandle()));
            }
        }

        std::string rootEntityName = rootEntity.HasComponent<TagComponent>()
            ? rootEntity.GetComponent<TagComponent>().Tag
            : "Unknown";

        LOG_INFO("Successfully instantiated prefab '", prefab.name,
            "' with root: '", rootEntityName, "'");
        LOG_DEBUG("Total entities created: ", localIDToEntity.size());
        LOG_INFO("===== End of PrefabInstantiator::INSTANTIATING PREFAB =====");
        return rootEntity;
    }

    Entity PrefabInstantiator::InstantiateEntity(Scene* scene,
        const PrefabEntityData& entityData,
        const Prefab& prefab,
        Entity sceneParent,
        std::unordered_map<u64, Entity>& localIDToEntity) {

        LOG_INFO("=== Start of PrefabInstantiator::InstantiateEntity =====");
        LOG_DEBUG("Instantiating entity: ", entityData.name,
            " (localID: ", entityData.localID,
            ", prefabParentLocalID: ", entityData.parentLocalID, ")");

        // 1. Create entity in scene
        Entity entity = scene->CreateEntity(entityData.name);
        if (!entity) {
            LOG_ERROR("Failed to create entity: ", entityData.name);
            return Entity{};
        }

        // 2. Store mapping from localID to scene entity
        localIDToEntity[entityData.localID] = entity;

        // 3. Create all components from prefab data
        for (const auto& componentData : entityData.components) {
            // Skip PrefabComponent - we'll add it separately
            if (componentData.type == ComponentTypeID::Prefab) {
                continue;
            }

            // Convert binary data to JSON string
            std::string jsonStr(componentData.serializedData.begin(),
                componentData.serializedData.end());

            if (componentData.type == ComponentTypeID::Transform) {
                if (!ComponentSerializer::DeserializeComponent(entity, componentData.type, jsonStr)) {
                    LOG_ERROR("Failed to deserialize Transform for: ", entityData.name);
                }
                // Clear the children array and reset parent - will be rebuilt later
                if (entity.HasComponent<TransformComponent>()) {
                    auto& transform = entity.GetComponent<TransformComponent>();
                    transform.Children.clear();
                    transform.Parent = u32_max;
                    LOG_DEBUG("  Cleared Transform relationships (will rebuild from prefab hierarchy)");
                }
            }
            else {
                // Deserialize other components normally
                if (!ComponentSerializer::DeserializeComponent(entity, componentData.type, jsonStr)) {
                    LOG_ERROR("Failed to deserialize component: ", componentData.typeName);
                }
            }
        }

        // 4. Add PrefabComponent to ALL entities in the hierarchy
        if (!entity.HasComponent<PrefabComponent>()) {
            entity.AddComponent<PrefabComponent>();
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();
        prefabComp.PrefabAssetGuid = prefab.guid;
        prefabComp.prefabName = prefab.name;
        prefabComp.prefabVersion = prefab.version;
        prefabComp.isPrefabRoot = (entityData.parentLocalID == 0); // Root if parentID is 0
        prefabComp.prefabLocalID = entityData.localID;

        LOG_DEBUG("  Added PrefabComponent - isRoot: ", prefabComp.isPrefabRoot);

        auto children = prefab.GetChildren(entityData.localID);
        LOG_DEBUG("  Entity has ", children.size(), " children in prefab");

        for (const auto* childData : children) {
            // Recursively instantiate child
            Entity childEntity = InstantiateEntity(scene, *childData, prefab, Entity{}, localIDToEntity);
            if (!childEntity) {
                LOG_ERROR("Failed to instantiate child entity: ", childData->name);
            }
            else {
                LOG_DEBUG("  Successfully created child: ", childData->name);
            }
        }

        LOG_INFO("=== End of PrefabInstantiator::InstantiateEntity =====\n\n");
        return entity;
    }


    bool PrefabInstantiator::RevertToPrefab(Entity entity, Scene* scene) {
        if (!entity.HasComponent<PrefabComponent>()) {
            LOG_ERROR("Entity does not have PrefabComponent");
            return false;
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();

        if (!prefabComp.isPrefabRoot) {
            LOG_ERROR("Can only revert from prefab root entity");
            return false;
        }

        // Load fresh prefab data from disk
        Prefab prefab;
        if (!PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
            LOG_ERROR("Failed to load prefab for revert");
            return false;
        }
        UpdateExistingEntitiesPrefabLocalID(entity, scene, prefab);
        LOG_INFO("===== REVERTING ENTIRE PREFAB HIERARCHY =====");
        LOG_INFO("Prefab: ", prefab.name);

        // Build localIDToEntity map from current scene
        std::unordered_map<u64, Entity> localIDToEntity;
        std::queue<Entity> queue;
        queue.push(entity);

        while (!queue.empty()) {
            Entity current = queue.front();
            queue.pop();

            // Only add entities that exist
            if (!current) continue;

            // Map by localID
            if (current.HasComponent<PrefabComponent>()) {
                auto& currentPrefab = current.GetComponent<PrefabComponent>();
                localIDToEntity[currentPrefab.prefabLocalID] = current;
                LOG_DEBUG("Mapped entity localID: ", currentPrefab.prefabLocalID);
            }

            // Add children to queue
            if (current.HasComponent<TransformComponent>()) {
                auto& transform = current.GetComponent<TransformComponent>();
                for (u32 childHandle : transform.Children) {
                    Entity childEntity(static_cast<entt::entity>(childHandle), &scene->GetRegistry());
                    if (childEntity) {
                        queue.push(childEntity);
                    }
                }
            }
        }
        // Copy the list first to avoid modifying while iterating
        auto addedHandlesCopy = prefabComp.addedEntityHandles;
        for (u32 addedHandle : addedHandlesCopy) {
            Entity addedEntity(static_cast<entt::entity>(addedHandle), &scene->GetRegistry());
            if (addedEntity) {
                std::string entityName = addedEntity.HasComponent<TagComponent>()
                    ? addedEntity.GetComponent<TagComponent>().Tag : "Unknown";
                LOG_INFO("Removing added entity: ", entityName);
                scene->DestroyEntity(addedEntity);
            }
        }
        prefabComp.addedEntityHandles.clear();

        // STEP 3: Restore deleted entities
        for (const auto& prefabEntity : prefab.entities) {
            auto it = localIDToEntity.find(prefabEntity.localID);
            if (it == localIDToEntity.end()) {
                // Check if this entity was marked as deleted
                bool wasDeleted = false;
                for (const auto& deletedData : prefabComp.deletedEntities) {
                    if (deletedData.prefabLocalID == prefabEntity.localID) {
                        wasDeleted = true;
                        break;
                    }
                }

                if (wasDeleted) {
                    LOG_INFO("Restoring deleted entity: ", prefabEntity.name);

                    // Create entity
                    Entity restoredEntity = scene->CreateEntity(prefabEntity.name);
                    if (!restoredEntity) continue;

                    // Add TagComponent first
                    auto& tag = restoredEntity.AddComponent<TagComponent>();
                    tag.Tag = prefabEntity.name;

                    // Add TransformComponent
                    restoredEntity.AddComponent<TransformComponent>();

                    // Deserialize all components from prefab (except Prefab component)
                    for (const auto& componentData : prefabEntity.components) {
                        if (componentData.type == ComponentTypeID::Prefab ||
                            componentData.type == ComponentTypeID::Tag) {
                            continue;
                        }

                        std::string jsonStr(componentData.serializedData.begin(),
                            componentData.serializedData.end());

                        if (!jsonStr.empty() && jsonStr != "{}") {
                            ComponentSerializer::DeserializeComponent(restoredEntity,
                                componentData.type, jsonStr);
                            LOG_DEBUG("  Restored component: ",
                                ComponentSerializer::GetComponentTypeName(componentData.type));
                        }
                    }

                    // Add PrefabComponent with correct localID
                    auto& restoredPrefabComp = restoredEntity.AddComponent<PrefabComponent>();
                    restoredPrefabComp.PrefabAssetGuid = prefabComp.PrefabAssetGuid;
                    restoredPrefabComp.prefabName = prefabComp.prefabName;
                    restoredPrefabComp.prefabVersion = prefabComp.prefabVersion;
                    restoredPrefabComp.isPrefabRoot = false;
                    restoredPrefabComp.prefabLocalID = prefabEntity.localID;

                    // Find and set parent
                    if (prefabEntity.parentLocalID != 0) {
                        auto parentIt = localIDToEntity.find(prefabEntity.parentLocalID);
                        if (parentIt != localIDToEntity.end()) {
                            Entity parentEntity = parentIt->second;
                            if (parentEntity.HasComponent<TransformComponent>()) {
                                auto& childTransform = restoredEntity.GetComponent<TransformComponent>();
                                auto& parentTransform = parentEntity.GetComponent<TransformComponent>();

                                // Set parent
                                childTransform.SetParent(parentEntity);
                                LOG_DEBUG("  Set parent for restored entity");
                            }
                        }
                    }
                    else {
                        // Parent to root
                        auto& childTransform = restoredEntity.GetComponent<TransformComponent>();
                        childTransform.SetParent(entity);
                        LOG_DEBUG("  Parented to root entity");
                    }

                    // Add to mapping
                    localIDToEntity[prefabEntity.localID] = restoredEntity;

                    // Add to root's childEntityIDs if it's a child
                    if (prefabEntity.parentLocalID == prefab.entities[0].localID) {
                        prefabComp.childEntityIDs.push_back(static_cast<u32>(restoredEntity.GetHandle()));
                        LOG_DEBUG("  Added to root's childEntityIDs");
                    }

                    LOG_INFO("Restored entity: ", prefabEntity.name);
                }
                else {
                    // Entity missing but NOT marked as deleted
                    LOG_WARNING("Prefab entity not found in scene and not marked as deleted: ",
                        prefabEntity.name, " (localID: ", prefabEntity.localID, ")");
                }
            }
        }

        // Clear deleted entities list since we've restored them
        prefabComp.deletedEntities.clear();
        LOG_INFO("Cleared deleted entities list after restoration");


        // Rebuild hierarchy relationships
        LOG_INFO("===== REBUILDING PREFAB HIERARCHY =====");
        for (const auto& prefabEntity : prefab.entities) {
            bool shouldExistInScene = true;
            if (prefabEntity.parentLocalID == 0) {
                shouldExistInScene = true; // Root always exists
            }
            else if (prefabEntity.parentLocalID == 1) { 
                shouldExistInScene = false; // Default to false

                // Find the root entity in scene
                Entity rootEntity = entity; // Assuming 'entity' is the root

                if (rootEntity.HasComponent<PrefabComponent>()) {
                    auto& rootPrefabComp = rootEntity.GetComponent<PrefabComponent>();
                    for (u32 childHandle : rootPrefabComp.childEntityIDs) {
                        Entity childEntity(static_cast<entt::entity>(childHandle), &scene->GetRegistry());
                        if (childEntity && childEntity.HasComponent<TagComponent>()) {
                            std::string childName = childEntity.GetComponent<TagComponent>().Tag;
                            if (childName == prefabEntity.name) {
                                shouldExistInScene = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (!shouldExistInScene) {
                LOG_DEBUG("Prefab entity not in scene (may have been deleted): ",
                    prefabEntity.name, " (localID: ", prefabEntity.localID, ")");
                continue; // Skip this entity
            }
            auto it = localIDToEntity.find(prefabEntity.localID);
            if (it == localIDToEntity.end()) {
                LOG_WARNING("Could not find scene entity for prefab localID: ", prefabEntity.localID);
                continue;
            }

            Entity sceneEntity = it->second;

            if (!sceneEntity.HasComponent<TransformComponent>()) continue;

            auto& transform = sceneEntity.GetComponent<TransformComponent>();

            // Handle parent relationship
            if (prefabEntity.parentLocalID != 0) {
                auto parentIt = localIDToEntity.find(prefabEntity.parentLocalID);
                if (parentIt != localIDToEntity.end()) {
                    Entity parentEntity = parentIt->second;

                    if (parentEntity.HasComponent<TransformComponent>()) {
                        auto& parentTransform = parentEntity.GetComponent<TransformComponent>();
                        u32 childHandle = static_cast<u32>(sceneEntity.GetHandle());

                        // Set parent
                        transform.Parent = static_cast<u32>(parentEntity.GetHandle());

                        // Add to parent's children if not already there
                        if (std::find(parentTransform.Children.begin(),
                            parentTransform.Children.end(), childHandle) == parentTransform.Children.end()) {
                            parentTransform.Children.push_back(childHandle);
                        }

                        LOG_DEBUG("Connected: ", prefabEntity.name, " -> parent localID ",
                            prefabEntity.parentLocalID);
                    }
                }
            }
        }
        LOG_INFO("===== FINISHED REBUILDING HIERARCHY =====");

        // Revert component-level overrides for all entities
        for (const auto& prefabEntity : prefab.entities) {
            auto it = localIDToEntity.find(prefabEntity.localID);
            if (it != localIDToEntity.end()) {
                Entity sceneEntity = it->second;

                if (!sceneEntity.HasComponent<PrefabComponent>()) continue;

                auto& entityPrefabComp = sceneEntity.GetComponent<PrefabComponent>();

                LOG_INFO("Reverting entity: ", prefabEntity.name);

                // Revert all modified components
                for (const auto& override : entityPrefabComp.componentOverrides) {
                    if (override.componentType == ComponentTypeID::Prefab) continue;

                    if (!override.originalComponentJSON.empty()) {
                        ComponentSerializer::DeserializeComponent(sceneEntity,
                            override.componentType, override.originalComponentJSON);
                        LOG_INFO("Reverted component: ",
                            ComponentSerializer::GetComponentTypeName(override.componentType));
                    }
                }

                // Clear all overrides
                entityPrefabComp.ClearAllOverrides();
            }
        }

        // Restore original component data
        LOG_INFO("===== STORING ORIGINAL COMPONENT DATA FOR ALL ENTITIES =====");
        for (const auto& prefabEntity : prefab.entities) {
            auto it = localIDToEntity.find(prefabEntity.localID);
            if (it == localIDToEntity.end()) {
                LOG_WARNING("Could not find instantiated entity for prefab localID: ", prefabEntity.localID);
                continue;
            }

            Entity sceneEntity = it->second;

            if (!sceneEntity.HasComponent<PrefabComponent>()) {
                sceneEntity.AddComponent<PrefabComponent>();
            }

            auto& entityPrefabComp = sceneEntity.GetComponent<PrefabComponent>();

            LOG_INFO("Processing entity: ", prefabEntity.name, " (localID: ", prefabEntity.localID, ")");

            // Store original component data
            for (const auto& componentData : prefabEntity.components) {
                if (componentData.type == ComponentTypeID::Prefab) continue;

                std::string jsonStr = ComponentSerializer::SerializeComponent(sceneEntity, componentData.type);

                if (!jsonStr.empty() && jsonStr != "{}") {
                    entityPrefabComp.StoreOriginalComponent(componentData.type, jsonStr);
                }
            }
        }
        LOG_INFO("===== FINISHED STORING ORIGINAL COMPONENT DATA FOR ALL ENTITIES =====");

        LOG_INFO("===== REVERT COMPLETE =====");
        return true;
    }


    bool PrefabInstantiator::ApplyOverridesToPrefab(Entity entity, Scene* scene) {
        if (!entity.HasComponent<PrefabComponent>()) {
            LOG_ERROR("Entity does not have PrefabComponent");
            return false;
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();

        if (!prefabComp.isPrefabRoot) {
            LOG_ERROR("Can only apply overrides from prefab root entity");
            return false;
        }

        // Load the prefab
        Prefab prefab;
        if (!PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
            LOG_ERROR("Failed to load prefab for GUID: ", prefabComp.PrefabAssetGuid.m_Value);
            return false;
        }

        LOG_INFO("===== APPLYING OVERRIDES FROM ENTIRE PREFAB HIERARCHY =====");
        LOG_INFO("Prefab: ", prefab.name);
        LOG_INFO("Current entities in prefab: ", prefab.entities.size());
        LOG_INFO("Current child entities in instance: ", prefabComp.childEntityIDs.size());
        UpdateExistingEntitiesPrefabLocalID(entity, scene, prefab);
    
        std::unordered_map<u32, u64> sceneHandleToPrefabLocalID;

        // Root entity mapping
        sceneHandleToPrefabLocalID[static_cast<u32>(entity.GetHandle())] = prefab.entities[0].localID;

        // Child entities mapping - use only what we actually have in the scene
        for (u32 childID : prefabComp.childEntityIDs) {
            Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
            if (!childEntity) continue;
            sceneHandleToPrefabLocalID[childID] = 0; // Placeholder, will be filled in later
        }

        // Apply overrides to existing entities
        LOG_INFO("Step 1: Applying overrides to existing entities");
        ApplyEntityOverrides(entity, scene, &prefab.entities[0]);

        for (u32 childID : prefabComp.childEntityIDs) {
            Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
            if (!childEntity) continue;
            PrefabEntityData* childPrefabData = nullptr;

            if (childEntity.HasComponent<TagComponent>()) {
                std::string childName = childEntity.GetComponent<TagComponent>().Tag;
                for (auto& prefabEntity : prefab.entities) {
                    if (prefabEntity.name == childName && prefabEntity.parentLocalID == prefab.entities[0].localID) {
                        childPrefabData = &prefabEntity;
                        sceneHandleToPrefabLocalID[childID] = prefabEntity.localID;

                        // FIX 1: Set prefabLocalID on scene entity if it exists
                        if (childEntity.HasComponent<PrefabComponent>()) {
                            auto& childPrefabComp = childEntity.GetComponent<PrefabComponent>();
                            childPrefabComp.prefabLocalID = prefabEntity.localID;
                        }
                        break;
                    }
                }
            }

            if (childPrefabData) {
                ApplyEntityOverrides(childEntity, scene, childPrefabData);
            }
        }

        LOG_INFO("Step 2: Removing deleted entities from prefab");

        // Create set of scene entity handles that exist
        std::set<u32> existingSceneHandles;
        existingSceneHandles.insert(static_cast<u32>(entity.GetHandle()));
        for (u32 childID : prefabComp.childEntityIDs) {
            existingSceneHandles.insert(childID);
        }

        std::set<u64> prefabLocalIDsToKeep;
        prefabLocalIDsToKeep.insert(prefab.entities[0].localID); // Always keep root

        for (const auto& [sceneHandle, prefabLocalID] : sceneHandleToPrefabLocalID) {
            if (prefabLocalID != 0) { // 0 is placeholder
                prefabLocalIDsToKeep.insert(prefabLocalID);
            }
        }

        for (int i = static_cast<int>(prefab.entities.size()) - 1; i >= 1; --i) {
            if (prefabLocalIDsToKeep.find(prefab.entities[i].localID) == prefabLocalIDsToKeep.end()) {
                LOG_INFO("Removing entity from prefab: ", prefab.entities[i].name,
                    " (localID: ", prefab.entities[i].localID, ")");
                prefab.entities.erase(prefab.entities.begin() + i);
            }
        }
        LOG_INFO("Step 3: Adding new entities to prefab");
        u32 nextLocalID = 1;
        for (const auto& prefabEntity : prefab.entities) {
            if (prefabEntity.localID >= nextLocalID) {
                nextLocalID = prefabEntity.localID + 1;
            }
        }

        for (u32 childID : prefabComp.childEntityIDs) {
            // Check if this child is already mapped to a prefab entity
            bool alreadyInPrefab = false;
            for (auto& prefabEntity : prefab.entities) {
                if (sceneHandleToPrefabLocalID.count(childID) &&
                    sceneHandleToPrefabLocalID[childID] == prefabEntity.localID &&
                    sceneHandleToPrefabLocalID[childID] != 0) {
                    alreadyInPrefab = true;
                    break;
                }
            }

            if (!alreadyInPrefab) {
                // This is a NEW entity
                Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
                if (!childEntity) continue;

                std::string childName = childEntity.HasComponent<TagComponent>()
                    ? childEntity.GetComponent<TagComponent>().Tag
                    : "Unknown";

                LOG_INFO("Adding new entity: ", childName);

                // Create new prefab entity data
                PrefabEntityData newEntityData;
                newEntityData.name = childName;
                newEntityData.localID = nextLocalID++;

                // FIX 2: Set prefabLocalID on the scene entity
                if (childEntity.HasComponent<PrefabComponent>()) {
                    auto& childPrefabComp = childEntity.GetComponent<PrefabComponent>();
                    childPrefabComp.prefabLocalID = newEntityData.localID;
                    LOG_DEBUG("Set prefabLocalID on new entity: ", newEntityData.localID);
                }

                // Find parent's localID in prefab
                newEntityData.parentLocalID = 0; // Default to no parent

                if (childEntity.HasComponent<TransformComponent>()) {
                    auto& transform = childEntity.GetComponent<TransformComponent>();

                    if (transform.Parent != u32_max) {
                        Entity parentEntity(static_cast<entt::entity>(transform.Parent), &scene->GetRegistry());

                        // Find parent's localID
                        if (static_cast<u32>(parentEntity.GetHandle()) == static_cast<u32>(entity.GetHandle())) {
                            // Parent is the root
                            newEntityData.parentLocalID = prefab.entities[0].localID;
                        }
                        else if (sceneHandleToPrefabLocalID.count(static_cast<u32>(parentEntity.GetHandle()))) {
                            // Parent is another entity in prefab
                            newEntityData.parentLocalID = sceneHandleToPrefabLocalID[static_cast<u32>(parentEntity.GetHandle())];
                        }

                        LOG_INFO("  Parent localID: ", newEntityData.parentLocalID);
                    }
                }

                // Serialize all components
                if (childEntity.HasComponent<TagComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Tag));
                }
                if (childEntity.HasComponent<TransformComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Transform));
                }

                if (childEntity.HasComponent<MeshRendererComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::MeshRenderer));
                }

                if (childEntity.HasComponent<CameraComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Camera));
                }

                if (childEntity.HasComponent<RigidbodyComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::RigidBody));
                }

                if (childEntity.HasComponent<LightComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Light));
                }

                if (childEntity.HasComponent<AudioComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Audio));
                }

                if (childEntity.HasComponent<ListenerComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Listerner));
                }

                if (childEntity.HasComponent<ReverbZoneComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::ReverbZone));
                }

                if (childEntity.HasComponent<BehaviourTreeComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::BehaviourTree));
                }

                if (childEntity.HasComponent<ParticleComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::ParticleSystem));
                }

                if (childEntity.HasComponent<ScriptComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Script));
                }

                if (childEntity.HasComponent<AnimatorComponent>()) {
                    newEntityData.components.push_back(
                        PrefabSerializer::SerializeEntityComponent(childEntity, ComponentTypeID::Animator));
                }

                // Add to prefab
                prefab.entities.push_back(newEntityData);

                // Track this newly added entity
                sceneHandleToPrefabLocalID[childID] = newEntityData.localID;

                LOG_INFO("  Added with localID: ", newEntityData.localID);
            }
        }

        // Increment prefab version
        prefab.version++;
        prefabComp.prefabVersion = prefab.version;
        LOG_INFO("Updated prefab version to: ", prefab.version);

        // Save the modified prefab back to disk
        std::string prefabPath = PrefabRegistry::Get().GetPrefabPath(prefabComp.PrefabAssetGuid);
        if (prefabPath.empty()) {
            LOG_ERROR("Could not find prefab path for GUID: ", prefabComp.PrefabAssetGuid.m_Value);
            return false;
        }

        LOG_INFO("Total entities in prefab: ", prefab.entities.size());

        if (!PrefabSerializer::SerializePrefab(prefab, prefabPath)) {
            LOG_ERROR("Failed to save prefab to: ", prefabPath);
            return false;
        }

        LOG_INFO("Successfully applied all overrides to prefab: ", prefab.name);

        // Update stored original component data to match new prefab state
        std::unordered_map<u64, Entity> localIDToEntity;

        // Map root entity
        localIDToEntity[prefab.entities[0].localID] = entity;

        // Map child entities using our tracking map
        for (const auto& [sceneHandle, prefabLocalID] : sceneHandleToPrefabLocalID) {
            if (sceneHandle != static_cast<u32>(entity.GetHandle()) && prefabLocalID != 0) {
                Entity childEntity(static_cast<entt::entity>(sceneHandle), &scene->GetRegistry());
                if (childEntity) {
                    localIDToEntity[prefabLocalID] = childEntity;
                    LOG_DEBUG("Mapped entity with localID: ", prefabLocalID);
                }
            }
        }

        LOG_DEBUG("Total entities in localIDToEntity map: ", localIDToEntity.size());

        StoreOriginalComponentDataForAllEntities(scene, prefab, localIDToEntity);

        LOG_INFO("===== FINISHED APPLYING OVERRIDES =====\n\n");

        return true;
    }


    void PrefabInstantiator::RemoveComponentByTypeID(Entity entity, ComponentTypeID type) {
        switch (type) {
        case ComponentTypeID::Transform:
            if (entity.HasComponent<TransformComponent>())
                entity.RemoveComponent<TransformComponent>();
            break;
        case ComponentTypeID::MeshRenderer:
            if (entity.HasComponent<MeshRendererComponent>())
                entity.RemoveComponent<MeshRendererComponent>();
            break;
        case ComponentTypeID::Camera:
            if (entity.HasComponent<CameraComponent>())
                entity.RemoveComponent<CameraComponent>();
            break;
        case ComponentTypeID::Light:
            if (entity.HasComponent<LightComponent>())
                entity.RemoveComponent<LightComponent>();
            break;
        case ComponentTypeID::RigidBody:
            if (entity.HasComponent<RigidbodyComponent>())
                entity.RemoveComponent<RigidbodyComponent>();
            break;
        case ComponentTypeID::Audio:
            if (entity.HasComponent<AudioComponent>())
                entity.RemoveComponent<AudioComponent>();
            break;
        case ComponentTypeID::Listerner:
            if (entity.HasComponent<ListenerComponent>())
                entity.RemoveComponent<ListenerComponent>();
            break;
        case ComponentTypeID::ReverbZone:
            if (entity.HasComponent<ReverbZoneComponent>())
                entity.RemoveComponent<ReverbZoneComponent>();
            break;
        case ComponentTypeID::BehaviourTree:
            if (entity.HasComponent<BehaviourTreeComponent>())
                entity.RemoveComponent<BehaviourTreeComponent>();
            break;
        case ComponentTypeID::ParticleSystem:
            if (entity.HasComponent<ParticleComponent>())
                entity.RemoveComponent<ParticleComponent>();
            break;
        case ComponentTypeID::Script:
            if (entity.HasComponent<ScriptComponent>())
                entity.RemoveComponent<ScriptComponent>();
            break;
        case ComponentTypeID::Animator:
            if (entity.HasComponent<AnimatorComponent>())
                entity.RemoveComponent<AnimatorComponent>();
            break;
        default:
            LOG_WARNING("Unknown component type for removal: ", static_cast<u32>(type));
            break;
        }
    }

   
    void PrefabInstantiator::StoreOriginalComponentDataForAllEntities(
        Scene* scene,
        const Prefab& prefab,
        const std::unordered_map<u64, Entity>& localIDToEntity) {

        LOG_INFO("===== STORING ORIGINAL COMPONENT DATA FOR ALL ENTITIES =====");

        // Iterate through all entities in the prefab
        for (const auto& prefabEntity : prefab.entities) {
            // Find the corresponding instantiated entity
            auto it = localIDToEntity.find(prefabEntity.localID);
            if (it == localIDToEntity.end()) {
                LOG_WARNING("Could not find instantiated entity for prefab localID: ", prefabEntity.localID);
                continue;
            }

            Entity sceneEntity = it->second;

            LOG_INFO("Processing entity: ", prefabEntity.name, " (localID: ", prefabEntity.localID, ")");

            // Ensure entity has PrefabComponent to store original data
            if (!sceneEntity.HasComponent<PrefabComponent>()) {
                sceneEntity.AddComponent<PrefabComponent>();
            }

            auto& prefabComp = sceneEntity.GetComponent<PrefabComponent>();

            for (const auto& componentData : prefabEntity.components) {
                if (componentData.type == ComponentTypeID::Prefab) {
                    continue; // Skip PrefabComponent
                }

                std::string componentName = ComponentSerializer::GetComponentTypeName(componentData.type);

               // to fix the serialize current entity component with special Transform handle
                std::string jsonStr;
                if (componentData.type == ComponentTypeID::Transform && sceneEntity.HasComponent<TransformComponent>()) {
                    // Temporarily clear children for serialization
                    auto& transform = sceneEntity.GetComponent<TransformComponent>();
                    std::vector<u32> originalChildren = transform.Children;
                    transform.Children.clear();

                    jsonStr = ComponentSerializer::SerializeComponent(sceneEntity, componentData.type);

                    // Restore children
                    transform.Children = originalChildren;

                    LOG_DEBUG("    Stored Transform without children");
                }
                else {
                    jsonStr = ComponentSerializer::SerializeComponent(sceneEntity, componentData.type);
                }

                if (jsonStr.empty() || jsonStr == "{}") {
                    LOG_WARNING("Failed to serialize component: ", componentName, " in entity: ", prefabEntity.name);
                    continue;
                }

                // Store the serialized JSON as the original state
                prefabComp.StoreOriginalComponent(componentData.type, jsonStr);
            }
        }

        LOG_INFO("===== FINISHED STORING ORIGINAL COMPONENT DATA FOR ALL ENTITIES =====");
    }

    void PrefabInstantiator::RevertEntityAndChildren(Entity entity, Scene* scene, const Prefab& prefab) {
        if (!entity.HasComponent<PrefabComponent>()) {
            return;
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();

        // Find this entity's data in the prefab
        const PrefabEntityData* entityData = nullptr;

        // If it's the root, get root entity
        if (prefabComp.isPrefabRoot) {
            entityData = prefab.GetRootEntity();
        }
        else {

            LOG_WARNING("Reverting child entity - using stored component data");
        }

        if (!entityData && prefabComp.isPrefabRoot) {
            LOG_ERROR("Could not find entity data in prefab");
            return;
        }

        LOG_INFO("Reverting entity: ", entity.HasComponent<TagComponent>() ?
            entity.GetComponent<TagComponent>().Tag : "Unknown");

        // Restore removed components
        auto removedComponents = prefabComp.GetRemovedComponents();
        for (ComponentTypeID removedType : removedComponents) {
            std::string originalJSON = prefabComp.GetOriginalComponentJSON(removedType);

            if (!originalJSON.empty()) {
                if (ComponentSerializer::DeserializeComponent(entity, removedType, originalJSON)) {
                    LOG_INFO("Restored removed component: ",
                        ComponentSerializer::GetComponentTypeName(removedType));
                }
            }
        }

        // Remove added components
        auto addedComponents = prefabComp.GetAddedComponents();
        for (ComponentTypeID addedType : addedComponents) {
            // Check if component exists in prefab
            bool existsInPrefab = false;
            if (entityData) {
                for (const auto& prefabComponent : entityData->components) {
                    if (prefabComponent.type == addedType) {
                        existsInPrefab = true;
                        break;
                    }
                }
            }

            if (!existsInPrefab) {
                RemoveComponentByTypeID(entity, addedType);
                LOG_INFO("Removed added component: ",
                    ComponentSerializer::GetComponentTypeName(addedType));
            }
        }

        // Revert all modified components using stored original data
        for (const auto& override : prefabComp.componentOverrides) {
            if (override.componentType == ComponentTypeID::Prefab) {
                continue;
            }

            // Use stored original JSON to revert
            if (!override.originalComponentJSON.empty()) {
                if (ComponentSerializer::DeserializeComponent(entity, override.componentType,
                    override.originalComponentJSON)) {
                    LOG_INFO("Reverted component: ",
                        ComponentSerializer::GetComponentTypeName(override.componentType));
                }
            }
        }

        // Clear all overrides
        prefabComp.ClearAllOverrides();

        LOG_INFO("Successfully reverted entity");

    }
    void PrefabInstantiator::ApplyEntityOverrides(Entity entity, Scene* scene, PrefabEntityData* entityData) {
        if (!entity.HasComponent<PrefabComponent>() || !entityData) {
            return;
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();

        LOG_INFO("Applying overrides for entity: ",
            entity.HasComponent<TagComponent>() ? entity.GetComponent<TagComponent>().Tag : "Unknown");

        // Handle removed components
        auto removedComponents = prefabComp.GetRemovedComponents();
        for (ComponentTypeID removedType : removedComponents) {
            auto& components = entityData->components;
            components.erase(
                std::remove_if(components.begin(), components.end(),
                    [removedType](const PrefabComponentData& c) { return c.type == removedType; }),
                components.end()
            );
            LOG_INFO("  Removed component from prefab: ",
                ComponentSerializer::GetComponentTypeName(removedType));
        }

        // Handle added components
        auto addedComponents = prefabComp.GetAddedComponents();
        for (ComponentTypeID addedType : addedComponents) {
            // Check if component already exists in prefab
            bool existsInPrefab = false;
            for (const auto& prefabComponent : entityData->components) {
                if (prefabComponent.type == addedType) {
                    existsInPrefab = true;
                    break;
                }
            }

            // Only add if it doesn't already exist in the prefab
            if (!existsInPrefab) {
                // Serialize the newly added component from the scene entity
                std::string currentJSON;
                if (addedType == ComponentTypeID::Transform) {
                    // Temporarily clear children for serialization
                    auto& transform = entity.GetComponent<TransformComponent>();
                    std::vector<u32> originalChildren = transform.Children;
                    transform.Children.clear();

                    currentJSON = ComponentSerializer::SerializeComponent(entity, addedType);

                    // Restore children
                    transform.Children = originalChildren;
                }
                else {
                    currentJSON = ComponentSerializer::SerializeComponent(entity, addedType);
                }

                if (!currentJSON.empty() && currentJSON != "{}") {
                    PrefabComponentData newComponent;
                    newComponent.type = addedType;
                    newComponent.typeName = ComponentSerializer::GetComponentTypeName(addedType);
                    newComponent.serializedData.assign(currentJSON.begin(), currentJSON.end());

                    entityData->components.push_back(newComponent);

                    LOG_INFO("  Added NEW component to prefab: ",
                        ComponentSerializer::GetComponentTypeName(addedType),
                        " (", currentJSON.length(), " bytes)");
                }
                else {
                    LOG_WARNING("  Failed to serialize added component: ",
                        ComponentSerializer::GetComponentTypeName(addedType));
                }
            }
        }

        // Apply modified components
        for (const auto& override : prefabComp.componentOverrides) {
            if (override.isAddedComponent || override.isRemovedComponent) {
                continue;
            }

            if (!override.HasOverrides()) {
                continue;
            }

            std::string currentJSON;
            if (override.componentType == ComponentTypeID::Transform) {
                // Temporarily clear children for serialization
                auto& transform = entity.GetComponent<TransformComponent>();
                std::vector<u32> originalChildren = transform.Children;
                transform.Children.clear();

                currentJSON = ComponentSerializer::SerializeComponent(entity, override.componentType);

                // Restore children
                transform.Children = originalChildren;

                LOG_INFO("  Serialized Transform without children - JSON length: ", currentJSON.length());
            }
            else {
                currentJSON = ComponentSerializer::SerializeComponent(entity, override.componentType);
            }

            bool componentFound = false;
            for (auto& prefabComponent : entityData->components) {
                if (prefabComponent.type == override.componentType) {
                    prefabComponent.serializedData.assign(currentJSON.begin(), currentJSON.end());
                    componentFound = true;
                    LOG_INFO("  Updated component in prefab: ",
                        ComponentSerializer::GetComponentTypeName(override.componentType));
                    break;
                }
            }

            if (!componentFound) {
                PrefabComponentData newComponent;
                newComponent.type = override.componentType;
                newComponent.typeName = ComponentSerializer::GetComponentTypeName(override.componentType);
                newComponent.serializedData.assign(currentJSON.begin(), currentJSON.end());
                entityData->components.push_back(newComponent);

                LOG_WARNING("  Component not found in prefab - adding as fallback: ",
                    ComponentSerializer::GetComponentTypeName(override.componentType));
            }
        }

        // Clear overrides
        prefabComp.ClearAllOverrides();
    }
    void PrefabInstantiator::RebuildPrefabHierarchy(const Prefab& prefab,
        const std::unordered_map<u64, Entity>& localIDToEntity,
        Scene* scene) {

        LOG_INFO("===== REBUILDING PREFAB HIERARCHY =====");

        // Iterate through all prefab entities and establish parent-child relationships
        for (const auto& prefabEntity : prefab.entities) {
            // Find the scene entity for this prefab entity
            auto it = localIDToEntity.find(prefabEntity.localID);
            if (it == localIDToEntity.end()) {
                LOG_WARNING("Could not find scene entity for prefab localID: ", prefabEntity.localID);
                continue;
            }

            Entity sceneEntity = it->second;

            if (!sceneEntity.HasComponent<TransformComponent>()) {
                LOG_WARNING("Entity missing TransformComponent: ", prefabEntity.name);
                continue;
            }

            auto& transform = sceneEntity.GetComponent<TransformComponent>();

            // Handle parent relationship
            if (prefabEntity.parentLocalID != 0) {
                // Find parent entity
                auto parentIt = localIDToEntity.find(prefabEntity.parentLocalID);
                if (parentIt != localIDToEntity.end()) {
                    Entity parentEntity = parentIt->second;

                    if (parentEntity && parentEntity.HasComponent<TransformComponent>()) {
                        auto& parentTransform = parentEntity.GetComponent<TransformComponent>();
                        u32 childHandle = static_cast<u32>(sceneEntity.GetHandle());

                        // Set parent
                        transform.Parent = static_cast<u32>(parentEntity.GetHandle());

                        // Add to parent's children if not already there
                        auto childIt = std::find(parentTransform.Children.begin(),
                            parentTransform.Children.end(), childHandle);
                        if (childIt == parentTransform.Children.end()) {
                            parentTransform.Children.push_back(childHandle);
                        }

                        LOG_DEBUG("Connected: ", prefabEntity.name, " -> parent localID ",
                            prefabEntity.parentLocalID);
                    }
                }
                else {
                    LOG_WARNING("Could not find parent entity for localID: ", prefabEntity.parentLocalID);
                }
            }
        }

        LOG_INFO("===== FINISHED REBUILDING HIERARCHY =====");
    }

    void  PrefabInstantiator::UpdateExistingEntitiesPrefabLocalID(Entity root, Scene* scene, const Prefab& prefab) {
        std::queue<Entity> queue;
        queue.push(root);

        while (!queue.empty()) {
            Entity current = queue.front();
            queue.pop();

            if (!current.HasComponent<PrefabComponent>()) {
                continue;
            }

            auto& prefabComp = current.GetComponent<PrefabComponent>();

            // Find this entity in the prefab by name
            if (current.HasComponent<TagComponent>()) {
                std::string name = current.GetComponent<TagComponent>().Tag;

                for (const auto& prefabEntity : prefab.entities) {
                    if (prefabEntity.name == name) {
                        // Update the prefabLocalID
                        prefabComp.prefabLocalID = prefabEntity.localID;
                        LOG_DEBUG("Updated prefabLocalID for ", name, " to ", prefabEntity.localID);
                        break;
                    }
                }
            }

            // Add children to queue
            if (current.HasComponent<TransformComponent>()) {
                auto& transform = current.GetComponent<TransformComponent>();
                for (u32 childHandle : transform.Children) {
                    Entity childEntity(static_cast<entt::entity>(childHandle), &scene->GetRegistry());
                    if (childEntity) {
                        queue.push(childEntity);
                    }
                }
            }
        }
    }
}
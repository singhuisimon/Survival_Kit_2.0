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
    bool PrefabInstantiator::CreatePrefabFromEntity(Entity rootEntity, const std::string& prefabName, const std::string& savePath) {
        LOG_INFO("===== Start of PrefabInstantiator::CreatePrefabFromEntity ==== ");
        if (!rootEntity) {
            LOG_ERROR("Cannot create prefab from invalid entity");
            return false;
        }

        if (!rootEntity.HasComponent<TagComponent>()) {
            LOG_ERROR("Entity must have TagComponent to create prefab");
            return false;
        }

        LOG_INFO("Creating prefab '", prefabName, "' from entity...");
        // check if file path exists
     
        if (!PrefabSerializer::SerializeEntityToPrefabFile(rootEntity, prefabName, savePath)) {
            LOG_ERROR("Failed to create and save prefab from entity: ", prefabName.c_str());
            return false;
        }
      

        LOG_INFO("Successfully created prefab: ", prefabName.c_str());
        LOG_INFO("===== End of PrefabInstantiator::CreatePrefabFromEntity ==== ");
        return true;
    }

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
        if (!scene || !prefab.IsValid()) {
            LOG_ERROR("Cannot instantiate invalid prefab into null scene");
            return Entity{};
        }
        if (!PrefabRegistry::Get().IsPrefabRegistered(prefab.guid)) {
            LOG_WARNING("Prefab not yet registered: ", prefab.name.c_str());
            std::string prefabPath = "Resources/Prefabs/" + prefab.name + ".prefab";
            PrefabRegistry::Get().RegisterPrefab(prefab.guid, prefabPath, prefab.name);
            LOG_INFO("Auto-registered prefab: ", prefab.name.c_str());
        }

        const PrefabEntityData* rootData = prefab.GetRootEntity();
        if (!rootData) {
            LOG_ERROR("Prefab has no root entity");
            return Entity{};
        }

        LOG_INFO("===== INSTANTIATING PREFAB =====");
        LOG_INFO("Prefab: ", prefab.name);
        LOG_INFO("Root entity in prefab: ", rootData->name);
        LOG_INFO("Total components: ", rootData->components.size());

        std::unordered_map<u64, Entity> localIDToEntity;

        // Recursively instantiate the hierarchy
        Entity rootEntity = InstantiateEntity(scene, *rootData, prefab, parent, localIDToEntity);

        if (!rootEntity) {
            LOG_ERROR("Failed to instantiate prefab root entity");
            return Entity{};
        }

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
        prefabComp->isPrefabRoot = true;

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

        return rootEntity;
    }

    Entity PrefabInstantiator::InstantiateEntity(Scene* scene,
        const PrefabEntityData& entityData,
        const Prefab& prefab,
        Entity sceneParent,  // Scene parent, NOT prefab parent!
        std::unordered_map<u64, Entity>& localIDToEntity) {
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
                //  Clear the children array and reset parent 
                if (entity.HasComponent<TransformComponent>()) {
                    auto& transform = entity.GetComponent<TransformComponent>();
                    transform.Children.clear(); // Clear prefab's children IDs
                    transform.Parent = u32_max; // Reset parent - we'll set it properly later
                    LOG_DEBUG("  Cleared Transform relationships (will rebuild with scene handles)");
                }
            }
            else {
                // Deserialize other components normally
                if (!ComponentSerializer::DeserializeComponent(entity, componentData.type, jsonStr)) {
                    LOG_ERROR("Failed to deserialize component: ", componentData.typeName);
                }
            }
        }

        // 4. Add PrefabComponent to ALL entities in the hierarchy (CRITICAL FIX!)
        if (!entity.HasComponent<PrefabComponent>()) {
            entity.AddComponent<PrefabComponent>();
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();
        prefabComp.PrefabAssetGuid = prefab.guid;
        prefabComp.prefabName = prefab.name;
        prefabComp.prefabVersion = prefab.version;
        prefabComp.isPrefabRoot = (entityData.parentLocalID == 0); // Root if parentID is 0

        LOG_DEBUG("  Added PrefabComponent - isRoot: ", prefabComp.isPrefabRoot);

        auto children = prefab.GetChildren(entityData.localID);
        LOG_DEBUG("  Entity has ", children.size(), " children in prefab");

        for (const auto* childData : children) {
            // Recursively instantiate child (pass Entity{} as sceneParent for now)
            Entity childEntity = InstantiateEntity(scene, *childData, prefab, Entity{}, localIDToEntity);
            if (!childEntity) {
                LOG_ERROR("Failed to instantiate child entity: ", childData->name);
            }
            else {
                LOG_DEBUG("  Successfully created child: ", childData->name);
            }
        }

        // 5. Set up PREFAB INTERNAL parent relationship (CRITICAL FIX!)
        if (entityData.parentLocalID != 0) {  // 0 = root in prefab
            auto parentIt = localIDToEntity.find(entityData.parentLocalID);
            if (parentIt != localIDToEntity.end()) {
                Entity prefabParent = parentIt->second;  // Parent WITHIN the prefab

                if (entity.HasComponent<TransformComponent>() &&
                    prefabParent.HasComponent<TransformComponent>()) {
                    auto& childTransform = entity.GetComponent<TransformComponent>();
                    auto& parentTransform = prefabParent.GetComponent<TransformComponent>();
                    childTransform.Parent = static_cast<u32>(prefabParent.GetHandle());
                    u32 childID = static_cast<u32>(entity.GetHandle());
                    auto it = std::find(parentTransform.Children.begin(),
                        parentTransform.Children.end(),
                        childID);
                    if (it == parentTransform.Children.end()) {
                        parentTransform.Children.push_back(childID);
                        LOG_DEBUG("  Added to parent's children list: parent=",
                            static_cast<u32>(prefabParent.GetHandle()),
                            " child=", childID);
                    }

                    //LOG_DEBUG("  Set prefab parent: localID ", entityData.parentLocalID);
                }
            }
            else {
                LOG_WARNING("  Prefab parent entity (localID: ", entityData.parentLocalID,
                    ") not found yet. Hierarchy might be broken.");
            }
        }

        // 6. Handle SCENE parent (only for root entity when called from InstantiatePrefab)
        if (sceneParent && entityData.parentLocalID == 0) {  // Only for root entity
            if (entity.HasComponent<TransformComponent>() &&
                sceneParent.HasComponent<TransformComponent>()) {

                auto& childTransform = entity.GetComponent<TransformComponent>();
                auto& parentTransform = sceneParent.GetComponent<TransformComponent>();

                childTransform.Parent = static_cast<u32>(sceneParent.GetHandle());

                u32 childID = static_cast<u32>(entity.GetHandle());
                auto it = std::find(parentTransform.Children.begin(),
                    parentTransform.Children.end(),
                    childID);
                if (it == parentTransform.Children.end()) {
                    parentTransform.Children.push_back(childID);
                }

                LOG_DEBUG("  Attached to scene parent: ", static_cast<u32>(sceneParent.GetHandle()));
            }
        }


        return entity;
    }

    // Optional: Create a separate function for component creation
    void PrefabInstantiator::CreateComponentFromPrefab(Entity entity,
        const PrefabComponentData& componentData) {
        // Convert binary data to JSON string
        std::string jsonStr(componentData.serializedData.begin(),
            componentData.serializedData.end());

        // Skip PrefabComponent
        if (componentData.type == ComponentTypeID::Prefab) {
            LOG_DEBUG("  Skipping PrefabComponent - added separately");
            return;
        }

        // Deserialize using ComponentSerializer
        if (!ComponentSerializer::DeserializeComponent(entity, componentData.type, jsonStr)) {
            LOG_ERROR("Failed to create component: ", componentData.typeName);
        }
        else {
            LOG_DEBUG("  Created component: ", componentData.typeName);
        }
    }

    void PrefabInstantiator::StoreOriginalComponentData(Entity entity, const Prefab& prefab) {
        if (!entity.HasComponent<PrefabComponent>()) {
            LOG_ERROR("Entity does not have PrefabComponent");
            return;
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();
        const PrefabEntityData* rootData = prefab.GetRootEntity();

        if (!rootData) {
            LOG_ERROR("Prefab has no root entity to store component data from");
            return;
        }

        LOG_INFO("===== STORING ORIGINAL COMPONENT DATA =====");
        LOG_INFO("Entity: ", entity.HasComponent<TagComponent>() ?
            entity.GetComponent<TagComponent>().Tag : "Unknown");
        LOG_INFO("Prefab: ", prefab.name);
        LOG_INFO("Total components in prefab: ", rootData->components.size());

        // Store original JSON for each component
        for (const auto& componentData : rootData->components) {
            if (componentData.type == ComponentTypeID::Prefab) {
                continue; // Skip PrefabComponent itself
            }

            std::string jsonStr(componentData.serializedData.begin(), componentData.serializedData.end());
            std::string componentName = ComponentSerializer::GetComponentTypeName(componentData.type);

            if (jsonStr.empty()) {
                LOG_WARNING("Empty JSON for component: ", componentName);
                continue;
            }

            prefabComp.StoreOriginalComponent(componentData.type, jsonStr);
            LOG_INFO("Stored original data for: ", componentName,
                " (", jsonStr.length(), " bytes)");
        }

        LOG_INFO("===== FINISHED STORING ORIGINAL COMPONENT DATA =====");
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

        LOG_INFO("===== REVERTING ENTIRE PREFAB HIERARCHY =====");
        LOG_INFO("Prefab: ", prefab.name);

        // Revert root entity
        RevertEntityAndChildren(entity, scene, prefab);

        // Revert all child entities
        for (u32 childID : prefabComp.childEntityIDs) {
            Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
            if (childEntity) {
                RevertEntityAndChildren(childEntity, scene, prefab);
            }
        }

        // Restore original component data for all entities
        LOG_INFO("Restoring original component data...");
        StoreOriginalComponentData(entity, prefab);

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

        // Build a map of existing prefab entities by their scene handle
        // We'll use the order to match them: root is first, then children in order
        std::unordered_map<u32, PrefabEntityData*> handleToPrefabEntity;

        // Root entity
        handleToPrefabEntity[static_cast<u32>(entity.GetHandle())] = &prefab.entities[0];

        // Map existing children (up to the number of entities in prefab)
        size_t childIndex = 0;
        for (u32 childID : prefabComp.childEntityIDs) {
            if (childIndex + 1 < prefab.entities.size()) {
                handleToPrefabEntity[childID] = &prefab.entities[childIndex + 1];
                childIndex++;
            }
            else {
                // This is a NEW entity that doesn't exist in prefab yet
                break;
            }
        }

        // Step 1: Apply overrides to existing entities
        LOG_INFO("Step 1: Applying overrides to existing entities");
        ApplyEntityOverrides(entity, scene, handleToPrefabEntity[static_cast<u32>(entity.GetHandle())]);

        for (u32 childID : prefabComp.childEntityIDs) {
            Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
            if (!childEntity) continue;

            auto it = handleToPrefabEntity.find(childID);
            if (it != handleToPrefabEntity.end()) {
                // Existing entity - apply overrides
                ApplyEntityOverrides(childEntity, scene, it->second);
            }
        }

        // Step 2: Add new entities to prefab
        LOG_INFO("Step 2: Adding new entities to prefab");
        u32 nextLocalID = static_cast<u32>(prefab.entities.size()) + 1;

        for (u32 childID : prefabComp.childEntityIDs) {
            auto it = handleToPrefabEntity.find(childID);
            if (it == handleToPrefabEntity.end()) {
                // This is a NEW entity
                Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
                if (!childEntity) continue;

                LOG_INFO("Adding new entity: ",
                    childEntity.GetComponent<TagComponent>().Tag);

                // Create new prefab entity data
                PrefabEntityData newEntityData;
                newEntityData.name = childEntity.GetComponent<TagComponent>().Tag;
                newEntityData.localID = nextLocalID++;

                // Find parent's localID in prefab
                if (childEntity.HasComponent<TransformComponent>()) {
                    auto& transform = childEntity.GetComponent<TransformComponent>();

                    if (transform.Parent != u32_max) {
                        Entity parentEntity(static_cast<entt::entity>(transform.Parent),
                            &scene->GetRegistry());

                        // Find parent's localID in prefab
                        auto parentIt = handleToPrefabEntity.find(static_cast<u32>(parentEntity.GetHandle()));
                        if (parentIt != handleToPrefabEntity.end()) {
                            newEntityData.parentLocalID = parentIt->second->localID;
                            LOG_INFO("  Parent localID: ", newEntityData.parentLocalID);
                        }
                        else {
                            // Parent is root
                            newEntityData.parentLocalID = 1; // Root's localID is 1
                            LOG_INFO("  Parent is root, parentLocalID: 1");
                        }
                    }
                }

                // Serialize all components
                if (childEntity.HasComponent<TransformComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Transform));
                }

                if (childEntity.HasComponent<TagComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Tag));
                }

                if (childEntity.HasComponent<MeshRendererComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::MeshRenderer));
                }

                if (childEntity.HasComponent<CameraComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Camera));
                }

                if (childEntity.HasComponent<RigidbodyComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::RigidBody));
                }

                if (childEntity.HasComponent<LightComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Light));
                }

                if (childEntity.HasComponent<AudioComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Audio));
                }

                if (childEntity.HasComponent<ListenerComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Listerner));
                }

                if (childEntity.HasComponent<ReverbZoneComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::ReverbZone));
                }

                if (childEntity.HasComponent<BehaviourTreeComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::BehaviourTree));
                }

                if (childEntity.HasComponent<ParticleComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::ParticleSystem));
                }

                if (childEntity.HasComponent<ScriptComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Script));
                }

                if (childEntity.HasComponent<AnimatorComponent>()) {
                    newEntityData.components.push_back(
                        SerializeEntityComponent(childEntity, ComponentTypeID::Animator));
                }

                // Add to prefab
                prefab.entities.push_back(newEntityData);

                // Add to map for potential children of this new entity
                handleToPrefabEntity[childID] = &prefab.entities.back();

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

        LOG_INFO("Saving prefab to: ", prefabPath.c_str());
        LOG_INFO("Total entities in prefab: ", prefab.entities.size());

        if (!PrefabSerializer::SerializePrefab(prefab, prefabPath)) {
            LOG_ERROR("Failed to save prefab to: ", prefabPath);
            return false;
        }

        LOG_INFO("Successfully applied all overrides to prefab: ", prefab.name);

        // Update stored original component data to match new prefab state
        std::unordered_map<u64, Entity> localIDToEntity;
        for (size_t i = 0; i < prefab.entities.size(); i++) {
            if (i == 0) {
                localIDToEntity[prefab.entities[i].localID] = entity;
            }
            else if (i - 1 < prefabComp.childEntityIDs.size()) {
                Entity childEntity(static_cast<entt::entity>(prefabComp.childEntityIDs[i - 1]),
                    &scene->GetRegistry());
                if (childEntity) {
                    localIDToEntity[prefab.entities[i].localID] = childEntity;
                }
            }
        }

        StoreOriginalComponentDataForAllEntities(scene, prefab, localIDToEntity);

        LOG_INFO("===== FINISHED APPLYING OVERRIDES =====");

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
    bool PrefabInstantiator::RevertComponentToPrefab(Entity entity, ComponentTypeID componentType) {
        if (!entity.HasComponent<PrefabComponent>()) {
            LOG_ERROR("Entity does not have PrefabComponent");
            return false;
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();
        std::string componentName = ComponentSerializer::GetComponentTypeName(componentType);

        LOG_INFO("===== REVERTING COMPONENT TO PREFAB STATE =====");
        LOG_INFO("Component: ", componentName);
        LOG_INFO("Entity: ", entity.HasComponent<TagComponent>() ?
            entity.GetComponent<TagComponent>().Tag : "Unknown");

        // Find the override for this component
        for (auto& override : prefabComp.componentOverrides) {
            if (override.componentType == componentType) {
                if (!override.originalComponentJSON.empty()) {
                    LOG_DEBUG("Original JSON size: ", override.originalComponentJSON.length(), " bytes");

                    // Deserialize original component data back to entity
                    if (ComponentSerializer::DeserializeComponent(entity, componentType, override.originalComponentJSON)) {
                        // Clear this specific override
                        prefabComp.ClearComponentOverride(componentType);
                        LOG_INFO("Successfully reverted component: ", componentName);
                        LOG_INFO("Component changes applied to scene immediately");
                        LOG_INFO("===== REVERT COMPLETE =====");
                        return true;
                    }
                    else {
                        LOG_ERROR("Failed to deserialize component: ", componentName);
                        LOG_ERROR("===== REVERT FAILED =====");
                        return false;
                    }
                }
                else {
                    LOG_WARNING("No original data stored for: ", componentName);
                    LOG_INFO("===== REVERT FAILED =====");
                    return false;
                }
            }
        }

        LOG_WARNING("No override found for component type: ", static_cast<u32>(componentType));
        LOG_INFO("===== REVERT FAILED =====");
        return false;
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

            // Store original component data by serializing the CURRENT state of components
            // (which now have been deserialized from the prefab)
            for (const auto& componentData : prefabEntity.components) {
                if (componentData.type == ComponentTypeID::Prefab) {
                    continue; // Skip PrefabComponent
                }

                std::string componentName = ComponentSerializer::GetComponentTypeName(componentData.type);

                // CRITICAL FIX: Serialize the current entity component to get its JSON
                // This is important because we need the properly formatted JSON that matches
                // what ComponentSerializer::DeserializeComponent expects
                std::string jsonStr = ComponentSerializer::SerializeComponent(sceneEntity, componentData.type);

                if (jsonStr.empty() || jsonStr == "{}") {
                    LOG_WARNING("Failed to serialize component: ", componentName, " in entity: ", prefabEntity.name);
                    continue;
                }

                // Store the serialized JSON as the original state
                prefabComp.StoreOriginalComponent(componentData.type, jsonStr);

                LOG_INFO("  Stored original data for: ", componentName,
                    " (", jsonStr.length(), " bytes)");
                LOG_DEBUG("    JSON: ", jsonStr.substr(0, 100)); // Log first 100 chars for verification
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
#if 0
    void PrefabInstantiator::ApplyEntityAndChildrenOverrides(Entity entity, Scene* scene, Prefab& prefab) {
        if (!entity.HasComponent<PrefabComponent>()) {
            return;
        }

        auto& prefabComp = entity.GetComponent<PrefabComponent>();

        LOG_INFO("Applying overrides from entity: ",
            entity.HasComponent<TagComponent>() ? entity.GetComponent<TagComponent>().Tag : "Unknown");

        // Find corresponding entity data in prefab by matching localID
        PrefabEntityData* mutableEntityData = nullptr;

        // For root entity
        if (prefabComp.isPrefabRoot) {
            mutableEntityData = const_cast<PrefabEntityData*>(prefab.GetRootEntity());
        }
        else {
          
            // Find the parent's PrefabComponent to get the child index
            if (entity.HasComponent<TransformComponent>()) {
                auto& transform = entity.GetComponent<TransformComponent>();

                if (transform.Parent != u32_max) {
                    Entity parentEntity(static_cast<entt::entity>(transform.Parent), &scene->GetRegistry());

                    if (parentEntity.HasComponent<PrefabComponent>()) {
                        auto& parentPrefabComp = parentEntity.GetComponent<PrefabComponent>();

                        // Find this entity's index in the parent's child list
                        u32 childIndex = 0;
                        u32 currentEntityHandle = static_cast<u32>(entity.GetHandle());

                        for (size_t i = 0; i < parentPrefabComp.childEntityIDs.size(); i++) {
                            if (parentPrefabComp.childEntityIDs[i] == currentEntityHandle) {
                                childIndex = static_cast<u32>(i);
                                break;
                            }
                        }

                        // Now find the corresponding entity in the prefab
                        // The child entities in the prefab come after the root
                        if (childIndex + 1 < prefab.entities.size()) {
                            mutableEntityData = &prefab.entities[childIndex + 1];
                            LOG_INFO("Found child entity in prefab at index: ", childIndex + 1);
                        }
                    }
                }
            }
        }

        if (!mutableEntityData) {
            LOG_ERROR("Could not find entity data in prefab");
            return;
        }

        // Handle removed components
        auto removedComponents = prefabComp.GetRemovedComponents();
        for (ComponentTypeID removedType : removedComponents) {
            auto& components = mutableEntityData->components;
            components.erase(
                std::remove_if(components.begin(), components.end(),
                    [removedType](const PrefabComponentData& c) { return c.type == removedType; }),
                components.end()
            );
            LOG_INFO("Removed component from prefab: ",
                ComponentSerializer::GetComponentTypeName(removedType));
        }

        // Handle added components
        auto addedComponents = prefabComp.GetAddedComponents();
        for (ComponentTypeID addedType : addedComponents) {
            bool exists = false;
            for (auto& prefabComponent : mutableEntityData->components) {
                if (prefabComponent.type == addedType) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                std::string currentJSON = ComponentSerializer::SerializeComponent(entity, addedType);
                if (!currentJSON.empty()) {
                    PrefabComponentData newComponent;
                    newComponent.type = addedType;
                    newComponent.typeName = ComponentSerializer::GetComponentTypeName(addedType);
                    newComponent.serializedData.assign(currentJSON.begin(), currentJSON.end());
                    mutableEntityData->components.push_back(newComponent);
                    LOG_INFO("Added new component to prefab: ",
                        ComponentSerializer::GetComponentTypeName(addedType));
                }
            }
        }

        // Apply modified components
        for (const auto& override : prefabComp.componentOverrides) {
            if (override.isAddedComponent || override.isRemovedComponent) {
                continue; // Already handled above
            }

            if (!override.HasOverrides()) {
                continue;
            }

            std::string currentJSON = ComponentSerializer::SerializeComponent(entity, override.componentType);

            bool componentFound = false;
            for (auto& prefabComponent : mutableEntityData->components) {
                if (prefabComponent.type == override.componentType) {
                    prefabComponent.serializedData.assign(currentJSON.begin(), currentJSON.end());
                    componentFound = true;
                    LOG_INFO("Updated component in prefab: ",
                        ComponentSerializer::GetComponentTypeName(override.componentType));
                    break;
                }
            }

            if (!componentFound && !currentJSON.empty()) {
                PrefabComponentData newComponent;
                newComponent.type = override.componentType;
                newComponent.typeName = ComponentSerializer::GetComponentTypeName(override.componentType);
                newComponent.serializedData.assign(currentJSON.begin(), currentJSON.end());
                mutableEntityData->components.push_back(newComponent);
                LOG_INFO("Added modified component to prefab: ",
                    ComponentSerializer::GetComponentTypeName(override.componentType));
            }
        }

        // Clear this entity's overrides
        prefabComp.ClearAllOverrides();
    }
#endif

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
                std::string currentJSON = ComponentSerializer::SerializeComponent(entity, addedType);

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

            std::string currentJSON = ComponentSerializer::SerializeComponent(entity, override.componentType);

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

    PrefabComponentData PrefabInstantiator::SerializeEntityComponent(Entity entity, ComponentTypeID type) {
        PrefabComponentData data;
        data.type = type;
        data.typeName = ComponentSerializer::GetComponentTypeName(type);

        std::string jsonStr = ComponentSerializer::SerializeComponent(entity, type);
        data.serializedData.assign(jsonStr.begin(), jsonStr.end());

        return data;
    }
}
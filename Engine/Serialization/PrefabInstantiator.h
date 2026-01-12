#pragma once
#ifndef __PREFAB_INSTANTIATOR_H__
#define __PREFAB_INSTANTIATOR_H__

#include "../Prefab/Prefab.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include <entt/entt.hpp>
#include <string>
#include <rapidjson/document.h>

// Forward declare rapidjson types to avoid including full rapidjson in header
namespace rapidjson {
    template<typename Encoding, typename Allocator> class GenericValue;
    template<typename BaseAllocator> class MemoryPoolAllocator;
    template<typename Encoding> struct UTF8;
}
namespace Engine {
    class PrefabInstantiator {
    private:
        static Entity InstantiateEntity(Scene* scene, const PrefabEntityData& entityData,
            const Prefab& prefab, Entity parent,
            std::unordered_map<u64, Entity>& localIDToEntity);

        //static void CreateComponentFromPrefab(Entity entity, const PrefabComponentData& componentData);

        static void StoreOriginalComponentDataForAllEntities(
            Scene* scene,
            const Prefab& prefab,
            const std::unordered_map<u64, Entity>& localIDToEntity);

        static void RemoveComponentByTypeID(Entity entity, ComponentTypeID type);

        static void ApplyEntityOverrides(Entity entity, Scene* scene, PrefabEntityData* entityData);
    public:
        static Entity InstantiatePrefabFromFile(Scene* scene, const std::string& filepath, Entity parent);

        static Entity InstantiatePrefab(Scene* scene, const Prefab& prefab, Entity parent);

        //static void StoreOriginalComponentData(Entity entity, const Prefab& prefab);

        // Revert entity to match its prefab
        static bool RevertToPrefab(Entity entity, Scene* scene);

        // Apply all overrides from an instance back to its prefab asset
        static bool ApplyOverridesToPrefab(Entity entity, Scene* scene);

        // Revert a single component to its prefab state
       /* static bool RevertComponentToPrefab(Entity entity, ComponentTypeID componentType);*/

        static void RevertEntityAndChildren(Entity entity, Scene* scene, const Prefab& prefab);
       
        static void RebuildPrefabHierarchy(const Prefab& prefab,
            const std::unordered_map<u64, Entity>& localIDToEntity,
            Scene* scene);

        static void UpdateExistingEntitiesPrefabLocalID(Entity root, Scene* scene, const Prefab& prefab);

        // static void RemoveComponentsNotInPrefab(Entity entity, const PrefabEntityData& prefabData);
    };
}
#endif // end of __PREFAB_INSTANTIATOR_H__
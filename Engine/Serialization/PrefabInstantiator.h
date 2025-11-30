/**
 * @file PrefabInstantiator.h
 * @brief System for instantiating prefabs into scenes
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

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

    /**
     * @brief Handles instantiation of prefabs into scenes
     */
    class PrefabInstantiator {
    public:
        /**
         * @brief Instantiate an entity prefab into a scene
         * @param scene Target scene
         * @param prefabGUID GUID of the prefab to instantiate
         * @param entityId Optional specific entity ID to use (entt::null for auto)
         * @return The created entity
         */
        static Entity InstantiateEntityPrefab(
            Scene* scene,
            xresource::instance_guid prefabGUID,
            entt::entity entityId = entt::null
        );

        /**
         * @brief Instantiate a scene prefab into a scene
         * @param scene Target scene
         * @param prefabGUID GUID of the scene prefab
         * @return The root entity of the instantiated scene prefab
         */
        static Entity InstantiateScenePrefab(
            Scene* scene,
            xresource::instance_guid prefabGUID
        );

        /**
         * @brief Apply overrides from PrefabComponent to an entity
         * @param entity Entity with PrefabComponent
         * @param scene Scene containing the entity
         */
       // static void ApplyOverrides(Entity entity, Scene* scene);

            /**
         * @brief Deserialize and create entity from JSON data
         * @param scene Target scene
         * @param entityJson JSON string of entity data
         * @param entityId Specific entity ID to use (entt::null for auto)
         * @return Created entity
         */
        static Entity DeserializeEntity(
            Scene* scene,
            const std::string& entityJson,
            entt::entity entityId = entt::null
        );

        /**************************************************************************
        * @brief
        * 	Creates entities from Json data and builds a mapping between source
            IDs and runtime entityIDs
        * @param scene
        *	Pointer to the target scene where entities will be created
        * @param entitiesArray 
        *   JSON array containing entity definitions and components
        * @param idMapping
        *   Output parameter that will be populated with mapping from 
            source IDs to entt::entity handles
        * @param rootEntity
        *   Output parameter that will contain the root entity of the hierarchy
        * @return
        *   Vector of all created entities in the order they were processed
        **************************************************************************/
        static std::vector<Entity>CreateEntitiesAndBuildIDMap(
            Scene* scene,
            const rapidjson::Value& entitiesArray,
            std::unordered_map<uint32_t, entt::entity>& idMapping,
            Entity& rootEntity);

        /**************************************************************************
        * @brief
        *   Resolves and establishes parent-child relationships between entities after creation
        * @param entities 
        *	Vector of all entities that need hierarchy resolution
        * @param idMapping 
        *   Mapping from source IDs to runtime entity handles used to resolve parent references
        **************************************************************************/
        static void FixTransformHierarchy(
            const std::vector<Entity>& entities,
            const std::unordered_map<uint32_t, entt::entity>& idMapping);

        /**************************************************************************
        * @brief
        *   Applies a prefab reference component to all entities in a collection
        * @param entities
        *	Vector of all entities to receive the prefab component
        * @param prefabGUID 
        *   Unique identifier of the prefab that these entities originated from
        **************************************************************************/
        static void ApplyPrefabComponentToAll(
            const std::vector<Entity>& entities,
            xresource::instance_guid prefabGUID);

        /**************************************************************************
        * @brief
        *   Identifies the root entity in a collection by finding 
            entities without parents
        * @param entities
        *	entities Vector of entities to search through
        * @return 
        *   The root entity
        **************************************************************************/
        static Entity FindRootEntity(const std::vector<Entity>& entities);
    private:

        /**
         * @brief Add a component to entity from JSON data
         * @param entity Target entity
         * @param componentType Type name of the component
         * @param properties JSON value containing component properties
         */
        template<typename ValueType>
        static void AddComponentFromJson(
            Entity entity,
            const std::string& componentType,
            const ValueType& properties
        );
    };

} // namespace Engine

#endif // __PREFAB_INSTANTIATOR_H__
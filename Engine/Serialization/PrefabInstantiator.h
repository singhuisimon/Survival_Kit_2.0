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
        static void ApplyOverrides(Entity entity, Scene* scene);

    private:
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
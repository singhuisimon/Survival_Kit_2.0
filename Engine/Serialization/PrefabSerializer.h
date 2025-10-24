/**
 * @file PrefabSerializer.h
 * @brief Serialization system for prefabs
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#ifndef __PREFAB_SERIALIZER_H__
#define __PREFAB_SERIALIZER_H__

#include "../Prefab/Prefab.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include <string>
#include <vector>
#include <memory>
#include <entt/entt.hpp>

namespace Engine {

    /**
     * @brief Handles serialization and deserialization of prefabs
     */
    class PrefabSerializer {
    public:
        /**
         * @brief Create an entity prefab from an existing entity
         * @param entity Entity to convert to prefab
         * @param name Name for the prefab
         * @return Shared pointer to the created prefab
         */
        static std::shared_ptr<Prefab> CreateEntityPrefab(Entity entity, const std::string& name);

        /**
         * @brief Create a scene prefab from selected entities
         * @param scene Scene containing the entities
         * @param entities Entities to include in the scene prefab
         * @param name Name for the prefab
         * @return Shared pointer to the created prefab
         */
        static std::shared_ptr<Prefab> CreateScenePrefab(
            Scene* scene,
            const std::vector<Entity>& entities,
            const std::string& name
        );

        /**
         * @brief Save prefab to file
         * @param prefab Prefab to save
         * @param filepath Path to save the prefab
         * @return True if successful
         */
        static bool SavePrefabToFile(const Prefab& prefab, const std::string& filepath);

        /**
         * @brief Load prefab from file
         * @param filepath Path to the prefab file
         * @return Shared pointer to loaded prefab, or nullptr on failure
         */
        static std::shared_ptr<Prefab> LoadPrefabFromFile(const std::string& filepath);

        /**
         * @brief Serialize a prefab to JSON string
         * @param prefab Prefab to serialize
         * @return JSON string representation
         */
        static std::string SerializePrefabToString(const Prefab& prefab);

        /**
         * @brief Deserialize a prefab from JSON string
         * @param jsonString JSON string to deserialize
         * @return Shared pointer to deserialized prefab
         */
        static std::shared_ptr<Prefab> DeserializePrefabFromString(const std::string& jsonString);

    private:
        /**
         * @brief Serialize a single entity to JSON string
         * @param entity Entity to serialize
         * @param dummyEntity Dummy parameter for interface compatibility
         * @return JSON string representation of the entity
         */
        static std::string SerializeEntity(Entity entity, Entity dummyEntity);

        /**
         * @brief Serialize multiple entities to JSON string (for scene prefabs)
         * @param entities Entities to serialize
         * @param registry EnTT registry containing the entities
         * @return JSON string representation of the entities
         */
        static std::string SerializeEntities(const std::vector<Entity>& entities, entt::registry& registry);
    };

} // namespace Engine

#endif // __PREFAB_SERIALIZER_H__
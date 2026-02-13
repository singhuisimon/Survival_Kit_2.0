#pragma once
#ifndef __PREFAB_HELPERS_H__
#define __PREFAB_HELPERS_H__

#include <string>
#include <vector>
#include <functional>

#include "../Utility/Types.h"

namespace Engine {

    class Entity;
    class Scene;
    enum class ComponentTypeID : u32;

    /**
     * @brief Helper class to reduce code duplication and fix memory leaks
     */
    class PrefabHelpers {
    public:
        /**
         * @brief Serialize an entire entity to JSON string
         * @param entity The entity to serialize
         * @return JSON string representation
         *
         * REPLACES: EditorHierarchyPanel::SerializeEntityForRevert() (500+ lines)
         */
        static std::string SerializeEntityToJSON(Entity entity);

        /**
         * @brief Find the root prefab entity by traversing up the hierarchy
         * @param entity Starting entity
         * @param scene Scene containing the entity
         * @return Root prefab entity or invalid entity if not found
         *
         * MEMORY SAFE: Has infinite loop protection
         */
        static Entity FindPrefabRoot(Entity entity, Scene* scene);

        /**
         * @brief Clean up entity's PrefabComponent data before deletion
         * @param entity Entity to clean up
         *
         * CRITICAL FOR MEMORY LEAK FIX!
         * Clears: componentOverrides, deletedEntities, addedEntityHandles, childEntityIDs
         */
        static void ClearPrefabComponentData(Entity entity);

        /**
         * @brief Full cleanup before entity deletion (includes transform relationships)
         * @param entity Entity to clean up
         * @param scene Scene containing the entity
         *
         * CRITICAL FOR MEMORY LEAK FIX!
         * Calls ClearPrefabComponentData + cleans parent-child relationships
         */
        static void CleanupEntityForDeletion(Entity entity, Scene* scene);

        /**
         * @brief Iterate through all components and call a function
         * @param entity Entity to check
         * @param callback Function called for each component type that exists
         *
         * ELIMINATES: Repeated if/HasComponent patterns
         */
        static void ForEachComponent(Entity entity,
            std::function<void(ComponentTypeID)> callback);
    };

} // namespace Engine

#endif // __PREFAB_HELPERS_H__
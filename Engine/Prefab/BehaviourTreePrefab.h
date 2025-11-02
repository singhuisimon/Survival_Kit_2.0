/**
 * @file BehaviourTreePrefab.h
 * @brief Integration with prefab system for behaviour trees
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "../BehaviourTree/BehaviourTree.h"
#include "../Serialization/BehaviourTreeSerializer.h"
#include "Prefab.h"
#include "PrefabRegistry.h"
#include "../Utility/Logger.h"
#include <memory>

namespace Engine {

    /**
     * @brief Helper class for creating and managing behaviour tree prefabs
     */
    class BehaviourTreePrefab {
    public:
        /**
         * @brief Create a prefab from a behaviour tree
         * @param tree The behaviour tree to convert to prefab
         * @param name Name for the prefab
         * @return Shared pointer to the created prefab
         */
        static std::shared_ptr<Prefab> CreatePrefab(const BehaviourTree& tree, const std::string& name);

        /**
         * @brief Load a behaviour tree from a prefab
         * @param prefabGUID GUID of the prefab
         * @return Shared pointer to the loaded behaviour tree, or nullptr if failed
         */
        static std::shared_ptr<BehaviourTree> LoadFromPrefab(xresource::instance_guid prefabGUID);

        /**
         * @brief Save a behaviour tree as a prefab and register it
         * @param tree The behaviour tree to save
         * @param name Name for the prefab
         * @return GUID of the created prefab
         */
        static xresource::instance_guid SaveAsPrefab(const BehaviourTree& tree, const std::string& name);

        /**
         * @brief Create a runtime instance of a tree from a prefab
         * @param prefabGUID GUID of the prefab
         * @return New tree instance (independent copy)
         */
        static std::shared_ptr<BehaviourTree> Instantiate(xresource::instance_guid prefabGUID);

        /**
         * @brief Update a prefab with a modified tree
         * @param prefabGUID GUID of the prefab to update
         * @param tree The updated tree
         * @return True if successful
         */
        static bool UpdatePrefab(xresource::instance_guid prefabGUID, const BehaviourTree& tree);

        /**
         * @brief Save a behaviour tree prefab to file
         * @param prefabGUID GUID of the prefab
         * @param filepath Path to save to
         * @return True if successful
         */
        static bool SavePrefabToFile(xresource::instance_guid prefabGUID, const std::string& filepath);

        /**
         * @brief Load a behaviour tree prefab from file and register it
         * @param filepath Path to load from
         * @param name Name for the prefab
         * @return GUID of the registered prefab
         */
        static xresource::instance_guid LoadPrefabFromFile(const std::string& filepath, const std::string& name);
    };

} // namespace Engine

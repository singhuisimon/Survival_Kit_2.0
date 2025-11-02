/**
 * @file BehaviourTreeEditor.h
 * @brief Editor utilities for behaviour tree creation and editing
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BehaviourTree.h"
#include "../Serialization/BehaviourTreeSerializer.h"
#include "../Prefab/BehaviourTreePrefab.h"
#include "BTNodeRegistry.h"
#include "../Utility/Logger.h"

namespace Engine {

    /**
     * @brief Editor helper class for behaviour tree manipulation
     * @details Provides high-level API for editor to create/edit/save trees
     */
    class BehaviourTreeEditor {
    public:
        /**
         * @brief Create a new empty behaviour tree
         */
        static std::shared_ptr<BehaviourTree> CreateNewTree(const std::string& name = "NewBehaviorTree");

        /**
         * @brief Load a tree from file
         */
        static std::shared_ptr<BehaviourTree> LoadTree(const std::string& filepath);

        /**
         * @brief Save a tree to file
         */
        static bool SaveTree(const BehaviourTree& tree, const std::string& filepath);

        /**
         * @brief Create a node of specified type
         */
        static std::shared_ptr<BTNode> CreateNode(const std::string& typeName);

        /**
         * @brief Add a child node to a parent
         */
        static bool AddChildNode(std::shared_ptr<BTNode> parent, std::shared_ptr<BTNode> child);

        /**
         * @brief Remove a child node from a parent by index
         */
        static bool RemoveChildNode(std::shared_ptr<BTNode> parent, size_t index);

        /**
         * @brief Set a node property
         */
        static void SetNodeProperty(std::shared_ptr<BTNode> node,
            const std::string& propertyName,
            const std::string& value);

        /**
         * @brief Get all available node types (for editor UI)
         */
        static std::vector<std::string> GetAllNodeTypes();

        /**
         * @brief Get node types by category (for organized editor UI)
         */
        static std::vector<std::string> GetNodeTypesByCategory(const std::string& category);

        /**
         * @brief Get all categories
         */
        static std::vector<std::string> GetAllCategories();

        /**
         * @brief Convert tree to prefab for reuse
         */
        static xresource::instance_guid ConvertToPrefab(const BehaviourTree& tree, const std::string& prefabName);

        /**
         * @brief Load tree from prefab
         */
        static std::shared_ptr<BehaviourTree> LoadFromPrefab(xresource::instance_guid prefabGUID);

        /**
         * @brief Validate tree structure (check for common errors)
         */
        static bool ValidateTree(const BehaviourTree& tree, std::vector<std::string>& errors);

        /**
         * @brief Clone/duplicate a tree
         */
        static std::shared_ptr<BehaviourTree> CloneTree(const BehaviourTree& original);

    private:
        /**
         * @brief Recursively validate a node and its children
         */
        static bool ValidateNode(std::shared_ptr<BTNode> node, std::vector<std::string>& errors);
    };

} // namespace Engine

 /**
  * @file BehaviourTree.h
  * @brief Editor utilities for behaviour tree creation and editing
  * @author Amanda Leow Boon Suan (90%)
  * @date 3/11/2025
  * Copyright (C) 2025 DigiPen Institute of Technology.
  * Reproduction or disclosure of this file or its contents without the
  * prior written consent of DigiPen Institute of Technology is prohibited.
  */

#pragma once

#include "BehaviourTree.h"
#include "../Serialization/BehaviourTreeSerializer.h"
#include "../Prefab/BehaviourTreePrefab.h"
#include "BTNodeRegistry.h"
#include "../Utility/Logger.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include "../Component/BehaviourTreeComponent.h"

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

        //NEWLY ADDED - YET TO TEST THE FOLLOWING:

        /**
         * @brief Remove a child node by finding it automatically
         */
        static bool RemoveChildNode(std::shared_ptr<BTNode> parent, std::shared_ptr<BTNode> child);

        /**
         * @brief Remove a child node by GUID
         */
        static bool RemoveChildNodeByGUID(std::shared_ptr<BTNode> parent, xresource::instance_guid childGUID);

        /**
         * @brief Find the index of a child within its parent
         */
        static int FindChildIndex(std::shared_ptr<BTNode> parent, std::shared_ptr<BTNode> child);

        /**
         * @brief Get property metadata for a node type
         */
        static BTNodeMetadata* GetNodePropertyMetadata(const std::string& typeName);

        /**
         * @brief Get all property names for a node type
         */
        static std::vector<std::string> GetNodePropertyNames(const std::string& typeName);

        /**
         * @brief Get property info (name, type) for a node type
         */
        static std::vector<std::pair<std::string, PropertyType>> GetNodePropertyInfo(const std::string& typeName);

        /**
         * @brief Get property value from a node instance
         */
        static std::string GetNodePropertyValue(std::shared_ptr<BTNode> node, const std::string& propertyName);

        /**
         * @brief Set property value using metadata system
         */
        static bool SetNodePropertyValue(std::shared_ptr<BTNode> node,
            const std::string& propertyName,
            const std::string& value);

        /**
        * @brief Rename a behaviour tree (in-memory only)
        */
        static bool RenameTree(std::shared_ptr<BehaviourTree> tree, const std::string& newName);

        /**
        * @brief Rename a behaviour tree (in-memory only)
        */
        static bool RenameTreeFIle(const std::string& oldPath,
            const std::string& newPath,
            Scene* scene = nullptr);

        /**
        * @brief Rename and save a tree to a new file
        */
        static bool RenameAndSave(std::shared_ptr<BehaviourTree> tree,
            const std::string& newName,
            const std::string& newFilePath = "");

        /**
        * @brief Rename actual file name on disk
        */
        static bool RenameFile(const std::string& oldPath,
            const std::string& newPath,
            Scene* scene = nullptr);

        /**
        * @brief Save a copy with optional modifications (Save as)
        */
        static bool SaveAs(const std::string& sourcePath,
            const std::string& newPath,
            const std::string& newName = "",
            bool generateNewGUID = true);

        //END OF NEWLY ADDED - AMANDA

    private:
        /**
         * @brief Recursively validate a node and its children
         */
        static bool ValidateNode(std::shared_ptr<BTNode> node, std::vector<std::string>& errors);
    };

} // namespace Engine

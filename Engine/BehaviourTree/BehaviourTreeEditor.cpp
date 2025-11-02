/**
 * @file BehaviourTreeEditor.h
 * @brief Editor utilities for behaviour tree creation and editing
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BehaviourTree.h"
#include "../Serialization/BehaviourTreeSerializer.h"
#include "BehaviourTreeEditor.h"
#include "../Prefab/BehaviourTreePrefab.h"
#include "BTNodeRegistry.h"
#include "../Utility/Logger.h"

namespace Engine {

    /**
     * @brief Create a new empty behaviour tree
     */
    std::shared_ptr<BehaviourTree> BehaviourTreeEditor::CreateNewTree(const std::string& name) {
        auto tree = std::make_shared<BehaviourTree>();
        tree->SetName(name);

        LOG_INFO("BehaviourTreeEditor: Created new tree '", name, "'");
        return tree;
    }

    /**
     * @brief Load a tree from file
     */
    std::shared_ptr<BehaviourTree> BehaviourTreeEditor::LoadTree(const std::string& filepath) {
        auto tree = BehaviourTreeSerializer::DeserializeFromFile(filepath);
        if (tree) {
            LOG_INFO("BehaviourTreeEditor: Loaded tree from ", filepath);
        }
        return tree;
    }

    /**
     * @brief Save a tree to file
     */
    bool BehaviourTreeEditor::SaveTree(const BehaviourTree& tree, const std::string& filepath) {
        bool success = BehaviourTreeSerializer::SerializeToFile(tree, filepath);
        if (success) {
            LOG_INFO("BehaviourTreeEditor: Saved tree '", tree.GetName(), "' to ", filepath);
        }
        return success;
    }

    /**
     * @brief Create a node of specified type
     */
    std::shared_ptr<BTNode> BehaviourTreeEditor::CreateNode(const std::string& typeName) {
        auto node = BTNodeRegistry::Get().CreateNode(typeName);
        if (node) {
            LOG_TRACE("BehaviourTreeEditor: Created node of type '", typeName, "'");
        }
        return node;
    }

    /**
     * @brief Add a child node to a parent
     */
    bool BehaviourTreeEditor::AddChildNode(std::shared_ptr<BTNode> parent, std::shared_ptr<BTNode> child) {
        if (!parent || !child) {
            LOG_ERROR("BehaviourTreeEditor: Cannot add child - null node");
            return false;
        }

        if (!parent->CanHaveChildren()) {
            LOG_ERROR("BehaviourTreeEditor: Node '", parent->GetTypeName(), "' cannot have children");
            return false;
        }

        parent->AddChild(child);
        LOG_TRACE("BehaviourTreeEditor: Added child '", child->GetName(),
            "' to parent '", parent->GetName(), "'");
        return true;
    }

    /**
     * @brief Remove a child node from a parent by index
     */
    bool BehaviourTreeEditor::RemoveChildNode(std::shared_ptr<BTNode> parent, size_t index) {
        if (!parent) {
            LOG_ERROR("BehaviourTreeEditor: Cannot remove child - null parent");
            return false;
        }

        parent->RemoveChild(index);
        LOG_TRACE("BehaviourTreeEditor: Removed child at index ", index,
            " from parent '", parent->GetName(), "'");
        return true;
    }

    /**
     * @brief Set a node property
     */
    void BehaviourTreeEditor::SetNodeProperty(std::shared_ptr<BTNode> node,
        const std::string& propertyName,
        const std::string& value) {
        if (!node) {
            LOG_ERROR("BehaviourTreeEditor: Cannot set property - null node");
            return;
        }

        node->SetProperty(propertyName, value);
        LOG_TRACE("BehaviourTreeEditor: Set property '", propertyName,
            "' = '", value, "' on node '", node->GetName(), "'");
    }

    /**
     * @brief Get all available node types (for editor UI)
     */
    std::vector<std::string> BehaviourTreeEditor::GetAllNodeTypes() {
        std::vector<std::string> types;
        for (const auto& [typeName, metadata] : BTNodeRegistry::Get().GetAllNodeTypes()) {
            types.push_back(typeName);
        }
        return types;
    }

    /**
     * @brief Get node types by category (for organized editor UI)
     */
    std::vector<std::string> BehaviourTreeEditor::GetNodeTypesByCategory(const std::string& category) {
        return BTNodeRegistry::Get().GetNodeTypesByCategory(category);
    }

    /**
     * @brief Get all categories
     */
    std::vector<std::string> BehaviourTreeEditor::GetAllCategories() {
        std::vector<std::string> categories;
        for (const auto& [typeName, metadata] : BTNodeRegistry::Get().GetAllNodeTypes()) {
            // Add unique categories
            if (std::find(categories.begin(), categories.end(), metadata.Category) == categories.end()) {
                categories.push_back(metadata.Category);
            }
        }
        return categories;
    }

    /**
     * @brief Convert tree to prefab for reuse
     */
    xresource::instance_guid BehaviourTreeEditor::ConvertToPrefab(const BehaviourTree& tree, const std::string& prefabName) {
        xresource::instance_guid guid = BehaviourTreePrefab::SaveAsPrefab(tree, prefabName);
        if (guid.m_Value != 0) {
            LOG_INFO("BehaviourTreeEditor: Converted tree '", tree.GetName(),
                "' to prefab '", prefabName, "'");
        }
        return guid;
    }

    /**
     * @brief Load tree from prefab
     */
    std::shared_ptr<BehaviourTree> BehaviourTreeEditor::LoadFromPrefab(xresource::instance_guid prefabGUID) {
        return BehaviourTreePrefab::LoadFromPrefab(prefabGUID);
    }

    /**
     * @brief Validate tree structure (check for common errors)
     */
    bool BehaviourTreeEditor::ValidateTree(const BehaviourTree& tree, std::vector<std::string>& errors) {
        errors.clear();

        if (!tree.GetRootNode()) {
            errors.push_back("Tree has no root node");
            return false;
        }

        // Recursive validation
        return ValidateNode(tree.GetRootNode(), errors);
    }

    /**
     * @brief Clone/duplicate a tree
     */
    std::shared_ptr<BehaviourTree> BehaviourTreeEditor::CloneTree(const BehaviourTree& original) {
        // Serialize and deserialize to create a deep copy
        std::string json = BehaviourTreeSerializer::SerializeToString(original);
        auto clone = BehaviourTreeSerializer::DeserializeFromString(json);

        if (clone) {
            // Generate new GUID for clone
            clone->SetGUID(xresource::instance_guid::GenerateGUIDCopy());
            clone->SetName(original.GetName() + "_Copy");

            LOG_INFO("BehaviourTreeEditor: Cloned tree '", original.GetName(), "'");
        }

        return clone;
    }

    /**
     * @brief Recursively validate a node and its children
     */
    bool BehaviourTreeEditor::ValidateNode(std::shared_ptr<BTNode> node, std::vector<std::string>& errors) {
        if (!node) {
            errors.push_back("Found null node in tree");
            return false;
        }

        bool valid = true;

        // Check if node type is registered
        if (!BTNodeRegistry::Get().IsNodeTypeRegistered(node->GetTypeName())) {
            errors.push_back(std::string("Unknown node type: ") + node->GetTypeName());
            valid = false;
        }

        // Validate children
        const auto& children = node->GetChildren();
        if (!children.empty() && !node->CanHaveChildren()) {
            errors.push_back(std::string("Node '") + node->GetName() +
                "' has children but shouldn't");
            valid = false;
        }

        // Recursively validate children
        for (const auto& child : children) {
            if (!ValidateNode(child, errors)) {
                valid = false;
            }
        }

        return valid;
    }

} // namespace Engine

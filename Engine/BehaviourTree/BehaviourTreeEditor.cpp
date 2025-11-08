/**
 * @file BehaviourTreeEditor.cpp
 * @brief Definition of BehaviourTreeEditor class for managing behaviour trees in the editor
 * @author Amanda Leow Boon Suan (90%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "BehaviourTree.h"
#include "BehaviourTreeEditor.h"
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

        std::string path = filepath;
        if (path.empty()) {
            path = "new_tree.json";
        }

        bool success = BehaviourTreeSerializer::SerializeToFile(tree, path);
        if (success) {
            LOG_INFO("BehaviourTreeEditor: Saved tree '", tree.GetName(), "' to ", path);
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


    //FROM HERE ONWARDS ALL NEW!!! - AMANDA
    bool BehaviourTreeEditor::RemoveChildNode(std::shared_ptr<BTNode> parent, std::shared_ptr<BTNode> child) {
        if (!parent || !child) {
            LOG_ERROR("BehaviourTreeEditor: Cannot remove child - null node");
            return false;
        }

        int index = parent->FindChildIndex(child);
        if (index < 0) {
            LOG_ERROR("BehaviourTreeEditor: Child node not found in parent");
            return false;
        }

        parent->RemoveChild(static_cast<size_t>(index));
        LOG_TRACE("BehaviourTreeEditor: Removed child '", child->GetName(),
            "' from parent '", parent->GetName(), "'");
        return true;
    }

    bool BehaviourTreeEditor::RemoveChildNodeByGUID(std::shared_ptr<BTNode> parent, xresource::instance_guid childGUID) {
        if (!parent) {
            LOG_ERROR("BehaviourTreeEditor: Cannot remove child - null parent");
            return false;
        }

        int index = parent->FindChildIndexByGUID(childGUID);
        if (index < 0) {
            LOG_ERROR("BehaviourTreeEditor: Child node with GUID not found in parent");
            return false;
        }

        parent->RemoveChild(static_cast<size_t>(index));
        LOG_TRACE("BehaviourTreeEditor: Removed child at index ", index,
            " from parent '", parent->GetName(), "'");
        return true;
    }

    int BehaviourTreeEditor::FindChildIndex(std::shared_ptr<BTNode> parent, std::shared_ptr<BTNode> child) {
        if (!parent || !child) {
            return -1;
        }
        return parent->FindChildIndex(child);
    }

    BTNodeMetadata* BehaviourTreeEditor::GetNodePropertyMetadata(const std::string& typeName) {
        // Get the type metadata from registry
        const BTNodeTypeMetadata* typeMetadata = BTNodeRegistry::Get().GetMetadata(typeName);
        if (!typeMetadata) {
            return nullptr;
        }

        // Return the property metadata (it's a shared_ptr, so we get the raw pointer)
        return typeMetadata->PropertyMetadata.get();
    }

    std::vector<std::string> BehaviourTreeEditor::GetNodePropertyNames(const std::string& typeName) {
        BTNodeMetadata* metadata = GetNodePropertyMetadata(typeName);
        if (!metadata) {
            return {};
        }

        std::vector<std::string> names;
        for (const auto& prop : metadata->GetProperties()) {
            names.push_back(prop.Name);
        }
        return names;
    }

    std::vector<std::pair<std::string, PropertyType>> BehaviourTreeEditor::GetNodePropertyInfo(const std::string& typeName) {
        BTNodeMetadata* metadata = GetNodePropertyMetadata(typeName);
        if (!metadata) {
            return {};
        }

        std::vector<std::pair<std::string, PropertyType>> info;
        for (const auto& prop : metadata->GetProperties()) {
            info.push_back({ prop.Name, prop.Type });
        }
        return info;
    }

    std::string BehaviourTreeEditor::GetNodePropertyValue(std::shared_ptr<BTNode> node, const std::string& propertyName) {
        if (!node) {
            LOG_ERROR("BehaviourTreeEditor: Cannot get property - null node");
            return "";
        }

        BTNodeMetadata* metadata = GetNodePropertyMetadata(node->GetTypeName());
        if (!metadata) {
            LOG_ERROR("BehaviourTreeEditor: No metadata found for node type: ", node->GetTypeName());
            return "";
        }

        return metadata->GetPropertyValue(node.get(), propertyName);
    }

    bool BehaviourTreeEditor::SetNodePropertyValue(std::shared_ptr<BTNode> node,
        const std::string& propertyName,
        const std::string& value) {
        if (!node) {
            LOG_ERROR("BehaviourTreeEditor: Cannot set property - null node");
            return false;
        }

        BTNodeMetadata* metadata = GetNodePropertyMetadata(node->GetTypeName());
        if (!metadata) {
            LOG_ERROR("BehaviourTreeEditor: No metadata found for node type: ", node->GetTypeName());
            return false;
        }

        bool success = metadata->SetPropertyValue(node.get(), propertyName, value);
        if (success) {
            LOG_TRACE("BehaviourTreeEditor: Set property '", propertyName,
                "' = '", value, "' on node '", node->GetName(), "' using metadata");
        }
        else {
            LOG_ERROR("BehaviourTreeEditor: Failed to set property '", propertyName,
                "' on node type ", node->GetTypeName());
        }
        return success;
    }

    bool BehaviourTreeEditor::RenameTree(std::shared_ptr<BehaviourTree> tree, const std::string& newName)
    {
        if (!tree) {
            LOG_ERROR("BehaviourTreeEditor::RenameTree: Null tree provided");
            return false;
        }

        if (newName.empty()) {
            LOG_ERROR("BehaviourTreeEditor::RenameTree: Cannot rename tree to empty name");
            return false;
        }

        std::string oldName = tree->GetName();
        tree->SetName(newName);

        LOG_INFO("BehaviourTreeEditor: Renamed tree from '", oldName, "' to '", newName, "'");
        return true;
    }

    static bool RenameTreeFIle(const std::string& oldPath,
        const std::string& newPath,
        Scene* scene = nullptr) {

        if (oldPath.empty() || newPath.empty()) {
            LOG_ERROR("BehaviourTreeEditor::RenameTreeFile: Empty path provided");
            return false;
        }

        if (oldPath == newPath) {
            LOG_WARNING("BehaviourTreeEditor::RenameTreeFile: Old and new paths are identical");
            return true;
        }

        // Step 1 - Load the tree from the old path
        LOG_INFO("BehaviourTreeEditor: Loading tree from '", oldPath, "'");
        auto tree = BehaviourTreeEditor::LoadTree(oldPath);
        if (!tree) {
            LOG_ERROR("BehaviourTreeEditor: Failed to load tree from '", oldPath, "'");
            return false;
        }

        // Step 2 - Extract new name from path (remove .json and path)
        std::string newName = newPath;

        // Remove .json extension if present
        size_t extPos = newName.find_last_of('.');
        if (extPos != std::string::npos) {
            std::string ext = newName.substr(extPos);
            if (ext == ".json" || ext == ".JSON") {
                newName = newName.substr(0, extPos);
            }
        }

        // Remove any path prefix
        size_t pathSep = newName.find_last_of("/\\");
        if (pathSep != std::string::npos) {
            newName = newName.substr(pathSep + 1);
        }

        // Step 3: Update the tree's internal
        tree->SetName(newName);

        // Step 4: Save to the new path
        LOG_INFO("BehaviourTreeEditor: Saving tree to new path '", newPath, "'");
        if (!BehaviourTreeEditor::SaveTree(*tree, newPath)) {
            LOG_ERROR("BehaviourTreeEditor: Failed to save tree to new path '", newPath, "'");
            return false;
        }

        // Step 5: Update all component references in the scene
        if (scene) {
            LOG_INFO("BehaviourTreeEditor: Updating component references in scene");

            auto& registry = scene->GetRegistry();
            auto view = registry.view<BehaviourTreeComponent>();

            int updatedCount = 0;
            for (auto entity : view) {
                Entity ent(entity, &registry);

                if (!ent.HasComponent<BehaviourTreeComponent>()) {
                    continue;
                }

                auto& btComp = ent.GetComponent<BehaviourTreeComponent>();

                // If this component was using the old path, update it
                if (btComp.TreeAssetPath == oldPath) {
                    btComp.TreeAssetPath = newPath;

                    // Also update the runtime instance if loaded
                    if (btComp.TreeInstance) {
                        btComp.TreeInstance->SetName(newName);
                    }

                    updatedCount++;
                }
            }

            LOG_INFO("BehaviourTreeEditor: Updated ", updatedCount, " component references");
        }
    }

    bool BehaviourTreeEditor::RenameAndSave(std::shared_ptr<BehaviourTree> tree,
        const std::string& newName,
        const std::string& newFilePath) {
        if (!tree) {
            LOG_ERROR("BehaviourTreeEditor::RenameAndSave: Null tree provided");
            return false;
        }

        if (newName.empty()) {
            LOG_ERROR("BehaviourTreeEditor::RenameAndSave: Empty name provided");
            return false;
        }

        // Update the tree's name
        tree->SetName(newName);

        // Determine the file path
        std::string filepath = newFilePath;
        if (filepath.empty()) {
            filepath = newName + ".json";
        }

        // Make sure path ends with .json
        if (filepath.find(".json") == std::string::npos) {
            filepath += ".json";
        }

        // Save the tree
        LOG_INFO("BehaviourTreeEditor: Renaming and saving tree to '", filepath, "'");
        if (!BehaviourTreeEditor::SaveTree(*tree, filepath)) {
            LOG_ERROR("BehaviourTreeEditor: Failed to save renamed tree");
            return false;
        }

        LOG_INFO("BehaviourTreeEditor: Successfully renamed and saved tree as '", newName, "'");
        return true;
    }

    bool BehaviourTreeEditor::RenameFile(const std::string& oldPath,
        const std::string& newPath,
        Scene* scene) {

        if (oldPath.empty() || newPath.empty()) {
            LOG_ERROR("BehaviourTreeEditor::RenameFile: Empty path provided");
            return false;
        }

        if (oldPath == newPath) {
            LOG_WARNING("BehaviourTreeEditor::RenameFile: Paths are identical");
            return true;
        }

        // Step 1: Get full paths (need both source and output locations)
        std::string oldSourcePath = BehaviourTreeSerializer::GetSourceFilePath("Sources/BT/" + oldPath);
        std::string oldOutputPath = getAssetFilePath("Sources/BT/" + oldPath);
        std::string newSourcePath = BehaviourTreeSerializer::GetSourceFilePath("Sources/BT/" + newPath);
        std::string newOutputPath = getAssetFilePath("Sources/BT/" + newPath);

        // Step 2: Ensure source file exists
        if (!std::filesystem::exists(oldSourcePath) && !std::filesystem::exists(oldOutputPath)) {
            LOG_ERROR("BehaviourTreeEditor::RenameFile: Source file not found at '", oldPath, "'");
            return false;
        }

        // Step 3 - Load the tree to update internal name
        auto tree = BehaviourTreeEditor::LoadTree(oldPath);
        if (!tree) {
            LOG_ERROR("BehaviourTreeEditor::RenameFile: Failed to load tree from '", oldPath, "'");
            return false;
        }

        // Step 4 - Extract new name from path
        std::string newName = newPath;
        size_t extPos = newName.find_last_of('.');
        if (extPos != std::string::npos) {
            newName = newName.substr(0, extPos);
        }
        size_t pathSep = newName.find_last_of("/\\");
        if (pathSep != std::string::npos) {
            newName = newName.substr(pathSep + 1);
        }

        // Step 5: Update tree's internal name (GUID will stay the same)
        tree->SetName(newName);

        // Step 6: Save to new location
        if (!SaveTree(*tree, newPath)) {
            LOG_ERROR("BehaviourTreeEditor::RenameFile: Failed to save to new path '", newPath, "'");
            return false;
        }

        // Step 7 - Delete all old files (both source and output)
        bool deletedSource = false;
        bool deletedOutput = false;

        if (std::filesystem::exists(oldSourcePath)) {
            std::error_code ec;
            deletedSource = std::filesystem::remove(oldSourcePath, ec);
            if (!deletedSource) {
                LOG_WARNING("BehaviourTreeEditor::RenameFile: Failed to delete old source file: ", ec.message());
            }
        }

        if (std::filesystem::exists(oldOutputPath)) {
            std::error_code ec;
            deletedOutput = std::filesystem::remove(oldOutputPath, ec);
            if (!deletedOutput) {
                LOG_WARNING("BehaviourTreeEditor::RenameFile: Failed to delete old output file: ", ec.message());
            }
        }

        // Step 8: Update scene component references if provided
        if (scene) {
            auto& registry = scene->GetRegistry();
            auto view = registry.view<BehaviourTreeComponent>();

            int updatedCount = 0;
            for (auto entity : view) {
                Entity ent(entity, &registry);
                if (!ent.HasComponent<BehaviourTreeComponent>()) continue;

                auto& btComp = ent.GetComponent<BehaviourTreeComponent>();

                // Update path references
                if (btComp.TreeAssetPath == oldPath) {
                    btComp.TreeAssetPath = newPath;
                    if (btComp.TreeInstance) {
                        btComp.TreeInstance->SetName(newName);
                    }
                    updatedCount++;
                }
            }

            LOG_INFO("BehaviourTreeEditor::RenameFile: Updated ", updatedCount, " component references");
        }

        LOG_INFO("BehaviourTreeEditor::RenameFile: Successfully renamed '", oldPath, "' to '", newPath, "'");
        return true;

    }

    bool BehaviourTreeEditor::SaveAs(const std::string& sourcePath,
        const std::string& newPath,
        const std::string& newName,
        bool generateNewGUID) {

        if (sourcePath.empty() || newPath.empty()) {
            LOG_ERROR("BehaviourTreeEditor::SaveAs: Empty path provided");
            return false;
        }

        // Step 1 - Load the source tree
        auto tree = BehaviourTreeEditor::LoadTree(sourcePath);
        if (!tree) {
            LOG_ERROR("BehaviourTreeEditor::SaveAs: Failed to load source tree from '", sourcePath, "'");
            return false;
        }

        // Step 2 - Update name if provided
        if (!newName.empty()) {
            tree->SetName(newName);
        }
        else {
            // Extract name from path if not provided
            std::string extractedName = newPath;
            size_t extPos = extractedName.find_last_of('.');

            if (extPos != std::string::npos) {
                extractedName = extractedName.substr(0, extPos);
            }
            size_t pathSep = extractedName.find_last_of("/\\");
            if (pathSep != std::string::npos) {
                extractedName = extractedName.substr(pathSep + 1);
            }
            tree->SetName(extractedName);
        }

        // Step 3 - Generate a new GUID
        if (generateNewGUID) {
            tree->SetGUID(xresource::instance_guid::GenerateGUIDCopy());
            LOG_INFO("BehaviourTreeEditor::SaveAs: Generated new GUID for copy");
        }
        else {
            LOG_INFO("BehaviourTreeEditor::SaveAs: Keeping original GUID");
        }

        // Step 4: Save to new location
        if (!SaveTree(*tree, newPath)) {
            LOG_ERROR("BehaviourTreeEditor::SaveAs: Failed to save to '", newPath, "'");
            return false;
        }

        LOG_INFO("BehaviourTreeEditor::SaveAs: Successfully saved copy from '",
            sourcePath, "' to '", newPath, "'");
        return true;
    }

} // namespace Engine

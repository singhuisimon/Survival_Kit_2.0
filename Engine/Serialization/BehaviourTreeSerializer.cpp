/**
 * @file BehaviourTreeSerializer.cpp
 * @brief Definition of BehaviourTreeSerializer for serializing and deserializing behaviour trees to/from JSON files.
 * @author Amanda Leow Boon Suan (100%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "../BehaviourTree/BehaviourTree.h"
#include "../BehaviourTree/BTNodeRegistry.h"
#include "BehaviourTreeSerializer.h"
#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace Engine {


    /**
     * @brief Serialize a behaviour tree to JSON string
     */
    std::string BehaviourTreeSerializer::SerializeToString(const BehaviourTree& tree) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();
    
        // Tree metadata
        doc.AddMember("Name", rapidjson::Value(tree.GetName().c_str(), allocator), allocator);
        doc.AddMember("GUID", rapidjson::Value(std::to_string(tree.GetGUID().m_Value).c_str(), allocator), allocator);
        doc.AddMember("Version", "1.0", allocator);
    
        // Serialize root node
        if (tree.GetRootNode()) {
            rapidjson::Value rootNode = SerializeNode(tree.GetRootNode(), allocator);
            doc.AddMember("Root", rootNode, allocator);
        }
    
        // Convert to string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
    
        return buffer.GetString();
    }
    
    /**
     * @brief Serialize a behaviour tree to file
     */
    bool BehaviourTreeSerializer::SerializeToFile(const BehaviourTree& tree, const std::string& filepath, bool prefab) {
        std::string json = SerializeToString(tree);
    
        bool sourceSuccess = false;
        bool outputSuccess = false;

        std::string fullpath;

        if (prefab) {
            fullpath = "Sources/Prefabs/" + filepath;
        }
        else {
            fullpath = "Sources/BT/" + filepath;
        }

    
        // 1. Save to SOURCE folder (Resources/Sources/BT in project root)
        std::string sourcePath = GetSourceFilePath(fullpath);
        if (!sourcePath.empty()) {
            sourceSuccess = WriteToFile(json, sourcePath, "SOURCE");
        }
        else {
            LOG_WARNING("BehaviourTreeSerializer: Could not determine source path");
        }
    
        // 2. Save to OUTPUT folder (executable's Resources/Sources/BT)
        std::string outputPath = getAssetFilePath(fullpath);
        outputSuccess = WriteToFile(json, outputPath, "OUTPUT");

		LOG_INFO("BEHAVIOURTREESERIALIZER: OUTPUTPAHT IS : ", outputPath);
    
        // Log results
        if (sourceSuccess && outputSuccess) {
            LOG_INFO("BehaviourTreeSerializer: Saved tree '", tree.GetName(), "' to BOTH locations");
            return true;
        }
        else if (outputSuccess) {
            LOG_WARNING("BehaviourTreeSerializer: Saved to OUTPUT only (source save failed)");
            return true; // Still usable at runtime
        }
        else {
            LOG_ERROR("BehaviourTreeSerializer: Failed to save tree");
            return false;
        }
    }
    
    /**
     * @brief Deserialize a behaviour tree from JSON string
     */
    std::shared_ptr<BehaviourTree> BehaviourTreeSerializer::DeserializeFromString(const std::string& json) {
        rapidjson::Document doc;
        doc.Parse(json.c_str());
    
        if (doc.HasParseError()) {
            LOG_ERROR("BehaviourTreeSerializer: JSON parse error");
            return nullptr;
        }
    
        auto tree = std::make_shared<BehaviourTree>();
    
        // Load metadata
        if (doc.HasMember("Name")) {
            tree->SetName(doc["Name"].GetString());
        }
    
        if (doc.HasMember("GUID")) {
            uint64_t guid = std::stoull(doc["GUID"].GetString());
            tree->SetGUID(xresource::instance_guid{ guid });
        }
        else {
            LOG_WARNING("BehaviourTreeSerializer: No GUID in file, generating new one");
            tree->SetGUID(xresource::instance_guid::GenerateGUIDCopy());
        }
    
        // Deserialize root node
        if (doc.HasMember("Root")) {
            auto rootNode = DeserializeNode(doc["Root"]);
            tree->SetRootNode(rootNode);
        }
    
        LOG_INFO("BehaviourTreeSerializer: Loaded tree '", tree->GetName(), ", ",
            std::hex, tree->GetGUID().m_Value, std::dec, "')");
        return tree;
    }
    
    /**
     * @brief Deserialize a behaviour tree from file
     */
    std::shared_ptr<BehaviourTree> BehaviourTreeSerializer::DeserializeFromFile(const std::string& filepath, bool prefab) {

        std::string fullpath;

        if (prefab) {
            fullpath = "Sources/Prefab/" + filepath;
        }
        else {
            fullpath = "Sources/BT/" + filepath;
        }

        fullpath = getAssetFilePath(fullpath);
        
        std::ifstream file(fullpath);
        if (!file.is_open()) {
            LOG_ERROR("BehaviourTreeSerializer: Failed to open file: ", fullpath);
            return nullptr;
        }
    
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
    
        return DeserializeFromString(buffer.str());
    }
    
    
    
    /**
     * @brief Get the source file path (project root Resources folder)
     */
    std::string BehaviourTreeSerializer::GetSourceFilePath(const std::string& relativePath) {
        std::string formattedPath = relativePath;
    
        for (char& c : formattedPath) {
            if (c == '\\') c = '/';
        }
    
        // Remove leading slash if present to avoid double slash
        if (!formattedPath.empty() && (formattedPath[0] == '/' || formattedPath[0] == '\\')) {
            formattedPath = formattedPath.substr(1);
        }
    
        std::filesystem::path exeDir = std::filesystem::current_path();
    
        //obtain this path:C:\Users\Admin\source\repos\Survival_Kit_2.0 <example>
        //^this path is where engine, external, game, out, resources is
        //std::filesystem::path root = exeDir.parent_path().parent_path().parent_path().parent_path();
        std::filesystem::path root = getRepository();

        //obtain the root + resource
        std::filesystem::path fullpath = root / "Resources" / formattedPath;
        std::filesystem::path directory = fullpath.parent_path();
    
        if (!std::filesystem::exists(directory)) {
            std::error_code ec;
            std::filesystem::create_directories(directory, ec);
            if (ec) {
                LOG_ERROR("Failed to create directory: ", directory.string());
                return "";
            }
        }

        if (!std::filesystem::exists(fullpath.generic_string())) {
            LOG_WARNING("[ASSET PATH] RESOURCE NOT FOUND: ", fullpath.generic_string());
        }
    
        LOG_INFO("PATH IS: ", fullpath.generic_string());
        return fullpath.generic_string();
    }
    
    
    /**
     * @brief Write JSON string to file with directory creation
     */
    bool BehaviourTreeSerializer::WriteToFile(const std::string& json, const std::string& fullPath, const std::string& location) {
        // Create directory structure if it doesn't exist
        std::filesystem::path path(fullPath);
        std::filesystem::path directory = path.parent_path();
    
        if (!std::filesystem::exists(directory)) {
            std::error_code ec;
            std::filesystem::create_directories(directory, ec);
            if (ec) {
                LOG_ERROR("Failed to create directory (", location, "): ", directory.string());
                return false;
            }
        }
    
        // Write to file
        std::ofstream file(fullPath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for writing (", location, "): ", fullPath);
            return false;
        }
    
        file << json;
        file.close();
    
        LOG_INFO("Saved to ", location, ": ", fullPath);
        return true;
    }
    
    /**
     * @brief Serialize a single node to JSON
     */
    rapidjson::Value BehaviourTreeSerializer::SerializeNode(std::shared_ptr<BTNode> node, rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value nodeObj(rapidjson::kObjectType);
    
        // Node metadata
        nodeObj.AddMember("Type", rapidjson::Value(node->GetTypeName(), allocator), allocator);
        nodeObj.AddMember("Name", rapidjson::Value(node->GetName().c_str(), allocator), allocator);
        nodeObj.AddMember("GUID", rapidjson::Value(std::to_string(node->GetGUID().m_Value).c_str(), allocator), allocator);
    
        // Serialize properties
        std::vector<std::pair<std::string, std::string>> properties;
        node->GetProperties(properties);
    
        if (!properties.empty()) {
            rapidjson::Value propsObj(rapidjson::kObjectType);
            for (const auto& [key, value] : properties) {
                propsObj.AddMember(
                    rapidjson::Value(key.c_str(), allocator),
                    rapidjson::Value(value.c_str(), allocator),
                    allocator
                );
            }
            nodeObj.AddMember("Properties", propsObj, allocator);
        }
    
        // Serialize children
        const auto& children = node->GetChildren();
        if (!children.empty()) {
            rapidjson::Value childrenArray(rapidjson::kArrayType);
            for (const auto& child : children) {
                childrenArray.PushBack(SerializeNode(child, allocator), allocator);
            }
            nodeObj.AddMember("Children", childrenArray, allocator);
        }
    
        return nodeObj;
    }
    
    /**
     * @brief Deserialize a single node from JSON
     */
    std::shared_ptr<BTNode> BehaviourTreeSerializer::DeserializeNode(const rapidjson::Value& nodeObj) {
        if (!nodeObj.IsObject() || !nodeObj.HasMember("Type")) {
            LOG_ERROR("BehaviourTreeSerializer: Invalid node data");
            return nullptr;
        }
    
        std::string typeName = nodeObj["Type"].GetString();
    
        // Create node from registry
        auto node = BTNodeRegistry::Get().CreateNode(typeName);
        if (!node) {
            LOG_ERROR("BehaviourTreeSerializer: Failed to create node of type '", typeName, "'");
            return nullptr;
        }
    
        // Load metadata
        if (nodeObj.HasMember("Name")) {
            node->SetName(nodeObj["Name"].GetString());
        }
    
        if (nodeObj.HasMember("GUID")) {
            uint64_t guid = std::stoull(nodeObj["GUID"].GetString());
            node->SetGUID(xresource::instance_guid{ guid });
        }
    
        // Load properties
        if (nodeObj.HasMember("Properties")) {
            const auto& propsObj = nodeObj["Properties"];
            for (auto it = propsObj.MemberBegin(); it != propsObj.MemberEnd(); ++it) {
                node->SetProperty(it->name.GetString(), it->value.GetString());
            }
        }
    
        // Load children
        if (nodeObj.HasMember("Children")) {
            const auto& childrenArray = nodeObj["Children"];
            for (rapidjson::SizeType i = 0; i < childrenArray.Size(); ++i) {
                auto child = DeserializeNode(childrenArray[i]);
                if (child) {
                    node->AddChild(child);
                }
            }
        }
    
        return node;
    }

} // namespace Engine

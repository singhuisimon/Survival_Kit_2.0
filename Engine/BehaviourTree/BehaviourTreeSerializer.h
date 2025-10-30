/**
 * @file BehaviourTreeSerializer.h
 * @brief Serialization for behaviour trees (save/load to JSON)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BehaviourTree.h"
#include "BTNodeRegistry.h"
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
     * @brief Handles serialization and deserialization of behaviour trees
     */
    class BehaviourTreeSerializer {
    public:
        /**
         * @brief Serialize a behaviour tree to JSON string
         */
        static std::string SerializeToString(const BehaviourTree& tree) {
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
        static bool SerializeToFile(const BehaviourTree& tree, const std::string& filepath) {
            std::string json = SerializeToString(tree);

            bool sourceSuccess = false;
            bool outputSuccess = false;

            // 1. Save to SOURCE folder (Resources/Sources/BT in project root)
            std::string sourcePath = GetSourceFilePath(filepath);
            if (!sourcePath.empty()) {
                sourceSuccess = WriteToFile(json, sourcePath, "SOURCE");
            }
            else {
                LOG_WARNING("BehaviourTreeSerializer: Could not determine source path");
            }

            // 2. Save to OUTPUT folder (executable's Resources/Sources/BT)
            std::string outputPath = getAssetFilePath(filepath);
            outputSuccess = WriteToFile(json, outputPath, "OUTPUT");

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
        static std::shared_ptr<BehaviourTree> DeserializeFromString(const std::string& json) {
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

            // Deserialize root node
            if (doc.HasMember("Root")) {
                auto rootNode = DeserializeNode(doc["Root"]);
                tree->SetRootNode(rootNode);
            }

            LOG_INFO("BehaviourTreeSerializer: Loaded tree '", tree->GetName(), "'");
            return tree;
        }

        /**
         * @brief Deserialize a behaviour tree from file
         */
        static std::shared_ptr<BehaviourTree> DeserializeFromFile(const std::string& filepath) {
            
            std::string fullpath = getAssetFilePath(filepath);
            
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

    private:

        /**
 * @brief Get the source file path (project root Resources folder)
 */
        static std::string GetSourceFilePath(const std::string& relativePath) {
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
            std::filesystem::path root = exeDir.parent_path().parent_path().parent_path().parent_path();

            //obtain the root + resource
            std::filesystem::path fullpath = root / "Resources" / formattedPath;

            if (!std::filesystem::exists(fullpath)) {
                LOG_WARNING("[ASSET PATH] RESOURCE NOT FOUND: ", fullpath.string());
            }

            LOG_INFO("PATH IS: ", fullpath.string());
            return fullpath.string();
        }


        /**
         * @brief Write JSON string to file with directory creation
         */
        static bool WriteToFile(const std::string& json, const std::string& fullPath, const std::string& location) {
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
        static rapidjson::Value SerializeNode(std::shared_ptr<BTNode> node, rapidjson::Document::AllocatorType& allocator) {
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
        static std::shared_ptr<BTNode> DeserializeNode(const rapidjson::Value& nodeObj) {
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
    };

} // namespace Engine

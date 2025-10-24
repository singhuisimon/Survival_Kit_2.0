#include "AI/BehaviourTree/BehaviourTreeLoader.h"
#include "Utility/Logger.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

namespace Engine {

    std::unique_ptr<BehaviourTree> BehaviourTreeLoader::LoadFromFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("[BTLoader] Failed to open file: ", filepath);
            return nullptr;
        }

        rapidjson::IStreamWrapper stream(file);
        rapidjson::Document doc;
        doc.ParseStream(stream);

        if (doc.HasParseError()) {
            LOG_ERROR("[BTLoader] JSON parse error in ", filepath);
            return nullptr;
        }

        if (!doc.HasMember("name") || !doc.HasMember("root")) {
            LOG_ERROR("[BTLoader] Invalid behaviour tree format (missing 'name' or 'root')");
            return nullptr;
        }

        std::string treeName = doc["name"].GetString();
        std::unique_ptr<BehaviourTree> tree = std::make_unique<BehaviourTree>(treeName);

        const rapidjson::Value& root = doc["root"];
        auto rootNode = ParseNode(root);
        if (!rootNode) {
            LOG_ERROR("[BTLoader] Failed to parse root node in ", filepath);
            return nullptr;
        }

        tree->SetRoot(std::move(rootNode));
        LOG_INFO("[BTLoader] Successfully loaded behaviour tree: ", treeName);

        return tree;
    }

    std::unique_ptr<BehaviourNode> BehaviourTreeLoader::ParseNode(const rapidjson::Value& nodeData) {
        if (!nodeData.HasMember("type") || !nodeData.HasMember("name")) {
            LOG_WARNING("[BTLoader] Node missing 'type' or 'name'");
            return nullptr;
        }

        std::string type = nodeData["type"].GetString();
        std::string name = nodeData["name"].GetString();

        auto node = CreateNodeByType(type, name);
        if (!node) return nullptr;

        // Recursively handle children
        if (nodeData.HasMember("children") && nodeData["children"].IsArray()) {
            for (auto& child : nodeData["children"].GetArray()) {
                auto childNode = ParseNode(child);
                if (childNode)
                    node->AddChild(std::move(childNode));
            }
        }

        return node;
    }

    std::unique_ptr<BehaviourNode> BehaviourTreeLoader::CreateNodeByType(const std::string& type, const std::string& name) {
        // For now, all nodes use base BehaviourNode.
        // Later, extend with derived types (Sequence, Selector, etc.)
        std::unique_ptr<BehaviourNode> node = std::make_unique<BehaviourNode>(name);
        LOG_TRACE("[BTLoader] Created node type: ", type, " (", name, ")");
        return node;
    }

} // namespace Engine

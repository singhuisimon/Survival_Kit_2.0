#include "AI/BehaviourTree/BehaviourTreeLoader.h"
#include "Utility/Logger.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

namespace Engine {

    std::unique_ptr<BehaviourTree> BehaviourTreeLoader::LoadFromFile(const std::string& filepath) {
        LOG_INFO("[BTLoader] Loading tree from: ", filepath);

        // Read file contents
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("[BTLoader] Failed to open file: ", filepath);
            return nullptr;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonContent = buffer.str();
        file.close();

        // Parse JSON
        rapidjson::Document doc;
        doc.Parse(jsonContent.c_str());

        if (doc.HasParseError()) {
            LOG_ERROR("[BTLoader] JSON parse error at offset ", doc.GetErrorOffset());
            return nullptr;
        }

        // Validate basic structure
        if (!doc.IsObject()) {
            LOG_ERROR("[BTLoader] Root must be a JSON object");
            return nullptr;
        }

        // Read tree name
        std::string treeName = "UnnamedTree";
        if (doc.HasMember("name") && doc["name"].IsString()) {
            treeName = doc["name"].GetString();
        }

        // Create tree
        auto tree = std::make_unique<BehaviourTree>(treeName);

        // Read description (optional)
        if (doc.HasMember("description") && doc["description"].IsString()) {
            LOG_INFO("[BTLoader] Tree description: ", doc["description"].GetString());
        }

        // Parse root node
        if (!doc.HasMember("root")) {
            LOG_ERROR("[BTLoader] Tree has no 'root' field");
            return nullptr;
        }

        const rapidjson::Value& rootData = doc["root"];
        std::unique_ptr<BehaviourNode> rootNode = ParseNode(rootData);

        if (!rootNode) {
            LOG_ERROR("[BTLoader] Failed to parse root node");
            return nullptr;
        }

        tree->SetRoot(std::move(rootNode));
        LOG_INFO("[BTLoader] Successfully loaded tree: ", treeName);

        return tree;
    }

    std::unique_ptr<BehaviourNode> BehaviourTreeLoader::ParseNode(const rapidjson::Value& nodeData) {
        if (!nodeData.IsObject()) {
            LOG_ERROR("[BTLoader] Node data must be an object");
            return nullptr;
        }

        // Read node type
        if (!nodeData.HasMember("type") || !nodeData["type"].IsString()) {
            LOG_ERROR("[BTLoader] Node missing 'type' field");
            return nullptr;
        }

        std::string type = nodeData["type"].GetString();

        // Read node name (optional)
        std::string name = type; // Default to type name
        if (nodeData.HasMember("name") && nodeData["name"].IsString()) {
            name = nodeData["name"].GetString();
        }

        // Create node by type
        std::unique_ptr<BehaviourNode> node = CreateNodeByType(type, name);
        if (!node) {
            LOG_ERROR("[BTLoader] Failed to create node of type: ", type);
            return nullptr;
        }

        // Parse parameters (if any)
        if (nodeData.HasMember("params") && nodeData["params"].IsObject()) {
            const rapidjson::Value& params = nodeData["params"];
            // TODO: Apply parameters to node (node-type specific)
            LOG_TRACE("[BTLoader] Node has parameters (not yet implemented)");
        }

        // Parse children (for composite nodes)
        if (nodeData.HasMember("children") && nodeData["children"].IsArray()) {
            const rapidjson::Value& childrenArray = nodeData["children"];

            for (rapidjson::SizeType i = 0; i < childrenArray.Size(); ++i) {
                std::unique_ptr<BehaviourNode> childNode = ParseNode(childrenArray[i]);
                if (childNode) {
                    node->AddChild(std::move(childNode));
                }
                else {
                    LOG_WARNING("[BTLoader] Failed to parse child ", i, " of node: ", name);
                }
            }
        }

        LOG_TRACE("[BTLoader] Parsed node: ", name, " (type: ", type, ")");
        return node;
    }

    std::unique_ptr<BehaviourNode> BehaviourTreeLoader::CreateNodeByType(
        const std::string& type,
        const std::string& name) {

        // Composite nodes
        //if (type == "Sequence") {
        //    return std::make_unique<SequenceNode>(name);
        //}
        //else if (type == "Selector") {
        //    return std::make_unique<SelectorNode>(name);
        //}
        //// Decorator nodes
        //else if (type == "Inverter") {
        //    return std::make_unique<InverterNode>(name);
        //}
        //else if (type == "Repeater") {
        //    return std::make_unique<RepeaterNode>(name);
        //}
        // Add more node types here as you implement them
        // Leaf nodes (actions, conditions)
        // else if (type == "Wait") {
        //     return std::make_unique<WaitNode>(name);
        // }
        // else if (type == "MoveToTarget") {
        //     return std::make_unique<MoveToTargetNode>(name);
        // }
        /*else {
            LOG_ERROR("[BTLoader] Unknown node type: ", type);
            return nullptr;
        }*/

        return nullptr;
    }

} // namespace Engine

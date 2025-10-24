#pragma once
#include <string>
#include <memory>
#include "AI/BehaviourTree/BehaviourTree.h"
#include "AI/BehaviourTree/BehaviourNode.h"
#include <rapidjson/document.h>

namespace Engine {

    /**
     * @brief Loads and builds BehaviourTree objects from JSON using RapidJSON.
     */
    class BehaviourTreeLoader {
    public:
        BehaviourTreeLoader() = default;
        ~BehaviourTreeLoader() = default;

        /**
         * @brief Load a BehaviourTree from a JSON file.
         * @param filepath Full path to the .json tree file.
         * @return Unique pointer to the loaded BehaviourTree, or nullptr on failure.
         */
        std::unique_ptr<BehaviourTree> LoadFromFile(const std::string& filepath);

    private:
        /**
         * @brief Recursively parses a RapidJSON node into a BehaviourNode hierarchy.
         */
        std::unique_ptr<BehaviourNode> ParseNode(const rapidjson::Value& nodeData);

        /**
         * @brief Creates a node instance by its "type" string.
         */
        std::unique_ptr<BehaviourNode> CreateNodeByType(const std::string& type, const std::string& name);
    };

} // namespace Engine

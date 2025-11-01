/**
 * @file BehaviourTreeSerializer.h
 * @brief Serialization for behaviour trees (save/load to JSON)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "../BehaviourTree/BehaviourTree.h"
#include "../BehaviourTree/BTNodeRegistry.h"
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
        static std::string SerializeToString(const BehaviourTree& tree);

        /**
         * @brief Serialize a behaviour tree to file
         */
        static bool SerializeToFile(const BehaviourTree& tree, const std::string& filepath, bool prefab = false);

        /**
         * @brief Deserialize a behaviour tree from JSON string
         */
        static std::shared_ptr<BehaviourTree> DeserializeFromString(const std::string& json);

        /**
         * @brief Deserialize a behaviour tree from file
         */
        static std::shared_ptr<BehaviourTree> DeserializeFromFile(const std::string& filepath, bool prefab = false);

    private:

        /**
        * @brief Get the source file path (project root Resources folder)
        */
        static std::string GetSourceFilePath(const std::string& relativePath);


        /**
         * @brief Write JSON string to file with directory creation
         */
        static bool WriteToFile(const std::string& json, const std::string& fullPath, const std::string& location);

        /**
         * @brief Serialize a single node to JSON
         */
        static rapidjson::Value SerializeNode(std::shared_ptr<BTNode> node, rapidjson::Document::AllocatorType& allocator);

        /**
         * @brief Deserialize a single node from JSON
         */
        static std::shared_ptr<BTNode> DeserializeNode(const rapidjson::Value& nodeObj);
    };

} // namespace Engine

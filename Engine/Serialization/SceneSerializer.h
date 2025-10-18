#pragma once
#include <string>
#include <memory>

namespace Engine {

    // Forward declarations
    class Scene;

    /**
     * @brief Serializes/Deserializes scenes to/from JSON
     */
    class SceneSerializer {
    public:
        SceneSerializer(Scene* scene);

        /**
         * @brief Serialize scene to JSON file
         * @param filepath Path to output file
         * @return True if successful
         */
        bool Serialize(const std::string& filepath);

        /**
         * @brief Serialize scene to JSON string
         * @return JSON string representation
         */
        std::string SerializeToString();

        /**
         * @brief Deserialize scene from JSON file
         * @param filepath Path to input file
         * @return True if successful
         */
        bool Deserialize(const std::string& filepath);

        /**
         * @brief Deserialize scene from JSON string
         * @param jsonString JSON string to parse
         * @return True if successful
         */
        bool DeserializeFromString(const std::string& jsonString);

    private:
        Scene* m_Scene;
    };

} // namespace Engine
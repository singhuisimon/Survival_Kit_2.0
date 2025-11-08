/**
 * @file BTBlackboard.h
 * @brief Type-safe blackboard for behaviour tree data storage
 * @author Amanda Leow Boon Suan (90%)
 * @date 5/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <glm/glm.hpp>
#include "../Utility/Logger.h"

namespace Engine {

    // Forward declaration
    class Entity;

    /**
     * @brief Type-safe blackboard value storage
     * @details Uses std::variant to store different types safely
     */
    using BlackboardValue = std::variant<
        bool,
        int,
        float,
        std::string,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        Entity*  // Pointer to entity for entity references
    >;

    /**
     * @brief Type-safe blackboard for behaviour tree data storage
     * @details Provides type-safe get/set operations with proper error handling
     */
    class BTBlackboard {
    public:
        /**
         * @brief Set a value in the blackboard
         * @tparam T The type of value to store
         * @param key The key to store the value under
         * @param value The value to store
         */
        template<typename T>
        void Set(const std::string& key, const T& value) {
            m_Data[key] = value;
        }

        /**
         * @brief Get a value from the blackboard
         * @tparam T The type of value to retrieve
         * @param key The key to retrieve the value from
         * @return Optional containing the value if found and correct type, empty otherwise
         */
        template<typename T>
        std::optional<T> Get(const std::string& key) const {
            auto it = m_Data.find(key);
            if (it == m_Data.end()) {
                return std::nullopt;
            }

            try {
                return std::get<T>(it->second);
            }
            catch (const std::bad_variant_access&) {
                LOG_WARNING("BTBlackboard: Type mismatch for key '", key, "'");
                return std::nullopt;
            }
        }

        /**
         * @brief Get a value with a default if not found
         * @tparam T The type of value to retrieve
         * @param key The key to retrieve the value from
         * @param defaultValue The value to return if key not found
         * @return The stored value or default
         */
        template<typename T>
        T GetOrDefault(const std::string& key, const T& defaultValue) const {
            auto value = Get<T>(key);
            return value.value_or(defaultValue);
        }

        /**
         * @brief Check if a key exists in the blackboard
         * @param key The key to check
         * @return True if key exists, false otherwise
         */
        bool Has(const std::string& key) const {
            return m_Data.find(key) != m_Data.end();
        }

        /**
         * @brief Remove a key from the blackboard
         * @param key The key to remove
         * @return True if key was removed, false if it didn't exist
         */
        bool Remove(const std::string& key) {
            return m_Data.erase(key) > 0;
        }

        /**
         * @brief Clear all values from the blackboard
         */
        void Clear() {
            m_Data.clear();
        }

        /**
         * @brief Get all keys in the blackboard
         * @return Vector of all keys
         */
        std::vector<std::string> GetAllKeys() const {
            std::vector<std::string> keys;
            keys.reserve(m_Data.size());
            for (const auto& [key, value] : m_Data) {
                keys.push_back(key);
            }
            return keys;
        }

        /**
         * @brief Get the type name of a stored value
         * @param key The key to check
         * @return String representing the type, or "Unknown" if not found
         */
        std::string GetTypeName(const std::string& key) const {
            auto it = m_Data.find(key);
            if (it == m_Data.end()) {
                return "Unknown";
            }

            return std::visit([](auto&& arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>) return "bool";
                else if constexpr (std::is_same_v<T, int>) return "int";
                else if constexpr (std::is_same_v<T, float>) return "float";
                else if constexpr (std::is_same_v<T, std::string>) return "string";
                else if constexpr (std::is_same_v<T, glm::vec2>) return "vec2";
                else if constexpr (std::is_same_v<T, glm::vec3>) return "vec3";
                else if constexpr (std::is_same_v<T, glm::vec4>) return "vec4";
                else if constexpr (std::is_same_v<T, Entity*>) return "entity";
                else return "unknown";
            }, it->second);
        }

        /**
         * @brief Convert a value to string (for serialization/debugging)
         * @param key The key to convert
         * @return String representation of the value, or empty string if not found
         */
        std::string ToString(const std::string& key) const {
            auto it = m_Data.find(key);
            if (it == m_Data.end()) {
                return "";
            }

            return std::visit([](auto&& arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>) {
                    return arg ? "true" : "false";
                }
                else if constexpr (std::is_same_v<T, int>) {
                    return std::to_string(arg);
                }
                else if constexpr (std::is_same_v<T, float>) {
                    return std::to_string(arg);
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, glm::vec2>) {
                    return std::to_string(arg.x) + "," + std::to_string(arg.y);
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    return std::to_string(arg.x) + "," + std::to_string(arg.y) + "," + std::to_string(arg.z);
                }
                else if constexpr (std::is_same_v<T, glm::vec4>) {
                    return std::to_string(arg.x) + "," + std::to_string(arg.y) + "," + 
                           std::to_string(arg.z) + "," + std::to_string(arg.w);
                }
                else if constexpr (std::is_same_v<T, Entity*>) {
                    return arg ? "Entity" : "null";
                }
                else {
                    return "";
                }
            }, it->second);
        }

        /**
         * @brief Set a value from string (for deserialization)
         * @param key The key to set
         * @param typeName The type name (bool, int, float, string, vec2, vec3, vec4, entity)
         * @param value The string value to parse
         * @return True if successfully set, false otherwise
         */
        bool FromString(const std::string& key, const std::string& typeName, const std::string& value) {
            try {
                if (typeName == "bool") {
                    Set(key, value == "true" || value == "1");
                }
                else if (typeName == "int") {
                    Set(key, std::stoi(value));
                }
                else if (typeName == "float") {
                    Set(key, std::stof(value));
                }
                else if (typeName == "string") {
                    Set(key, value);
                }
                else if (typeName == "vec2") {
                    size_t pos = value.find(',');
                    if (pos == std::string::npos) return false;
                    glm::vec2 vec;
                    vec.x = std::stof(value.substr(0, pos));
                    vec.y = std::stof(value.substr(pos + 1));
                    Set(key, vec);
                }
                else if (typeName == "vec3") {
                    size_t pos1 = value.find(',');
                    size_t pos2 = value.find(',', pos1 + 1);
                    if (pos1 == std::string::npos || pos2 == std::string::npos) return false;
                    glm::vec3 vec;
                    vec.x = std::stof(value.substr(0, pos1));
                    vec.y = std::stof(value.substr(pos1 + 1, pos2 - pos1 - 1));
                    vec.z = std::stof(value.substr(pos2 + 1));
                    Set(key, vec);
                }
                else if (typeName == "vec4") {
                    size_t pos1 = value.find(',');
                    size_t pos2 = value.find(',', pos1 + 1);
                    size_t pos3 = value.find(',', pos2 + 1);
                    if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) return false;
                    glm::vec4 vec;
                    vec.x = std::stof(value.substr(0, pos1));
                    vec.y = std::stof(value.substr(pos1 + 1, pos2 - pos1 - 1));
                    vec.z = std::stof(value.substr(pos2 + 1, pos3 - pos2 - 1));
                    vec.w = std::stof(value.substr(pos3 + 1));
                    Set(key, vec);
                }
                else if (typeName == "entity") {
                    // For entity, we'll just store nullptr for now
                    // This needs to be handled specially during deserialization
                    Set<Entity*>(key, nullptr);
                }
                else {
                    return false;
                }
                return true;
            }
            catch (...) {
                LOG_WARNING("BTBlackboard: Failed to parse value for key '", key, "'");
                return false;
            }
        }

        /**
         * @brief Get the number of entries in the blackboard
         */
        size_t Size() const {
            return m_Data.size();
        }

        /**
         * @brief Check if the blackboard is empty
         */
        bool Empty() const {
            return m_Data.empty();
        }

    private:
        std::unordered_map<std::string, BlackboardValue> m_Data;
    };

} // namespace Engine

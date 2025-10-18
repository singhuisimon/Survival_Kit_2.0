#pragma once
#include "Property.h"

// Standard Library
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <string>

namespace Engine {

    /**
     * @brief Central registry for component reflection metadata
     * @details Singleton that stores property information for all component types
     */
    class ReflectionRegistry {
    public:
        static ReflectionRegistry& Get() {
            static ReflectionRegistry instance;
            return instance;
        }

        ReflectionRegistry(const ReflectionRegistry&) = delete;
        ReflectionRegistry& operator=(const ReflectionRegistry&) = delete;

        /**
         * @brief Register a component type with its properties
         */
        template<typename T>
        ComponentMetadata& RegisterComponent(const std::string& name) {
            std::type_index typeId(typeid(T));

            // Create metadata and get pointer before moving
            auto metadata = std::make_unique<ComponentMetadata>(name);
            ComponentMetadata* metadataPtr = metadata.get();

            // Store in maps
            m_ComponentMetadata.insert(std::make_pair(typeId, std::move(metadata)));
            m_ComponentsByName.insert(std::make_pair(name, typeId));

            return *metadataPtr;
        }

        /**
         * @brief Get component metadata by type
         */
        template<typename T>
        ComponentMetadata* GetMetadata() {
            std::type_index typeId(typeid(T));
            auto it = m_ComponentMetadata.find(typeId);
            return (it != m_ComponentMetadata.end()) ? it->second.get() : nullptr;
        }

        /**
         * @brief Get component metadata by name
         */
        ComponentMetadata* GetMetadata(const std::string& name) {
            auto it = m_ComponentsByName.find(name);
            if (it != m_ComponentsByName.end()) {
                auto metaIt = m_ComponentMetadata.find(it->second);
                if (metaIt != m_ComponentMetadata.end()) {
                    return metaIt->second.get();
                }
            }
            return nullptr;
        }

        /**
         * @brief Get all registered component types
         */
        const std::unordered_map<std::type_index, std::unique_ptr<ComponentMetadata>>&
            GetAllMetadata() const {
            return m_ComponentMetadata;
        }

        /**
         * @brief Check if a component type is registered
         */
        template<typename T>
        bool IsRegistered() const {
            std::type_index typeId(typeid(T));
            return m_ComponentMetadata.find(typeId) != m_ComponentMetadata.end();
        }

    private:
        ReflectionRegistry() = default;

        std::unordered_map<std::type_index, std::unique_ptr<ComponentMetadata>> m_ComponentMetadata;
        std::unordered_map<std::string, std::type_index> m_ComponentsByName;
    };

    /**
     * @brief Helper macro to register component properties
     */
#define REGISTER_COMPONENT(Type) \
        Engine::ReflectionRegistry::Get().RegisterComponent<Type>(#Type)

} // namespace Engine
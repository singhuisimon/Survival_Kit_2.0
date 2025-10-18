#pragma once
#include <entt/entt.hpp>
#include <cstdint>

namespace Engine {

    /**
     * @brief Entity wrapper around EnTT's entity handle
     * @details Provides a cleaner API for component operations
     */
    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity handle, entt::registry* registry)
            : m_EntityHandle(handle), m_Registry(registry) {
        }

        /**
         * @brief Add a component to this entity
         * @tparam T Component type
         * @tparam Args Constructor arguments
         * @return Reference to the created component
         */
        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            if (HasComponent<T>()) {
                // Component already exists, return existing
                return GetComponent<T>();
            }
            return m_Registry->emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        /**
         * @brief Get a component from this entity
         * @tparam T Component type
         * @return Reference to the component
         */
        template<typename T>
        T& GetComponent() {
            return m_Registry->get<T>(m_EntityHandle);
        }

        /**
         * @brief Check if entity has a component
         * @tparam T Component type
         * @return True if component exists
         */
        template<typename T>
        bool HasComponent() {
            return m_Registry->all_of<T>(m_EntityHandle);
        }

        /**
         * @brief Remove a component from this entity
         * @tparam T Component type
         */
        template<typename T>
        void RemoveComponent() {
            m_Registry->remove<T>(m_EntityHandle);
        }

        // Conversion operators
        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint32_t() const { return (uint32_t)m_EntityHandle; }

        // Comparison operators
        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Registry == other.m_Registry;
        }

        bool operator!=(const Entity& other) const {
            return !(*this == other);
        }

    private:
        entt::entity m_EntityHandle = entt::null;
        entt::registry* m_Registry = nullptr;
    };

} // namespace Engine
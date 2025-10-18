#pragma once
#include "Entity.h"
#include <entt/entt.hpp>
#include <string>

namespace Engine {

    // Forward declaration
    class SceneSerializer;

    /**
     * @brief Scene manages a collection of entities and their components
     * @details Wraps EnTT registry and provides high-level scene management
     */
    class Scene {
    public:
        Scene(const std::string& name = "Untitled Scene");
        ~Scene() = default;

        /**
         * @brief Create a new entity in this scene
         * @param name Optional name for the entity
         * @return The created entity
         */
        Entity CreateEntity(const std::string& name = "Entity");

        /**
         * @brief Destroy an entity and all its components
         * @param entity The entity to destroy
         */
        void DestroyEntity(Entity entity);

        /**
         * @brief Update all systems in the scene
         * @param deltaTime Time elapsed since last frame
         */
        void OnUpdate(float deltaTime);

        /**
         * @brief Get the underlying EnTT registry
         * @return Reference to the registry
         */
        entt::registry& GetRegistry() { return m_Registry; }

        /**
         * @brief Get scene name
         */
        const std::string& GetName() const { return m_Name; }

        /**
         * @brief Set scene name
         */
        void SetName(const std::string& name) { m_Name = name; }

        /**
         * @brief Save scene to file
         * @param filepath Path to save location
         * @return True if successful
         */
        bool SaveToFile(const std::string& filepath);

        /**
         * @brief Load scene from file
         * @param filepath Path to scene file
         * @return True if successful
         */
        bool LoadFromFile(const std::string& filepath);

    private:
        std::string m_Name;
        entt::registry m_Registry;

        friend class SceneSerializer;
    };

} // namespace Engine
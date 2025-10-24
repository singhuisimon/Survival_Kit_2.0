#pragma once
#include "Entity.h"
#include "ECS/SystemRegistry.h"
#include <entt/entt.hpp>
#include <string>
#include "../xresource_guid/include/xresource_guid.h"

namespace Engine {

    // Forward declarations
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

        // ===== PREFAB SYSTEM =====

        /**
         * @brief Create entity from prefab
         * @param prefabGUID GUID of the prefab to instantiate
         * @param name Optional name for the entity (overrides prefab name)
         * @return The created entity
         */
        Entity CreateEntityFromPrefab(
            xresource::instance_guid prefabGUID,
            const std::string& name = ""
        );

        /**
         * @brief Instantiate scene prefab into this scene
         * @param prefabGUID GUID of the scene prefab
         * @return The root entity of the instantiated scene prefab
         */
        Entity InstantiateScenePrefab(xresource::instance_guid prefabGUID);

        /**
         * @brief Convert prefab instance to regular entity (unpack/break prefab link)
         * @param entity Entity to unpack
         */
        void UnpackPrefabInstance(Entity entity);

        // ===== SYSTEM MANAGEMENT =====

        /**
         * @brief Add a system to the scene
         * @tparam T System type
         * @tparam Args Constructor argument types
         * @param args Arguments to pass to system constructor
         * @return Pointer to the created system
         *
         * Example:
         * ```cpp
         * auto* physics = scene->AddSystem<PhysicsSystem>();
         * auto* renderer = scene->AddSystem<RenderSystem>(width, height);
         * ```
         */
        template<typename T, typename... Args>
        T* AddSystem(Args&&... args) {
            return m_SystemRegistry.AddSystem<T>(std::forward<Args>(args)...);
        }

        /**
         * @brief Remove a system from the scene
         * @tparam T System type to remove
         * @return True if system was found and removed
         */
        template<typename T>
        bool RemoveSystem() {
            return m_SystemRegistry.RemoveSystem<T>();
        }

        /**
         * @brief Get a system from the scene
         * @tparam T System type to get
         * @return Pointer to system, or nullptr if not found
         */
        template<typename T>
        T* GetSystem() {
            return m_SystemRegistry.GetSystem<T>();
        }

        /**
         * @brief Check if scene has a system
         * @tparam T System type to check
         * @return True if system exists
         */
        template<typename T>
        bool HasSystem() const {
            return m_SystemRegistry.HasSystem<T>();
        }

        /**
         * @brief Get the system registry
         */
        SystemRegistry& GetSystemRegistry() { return m_SystemRegistry; }

        /**
         * @brief Initialize all systems
         * @details Called automatically when scene is loaded/created
         */
        void InitializeSystems() {
            m_SystemRegistry.OnInit(this);
        }

        /**
         * @brief Shutdown all systems
         * @details Called automatically when scene is destroyed
         */
        void ShutdownSystems() {
            m_SystemRegistry.OnShutdown(this);
        }

    private:
        std::string m_Name;
        entt::registry m_Registry;
        SystemRegistry m_SystemRegistry;

        friend class SceneSerializer;
    };

} // namespace Engine
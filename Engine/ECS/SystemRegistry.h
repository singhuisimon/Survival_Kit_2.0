#pragma once
#include "System.h"
#include "../Utility/Logger.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace Engine {

    // Forward declaration to avoid circular dependency
    class Scene;

    /**
     * @brief Registry for managing all systems in a scene
     * @details Handles system creation, destruction, and execution order
     */
    class SystemRegistry {
    public:
        SystemRegistry() = default;
        ~SystemRegistry() = default;

        // Delete copy operations
        SystemRegistry(const SystemRegistry&) = delete;
        SystemRegistry& operator=(const SystemRegistry&) = delete;

        /**
         * @brief Add a new system to the registry
         * @tparam T System type (must inherit from System)
         * @tparam Args Constructor argument types
         * @param args Arguments to pass to system constructor
         * @return Pointer to the created system
         */
        template<typename T, typename... Args>
        T* AddSystem(Args&&... args) {
            static_assert(std::is_base_of<System, T>::value,
                "T must inherit from System");

            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = system.get();

            LOG_INFO("Adding system: ", system->GetName(),
                " (Priority: ", system->GetPriority(), ")");

            m_Systems.push_back(std::move(system));
            SortSystems();

            return ptr;
        }

        /**
         * @brief Remove a system by type
         * @tparam T System type to remove
         * @return True if system was found and removed
         */
        template<typename T>
        bool RemoveSystem() {
            static_assert(std::is_base_of<System, T>::value,
                "T must inherit from System");

            for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it) {
                if (dynamic_cast<T*>(it->get())) {
                    LOG_INFO("Removing system: ", (*it)->GetName());
                    m_Systems.erase(it);
                    return true;
                }
            }

            LOG_WARNING("System not found for removal");
            return false;
        }

        /**
         * @brief Get a system by type
         * @tparam T System type to get
         * @return Pointer to system, or nullptr if not found
         */
        template<typename T>
        T* GetSystem() {
            static_assert(std::is_base_of<System, T>::value,
                "T must inherit from System");

            for (auto& system : m_Systems) {
                if (T* ptr = dynamic_cast<T*>(system.get())) {
                    return ptr;
                }
            }
            return nullptr;
        }

        /**
         * @brief Check if a system exists
         * @tparam T System type to check
         * @return True if system exists
         */
        template<typename T>
        bool HasSystem() const {
            static_assert(std::is_base_of<System, T>::value,
                "T must inherit from System");

            for (const auto& system : m_Systems) {
                if (dynamic_cast<T*>(system.get())) {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Initialize all systems
         * @param scene The scene to initialize systems with
         */
        void OnInit(Scene* scene) {
            LOG_INFO("Initializing ", m_Systems.size(), " systems...");

            for (auto& system : m_Systems) {
                LOG_TRACE("  Initializing: ", system->GetName());
                system->OnInit(scene);
            }

            LOG_INFO("All systems initialized");
        }

        /**
         * @brief Update all enabled systems
         * @param scene The scene to update
         * @param ts Time elapsed since last frame
         */
        void OnUpdate(Scene* scene, Timestep ts) {
            for (auto& system : m_Systems) {
                if (system->IsEnabled()) {
                    system->OnUpdate(scene, ts);
                }
            }
        }

        /**
         * @brief Shutdown all systems
         * @param scene The scene being shut down
         */
        void OnShutdown(Scene* scene) {
            LOG_INFO("Shutting down ", m_Systems.size(), " systems...");

            // Shutdown in reverse order
            for (auto it = m_Systems.rbegin(); it != m_Systems.rend(); ++it) {
                LOG_TRACE("  Shutting down: ", (*it)->GetName());
                (*it)->OnShutdown(scene);
            }

            m_Systems.clear();
            LOG_INFO("All systems shut down");
        }

        /**
         * @brief Get number of systems
         */
        size_t GetSystemCount() const {
            return m_Systems.size();
        }

        /**
         * @brief Get all systems (const)
         */
        const std::vector<std::unique_ptr<System>>& GetSystems() const {
            return m_Systems;
        }

    private:
        /**
         * @brief Sort systems by priority (lower = earlier execution)
         */
        void SortSystems() {
            std::sort(m_Systems.begin(), m_Systems.end(),
                [](const std::unique_ptr<System>& a, const std::unique_ptr<System>& b) {
                    return a->GetPriority() < b->GetPriority();
                });

            // Log system order
            LOG_TRACE("System execution order:");
            for (const auto& system : m_Systems) {
                LOG_TRACE("  [", system->GetPriority(), "] ", system->GetName());
            }
        }

        std::vector<std::unique_ptr<System>> m_Systems;
    };

} // namespace Engine
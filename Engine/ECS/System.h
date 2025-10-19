#pragma once
#include "../Utility/Timestep.h"

namespace Engine {

    // Forward declaration to avoid circular dependency
    class Scene;

    /**
     * @brief Base class for all game systems
     * @details Systems process components every frame in a specific order.
     *          Inherit from this to create your own systems (PhysicsSystem, RenderSystem, etc.)
     *
     * Example:
     * ```cpp
     * class PhysicsSystem : public System {
     * public:
     *     void OnUpdate(Scene* scene, Timestep ts) override {
     *         auto view = scene->GetRegistry().view<TransformComponent, RigidbodyComponent>();
     *         for (auto entity : view) {
     *             // Process physics...
     *         }
     *     }
     *     int GetPriority() const override { return 10; } // Run early
     * };
     * ```
     */
    class System {
    public:
        virtual ~System() = default;

        /**
         * @brief Called once when system is added to scene
         * @param scene The scene this system belongs to
         * @details Use this to initialize system-specific data
         */
        virtual void OnInit(Scene* scene) {
            (void)scene; // Suppress unused parameter warning
        }

        /**
         * @brief Called every frame to update the system
         * @param scene The scene to operate on
         * @param ts Time elapsed since last frame
         * @details This is where your system logic goes
         */
        virtual void OnUpdate(Scene* scene, Timestep ts) = 0;

        /**
         * @brief Called when system is removed or scene is destroyed
         * @param scene The scene this system belongs to
         * @details Use this to cleanup system-specific resources
         */
        virtual void OnShutdown(Scene* scene) {
            (void)scene; // Suppress unused parameter warning
        }

        /**
         * @brief Get system execution priority
         * @return Priority value (lower = earlier execution)
         * @details Default is 100. Common values:
         *   - Input: 0
         *   - Physics: 10
         *   - Animation: 20
         *   - Transform: 30
         *   - Rendering: 100
         *   - UI: 200
         */
        virtual int GetPriority() const {
            return 100;
        }

        /**
         * @brief Get system name for debugging
         */
        virtual const char* GetName() const {
            return "System";
        }

        /**
         * @brief Check if system is enabled
         */
        bool IsEnabled() const {
            return m_Enabled;
        }

        /**
         * @brief Enable or disable system
         * @param enabled True to enable, false to disable
         * @details Disabled systems are not updated
         */
        void SetEnabled(bool enabled) {
            m_Enabled = enabled;
        }

    protected:
        bool m_Enabled = true;
    };

} // namespace Engine
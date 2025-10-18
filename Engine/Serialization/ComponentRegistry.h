#pragma once

namespace Engine {

    /**
     * @brief Centralized component registration
     * @details Call RegisterAllComponents() at startup to register all component types
     */
    class ComponentRegistry {
    public:
        /**
         * @brief Register all component types with the reflection system
         * @details Must be called before any serialization operations
         */
        static void RegisterAllComponents();
    };

} // namespace Engine
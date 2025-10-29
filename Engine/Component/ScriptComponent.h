#pragma once
#include <string>
#include <memory>
#include <cstdint>

namespace Engine {

    /**
     * @brief Component that attaches a C# script to an entity
     * @details The script can implement OnStart, OnUpdate, and OnDestroy methods
     */
    struct ScriptComponent {
        std::string ScriptClassName;  // e.g., "Game.PlayerController"
        void* ScriptInstance = nullptr;  // Mono object instance
        bool Started = false;

        ScriptComponent() = default;
        ScriptComponent(const std::string& className) 
            : ScriptClassName(className) {}
    };

} // namespace Engine

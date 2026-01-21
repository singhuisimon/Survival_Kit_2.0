#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include "../Serialization/ComponentRegistry.h"

namespace Engine {

    /**
     * @brief Component that attaches a C# script to an entity
     * @details The script can implement OnStart, OnUpdate, and OnDestroy methods
     */
    struct ScriptComponent {
        static constexpr ComponentTypeID TypeID = ComponentTypeID::Script;
        static constexpr const char* TypeName = "ScriptComponent";

        xresource::instance_guid ComponentGUID;
        std::string ScriptClassName;  // e.g., "Game.PlayerController"
        void* ScriptInstance = nullptr;  // Mono object instance
        bool Started = false;
        uint32_t GCHandle = 0;

        ScriptComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
        {
        }

        ScriptComponent(const std::string& className) 
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , ScriptClassName(className) {}
    };
    
} // namespace Engine

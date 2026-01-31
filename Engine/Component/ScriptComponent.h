#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../Serialization/ComponentRegistry.h"

// ScriptComponent.h MUST be self-contained.
// It uses xresource::instance_guid, so it must include whichever header defines it.
// We use __has_include so it works regardless of where you actually defined xresource.

#if __has_include("../Asset/ResourceHelpers.h")
#include "../Asset/ResourceHelpers.h"
#elif __has_include("../Asset/ResourceManager.h")
#include "../Asset/ResourceManager.h"
#elif __has_include("../Asset/AssetManager.h")
#include "../Asset/AssetManager.h"
#elif __has_include("../Asset/ResourceTypes.h")
#include "../Asset/ResourceTypes.h"
#elif __has_include("xresource.h")
#include "xresource.h"
#elif __has_include(<xresource/instance_guid.hpp>)
#include <xresource/instance_guid.hpp>
#else
#error "ScriptComponent.h requires a header that defines xresource::instance_guid"
#endif

namespace Engine {

    /**
     * @brief Component that attaches a C# script to an entity
     * @details The script can implement OnStart, OnUpdate, and OnDestroy methods
     */
    struct ScriptComponent {
        static constexpr ComponentTypeID TypeID = ComponentTypeID::Script;
        static constexpr const char *TypeName = "ScriptComponent";

        xresource::instance_guid ComponentGUID;
        std::string ScriptClassName;

        // Runtime-only (must never be serialized to disk)
        void *ScriptInstance = nullptr;
        bool Started = false;
        uint32_t GCHandle = 0;

        // Persisted editor-authored values (safe across STOP/hotreload)
        std::unordered_map<std::string, std::vector<std::uint8_t>> SerializedFields;

        ScriptComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()) {}

        ScriptComponent(const std::string &className)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , ScriptClassName(className) {}
    };

} // namespace Engine

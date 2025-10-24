#pragma once
#include <string>
#include <vector>

namespace sk2
{
    // Attach to any entity that should run C# scripts.
    struct ScriptComponent
    {
        // Fully-qualified type names (in ManagedScripts.dll), e.g. "TestScript" or "Player.Move"
        std::vector<std::string> Scripts;
        bool Bound{ false }; // set by ScriptingSystem once instances are created
    };
}

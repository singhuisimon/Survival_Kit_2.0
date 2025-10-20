#pragma once
#include <string>
#include <vector>

namespace Engine {

    /**
     * @brief Component that allows C# scripts to be attached to entities
     */
    struct ScriptComponent {
        // List of script class names attached to this entity
        std::vector<std::string> ScriptNames;

        // Whether scripts are enabled
        bool Enabled = true;

        // Instance IDs for the managed side (internal use)
        std::vector<int> ScriptInstanceIds;

        ScriptComponent() = default;

        ScriptComponent(const std::string& scriptName) {
            AddScript(scriptName);
        }

        void AddScript(const std::string& scriptName) {
            // Check if script already exists
            auto it = std::find(ScriptNames.begin(), ScriptNames.end(), scriptName);
            if (it == ScriptNames.end()) {
                ScriptNames.push_back(scriptName);
            }
        }

        void RemoveScript(const std::string& scriptName) {
            auto it = std::find(ScriptNames.begin(), ScriptNames.end(), scriptName);
            if (it != ScriptNames.end()) {
                ScriptNames.erase(it);
            }
        }

        bool HasScript(const std::string& scriptName) const {
            return std::find(ScriptNames.begin(), ScriptNames.end(), scriptName) != ScriptNames.end();
        }

        void ClearScripts() {
            ScriptNames.clear();
            ScriptInstanceIds.clear();
        }
    };

} // namespace Engine
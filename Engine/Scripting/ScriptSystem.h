#pragma once
#include "ECS/System.h"
#include "ScriptComponent.h"
#include <memory>

// Forward declarations
namespace Core {
    class Application;
}

namespace Engine {

    class Scene;

    /**
     * @brief System that manages and executes C# scripts
     */
    class ScriptSystem : public System {
    public:
        ScriptSystem();
        ~ScriptSystem();

        void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, Timestep ts) override;
        void OnShutdown(Scene* scene) override;

        int GetPriority() const override { return 50; } // Run in game logic phase
        const char* GetName() const override { return "ScriptSystem"; }

        // Script management
        bool CreateScript(const std::string& scriptName);
        void ReloadScripts();
        void CheckForHotReload();

        // Static access for debugging
        static ScriptSystem* GetInstance() { return s_Instance; }

    private:
        std::unique_ptr<Core::Application> m_ScriptEngine;
        bool m_Initialized = false;
        float m_HotReloadCheckTimer = 0.0f;
        Scene* m_Scene = nullptr;

        void InitializeScriptEngine();
        void ShutdownScriptEngine();
        void AttachScriptsToEntities(Scene* scene);
        void InitializeBridge(Scene* scene);
        void ShutdownBridge();

        static ScriptSystem* s_Instance;
    };

} // namespace Engine
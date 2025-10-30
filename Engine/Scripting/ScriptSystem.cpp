#include "ScriptSystem.h"
#include "MonoScriptEngine.h"
#include "../Component/ScriptComponent.h"
#include "../ECS/Scene.h"
#include "../Utility/Logger.h"
#include <mono/metadata/object.h>
#include "ScriptReloader.h"
#include <filesystem>  // ADD THIS for filesystem operations

namespace Engine {

    static Scene* s_CurrentScene = nullptr;

    void ScriptSystem::OnInit(Scene* scene) {
        s_CurrentScene = scene;
        m_Scene = scene;  // Store it in member variable too
        LOG_INFO("[ScriptSystem] Initialized");
    }
    void ScriptSystem::OnUpdate(Scene* scene, Timestep ts) {

        if (ScriptReloader::GetInstance().IsReloadRequested()) {
            LOG_INFO("[Hot-Reload] Reloading scripts...");

            // Step 1: Destroy all script instances
            auto& registry = scene->GetRegistry();
            auto view = registry.view<ScriptComponent>();

            for (auto entity : view) {
                auto& script = view.get<ScriptComponent>(entity);
                if (script.ScriptInstance) {
                    MonoScriptEngine::GetInstance().DestroyScriptInstance(
                        (MonoObject*)script.ScriptInstance);
                    script.ScriptInstance = nullptr;
                    script.Started = false;
                }
            }

            // Step 2: Unload old assembly (this releases the DLL lock!)
           // LOG_INFO("[Hot-Reload] Unloading old assembly...");
            std::string tempDllPath = ScriptReloader::GetInstance().GetTempDllPath();
            MonoScriptEngine::GetInstance().ReloadAssembly();


            // Step 3: Swap temp DLL to real DLL
            //std::string tempDllPath = ScriptReloader::GetInstance().GetTempDllPath();
            if (!tempDllPath.empty()) {
                try {
                    std::string finalDllPath = "GameScripts.dll";

                    // Delete old DLL (now safe since assembly is unloaded)
                    if (std::filesystem::exists(finalDllPath)) {
                        std::filesystem::remove(finalDllPath);
                        LOG_INFO("[Hot-Reload] Deleted old DLL");
                    }

                    // Rename temp to final
                    std::filesystem::rename(tempDllPath, finalDllPath);
                    LOG_INFO("[Hot-Reload]  New DLL installed: ", finalDllPath);

                    ScriptReloader::GetInstance().ClearTempDllPath();
                }
                catch (const std::exception& e) {
                    LOG_ERROR("[Hot-Reload] Failed to swap DLL: ", e.what());
                }
            }

            // Step 4: Reload assembly with new DLL
            //MonoScriptEngine::GetInstance().ReloadAssembly();

            // Clear reload request
            ScriptReloader::GetInstance().ClearReloadRequest();

            LOG_INFO("[Hot-Reload] Complete! Scripts will be recreated.");
        }
        float deltaTime = ts.GetSeconds();

        auto& registry = scene->GetRegistry();
        auto view = registry.view<ScriptComponent>();

        static int totalUpdates = 0;
        totalUpdates++;

        for (auto entity : view) {
            auto& script = view.get<ScriptComponent>(entity);

            if (script.ScriptClassName.empty()) {
                continue;
            }

            // Create instance if needed
            if (!script.ScriptInstance) {
                LOG_INFO("[ScriptSystem] Creating script instance: ", script.ScriptClassName);

                script.ScriptInstance = MonoScriptEngine::GetInstance()
                    .CreateScriptInstance(script.ScriptClassName);

                if (!script.ScriptInstance) {
                    LOG_ERROR("[ScriptSystem] Failed to create script instance");
                    continue;
                }

                MonoScriptEngine::GetInstance().SetFieldValue(
                    (MonoObject*)script.ScriptInstance, "EntityID", &entity);
            }

            if (script.ScriptInstance) {
                // Call OnStart
                if (!script.Started) {
                    LOG_INFO("[ScriptSystem] Calling OnStart");
                    MonoScriptEngine::GetInstance().CallMethod(
                        (MonoObject*)script.ScriptInstance, "OnStart");
                    script.Started = true;
                }

                // Call OnUpdate - ADD DETAILED LOGS HERE
                if (totalUpdates % 60 == 0) {  // Log every 60 frames
                   // LOG_INFO("[ScriptSystem] *** Calling C# OnUpdate (update #", totalUpdates, ") ***");
                }

                void* params[1] = { &deltaTime };
                MonoScriptEngine::GetInstance().CallMethod(
                    (MonoObject*)script.ScriptInstance, "OnUpdate",
                    params, 1);

                if (totalUpdates % 60 == 0) {
                    //LOG_INFO("[ScriptSystem] *** C# OnUpdate returned successfully ***");
                }
            }
        }
    }



    void ScriptSystem::OnShutdown(Scene* scene) {  // Added Scene* parameter
        auto& registry = scene->GetRegistry();  // Use parameter instead of s_CurrentScene
        auto view = registry.view<ScriptComponent>();

        for (auto entity : view) {
            auto& script = view.get<ScriptComponent>(entity);
            if (script.ScriptInstance) {
                MonoScriptEngine::GetInstance().CallMethod(
                    (MonoObject*)script.ScriptInstance, "OnDestroy");
                MonoScriptEngine::GetInstance().DestroyScriptInstance(
                    (MonoObject*)script.ScriptInstance);
            }
        }

        s_CurrentScene = nullptr;
        m_Scene = nullptr;
        LOG_INFO("[ScriptSystem] Shutdown");
    }

} // namespace Engine

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

        // Step 2: Reload assembly (releases old DLL lock)
        LOG_INFO("[Hot-Reload] Reloading assembly...");
        MonoScriptEngine::GetInstance().ReloadAssembly();

        // Step 3: NOW swap the DLL (wait a bit to ensure lock is released)
        std::string tempDllPath = ScriptReloader::GetInstance().GetTempDllPath();
        if (!tempDllPath.empty()) {
            try {
                std::string finalDllPath = "GameScripts.dll";

                // Delete old DLL (retry if needed)
                int retries = 3;
                while (retries > 0 && std::filesystem::exists(finalDllPath)) {
                    try {
                        std::filesystem::remove(finalDllPath);
                        LOG_INFO("[Hot-Reload] Old DLL deleted");
                        break;
                    }
                    catch (const std::exception& e) {
                        LOG_WARNING("[Hot-Reload] Delete failed, retry... (", retries - 1, " left)");
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        retries--;
                    }
                }

                // Rename temp to final (retry if needed)
                retries = 3;
                while (retries > 0) {
                    try {
                        std::filesystem::rename(tempDllPath, finalDllPath);
                        LOG_INFO("[Hot-Reload]  New DLL installed successfully!");
                        ScriptReloader::GetInstance().ClearTempDllPath();
                        break;
                    }
                    catch (const std::exception& e) {
                        LOG_WARNING("[Hot-Reload] Rename failed, retry... (", retries - 1, " left)");
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        retries--;

                        if (retries == 0) {
                            LOG_ERROR("[Hot-Reload] Failed to swap DLL after retries: ", e.what());
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("[Hot-Reload] Unexpected error: ", e.what());
            }
        }

        ScriptReloader::GetInstance().ClearReloadRequest();
        LOG_INFO("[Hot-Reload] Complete!");
    
        float deltaTime = ts.GetSeconds();

       // auto& registry = scene->GetRegistry();
        //auto view = registry.view<ScriptComponent>();

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

#include "ScriptSystem.h"
#include "MonoScriptEngine.h"
#include "../Component/ScriptComponent.h"
#include "../ECS/Scene.h"
#include "../Utility/Logger.h"
#include <mono/metadata/object.h>

namespace Engine {

    static Scene* s_CurrentScene = nullptr;

    void ScriptSystem::OnInit(Scene* scene) {
        s_CurrentScene = scene;
        m_Scene = scene;  // Store it in member variable too
        LOG_INFO("[ScriptSystem] Initialized");
    }

    void ScriptSystem::OnUpdate(Scene* scene, Timestep ts) {  // Changed signature
        float deltaTime = ts.GetSeconds();  // Extract float from Timestep

        auto& registry = scene->GetRegistry();
        auto view = registry.view<ScriptComponent>();

        for (auto entity : view) {
            auto& script = view.get<ScriptComponent>(entity);

            // Create instance if needed
            if (!script.ScriptInstance && !script.ScriptClassName.empty()) {
                script.ScriptInstance = MonoScriptEngine::GetInstance()
                    .CreateScriptInstance(script.ScriptClassName);

                if (script.ScriptInstance) {
                    // Set entity ID
                    MonoScriptEngine::GetInstance().SetFieldValue(
                        (MonoObject*)script.ScriptInstance, "EntityID",
                        &entity);
                }
            }

            if (script.ScriptInstance) {
                // Call OnStart if not started
                if (!script.Started) {
                    MonoScriptEngine::GetInstance().CallMethod(
                        (MonoObject*)script.ScriptInstance, "OnStart");
                    script.Started = true;
                }

                // Call OnUpdate
                void* params[1] = { &deltaTime };
                MonoScriptEngine::GetInstance().CallMethod(
                    (MonoObject*)script.ScriptInstance, "OnUpdate",
                    params, 1);
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

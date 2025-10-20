#include "ScriptSystem.h"
#include "ScriptComponent.h"
#include "IScriptEngineInterface.h"
#include "ECS/Scene.h"
#include "ECS/Components.h"
#include "Utility/Logger.h"

// Include ScriptCore headers
#include "../../ScriptCore/Application.h"
#include "../../ScriptCore/ScriptBridge.h"

namespace Engine {

    ScriptSystem* ScriptSystem::s_Instance = nullptr;

    ScriptSystem::ScriptSystem() {
        s_Instance = this;
        m_ScriptEngine = std::make_unique<Core::Application>();
    }

    ScriptSystem::~ScriptSystem() {
        if (m_Initialized) {
            ShutdownScriptEngine();
        }
        s_Instance = nullptr;
    }

    void ScriptSystem::OnInit(Scene* scene) {
        LOG_INFO("=== Initializing Script System ===");
        m_Scene = scene;

        // Initialize the bridge first
        InitializeBridge(scene);

        // Then initialize the script engine
        InitializeScriptEngine();

        // Finally, attach scripts to entities
        AttachScriptsToEntities(scene);

        LOG_INFO("=== Script System Ready ===");
    }

    void ScriptSystem::OnUpdate(Scene* scene, Timestep ts) {
        if (!m_Initialized) return;

        // Check for hot reload every 2 seconds
        m_HotReloadCheckTimer += ts;
        if (m_HotReloadCheckTimer >= 2.0f) {
            CheckForHotReload();
            m_HotReloadCheckTimer = 0.0f;
        }

        // Update scripts for entities with ScriptComponent
        auto& registry = scene->GetRegistry();
        auto view = registry.view<ScriptComponent>();

        for (auto entity : view) {
            auto& scriptComp = view.get<ScriptComponent>(entity);
            if (scriptComp.Enabled && !scriptComp.ScriptNames.empty()) {
                // Update this entity's scripts
                int entityId = static_cast<int>(static_cast<uint32_t>(entity));
                m_ScriptEngine->UpdateScriptForEntity(entityId);
            }
        }
    }

    void ScriptSystem::OnShutdown(Scene* scene) {
        LOG_INFO("Shutting down Script System");
        ShutdownScriptEngine();
        ShutdownBridge();
        m_Scene = nullptr;
    }

    void ScriptSystem::InitializeBridge(Scene* scene) {
        LOG_INFO("Initializing Script - ECS Bridge...");

        // Initialize the Engine side interface
        IScriptEngineInterface::Initialize(scene);

        // Pass the function pointers to ScriptCore
        Core::ScriptBridge::Initialize(
            // Convert std::function to raw function pointers using lambdas
            [](const std::string& name) -> uint32_t {
                return IScriptEngineInterface::CreateEntity ?
                    IScriptEngineInterface::CreateEntity(name) : 0;
            },
            [](uint32_t id) {
                if (IScriptEngineInterface::DestroyEntity)
                    IScriptEngineInterface::DestroyEntity(id);
            },
            [](uint32_t id) -> std::string {
                return IScriptEngineInterface::GetEntityName ?
                    IScriptEngineInterface::GetEntityName(id) : "";
            },
            [](uint32_t id) -> glm::vec3 {
                return IScriptEngineInterface::GetPosition ?
                    IScriptEngineInterface::GetPosition(id) : glm::vec3(0);
            },
            [](uint32_t id, const glm::vec3& pos) {
                if (IScriptEngineInterface::SetPosition)
                    IScriptEngineInterface::SetPosition(id, pos);
            },
            [](uint32_t id) -> glm::vec3 {
                return IScriptEngineInterface::GetRotation ?
                    IScriptEngineInterface::GetRotation(id) : glm::vec3(0);
            },
            [](uint32_t id, const glm::vec3& rot) {
                if (IScriptEngineInterface::SetRotation)
                    IScriptEngineInterface::SetRotation(id, rot);
            },
            [](uint32_t id) -> glm::vec3 {
                return IScriptEngineInterface::GetScale ?
                    IScriptEngineInterface::GetScale(id) : glm::vec3(1);
            },
            [](uint32_t id, const glm::vec3& scale) {
                if (IScriptEngineInterface::SetScale)
                    IScriptEngineInterface::SetScale(id, scale);
            }
        );

        LOG_INFO("Bridge initialized successfully");
    }

    void ScriptSystem::ShutdownBridge() {
        Core::ScriptBridge::Shutdown();
        IScriptEngineInterface::Shutdown();
    }

    void ScriptSystem::InitializeScriptEngine() {
        try {
            LOG_INFO("Starting C# script engine...");
            m_ScriptEngine->InitializeScripting();
            m_Initialized = true;
            LOG_INFO("Script engine initialized successfully");
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize script engine: ", e.what());
            m_Initialized = false;
        }
    }

    void ScriptSystem::ShutdownScriptEngine() {
        if (m_Initialized) {
            m_ScriptEngine->ShutdownScripting();
            m_Initialized = false;
        }
    }

    void ScriptSystem::AttachScriptsToEntities(Scene* scene) {
        LOG_INFO("Attaching scripts to entities...");

        auto& registry = scene->GetRegistry();
        auto view = registry.view<ScriptComponent, TagComponent>();

        int attachedCount = 0;
        for (auto entity : view) {
            auto& scriptComp = view.get<ScriptComponent>(entity);
            auto& tag = view.get<TagComponent>(entity);

            if (scriptComp.ScriptNames.empty()) continue;

            // Add each script to the entity in the managed side
            int entityId = static_cast<int>(static_cast<uint32_t>(entity));

            for (const auto& scriptName : scriptComp.ScriptNames) {
                if (m_ScriptEngine->AddScript(entityId, scriptName.c_str())) {
                    LOG_INFO("   Attached '", scriptName, "' to '", tag.Tag, "' (ID: ", entityId, ")");
                    attachedCount++;
                }
                else {
                    LOG_WARNING("   Failed to attach '", scriptName, "' to '", tag.Tag, "'");
                }
            }
        }

        LOG_INFO("Attached ", attachedCount, " script(s) total");
    }

    void ScriptSystem::CheckForHotReload() {
        m_ScriptEngine->CheckAndReloadScripts();

        // Re-attach scripts after reload
        if (m_Scene) {
            AttachScriptsToEntities(m_Scene);
        }
    }

    void ScriptSystem::ReloadScripts() {
        LOG_INFO("Manual script reload requested...");
        m_ScriptEngine->ReloadScripts();

        if (m_Scene) {
            AttachScriptsToEntities(m_Scene);
        }
    }

    bool ScriptSystem::CreateScript(const std::string& scriptName) {
        return m_ScriptEngine->CreateMonoBehaviourScript(scriptName);
    }

} // namespace Engine
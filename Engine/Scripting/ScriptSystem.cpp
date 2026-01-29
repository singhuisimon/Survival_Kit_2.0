#include "ScriptSystem.h"
#include "MonoScriptEngine.h"
#include "ScriptHandleUtils.h"

#include "../Component/ScriptComponent.h"
#include "../ECS/Scene.h"
#include "../Utility/Logger.h"

namespace Engine
{
	static Scene *s_CurrentScene = nullptr;

	void ScriptSystem::OnInit(Scene *scene)
	{
		s_CurrentScene = scene;
		SetScriptingCurrentScene(scene);

		m_Scene = scene;
		LOG_INFO("[ScriptSystem] Initialized");
	}

  void ScriptSystem::OnUpdate(Scene *scene, Timestep ts)
  {
    if (m_IsShuttingDown) return;

    float deltaTime = ts.GetSeconds();
    auto &registry = scene->GetRegistry();
    auto view = registry.view<ScriptComponent>();
    auto &se = MonoScriptEngine::GetInstance();

    m_FixedAccumulator += deltaTime;
    if (m_FixedAccumulator > m_FixedDeltaSeconds * m_MaxFixedSubsteps)
      m_FixedAccumulator = m_FixedDeltaSeconds * m_MaxFixedSubsteps;

    std::vector<entt::entity> entities(view.begin(), view.end());

    for (auto entity : entities)
    {
      if (!registry.valid(entity)) continue;
      auto *script = registry.try_get<ScriptComponent>(entity);
      if (!script || script->ScriptClassName.empty()) continue;

      if (script->GCHandle == 0)
      {
        MonoObject *created = nullptr;
        uint32_t handle = se.CreateScriptInstanceHandle(
          script->ScriptClassName, &created, false);

        if (handle == 0 || !created) continue;

        se.BindEntityID(created, static_cast<uint32_t>(entity));
        script->GCHandle = handle;
        script->Started = false;
      }

      MonoObject *inst = se.GetObjectFromGCHandle(script->GCHandle);
      if (!inst)
      {
        LOG_WARNING("[ScriptSystem] Failed to resolve: ", script->ScriptClassName);
        se.DestroyScriptHandle(script->GCHandle);
        script->GCHandle = 0;
        script->Started = false;
        continue;
      }

      if (!script->Started)
      {
        se.CallMethod(inst, "OnStart");
        script->Started = true;
      }
    }

    int steps = 0;
    while (m_FixedAccumulator >= m_FixedDeltaSeconds && steps < m_MaxFixedSubsteps)
    {
      float fixedDt = m_FixedDeltaSeconds;
      void *fixedParams[1] = { &fixedDt };

      for (auto entity : entities)
      {
        if (!registry.valid(entity)) continue;
        auto *script = registry.try_get<ScriptComponent>(entity);
        if (!script || !script->Started || script->GCHandle == 0) continue;

        MonoObject *inst = se.GetObjectFromGCHandle(script->GCHandle);
        if (inst)
        {
          se.CallMethod(inst, "OnFixedUpdate", fixedParams, 1);
        }
      }

      m_FixedAccumulator -= m_FixedDeltaSeconds;
      ++steps;
    }

    // Pass 3: Update (once per frame)
    void *params[1] = { &deltaTime };
    for (auto entity : entities)
    {
      if (!registry.valid(entity)) continue;
      auto *script = registry.try_get<ScriptComponent>(entity);
      if (!script || !script->Started || script->GCHandle == 0) continue;

      MonoObject *inst = se.GetObjectFromGCHandle(script->GCHandle);
      if (inst)
      {
        se.CallMethod(inst, "OnUpdate", params, 1);
      }
    }
  }

	void ScriptSystem::OnShutdown(Scene *scene)
	{
		m_IsShuttingDown = true;

		auto &registry = scene->GetRegistry();
		auto view = registry.view<ScriptComponent>();

		auto &se = MonoScriptEngine::GetInstance();

		for (auto entity : view)
		{
			auto &script = view.get<ScriptComponent>(entity);

			if (script.GCHandle != 0)
			{
				se.DestroyScriptHandle(script.GCHandle);
				script.GCHandle = 0;
			}

			script.ScriptInstance = nullptr;
			script.Started = false;
		}

		s_CurrentScene = nullptr;
		m_Scene = nullptr;
		LOG_INFO("[ScriptSystem] Shutdown");
	}
} // namespace Engine

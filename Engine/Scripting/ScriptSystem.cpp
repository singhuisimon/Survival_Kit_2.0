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
		if (m_IsShuttingDown)
			return;

		float deltaTime = ts.GetSeconds();

		auto &registry = scene->GetRegistry();
		auto view = registry.view<ScriptComponent>();

		auto &se = MonoScriptEngine::GetInstance();

		for (auto entity : view)
		{
			auto &script = view.get<ScriptComponent>(entity);

			if (script.ScriptClassName.empty())
				continue;

			// Heal legacy components (pointer present but no handle)
			ScriptHandleUtil::EnsureHandleIfLegacyPointerPresent(script, false);

			// Create instance if needed (HANDLE-FIRST)
			if (script.GCHandle == 0)
			{
				MonoObject *created = nullptr;
				uint32_t handle = se.CreateScriptInstanceHandle(script.ScriptClassName, &created, false);
				if (handle == 0 || !created)
					continue;

				se.BindEntityID(created, static_cast<std::uint32_t>(entity));

				script.GCHandle = handle;
				script.ScriptInstance = created; // cache
				script.Started = false;
			}

			MonoObject *inst = ScriptHandleUtil::Resolve(script);
			if (!inst)
			{
				// If domain was unloaded / handle broken, clear and try again next frame
				LOG_WARNING("[ScriptSystem] Script instance resolve failed for: ", script.ScriptClassName);
				if (script.GCHandle != 0)
					se.DestroyScriptHandle(script.GCHandle);

				script.GCHandle = 0;
				script.ScriptInstance = nullptr;
				script.Started = false;
				continue;
			}

			// Call OnStart once
			if (!script.Started)
			{
				se.CallMethod(inst, "OnStart");
				script.Started = true;
			}

			// Call OnUpdate every frame
			void *params[1] = { &deltaTime };
			se.CallMethod(inst, "OnUpdate", params, 1);
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

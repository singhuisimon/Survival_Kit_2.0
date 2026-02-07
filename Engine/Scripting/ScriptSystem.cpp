#include "ScriptSystem.h"
#include "MonoScriptEngine.h"
#include "ScriptHandleUtils.h"
#include "../Component/ScriptComponent.h"
#include "../ECS/Scene.h"
#include "../Utility/Logger.h"

namespace Engine {
	static Scene *s_CurrentScene = nullptr;

	void ScriptSystem::OnInit(Scene *scene) {
		if(m_Scene) {
			auto &se = MonoScriptEngine::GetInstance();
			se.EnsureAllScriptInstances(m_Scene, true);
		}

		s_CurrentScene = scene;
		SetScriptingCurrentScene(scene);
		m_Scene = scene;
		LOG_INFO("[ScriptSystem] Initialized");
	}

	void ScriptSystem::OnUpdate(Scene *scene, Timestep ts) {
		if(m_IsShuttingDown) return;

		float deltaTime = ts.GetSeconds();
		auto &registry = scene->GetRegistry();
		auto view = registry.view<ScriptComponent>();
		auto &se = MonoScriptEngine::GetInstance();

		m_FixedAccumulator += deltaTime;
		if(m_FixedAccumulator > m_FixedDeltaSeconds * m_MaxFixedSubsteps)
			m_FixedAccumulator = m_FixedDeltaSeconds * m_MaxFixedSubsteps;

		std::vector<entt::entity> entities(view.begin(), view.end());

		// Ensure instances
		for(auto entity : entities) {
			if(!registry.valid(entity)) continue;
			auto *script = registry.try_get<ScriptComponent>(entity);
			if(!script || script->ScriptClassName.empty()) continue;

			if(script->GCHandle != 0) {
				MonoObject *inst = se.GetObjectFromGCHandle(script->GCHandle);
				if(!inst || !se.InstanceMatchesClass(inst, script->ScriptClassName)) {
					se.DestroyScriptHandle(script->GCHandle);
					script->GCHandle = 0;
					script->ScriptInstance = nullptr;
					script->Started = false;
				}
			}

			if(script->GCHandle == 0) {
				MonoObject *created = nullptr;
				uint32_t handle = se.CreateScriptInstanceHandle(script->ScriptClassName, &created, false);
				if(handle && created) {
					se.BindEntityID(created, static_cast<uint32_t>(entity));
					se.ApplySerializedFieldsFromComponent(static_cast<uint32_t>(entity), created);
					script->GCHandle = handle;
					script->ScriptInstance = created;
					script->Started = false;
				}
			}

			if(script->GCHandle == 0) continue;

			// Resolve Fresh
			MonoObject *inst = se.GetObjectFromGCHandle(script->GCHandle);
			if(!inst) continue;

			script->ScriptInstance = inst;

			if(!script->Started) {
				se.CallMethod(inst, "OnStart");
				script->Started = true;
			}
		}

		// Fixed Update
		int steps = 0;
		while(m_FixedAccumulator >= m_FixedDeltaSeconds && steps < m_MaxFixedSubsteps) {
			float fixedDt = m_FixedDeltaSeconds;
			void *fixedParams[1] = { &fixedDt };
			for(auto entity : entities) {
				if(!registry.valid(entity)) continue;
				auto *script = registry.try_get<ScriptComponent>(entity);
				if(!script || !script->Started || script->GCHandle == 0) continue;
				MonoObject *inst = se.GetObjectFromGCHandle(script->GCHandle);
				if(inst) se.CallMethod(inst, "OnFixedUpdate", fixedParams, 1);
			}
			m_FixedAccumulator -= m_FixedDeltaSeconds;
			++steps;
		}

		// Update
		void *params[1] = { &deltaTime };
		for(auto entity : entities) {
			if(!registry.valid(entity)) continue;
			auto *script = registry.try_get<ScriptComponent>(entity);
			if(!script || !script->Started || script->GCHandle == 0) continue;
			MonoObject *inst = se.GetObjectFromGCHandle(script->GCHandle);
			if(inst) se.CallMethod(inst, "OnUpdate", params, 1);
		}
	}

	void ScriptSystem::OnShutdown(Scene *scene) {
		m_IsShuttingDown = true;
		auto &registry = scene->GetRegistry();
		auto view = registry.view<ScriptComponent>();
		auto &se = MonoScriptEngine::GetInstance();
		for(auto entity : view) {
			auto &script = view.get<ScriptComponent>(entity);
			if(script.GCHandle != 0) {
				// Call OnDestroy before destroying handle so scripts can unsubscribe events
				if(script.Started) {
					MonoObject *inst = se.GetObjectFromGCHandle(script.GCHandle);
					if(inst) {
						se.CallMethod(inst, "OnDestroy");
					}
				}
				se.DestroyScriptHandle(script.GCHandle);
				script.GCHandle = 0;
			}
			script.ScriptInstance = nullptr;
			script.Started = false;
		}
		s_CurrentScene = nullptr;
		m_Scene = nullptr;
	}
}
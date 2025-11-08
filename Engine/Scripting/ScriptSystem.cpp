#include "ScriptSystem.h"
#include "MonoScriptEngine.h"
#include "../Component/ScriptComponent.h"
#include "../ECS/Scene.h"
#include "../Utility/Logger.h"
#include <mono/metadata/object.h>
#include "ScriptReloader.h"
#include <filesystem>  // ADD THIS for filesystem operations

namespace Engine
{

	static Scene *s_CurrentScene = nullptr;

	void ScriptSystem::OnInit(Scene *scene)
	{
		s_CurrentScene = scene;
		SetScriptingCurrentScene(scene);

		m_Scene = scene;  // Store it in member variable too
		LOG_INFO("[ScriptSystem] Initialized");
	}
	void ScriptSystem::OnUpdate(Scene *scene, Timestep ts)
	{
		// Hot-reload check: Only reload if DLL file has actually changed
		static std::filesystem::file_time_type lastModifiedTime;
		static bool initialized = false;
		(void)ts;
		std::string dllPath = "GameScripts.dll.tmp";

		// Initialize the last modified time on first run
		if (!initialized && std::filesystem::exists(dllPath))
		{
			lastModifiedTime = std::filesystem::last_write_time(dllPath);
			initialized = true;
		}

		// Check if DLL has been modified
		bool shouldReload = false;
		if (std::filesystem::exists(dllPath))
		{
			auto currentModifiedTime = std::filesystem::last_write_time(dllPath);
			if (currentModifiedTime != lastModifiedTime)
			{
				shouldReload = true;
				lastModifiedTime = currentModifiedTime;
			}
		}

		// Only reload if file actually changed
		if (shouldReload)
		{
			LOG_INFO("[Hot-Reload] DLL change detected, reloading scripts...");

			// Step 1: Destroy all script instances
			auto &registry = scene->GetRegistry();
			auto view = registry.view<ScriptComponent>();

			for (auto entity : view)
			{
				auto &script = view.get<ScriptComponent>(entity);
				if (script.ScriptInstance)
				{
					MonoScriptEngine::GetInstance().DestroyScriptInstance(
						(MonoObject *)script.ScriptInstance);
					script.ScriptInstance = nullptr;
					script.Started = false;
				}
			}
			;
			std::filesystem::copy_file(ScriptReloader::GetInstance().GetTempDllPath(), dllPath,
				std::filesystem::copy_options::overwrite_existing);
			LOG_INFO("Copying .tmp to .dll to replace existing");


			// Step 2: Reload assembly (releases old DLL lock)
			LOG_INFO("[Hot-Reload] Reloading assembly...");
			MonoScriptEngine::GetInstance().ReloadAssembly();

			LOG_INFO("[Hot-Reload] Complete!");
		}

		// Normal script update logic (runs every frame)
		float deltaTime = ts.GetSeconds();
		auto &registry = scene->GetRegistry();
		auto view = registry.view<ScriptComponent>();

		for (auto entity : view)
		{
			auto &script = view.get<ScriptComponent>(entity);

			if (script.ScriptClassName.empty())
			{
				continue;
			}

			// Create instance if needed
			if (!script.ScriptInstance)
			{
				script.ScriptInstance = MonoScriptEngine::GetInstance()
					.CreateScriptInstance(script.ScriptClassName);

				if (!script.ScriptInstance)
				{
					LOG_ERROR("[ScriptSystem] Failed to create script instance: ", script.ScriptClassName);
					continue;
				}

				MonoScriptEngine::GetInstance().SetFieldValue(
					(MonoObject *)script.ScriptInstance, "EntityID", &entity);
			}

			if (script.ScriptInstance)
			{
				// Call OnStart once
				if (!script.Started)
				{
					MonoScriptEngine::GetInstance().CallMethod(
						(MonoObject *)script.ScriptInstance, "OnStart");
					script.Started = true;
				}

				// Call OnUpdate every frame
				void *params[1] = { &deltaTime };
				MonoScriptEngine::GetInstance().CallMethod(
					(MonoObject *)script.ScriptInstance, "OnUpdate",
					params, 1);
			}
		}
	}

	void ScriptSystem::OnShutdown(Scene *scene)
	{  // Added Scene* parameter
		auto &registry = scene->GetRegistry();  // Use parameter instead of s_CurrentScene
		auto view = registry.view<ScriptComponent>();

		for (auto entity : view)
		{
			auto &script = view.get<ScriptComponent>(entity);
			if (script.ScriptInstance)
			{
				MonoScriptEngine::GetInstance().CallMethod(
					(MonoObject *)script.ScriptInstance, "OnDestroy");
				MonoScriptEngine::GetInstance().DestroyScriptInstance(
					(MonoObject *)script.ScriptInstance);
			}
		}

		s_CurrentScene = nullptr;
		m_Scene = nullptr;
		LOG_INFO("[ScriptSystem] Shutdown");
	}

} // namespace Engine

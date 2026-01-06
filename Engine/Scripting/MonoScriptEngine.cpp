#include <windows.h>  // Add at top

#include "MonoScriptEngine.h"
#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h" 
#include "../ECS/Components.h"
#include "../Core/input.h"
#include "Core/Application.h"
#include "../Audio/AudioManager.h"
#include "../Event/EventSystem.h"
#include "ScriptReloader.h"


// Prefabs headers
#include "../Serialization/PrefabSerializer.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Prefab/PrefabRegistry.h"

#include "../Physics/PhysicsAPI.h"

// Mono headers
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

#include <filesystem>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "InternalCalls.h"


namespace Engine
{
	MonoScriptEngine &MonoScriptEngine::GetInstance()
	{
		static MonoScriptEngine instance;
		return instance;
	}

	bool MonoScriptEngine::IsValidMonoObject(MonoObject *instance)
	{
		if (!instance)
			return false;

		// Check if pointer looks valid (basic sanity check)
		if ((uintptr_t)instance == 0xFFFFFFFFFFFFFFFF ||
			(uintptr_t)instance < 0x10000)
		{
			LOG_WARNING("IsValidMonoObject: Invalid pointer detected");
			return false;
		}

		MonoDomain *currentDomain = mono_domain_get();
		if (!currentDomain)
		{
			LOG_WARNING("IsValidMonoObject: No current Mono domain");
			return false;
		}

		// Wrap in try-catch for safety (if using C++ exceptions)
		try
		{
			MonoDomain *instanceDomain = mono_object_get_domain(instance);
			if (!instanceDomain || instanceDomain != currentDomain)
			{
				LOG_WARNING("IsValidMonoObject: Instance is from a different/unloaded domain");
				return false;
			}
		}
		catch (...)
		{
			LOG_WARNING("IsValidMonoObject: Exception while checking domain");
			return false;
		}

		return true;
	}

	void MonoScriptEngine::EnsureCorrectDomain()
	{
		if (!m_AppDomain) return;

		MonoDomain *current = mono_domain_get();

		if (current != m_AppDomain)
		{
			LOG_WARNING("EnsureCorrectDomain: Wrong domain active!");
			LOG_WARNING("  Current:  " + std::to_string((uintptr_t)current));
			LOG_WARNING("  Expected: " + std::to_string((uintptr_t)m_AppDomain));
			LOG_WARNING("  Forcing switch to correct domain...");

			mono_domain_set(m_AppDomain, false);

			MonoDomain *after = mono_domain_get();
			if (after == m_AppDomain)
			{
				LOG_INFO("  Successfully switched to correct domain");
			}
			else
			{
				LOG_ERROR("   FAILED to switch domain!");
				LOG_ERROR("  After switch: " + std::to_string((uintptr_t)after));
			}
		}
	}

	bool MonoScriptEngine::IsInCorrectDomain()
	{
		if (!m_AppDomain) return false;
		return mono_domain_get() == m_AppDomain;
	}

	// Pointer to Engine.EventSystem.RaiseFromNative(string, string)
	static MonoMethod *s_EventSystemRaiseFromNative = nullptr;

	// Refresh the managed event binding whenever the C# assembly is (re)loaded
	static void RefreshEventBindings(MonoImage *appImage)
	{
		s_EventSystemRaiseFromNative = nullptr;

		if (!appImage)
			return;

		MonoClass *eventSystemClass = mono_class_from_name(appImage, "Engine", "EventSystem");
		if (!eventSystemClass)
		{
			LOG_WARNING("[Mono] Engine.EventSystem class not found - script events will not be delivered to C#");
			return;
		}

		s_EventSystemRaiseFromNative =
			mono_class_get_method_from_name(eventSystemClass, "RaiseFromNative", 2);

		if (!s_EventSystemRaiseFromNative)
		{
			LOG_WARNING("[Mono] Engine.EventSystem.RaiseFromNative(string,string) not found");
		}
	}

	void MonoScriptEngine::Initialize(const std::string &assemblyPath)
	{
		// Guard against double initialization
		static bool s_Initialized = false;
		if (s_Initialized)
		{
			LOG_WARNING("Mono Script Engine already initialized, skipping...");
			return;
		}

		LOG_INFO("Initializing Mono Script Engine...");

		m_AssemblyPath = assemblyPath;

		// Get the exe directory on Windows
		WCHAR exePath[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, exePath, MAX_PATH);

		std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
		std::filesystem::path monoLibPath = exeDir / "mono" / "lib";

		std::string monoLibPathStr = monoLibPath.generic_string();
		mono_set_assemblies_path(monoLibPathStr.c_str());

		LOG_INFO("[Mono] Assembly path set to: ", monoLibPathStr);

		// Verify mono/lib exists
		if (!std::filesystem::exists(monoLibPath))
		{
			LOG_ERROR("[Mono] ERROR: mono/lib not found at: ", monoLibPathStr);
			LOG_ERROR("[Mono] Make sure Mono runtime is in the build output directory");
		}

		// Initialize Mono JIT
		m_RootDomain = mono_jit_init("EngineRuntime");
		if (!m_RootDomain)
		{
			LOG_ERROR("Failed to initialize Mono JIT");
			return;
		}

		LOG_INFO(" Mono JIT initialized");
		LOG_INFO(" Root domain created: " + std::to_string((uintptr_t)m_RootDomain));
		m_AppDomain = m_RootDomain;
		// Create app domain
		//m_AppDomain = mono_domain_create_appdomain(const_cast<char *>("EngineAppDomain"), nullptr);

		LOG_INFO("Using single domain mode (no separate app domain)");
		LOG_INFO("  Root domain: " + std::to_string((uintptr_t)m_RootDomain));
		LOG_INFO("  App domain:  " + std::to_string((uintptr_t)m_AppDomain) + " (same as root)");



		/*	if (!m_AppDomain)
			{
				LOG_ERROR("Failed to create Mono app domain");
				return;
			}*/
		mono_domain_set(m_AppDomain, false);



		// Load assembly (only if it exists)
		if (!assemblyPath.empty() && std::filesystem::exists("GameScripts.dll"))
		{
			LoadAssembly("GameScripts.dll");
		}
		else
		{
			LOG_WARNING("Assembly not found: ", assemblyPath);
			LOG_WARNING("Mono initialized but no scripts will be loaded");
			LOG_WARNING("ScriptComponents will be ignored");
		}

		// Register internal calls (C++ functions callable from C#)
		RegisterInternalCalls();

		// Hook native ScriptEvent channel into managed Engine.EventSystem
		EventSystem::Instance().Subscribe<ScriptEvent>(
			[](ScriptEvent const &ev)
			{
				if (!s_EventSystemRaiseFromNative)
					return;

				MonoDomain *domain = mono_domain_get();
				if (!domain)
					return;

				MonoString *nameStr = mono_string_new(domain, ev.name.c_str());
				MonoString *payloadStr = mono_string_new(domain, ev.payload.c_str());

				void *args[2] = { nameStr, payloadStr };
				mono_runtime_invoke(s_EventSystemRaiseFromNative, nullptr, args, nullptr);
			});

		s_Initialized = true;
		LOG_INFO("Mono Script Engine initialized");

	}



	void MonoScriptEngine::Shutdown()
	{
		LOG_INFO("Shutting down Mono Script Engine...");

		// Unload assembly (clears mAppAssembly and mAppImage)
		UnloadAssembly();

		// The OS will clean up Mono memory when the process exits


		// Just null the pointers
		m_RootDomain = nullptr;
		m_AppDomain = nullptr;



		LOG_INFO("Mono Script Engine shut down");
	}


	void MonoScriptEngine::LoadAssembly(const std::string &path)
	{
		LOG_INFO("Loading C# assembly: ", path);

		if (!std::filesystem::exists(path))
		{
			LOG_ERROR("Assembly file not found: ", path);
			return;
		}

		// Load assembly from file
		m_AppAssembly = mono_domain_assembly_open(m_AppDomain, path.c_str());
		if (!m_AppAssembly)
		{
			LOG_ERROR("Failed to load assembly: ", path);
			return;
		}

		m_AppImage = mono_assembly_get_image(m_AppAssembly);
		if (!m_AppImage)
		{
			LOG_ERROR("Failed to get assembly image");
			return;
		}

		LOG_INFO("Assembly loaded successfully");

		// NEW: refresh event bindings to Engine.EventSystem
		RefreshEventBindings(m_AppImage);
	}

	void MonoScriptEngine::UnloadAssembly()
	{
		m_ClassCache.clear();
		m_AppImage = nullptr;
		m_AppAssembly = nullptr;
	}

	//void MonoScriptEngine::ReloadAssembly()
	//{
	//	LOG_INFO("Hot-reload: Starting...");
	//	ClearAllInstances();
	//	LOG_INFO("Hot-reload: Cleared instance tracking");
	//
	//	UnloadAssembly();
	//	mono_domain_set(m_RootDomain, false);
	//	mono_domain_unload(m_AppDomain);
	//
	//	LOG_INFO("Hot-reload: Domain unloaded");
	//
	//	ScriptReloader::GetInstance().FinalizeDllSwap();
	//	LOG_INFO("Hot-reload: DLL swapped");
	//
	//	m_AppDomain = mono_domain_create_appdomain(const_cast<char *>("EngineAppDomain"), nullptr);
	//	mono_domain_set(m_AppDomain, true);
	//
	//	LoadAssembly(m_AssemblyPath);
	//	RegisterInternalCalls();
	//
	//	LOG_INFO("Hot-reload: Domain reloaded");
	//	LOG_INFO("Hot-reload: Complete!");
	//}


	MonoClass *MonoScriptEngine::GetScriptClass(const std::string &className)
	{
		// Check if app image is loaded
		if (!m_AppImage)
		{
			LOG_ERROR("Cannot get script class '", className, "': Assembly not loaded");
			return nullptr;
		}

		// Check cache first
		auto it = m_ClassCache.find(className);
		if (it != m_ClassCache.end())
		{
			return it->second;
		}

		// Parse namespace and class name
		size_t lastDot = className.find_last_of('.');
		std::string namespaceName = lastDot != std::string::npos ? className.substr(0, lastDot) : "";
		std::string classNameOnly = lastDot != std::string::npos ? className.substr(lastDot + 1) : className;

		// Get class from image
		MonoClass *klass = mono_class_from_name(
			m_AppImage,
			namespaceName.c_str(),
			classNameOnly.c_str()
		);

		if (!klass)
		{
			LOG_ERROR("Failed to find class: ", className);
			return nullptr;
		}

		// Cache for future use
		m_ClassCache[className] = klass;
		return klass;
	}


	MonoObject *MonoScriptEngine::CreateScriptInstance(const std::string &className)
	{
		if (!m_AppImage)
		{
			// Assembly not loaded - silently skip
			return nullptr;
		}
		EnsureCorrectDomain();
		//LOG_INFO("=== CreateScriptInstance: " + className + " ===");
		MonoDomain *current = mono_domain_get();
		//LOG_INFO("  Current: " + std::to_string((uintptr_t)current));
		//LOG_INFO("  Root:    " + std::to_string((uintptr_t)m_RootDomain));
		//LOG_INFO("  App:     " + std::to_string((uintptr_t)m_AppDomain));

		if (current != m_AppDomain)
		{
			LOG_ERROR("   Still in wrong domain after EnsureCorrectDomain!");
			//LOG_ERROR("  Switching to APP domain...");
			//mono_domain_set(m_AppDomain, false);
			//current = mono_domain_get();
			//LOG_INFO("  Switched to: " + std::to_string((uintptr_t)current));
		}
		MonoClass *klass = GetScriptClass(className);
		if (!klass)
		{
			return nullptr;
		}

		MonoDomain *currentDomain = mono_domain_get();

		//LOG_INFO("=== CreateScriptInstance: " + className + " ===");
		//LOG_INFO("  Current domain: " + std::to_string((uintptr_t)currentDomain));
		//LOG_INFO("  Root domain:    " + std::to_string((uintptr_t)m_RootDomain));
		//LOG_INFO("  App domain:     " + std::to_string((uintptr_t)m_RootDomain));

		if (currentDomain != m_RootDomain)
		{
			LOG_ERROR("   WRONG DOMAIN! Currently in ROOT domain!");
			LOG_ERROR("  Switching to APP domain...");
			mono_domain_set(m_AppDomain, false);
			currentDomain = mono_domain_get();
			LOG_INFO("  Switched to: " + std::to_string((uintptr_t)currentDomain));
		}

		// Allocate object
		MonoObject *instance = mono_object_new(m_AppDomain, klass);
		if (!instance)
		{
			LOG_ERROR("Failed to create instance of: ", className);
			return nullptr;
		}

		// Call constructor
		mono_runtime_object_init(instance);

		uint32_t handle = mono_gchandle_new(instance, false);
		m_ObjectToHandle[instance] = handle;

		return instance;
	}


	void MonoScriptEngine::DestroyScriptInstance(MonoObject *instance)
	{
		if (!instance)
			return;

		// Look up the GC handle first
		auto it = m_ObjectToHandle.find(instance);
		if (it == m_ObjectToHandle.end())
		{
			LOG_WARNING("No GC handle found for instance during destroy");
			return;
		}

		// Get fresh instance from handle
		MonoObject *freshInstance = mono_gchandle_get_target(it->second);

		if (freshInstance)
		{
			// Try to call OnDestroy with the fresh instance
			try
			{
				MonoClass *klass = mono_object_get_class(freshInstance);
				if (klass)
				{
					MonoMethod *destroyMethod = mono_class_get_method_from_name(klass, "OnDestroy", 0);
					if (destroyMethod)
					{
						EnsureCorrectDomain();
						mono_runtime_invoke(destroyMethod, freshInstance, nullptr, nullptr);
					}
				}
			}
			catch (...)
			{
				LOG_WARNING("Exception during OnDestroy call");
			}
		}

		// Free the GC handle
		mono_gchandle_free(it->second);
		m_ObjectToHandle.erase(it);
		LOG_INFO("Freed GC handle for script instance");
	}

	MonoMethod *MonoScriptEngine::GetMethod(MonoClass *klass, const std::string &methodName, int paramCount)
	{
		if (!klass)
		{
			return nullptr;
		}

		MonoMethod *method = mono_class_get_method_from_name(klass, methodName.c_str(), paramCount);
		return method;
	}

	void MonoScriptEngine::CallMethod(MonoObject *instance, const std::string &methodName)
	{
		CallMethod(instance, methodName, nullptr, 0);
	}

	void MonoScriptEngine::CallMethod(MonoObject *instance, const std::string &methodName, void **params, int paramCount)
	{
		if (!instance)
		{
			return;
		}
		EnsureCorrectDomain();

		if (!IsValidMonoObject(instance))
		{
			LOG_WARNING("CallMethod: Instance is from unloaded domain, skipping ", methodName);
			return;
		}
		MonoClass *klass = mono_object_get_class(instance);
		MonoMethod *method = GetMethod(klass, methodName, paramCount);

		if (!method)
		{
			// Method doesn't exist, which is fine (not all scripts need all methods)
			return;
		}

		// Invoke method
		MonoObject *exception = nullptr;
		mono_runtime_invoke(method, instance, params, &exception);

		if (exception)
		{
			MonoClass *exceptionClass = mono_object_get_class(exception);
			const char *exceptionName = mono_class_get_name(exceptionClass);

			MonoClass *objClass = mono_object_get_class(instance);
			const char *className = mono_class_get_name(objClass);
			const char *classNs = mono_class_get_namespace(objClass);

			LOG_ERROR(
				"Exception in script ",
				(classNs ? classNs : ""),
				(classNs && classNs[0] ? "." : ""),
				className,
				"::",
				methodName,
				" - ",
				exceptionName
			);

			// Get full managed exception string (includes stack trace)
			MonoString *excStr = mono_object_to_string(exception, nullptr);
			if (excStr)
			{
				char *excCStr = mono_string_to_utf8(excStr);
				if (excCStr)
				{
					LOG_ERROR("  Exception.ToString(): ", excCStr);
					mono_free(excCStr);
				}
			}
		}

	}

	MonoObject *MonoScriptEngine::GetObjectFromHandle(void *instancePtr)
	{
		if (!instancePtr)
			return nullptr;

		MonoObject *obj = (MonoObject *)instancePtr;

		auto it = m_ObjectToHandle.find(obj);
		if (it == m_ObjectToHandle.end())
		{
			LOG_WARNING("No GC handle found for instance");
			return nullptr;
		}

		// Verify handle is valid
		if (it->second == 0)
		{
			LOG_WARNING("Invalid GC handle (0)");
			return nullptr;
		}

		EnsureCorrectDomain();

		// Get the actual object from the GC handle
		MonoObject *target = mono_gchandle_get_target(it->second);

		if (!target)
		{
			LOG_WARNING("GC handle target is null (object was collected)");
			return nullptr;
		}

		return target;
	}
	// Replace the existing SetFieldValue method in MonoScriptEngine with this corrected version:

	void MonoScriptEngine::SetFieldValue(MonoObject *instance, const std::string &fieldName, void *value)
	{
		if (!instance)
		{
			LOG_ERROR("SetFieldValue: instance is null");
			return;
		}

		if (!value)
		{
			LOG_ERROR("SetFieldValue: value pointer is null for field ", fieldName);
			return;
		}

		// Walk the inheritance chain to find a field or property called `fieldName`.
		MonoClass *klass = mono_object_get_class(instance);
		if (!klass)
		{
			LOG_ERROR("SetFieldValue: instance has no class for field: ", fieldName);
			return;
		}

		// Get class name for logging
		const char *className = mono_class_get_name(klass);
		const char *classNs = mono_class_get_namespace(klass);

		// 1) Try to find a FIELD with this name in the type hierarchy.
		MonoClass *currentClass = klass;
		MonoClassField *field = nullptr;

		while (currentClass && !field)
		{
			field = mono_class_get_field_from_name(currentClass, fieldName.c_str());
			if (field)
			{
				//LOG_INFO("[SetFieldValue] Found FIELD '", fieldName, "' in class ",
				//	classNs ? classNs : "", classNs ? "." : "", className);
				break;
			}
			currentClass = mono_class_get_parent(currentClass);
		}

		if (field)
		{
			// For fields, mono_field_set_value expects a POINTER to the value
			mono_field_set_value(instance, field, value);

			// Verify it was set (for uint32)
			uint32_t readBack = 0;
			mono_field_get_value(instance, field, &readBack);
			//LOG_INFO("[SetFieldValue] Set field '", fieldName, "' to ", *(uint32_t *)value,
			//	", read back: ", readBack);
			return;
		}

		// 2) Fall back to a PROPERTY setter (supports auto-properties).
		currentClass = klass;
		MonoProperty *prop = nullptr;

		while (currentClass && !prop)
		{
			prop = mono_class_get_property_from_name(currentClass, fieldName.c_str());
			if (prop)
			{
				LOG_INFO("[SetFieldValue] Found PROPERTY '", fieldName, "' in class ",
					classNs ? classNs : "", classNs ? "." : "", className);
				break;
			}
			currentClass = mono_class_get_parent(currentClass);
		}

		if (!prop)
		{
			LOG_ERROR("[SetFieldValue] Field/Property '", fieldName, "' not found in class ",
				classNs ? classNs : "", classNs ? "." : "", className, " or its parents");
			return;
		}

		MonoMethod *setter = mono_property_get_set_method(prop);
		if (!setter)
		{
			LOG_ERROR("[SetFieldValue] Property '", fieldName, "' has no setter");
			return;
		}

		// CRITICAL FIX: For property setters, we need to pass a POINTER TO A POINTER
		// The value pointer itself becomes the argument
		void *args[1] = { value };

		LOG_INFO("[SetFieldValue] Calling property setter for '", fieldName,
			"' with value ", *(uint32_t *)value);

		MonoObject *exception = nullptr;
		mono_runtime_invoke(setter, instance, args, &exception);

		if (exception)
		{
			MonoString *excStr = mono_object_to_string(exception, nullptr);
			char *excCStr = excStr ? mono_string_to_utf8(excStr) : nullptr;
			LOG_ERROR("[SetFieldValue] Exception while setting property '", fieldName,
				"': ", (excCStr ? excCStr : "<null>"));
			if (excCStr)
				mono_free(excCStr);
			return;
		}

		// Verify it was set by reading it back
		MonoMethod *getter = mono_property_get_get_method(prop);
		if (getter)
		{
			MonoObject *result = mono_runtime_invoke(getter, instance, nullptr, nullptr);
			if (result)
			{
				void *unboxed = mono_object_unbox(result);
				if (unboxed)
				{
					uint32_t readBack = *(uint32_t *)unboxed;
					LOG_INFO("[SetFieldValue] Property '", fieldName, "' set to ",
						*(uint32_t *)value, ", read back: ", readBack);

					if (readBack != *(uint32_t *)value)
					{
						LOG_ERROR("[SetFieldValue] VALUE MISMATCH! Expected ", *(uint32_t *)value,
							" but got ", readBack);
					}
				}
			}
		}
	}

	void *MonoScriptEngine::GetFieldValue(MonoObject *instance, const std::string &fieldName)
	{
		if (!instance)
		{
			return nullptr;
		}

		MonoClass *klass = mono_object_get_class(instance);
		MonoClassField *field = mono_class_get_field_from_name(klass, fieldName.c_str());

		if (!field)
		{
			LOG_ERROR("Field not found: ", fieldName);
			return nullptr;
		}

		void *value = nullptr;
		mono_field_get_value(instance, field, &value);
		return value;
	}

	void MonoScriptEngine::BindEntityID(MonoObject *instance, std::uint32_t entityID)
	{
		if (!instance)
		{
			LOG_ERROR("[BindEntityID] instance is null!");
			return;
		}

		// Verify instance is from current domain
		if (!IsValidMonoObject(instance))
		{
			LOG_ERROR("[BindEntityID] instance is from wrong/unloaded domain!");
			return;
		}

		MonoClass *klass = mono_object_get_class(instance);
		if (!klass)
		{
			LOG_ERROR("[BindEntityID] Failed to get class from instance");
			return;
		}

		const char *className = mono_class_get_name(klass);
		const char *classNs = mono_class_get_namespace(klass);

		//LOG_INFO("[BindEntityID] Attempting to BindInternalCall EntityID=", entityID, " to instance of ",
		//	classNs ? classNs : "", classNs ? "." : "", className);

		// Make a local copy - CRITICAL: pass the ADDRESS of this copy
		std::uint32_t idCopy = entityID;

		//LOG_INFO("[BindEntityID] Calling SetFieldValue with idCopy address: ",
		//	(void *)&idCopy, ", value: ", idCopy);

		// SetFieldValue needs the ADDRESS of the value
		SetFieldValue(instance, "EntityID", &idCopy);

		// VERIFICATION: Try to read it back multiple ways
		//LOG_INFO("[BindEntityID] === Verification Phase ===");

		// Method 1: Try reading as a property
		MonoProperty *prop = nullptr;
		MonoClass *currentClass = klass;

		while (currentClass && !prop)
		{
			prop = mono_class_get_property_from_name(currentClass, "EntityID");
			currentClass = mono_class_get_parent(currentClass);
		}

		if (prop)
		{
			MonoMethod *getter = mono_property_get_get_method(prop);
			if (getter)
			{
				MonoObject *exception = nullptr;
				MonoObject *result = mono_runtime_invoke(getter, instance, nullptr, &exception);

				if (exception)
				{
					MonoString *excStr = mono_object_to_string(exception, nullptr);
					char *cStr = excStr ? mono_string_to_utf8(excStr) : nullptr;
					//LOG_ERROR("[BindEntityID] Exception reading EntityID property: ",
					//	cStr ? cStr : "<null>");
					if (cStr) mono_free(cStr);
				}
				else if (result)
				{
					void *unboxed = mono_object_unbox(result);
					if (unboxed)
					{
						uint32_t verifyID = *reinterpret_cast<uint32_t *>(unboxed);
						if (verifyID == entityID)
						{
							//LOG_INFO("[BindEntityID] SUCCESS! EntityID=", entityID,
							//	" verified via property getter");
						}
						else
						{
							//LOG_ERROR("[BindEntityID] FAILED! Set ", entityID,
							//	" but property getter returned ", verifyID);
						}
						return; // Exit after property check
					}
					else
					{
						LOG_ERROR("[BindEntityID] Failed to unbox property result");
					}
				}
				else
				{
					LOG_ERROR("[BindEntityID] Property getter returned null");
				}
			}
			else
			{
				LOG_WARNING("[BindEntityID] Property has no getter");
			}
		}

		// Method 2: Try reading as a field
		currentClass = klass;
		MonoClassField *field = nullptr;

		while (currentClass && !field)
		{
			field = mono_class_get_field_from_name(currentClass, "EntityID");
			currentClass = mono_class_get_parent(currentClass);
		}

		if (field)
		{
			uint32_t verifyID = 0;
			mono_field_get_value(instance, field, &verifyID);

			if (verifyID == entityID)
			{
				LOG_INFO("[BindEntityID] SUCCESS! EntityID=", entityID,
					" verified via field access");
			}
			else
			{
				LOG_ERROR("[BindEntityID] FAILED! Set ", entityID,
					" but field access returned ", verifyID);
			}
		}
		else
		{
			LOG_ERROR("[BindEntityID] Could not find EntityID as field or property!");
		}
	}

	// ------------------------------------------------
	// Helper: Initialize ScriptComponent for new entity
	// ------------------------------------------------
	static void InitializeScriptComponentForEntity(Entity entity)
	{
		if (!entity)
			return;

		if (!entity.HasComponent<ScriptComponent>())
			return;

		auto &sc = entity.GetComponent<ScriptComponent>();

		// No script assigned on this entity
		if (sc.ScriptClassName.empty())
			return;

		auto &se = MonoScriptEngine::GetInstance();

		uint64_t eid = static_cast<uint32_t>(entity);

		// If there's already a managed instance (e.g. cloned from prefab),
		// just (re)BindInternalCall EntityID to be safe.
		if (sc.ScriptInstance)
		{
			LOG_INFO("[Prefab] Rebinding EntityID on existing script instance '",
				sc.ScriptClassName, "' for entity ", eid);
			se.BindEntityID(static_cast<MonoObject *>(sc.ScriptInstance), eid);
			return;
		}

		// Otherwise create a fresh managed instance
		MonoObject *instance = se.CreateScriptInstance(sc.ScriptClassName);
		if (!instance)
		{
			LOG_ERROR("[Prefab] Failed to create script instance for class '",
				sc.ScriptClassName, "' on entity ", eid);
			return;
		}

		se.BindEntityID(instance, eid);

		sc.ScriptInstance = instance;
		sc.Started = false; // ScriptSystem will call OnStart on next update

		LOG_INFO("[Prefab] Initialized script '", sc.ScriptClassName,
			"' for entity ", eid, " and bound EntityID");
	}

	// Expose these functions for external use
	void SetScriptingInputSystem(Input *input)
	{
		InternalCalls::SetInputSystem(input);
	}

	void SetScriptingCurrentScene(Scene *scene)
	{
		InternalCalls::SetCurrentScene(scene);
	}

	void SetScriptingAudioManager(AudioManager *audioManager)
	{
		InternalCalls::SetAudioManager(audioManager);
	}

	static void BindInternalCall(const char *managedName, void *fn)
	{
		mono_add_internal_call(managedName, fn);
	}

	template <typename T>
	static void BindInternalCall(const char *managedName, T fn)
	{
		mono_add_internal_call(managedName, reinterpret_cast<void *>(fn));
	}


	void MonoScriptEngine::RegisterInternalCalls()
	{
		LOG_INFO("Registering internal calls...");

		// =====================================================================
		// ECS / Scene
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Scene_CreateEntity",
			reinterpret_cast<void *>(InternalCalls::Scene_CreateEntity));
		BindInternalCall("Engine.InternalCalls::Scene_DestroyEntity",
			reinterpret_cast<void *>(InternalCalls::Scene_DestroyEntity));
		BindInternalCall("Engine.InternalCalls::Entity_AddScript",
			reinterpret_cast<void *>(InternalCalls::Entity_AddScript));

		BindInternalCall("Engine.InternalCalls::Scene_FindEntityByName",
			reinterpret_cast<void *>(InternalCalls::Scene_FindEntityByName));

		// =====================================================================
		// Transform
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Transform_GetParent",
			reinterpret_cast<void *>(InternalCalls::Transform_GetParent));

		BindInternalCall("Engine.InternalCalls::Transform_GetPosition",
			reinterpret_cast<void *>(InternalCalls::Transform_GetPosition));
		BindInternalCall("Engine.InternalCalls::Transform_SetPosition",
			reinterpret_cast<void *>(InternalCalls::Transform_SetPosition));

		BindInternalCall("Engine.InternalCalls::Transform_GetRotation",
			reinterpret_cast<void *>(InternalCalls::Transform_GetRotation));
		BindInternalCall("Engine.InternalCalls::Transform_SetRotation",
			reinterpret_cast<void *>(InternalCalls::Transform_SetRotation));

		BindInternalCall("Engine.InternalCalls::Transform_GetScale",
			reinterpret_cast<void *>(InternalCalls::Transform_GetScale));
		BindInternalCall("Engine.InternalCalls::Transform_SetScale",
			reinterpret_cast<void *>(InternalCalls::Transform_SetScale));

		// =====================================================================
		// Logging
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::LogMessage",
			reinterpret_cast<void *>(InternalCalls::LogMessage));
		BindInternalCall("Engine.InternalCalls::LogError",
			reinterpret_cast<void *>(InternalCalls::LogError));
		BindInternalCall("Engine.InternalCalls::LogWarning",
			reinterpret_cast<void *>(InternalCalls::LogWarning));

		// =====================================================================
		// Entity helpers
		// =====================================================================
		BindInternalCall("Engine.Entity::GetEntityID_Native",
			reinterpret_cast<void *>(InternalCalls::Entity_GetEntityID));
		BindInternalCall("Engine.Entity::HasComponent_Native",
			reinterpret_cast<void *>(InternalCalls::Entity_HasComponent));

		BindInternalCall("Engine.InternalCalls::EntityHasCamera",
			reinterpret_cast<void *>(InternalCalls::EntityHasCamera));
		BindInternalCall("Engine.InternalCalls::EntityHasRigidBody",
			reinterpret_cast<void *>(InternalCalls::EntityHasRigidBody));

		// =====================================================================
		// Prefab
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Prefab_Instantiate",
			reinterpret_cast<void *>(InternalCalls::Prefab_Instantiate));
		BindInternalCall("Engine.InternalCalls::Prefab_InstantiateScene",
			reinterpret_cast<void *>(InternalCalls::Prefab_InstantiateScene));
		BindInternalCall("Engine.InternalCalls::Prefab_InstantiateWithTransform",
			reinterpret_cast<void *>(InternalCalls::Prefab_InstantiateWithTransform));

		// =====================================================================
		// Input
		// =====================================================================
		BindInternalCall("Engine.Input::IsKeyPressed_Native",
			reinterpret_cast<void *>(InternalCalls::Input_IsKeyPressed));
		BindInternalCall("Engine.Input::IsKeyReleased_Native",
			reinterpret_cast<void *>(InternalCalls::Input_IsKeyReleased));
		BindInternalCall("Engine.Input::IsMouseButtonPressed_Native",
			reinterpret_cast<void *>(InternalCalls::Input_IsMouseButtonPressed));
		BindInternalCall("Engine.Input::GetMousePosition_Native",
			reinterpret_cast<void *>(InternalCalls::Input_GetMousePosition));

		BindInternalCall("Engine.InternalCalls::Input_GetMouseDelta",
			reinterpret_cast<void *>(InternalCalls::Input_GetMouseDelta));

		// =====================================================================
		// Physics / Rigidbody
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Entity_AddRigidBody",
			reinterpret_cast<void *>(InternalCalls::Entity_AddRigidBody));

		// Rigidbody velocity
		BindInternalCall("Engine.InternalCalls::Rigidbody_GetVelocity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetVelocity));
		BindInternalCall("Engine.InternalCalls::Rigidbody_SetVelocity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetVelocity));
		BindInternalCall("Engine.InternalCalls::Rigidbody_AddVelocity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_AddVelocity));

		// Rigidbody scalar / flags
		BindInternalCall("Engine.InternalCalls::Rigidbody_GetMass",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetMass));
		BindInternalCall("Engine.InternalCalls::Rigidbody_SetMass",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetMass));
		BindInternalCall("Engine.InternalCalls::Rigidbody_GetIsKinematic",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetIsKinematic));
		BindInternalCall("Engine.InternalCalls::Rigidbody_SetIsKinematic",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetIsKinematic));
		BindInternalCall("Engine.InternalCalls::Rigidbody_GetUseGravity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetUseGravity));
		BindInternalCall("Engine.InternalCalls::Rigidbody_SetUseGravity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetUseGravity));

		// Rigidbody helpers / forces
		BindInternalCall("Engine.InternalCalls::Rigidbody_GetSpeed",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetSpeed));
		BindInternalCall("Engine.InternalCalls::Rigidbody_IsMoving",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_IsMoving));
		BindInternalCall("Engine.InternalCalls::Rigidbody_IsStatic",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_IsStatic));
		BindInternalCall("Engine.InternalCalls::Rigidbody_AddForce",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_AddForce));
		BindInternalCall("Engine.InternalCalls::Rigidbody_Stop",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_Stop));

		// Collision events (PhysicsAPI-only)
		BindInternalCall("Engine.InternalCalls::Physics_EnableCollisionEvents",
			reinterpret_cast<void *>(InternalCalls::Physics_EnableCollisionEvents));
		BindInternalCall("Engine.InternalCalls::Physics_BeginCollisionFrame",
			reinterpret_cast<void *>(InternalCalls::Physics_BeginCollisionFrame));
		BindInternalCall("Engine.InternalCalls::Physics_GetCollisionCount",
			reinterpret_cast<void *>(InternalCalls::Physics_GetCollisionCount));
		BindInternalCall("Engine.InternalCalls::Physics_GetCollisionPair",
			reinterpret_cast<void *>(InternalCalls::Physics_GetCollisionPair));

		// =====================================================================
		// Entity / component adders
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Entity_AddTag",
			reinterpret_cast<void *>(InternalCalls::Entity_AddTag));
		BindInternalCall("Engine.InternalCalls::Entity_AddCamera",
			reinterpret_cast<void *>(InternalCalls::Entity_AddCamera));
		BindInternalCall("Engine.InternalCalls::Entity_AddAudio",
			reinterpret_cast<void *>(InternalCalls::Entity_AddAudio));
		BindInternalCall("Engine.InternalCalls::Entity_AddMeshRenderer",
			reinterpret_cast<void *>(InternalCalls::Entity_AddMeshRenderer));

		// =====================================================================
		// Tag
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Tag_GetTag",
			reinterpret_cast<void *>(InternalCalls::Tag_GetTag));
		BindInternalCall("Engine.InternalCalls::Tag_SetTag",
			reinterpret_cast<void *>(InternalCalls::Tag_SetTag));
		BindInternalCall("Engine.InternalCalls::Scene_FindEntitiesByTag",
			reinterpret_cast<void *>(InternalCalls::Scene_FindEntitiesByTag));

		// =====================================================================
		// Camera
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Camera_GetEnabled",
			reinterpret_cast<void *>(InternalCalls::Camera_GetEnabled));
		BindInternalCall("Engine.InternalCalls::Camera_SetEnabled",
			reinterpret_cast<void *>(InternalCalls::Camera_SetEnabled));
		BindInternalCall("Engine.InternalCalls::Camera_GetPrimary",
			reinterpret_cast<void *>(InternalCalls::Camera_GetPrimary));
		BindInternalCall("Engine.InternalCalls::Camera_SetPrimary",
			reinterpret_cast<void *>(InternalCalls::Camera_SetPrimary));
		BindInternalCall("Engine.InternalCalls::Camera_GetFOV",
			reinterpret_cast<void *>(InternalCalls::Camera_GetFOV));
		BindInternalCall("Engine.InternalCalls::Camera_SetFOV",
			reinterpret_cast<void *>(InternalCalls::Camera_SetFOV));
		BindInternalCall("Engine.InternalCalls::Camera_GetNear",
			reinterpret_cast<void *>(InternalCalls::Camera_GetNear));
		BindInternalCall("Engine.InternalCalls::Camera_SetNear",
			reinterpret_cast<void *>(InternalCalls::Camera_SetNear));
		BindInternalCall("Engine.InternalCalls::Camera_GetFar",
			reinterpret_cast<void *>(InternalCalls::Camera_GetFar));
		BindInternalCall("Engine.InternalCalls::Camera_SetFar",
			reinterpret_cast<void *>(InternalCalls::Camera_SetFar));
		BindInternalCall("Engine.InternalCalls::Camera_GetTarget",
			reinterpret_cast<void *>(InternalCalls::Camera_GetTarget));
		BindInternalCall("Engine.InternalCalls::Camera_SetTarget",
			reinterpret_cast<void *>(InternalCalls::Camera_SetTarget));

		// =====================================================================
		// MeshRenderer
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::MeshRenderer_GetVisible",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetVisible));
		BindInternalCall("Engine.InternalCalls::MeshRenderer_SetVisible",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetVisible));
		BindInternalCall("Engine.InternalCalls::MeshRenderer_GetShadowCast",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetShadowCast));
		BindInternalCall("Engine.InternalCalls::MeshRenderer_SetShadowCast",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetShadowCast));
		BindInternalCall("Engine.InternalCalls::MeshRenderer_GetShadowReceive",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetShadowReceive));
		BindInternalCall("Engine.InternalCalls::MeshRenderer_SetShadowReceive",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetShadowReceive));
		BindInternalCall("Engine.InternalCalls::MeshRenderer_GetGlobalIlluminate",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetGlobalIlluminate));
		BindInternalCall("Engine.InternalCalls::MeshRenderer_SetGlobalIlluminate",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetGlobalIlluminate));

		// =====================================================================
		// AudioComponent
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Audio_Play",
			reinterpret_cast<void *>(InternalCalls::Audio_Play));
		BindInternalCall("Engine.InternalCalls::Audio_Stop",
			reinterpret_cast<void *>(InternalCalls::Audio_Stop));
		BindInternalCall("Engine.InternalCalls::Audio_Pause",
			reinterpret_cast<void *>(InternalCalls::Audio_Pause));

		BindInternalCall("Engine.InternalCalls::Audio_GetVolume",
			reinterpret_cast<void *>(InternalCalls::Audio_GetVolume));
		BindInternalCall("Engine.InternalCalls::Audio_SetVolume",
			reinterpret_cast<void *>(InternalCalls::Audio_SetVolume));

		BindInternalCall("Engine.InternalCalls::Audio_GetPitch",
			reinterpret_cast<void *>(InternalCalls::Audio_GetPitch));
		BindInternalCall("Engine.InternalCalls::Audio_SetPitch",
			reinterpret_cast<void *>(InternalCalls::Audio_SetPitch));

		BindInternalCall("Engine.InternalCalls::Audio_GetLoop",
			reinterpret_cast<void *>(InternalCalls::Audio_GetLoop));
		BindInternalCall("Engine.InternalCalls::Audio_SetLoop",
			reinterpret_cast<void *>(InternalCalls::Audio_SetLoop));

		BindInternalCall("Engine.InternalCalls::Audio_GetMute",
			reinterpret_cast<void *>(InternalCalls::Audio_GetMute));
		BindInternalCall("Engine.InternalCalls::Audio_SetMute",
			reinterpret_cast<void *>(InternalCalls::Audio_SetMute));

		BindInternalCall("Engine.InternalCalls::Audio_GetIs3D",
			reinterpret_cast<void *>(InternalCalls::Audio_GetIs3D));
		BindInternalCall("Engine.InternalCalls::Audio_SetIs3D",
			reinterpret_cast<void *>(InternalCalls::Audio_SetIs3D));

		BindInternalCall("Engine.InternalCalls::Audio_SetFile",
			reinterpret_cast<void *>(InternalCalls::Audio_SetFile));

		// AudioComponent extensions
		BindInternalCall("Engine.InternalCalls::Audio_GetMinDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_GetMinDistance));
		BindInternalCall("Engine.InternalCalls::Audio_SetMinDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_SetMinDistance));
		BindInternalCall("Engine.InternalCalls::Audio_GetMaxDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_GetMaxDistance));
		BindInternalCall("Engine.InternalCalls::Audio_SetMaxDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_SetMaxDistance));
		BindInternalCall("Engine.InternalCalls::Audio_GetRolloffMode",
			reinterpret_cast<void *>(InternalCalls::Audio_GetRolloffMode));
		BindInternalCall("Engine.InternalCalls::Audio_SetRolloffMode",
			reinterpret_cast<void *>(InternalCalls::Audio_SetRolloffMode));
		BindInternalCall("Engine.InternalCalls::Audio_GetDopplerLevel",
			reinterpret_cast<void *>(InternalCalls::Audio_GetDopplerLevel));
		BindInternalCall("Engine.InternalCalls::Audio_SetDopplerLevel",
			reinterpret_cast<void *>(InternalCalls::Audio_SetDopplerLevel));
		BindInternalCall("Engine.InternalCalls::Audio_GetPan2D",
			reinterpret_cast<void *>(InternalCalls::Audio_GetPan2D));
		BindInternalCall("Engine.InternalCalls::Audio_SetPan2D",
			reinterpret_cast<void *>(InternalCalls::Audio_SetPan2D));
		BindInternalCall("Engine.InternalCalls::Audio_GetReverbMix",
			reinterpret_cast<void *>(InternalCalls::Audio_GetReverbMix));
		BindInternalCall("Engine.InternalCalls::Audio_SetReverbMix",
			reinterpret_cast<void *>(InternalCalls::Audio_SetReverbMix));

		// =====================================================================
		// AudioManager (global controls)
		// =====================================================================
		BindInternalCall("Engine.AudioManager::AudioManager_SetGroupVolume",
			reinterpret_cast<void *>(InternalCalls::AudioManager_SetGroupVolume));
		BindInternalCall("Engine.AudioManager::AudioManager_GetGroupVolume",
			reinterpret_cast<void *>(InternalCalls::AudioManager_GetGroupVolume));
		BindInternalCall("Engine.AudioManager::AudioManager_SetGroupPitch",
			reinterpret_cast<void *>(InternalCalls::AudioManager_SetGroupPitch));
		BindInternalCall("Engine.AudioManager::AudioManager_GetGroupPitch",
			reinterpret_cast<void *>(InternalCalls::AudioManager_GetGroupPitch));
		BindInternalCall("Engine.AudioManager::AudioManager_SetGroupMute",
			reinterpret_cast<void *>(InternalCalls::AudioManager_SetGroupMute));
		BindInternalCall("Engine.AudioManager::AudioManager_IsGroupMuted",
			reinterpret_cast<void *>(InternalCalls::AudioManager_IsGroupMuted));

		BindInternalCall("Engine.AudioManager::AudioManager_PauseGroup",
			reinterpret_cast<void *>(InternalCalls::AudioManager_PauseGroup));
		BindInternalCall("Engine.AudioManager::AudioManager_PauseAll",
			reinterpret_cast<void *>(InternalCalls::AudioManager_PauseAll));
		BindInternalCall("Engine.AudioManager::AudioManager_StopByType",
			reinterpret_cast<void *>(InternalCalls::AudioManager_StopByType));
		BindInternalCall("Engine.AudioManager::AudioManager_StopAll",
			reinterpret_cast<void *>(InternalCalls::AudioManager_StopAll));

		BindInternalCall("Engine.AudioManager::AudioManager_CreateDSP",
			reinterpret_cast<void *>(InternalCalls::AudioManager_CreateDSP));
		BindInternalCall("Engine.AudioManager::AudioManager_EnableDSP",
			reinterpret_cast<void *>(InternalCalls::AudioManager_EnableDSP));
		BindInternalCall("Engine.AudioManager::AudioManager_SetDSPParameter",
			reinterpret_cast<void *>(InternalCalls::AudioManager_SetDSPParameter));
		BindInternalCall("Engine.AudioManager::AudioManager_ReleaseSpecificDSPinGroup",
			reinterpret_cast<void *>(InternalCalls::AudioManager_ReleaseSpecificDSPinGroup));
		BindInternalCall("Engine.AudioManager::AudioManager_ReleaseDSPByGroup",
			reinterpret_cast<void *>(InternalCalls::AudioManager_ReleaseDSPByGroup));
		BindInternalCall("Engine.AudioManager::AudioManager_ReleaseAllDSPs",
			reinterpret_cast<void *>(InternalCalls::AudioManager_ReleaseAllDSPs));

		BindInternalCall("Engine.AudioManager::AudioManager_SetListenerAttributes",
			reinterpret_cast<void *>(InternalCalls::AudioManager_SetListenerAttributes));

		// =====================================================================
		// Event System
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Event_Publish",
			reinterpret_cast<void *>(InternalCalls::Event_Publish));

		// =====================================================================
		// Quaternion
		// =====================================================================
		BindInternalCall("Engine.InternalCalls::Quat_FromAxisAngle",
			reinterpret_cast<void *>(InternalCalls::Quat_FromAxisAngle));
		BindInternalCall("Engine.InternalCalls::Quat_GetForward",
			reinterpret_cast<void *>(InternalCalls::Quat_GetForward));
		BindInternalCall("Engine.InternalCalls::Quat_GetRight",
			reinterpret_cast<void *>(InternalCalls::Quat_GetRight));
		BindInternalCall("Engine.InternalCalls::Quat_GetUp",
			reinterpret_cast<void *>(InternalCalls::Quat_GetUp));
		BindInternalCall("Engine.InternalCalls::Quat_RotateVector",
			reinterpret_cast<void *>(InternalCalls::Quat_RotateVector));
		BindInternalCall("Engine.InternalCalls::Quat_Multiply",
			reinterpret_cast<void *>(InternalCalls::Quat_Multiply));
		BindInternalCall("Engine.InternalCalls::Quat_Slerp",
			reinterpret_cast<void *>(InternalCalls::Quat_Slerp));
		BindInternalCall("Engine.InternalCalls::Quat_Inverse",
			reinterpret_cast<void *>(InternalCalls::Quat_Inverse));
		BindInternalCall("Engine.InternalCalls::Quat_ToEuler",
			reinterpret_cast<void *>(InternalCalls::Quat_ToEuler));
		BindInternalCall("Engine.InternalCalls::Quat_FromEuler",
			reinterpret_cast<void *>(InternalCalls::Quat_FromEuler));
		BindInternalCall("Engine.InternalCalls::Quat_Normalize",
			reinterpret_cast<void *>(InternalCalls::Quat_Normalize));
		BindInternalCall("Engine.InternalCalls::Quat_Length",
			reinterpret_cast<void *>(InternalCalls::Quat_Length));
		BindInternalCall("Engine.InternalCalls::Quat_Dot",
			reinterpret_cast<void *>(InternalCalls::Quat_Dot));

		LOG_INFO("Internal calls registered");
	}

} // namespace Engine

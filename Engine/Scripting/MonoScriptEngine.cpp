
#include <windows.h>  // Add at top

#include "MonoScriptEngine.h"
#include "../Utility/Logger.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include "../ECS/Components.h"
#include "../Core/input.h"
#include "Core/Application.h"

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


namespace Engine
{


	MonoScriptEngine &MonoScriptEngine::GetInstance()
	{
		static MonoScriptEngine instance;
		return instance;
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

		// Create app domain
		m_AppDomain = mono_domain_create_appdomain(const_cast<char *>("EngineAppDomain"), nullptr);
		if (!m_AppDomain)
		{
			LOG_ERROR("Failed to create Mono app domain");
			return;
		}
		mono_domain_set(m_AppDomain, true);

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

		s_Initialized = true;
		LOG_INFO("Mono Script Engine initialized");
	}



	void MonoScriptEngine::Shutdown()
	{
		LOG_INFO("Shutting down Mono Script Engine...");

		UnloadAssembly();

		if (m_AppDomain)
		{
			mono_domain_set(m_RootDomain, false);
			mono_domain_unload(m_AppDomain);
			m_AppDomain = nullptr;
		}

		if (m_RootDomain)
		{
			mono_jit_cleanup(m_RootDomain);
			m_RootDomain = nullptr;
		}

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
	}

	void MonoScriptEngine::UnloadAssembly()
	{
		m_ClassCache.clear();
		m_AppImage = nullptr;
		m_AppAssembly = nullptr;
	}

	void MonoScriptEngine::ReloadAssembly()
	{
		LOG_INFO("Reloading assembly...");
		UnloadAssembly();

		// Recreate app domain
		mono_domain_set(m_RootDomain, false);
		mono_domain_unload(m_AppDomain);
		m_AppDomain = mono_domain_create_appdomain(const_cast<char *>("EngineAppDomain"), nullptr);
		mono_domain_set(m_AppDomain, true);

		LoadAssembly(m_AssemblyPath);
		RegisterInternalCalls();
	}

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

		MonoClass *klass = GetScriptClass(className);
		if (!klass)
		{
			return nullptr;
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

		return instance;
	}


	void MonoScriptEngine::DestroyScriptInstance(MonoObject *instance)
	{
		// Mono uses garbage collection, so we just need to clear references
		// The GC will handle cleanup
		if (instance)
		{
			// Optionally call OnDestroy if the class has it
			MonoClass *klass = mono_object_get_class(instance);
			MonoMethod *destroyMethod = mono_class_get_method_from_name(klass, "OnDestroy", 0);
			if (destroyMethod)
			{
				mono_runtime_invoke(destroyMethod, instance, nullptr, nullptr);
			}
		}
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

	void MonoScriptEngine::SetFieldValue(MonoObject *instance, const std::string &fieldName, void *value)
	{
		if (!instance)
		{
			return;
		}

		MonoClass *klass = mono_object_get_class(instance);
		MonoClassField *field = mono_class_get_field_from_name(klass, fieldName.c_str());

		if (!field)
		{
			LOG_ERROR("Field not found: ", fieldName);
			return;
		}

		mono_field_set_value(instance, field, value);
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

	// ============================================
	// Internal Calls - C++ functions callable from C#
	// ============================================

	// Forward declarations for internal call functions
	namespace InternalCalls
	{
		static uint64_t Scene_CreateEntity(MonoString *nameStr);
		static void     Scene_DestroyEntity(uint64_t entityID);
		static void     Entity_AddScript(uint64_t entityID, MonoString *classFullNameStr);

		static void Log(MonoString *message);
		static void LogError(MonoString *message);
		static void LogWarning(MonoString *message);

		static uint64_t Entity_GetEntityID(MonoObject *entityObj);
		static uint64_t Scene_FindEntityByName(MonoString *nameString);
		static bool Entity_HasComponent(uint64_t entityID, MonoReflectionType *componentType);

		static void Transform_GetPosition(uint64_t entityID, glm::vec3 *outPosition);
		static void Transform_SetPosition(uint64_t entityID, glm::vec3 *position);
		static void Transform_GetRotation(uint64_t entityID, glm::vec3 *outRotation);
		static void Transform_SetRotation(uint64_t entityID, glm::vec3 *rotation);
		static void Transform_GetScale(uint64_t entityID, glm::vec3 *outScale);
		static void Transform_SetScale(uint64_t entityID, glm::vec3 *scale);
		static void Transform_Move(uint64_t entityID, float deltaX, float deltaY, float deltaZ);

		static bool Input_IsKeyPressed(int keyCode);
		static bool Input_IsMouseButtonPressed(int button);
		static void Input_GetMousePosition(glm::vec2 *outPosition);
		//static Input* s_InputSystem = nullptr;  // Will be set later!

		// Prefab instantiation
		static uint64_t Prefab_Instantiate(MonoString *prefabPathStr);

		//Physics bindings
		static void Entity_AddRigidBody(uint64_t entityID);

		// ---- Rigidbody velocity access (ECS-level) ----
		static void Rigidbody_GetVelocity(uint64_t entityID, glm::vec3 *outVel);
		static void Rigidbody_SetVelocity(uint64_t entityID, glm::vec3 *inVel);
		static void Rigidbody_AddVelocity(uint64_t entityID, glm::vec3 *delta);

		// ---- Rigidbody scalar properties ----
		static float Rigidbody_GetMass(uint64_t entityID);
		static void  Rigidbody_SetMass(uint64_t entityID, float mass);
		static bool  Rigidbody_GetIsKinematic(uint64_t entityID);
		static void  Rigidbody_SetIsKinematic(uint64_t entityID, bool isKinematic);
		static bool  Rigidbody_GetUseGravity(uint64_t entityID);
		static void  Rigidbody_SetUseGravity(uint64_t entityID, bool useGravity);

		// ---- Rigidbody helpers ----
		static float Rigidbody_GetSpeed(uint64_t entityID);
		static bool  Rigidbody_IsMoving(uint64_t entityID);
		static bool  Rigidbody_IsStatic(uint64_t entityID);

		// ---- Rigidbody forces ----
		static void  Rigidbody_AddForce(uint64_t entityID, glm::vec3 *force);
		static void  Rigidbody_Stop(uint64_t entityID);


		// ---- Collision events (via PhysicsAPI only) ----
		static void Physics_EnableCollisionEvents();
		static void Physics_BeginCollisionFrame();
		static int  Physics_GetCollisionCount();
		static void Physics_GetCollisionPair(int index, uint32_t *a, uint32_t *b);

		// Collision events
		static void  Physics_EnableCollisionEvents();
		static void  Physics_BeginCollisionFrame();
		static int   Physics_GetCollisionCount();
		static void  Physics_GetCollisionPair(int index, uint32_t *a, uint32_t *b);

		// ===== Entity / component adders =====
		static void Entity_AddTag(uint64_t entityID);
		static void Entity_AddCamera(uint64_t entityID);
		static void Entity_AddAudio(uint64_t entityID);
		static void Entity_AddMeshRenderer(uint64_t entityID);

		// ===== TagComponent =====
		static MonoString *Tag_GetTag(uint64_t entityID);
		static void        Tag_SetTag(uint64_t entityID, MonoString *tag);

		// ===== CameraComponent =====
		static bool  Camera_GetEnabled(uint64_t entityID);
		static void  Camera_SetEnabled(uint64_t entityID, bool enabled);
		static bool  Camera_GetPrimary(uint64_t entityID);
		static void  Camera_SetPrimary(uint64_t entityID, bool primary);
		static float Camera_GetFOV(uint64_t entityID);
		static void  Camera_SetFOV(uint64_t entityID, float fov);
		static float Camera_GetNear(uint64_t entityID);
		static void  Camera_SetNear(uint64_t entityID, float nearPlane);
		static float Camera_GetFar(uint64_t entityID);
		static void  Camera_SetFar(uint64_t entityID, float farPlane);
		static void  Camera_GetTarget(uint64_t entityID, glm::vec3 *outTarget);
		static void  Camera_SetTarget(uint64_t entityID, glm::vec3 *inTarget);

		// ===== MeshRendererComponent =====
		static bool MeshRenderer_GetVisible(uint64_t entityID);
		static void MeshRenderer_SetVisible(uint64_t entityID, bool visible);
		static bool MeshRenderer_GetShadowCast(uint64_t entityID);
		static void MeshRenderer_SetShadowCast(uint64_t entityID, bool cast);
		static bool MeshRenderer_GetShadowReceive(uint64_t entityID);
		static void MeshRenderer_SetShadowReceive(uint64_t entityID, bool receive);
		static bool MeshRenderer_GetGlobalIlluminate(uint64_t entityID);
		static void MeshRenderer_SetGlobalIlluminate(uint64_t entityID, bool gi);

		// ===== AudioComponent =====
		static void  Audio_Play(uint64_t entityID);
		static void  Audio_Stop(uint64_t entityID);
		static void  Audio_Pause(uint64_t entityID);

		static float Audio_GetVolume(uint64_t entityID);
		static void  Audio_SetVolume(uint64_t entityID, float volume);

		static float Audio_GetPitch(uint64_t entityID);
		static void  Audio_SetPitch(uint64_t entityID, float pitch);

		static bool  Audio_GetLoop(uint64_t entityID);
		static void  Audio_SetLoop(uint64_t entityID, bool loop);

		static bool  Audio_GetMute(uint64_t entityID);
		static void  Audio_SetMute(uint64_t entityID, bool mute);

		static bool  Audio_GetIs3D(uint64_t entityID);
		static void  Audio_SetIs3D(uint64_t entityID, bool is3d);

		static void  Audio_SetFile(uint64_t entityID, MonoString *path);


	}

	void MonoScriptEngine::RegisterInternalCalls()
	{
		LOG_INFO("Registering internal calls...");

		// ECS Bindings
		mono_add_internal_call("Engine.InternalCalls::Scene_CreateEntity", (void *)InternalCalls::Scene_CreateEntity);
		mono_add_internal_call("Engine.InternalCalls::Entity_AddScript", (void *)InternalCalls::Entity_AddScript);
		mono_add_internal_call("Engine.InternalCalls::Scene_DestroyEntity", (void *)InternalCalls::Scene_DestroyEntity);

		// Logging
		mono_add_internal_call("Engine.InternalCalls::Log", (void *)InternalCalls::Log);
		mono_add_internal_call("Engine.InternalCalls::LogError", (void *)InternalCalls::LogError);
		mono_add_internal_call("Engine.InternalCalls::LogWarning", (void *)InternalCalls::LogWarning);

		// Entity
		mono_add_internal_call("Engine.Entity::GetEntityID_Native", (void *)InternalCalls::Entity_GetEntityID);
		mono_add_internal_call("Engine.Entity::HasComponent_Native", (void *)InternalCalls::Entity_HasComponent);

		// Prefab
		mono_add_internal_call("Engine.InternalCalls::Prefab_Instantiate", (void *)InternalCalls::Prefab_Instantiate);

		// Transform
		mono_add_internal_call("Engine.Transform::GetPosition_Native", (void *)InternalCalls::Transform_GetPosition);
		mono_add_internal_call("Engine.Transform::SetPosition_Native", (void *)InternalCalls::Transform_SetPosition);
		mono_add_internal_call("Engine.Transform::GetRotation_Native", (void *)InternalCalls::Transform_GetRotation);
		mono_add_internal_call("Engine.Transform::SetRotation_Native", (void *)InternalCalls::Transform_SetRotation);
		mono_add_internal_call("Engine.Transform::GetScale_Native", (void *)InternalCalls::Transform_GetScale);
		mono_add_internal_call("Engine.Transform::SetScale_Native", (void *)InternalCalls::Transform_SetScale);
		mono_add_internal_call("Engine.InternalCalls::Transform_Move", (void *)InternalCalls::Transform_Move);
		mono_add_internal_call("Engine.InternalCalls::Transform_SetPosition", (void *)InternalCalls::Transform_SetPosition);

		// Input
		mono_add_internal_call("Engine.Input::IsKeyPressed_Native", (void *)InternalCalls::Input_IsKeyPressed);
		mono_add_internal_call("Engine.Input::IsMouseButtonPressed_Native", (void *)InternalCalls::Input_IsMouseButtonPressed);
		mono_add_internal_call("Engine.Input::GetMousePosition_Native", (void *)InternalCalls::Input_GetMousePosition);
		mono_add_internal_call("Engine.InternalCalls::Scene_FindEntityByName", (void *)InternalCalls::Scene_FindEntityByName);

		// Physics
		mono_add_internal_call("Engine.InternalCalls::Entity_AddRigidBody", (void *)InternalCalls::Entity_AddRigidBody);

		// Entity / components
		mono_add_internal_call("Engine.InternalCalls::Entity_AddTag", (void *)InternalCalls::Entity_AddTag);
		mono_add_internal_call("Engine.InternalCalls::Entity_AddCamera", (void *)InternalCalls::Entity_AddCamera);
		mono_add_internal_call("Engine.InternalCalls::Entity_AddAudio", (void *)InternalCalls::Entity_AddAudio);
		mono_add_internal_call("Engine.InternalCalls::Entity_AddMeshRenderer", (void *)InternalCalls::Entity_AddMeshRenderer);


		// Rigidbody (component-level velocity)
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_GetVelocity", (void *)InternalCalls::Rigidbody_GetVelocity);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_SetVelocity", (void *)InternalCalls::Rigidbody_SetVelocity);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_AddVelocity", (void *)InternalCalls::Rigidbody_AddVelocity);

		// Rigidbody scalar/flag bindings
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_GetMass", (void *)InternalCalls::Rigidbody_GetMass);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_SetMass", (void *)InternalCalls::Rigidbody_SetMass);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_GetIsKinematic", (void *)InternalCalls::Rigidbody_GetIsKinematic);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_SetIsKinematic", (void *)InternalCalls::Rigidbody_SetIsKinematic);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_GetUseGravity", (void *)InternalCalls::Rigidbody_GetUseGravity);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_SetUseGravity", (void *)InternalCalls::Rigidbody_SetUseGravity);

		// Rigidbody helpers
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_GetSpeed", (void *)InternalCalls::Rigidbody_GetSpeed);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_IsMoving", (void *)InternalCalls::Rigidbody_IsMoving);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_IsStatic", (void *)InternalCalls::Rigidbody_IsStatic);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_AddForce", (void *)InternalCalls::Rigidbody_AddForce);
		mono_add_internal_call("Engine.InternalCalls::Rigidbody_Stop", (void *)InternalCalls::Rigidbody_Stop);


		// Physics collisions (keep using PhysicsAPI ONLY for this)
		mono_add_internal_call("Engine.InternalCalls::Physics_EnableCollisionEvents", (void *)InternalCalls::Physics_EnableCollisionEvents);
		mono_add_internal_call("Engine.InternalCalls::Physics_BeginCollisionFrame", (void *)InternalCalls::Physics_BeginCollisionFrame);
		mono_add_internal_call("Engine.InternalCalls::Physics_GetCollisionCount", (void *)InternalCalls::Physics_GetCollisionCount);
		mono_add_internal_call("Engine.InternalCalls::Physics_GetCollisionPair", (void *)InternalCalls::Physics_GetCollisionPair);

		// Collision event access
		mono_add_internal_call("Engine.InternalCalls::Physics_EnableCollisionEvents", (void *)InternalCalls::Physics_EnableCollisionEvents);
		mono_add_internal_call("Engine.InternalCalls::Physics_BeginCollisionFrame", (void *)InternalCalls::Physics_BeginCollisionFrame);
		mono_add_internal_call("Engine.InternalCalls::Physics_GetCollisionCount", (void *)InternalCalls::Physics_GetCollisionCount);
		mono_add_internal_call("Engine.InternalCalls::Physics_GetCollisionPair", (void *)InternalCalls::Physics_GetCollisionPair);

		// Tag
		mono_add_internal_call("Engine.InternalCalls::Tag_GetTag", (void *)InternalCalls::Tag_GetTag);
		mono_add_internal_call("Engine.InternalCalls::Tag_SetTag", (void *)InternalCalls::Tag_SetTag);

		// Camera
		mono_add_internal_call("Engine.InternalCalls::Camera_GetEnabled", (void *)InternalCalls::Camera_GetEnabled);
		mono_add_internal_call("Engine.InternalCalls::Camera_SetEnabled", (void *)InternalCalls::Camera_SetEnabled);
		mono_add_internal_call("Engine.InternalCalls::Camera_GetPrimary", (void *)InternalCalls::Camera_GetPrimary);
		mono_add_internal_call("Engine.InternalCalls::Camera_SetPrimary", (void *)InternalCalls::Camera_SetPrimary);
		mono_add_internal_call("Engine.InternalCalls::Camera_GetFOV", (void *)InternalCalls::Camera_GetFOV);
		mono_add_internal_call("Engine.InternalCalls::Camera_SetFOV", (void *)InternalCalls::Camera_SetFOV);
		mono_add_internal_call("Engine.InternalCalls::Camera_GetNear", (void *)InternalCalls::Camera_GetNear);
		mono_add_internal_call("Engine.InternalCalls::Camera_SetNear", (void *)InternalCalls::Camera_SetNear);
		mono_add_internal_call("Engine.InternalCalls::Camera_GetFar", (void *)InternalCalls::Camera_GetFar);
		mono_add_internal_call("Engine.InternalCalls::Camera_SetFar", (void *)InternalCalls::Camera_SetFar);
		mono_add_internal_call("Engine.InternalCalls::Camera_GetTarget", (void *)InternalCalls::Camera_GetTarget);
		mono_add_internal_call("Engine.InternalCalls::Camera_SetTarget", (void *)InternalCalls::Camera_SetTarget);

		// MeshRenderer
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_GetVisible", (void *)InternalCalls::MeshRenderer_GetVisible);
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_SetVisible", (void *)InternalCalls::MeshRenderer_SetVisible);
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_GetShadowCast", (void *)InternalCalls::MeshRenderer_GetShadowCast);
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_SetShadowCast", (void *)InternalCalls::MeshRenderer_SetShadowCast);
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_GetShadowReceive", (void *)InternalCalls::MeshRenderer_GetShadowReceive);
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_SetShadowReceive", (void *)InternalCalls::MeshRenderer_SetShadowReceive);
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_GetGlobalIlluminate", (void *)InternalCalls::MeshRenderer_GetGlobalIlluminate);
		mono_add_internal_call("Engine.InternalCalls::MeshRenderer_SetGlobalIlluminate", (void *)InternalCalls::MeshRenderer_SetGlobalIlluminate);

		// Audio
		mono_add_internal_call("Engine.InternalCalls::Audio_Play", (void *)InternalCalls::Audio_Play);
		mono_add_internal_call("Engine.InternalCalls::Audio_Stop", (void *)InternalCalls::Audio_Stop);
		mono_add_internal_call("Engine.InternalCalls::Audio_Pause", (void *)InternalCalls::Audio_Pause);

		mono_add_internal_call("Engine.InternalCalls::Audio_GetVolume", (void *)InternalCalls::Audio_GetVolume);
		mono_add_internal_call("Engine.InternalCalls::Audio_SetVolume", (void *)InternalCalls::Audio_SetVolume);

		mono_add_internal_call("Engine.InternalCalls::Audio_GetPitch", (void *)InternalCalls::Audio_GetPitch);
		mono_add_internal_call("Engine.InternalCalls::Audio_SetPitch", (void *)InternalCalls::Audio_SetPitch);

		mono_add_internal_call("Engine.InternalCalls::Audio_GetLoop", (void *)InternalCalls::Audio_GetLoop);
		mono_add_internal_call("Engine.InternalCalls::Audio_SetLoop", (void *)InternalCalls::Audio_SetLoop);

		mono_add_internal_call("Engine.InternalCalls::Audio_GetMute", (void *)InternalCalls::Audio_GetMute);
		mono_add_internal_call("Engine.InternalCalls::Audio_SetMute", (void *)InternalCalls::Audio_SetMute);

		mono_add_internal_call("Engine.InternalCalls::Audio_GetIs3D", (void *)InternalCalls::Audio_GetIs3D);
		mono_add_internal_call("Engine.InternalCalls::Audio_SetIs3D", (void *)InternalCalls::Audio_SetIs3D);

		mono_add_internal_call("Engine.InternalCalls::Audio_SetFile", (void *)InternalCalls::Audio_SetFile);





		LOG_INFO("Internal calls registered");
	}

	// ============================================
	// Internal Call Implementations
	// ============================================

	namespace InternalCalls
	{
		// Global scene pointer (set by ScriptSystem)
		static Scene *s_CurrentScene = nullptr;
		//auto& input = GetInput();
		static Input *s_InputSystem = nullptr;

		uint64_t Scene_CreateEntity(MonoString *nameStr)
		{
			if (!InternalCalls::s_CurrentScene)
			{
				LOG_ERROR("[InternalCall] Scene_CreateEntity: current scene is null");
				return 0;
			}

			// Name (optional)
			std::string name = "Entity";
			if (nameStr)
			{
				char *c = mono_string_to_utf8(nameStr);
				if (c)
				{
					name = c; mono_free(c);
				}
			}

			// Create via your scene API
			Entity e = InternalCalls::s_CurrentScene->CreateEntity(name.c_str());
			const uint64_t id = static_cast<uint32_t>(e);
			LOG_INFO("[InternalCall] Created entity '", name, "' (ID=", id, ")");

			return id;
		}

		static void Entity_AddScript(uint64_t entityID, MonoString *classFullNameStr)
		{
			if (!InternalCalls::s_CurrentScene)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: current scene is null");
				return;
			}
			if (!classFullNameStr)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: class name is null");
				return;
			}

			// Resolve target entity
			Entity e = InternalCalls::s_CurrentScene->GetEntity(static_cast<entt::entity>(entityID));
			if (!e)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: entity ID=", entityID, " is invalid");
				return;
			}

			// Managed class name (e.g., "Game.Bullet")
			char *c = mono_string_to_utf8(classFullNameStr);
			std::string klass = c ? c : "";
			if (c) mono_free(c);
			if (klass.empty())
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: empty class name");
				return;
			}

			// Create managed instance
			auto &se = MonoScriptEngine::GetInstance();
			MonoObject *instance = se.CreateScriptInstance(klass);
			if (!instance)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: failed to create instance of ", klass);
				return;
			}

			// Set EntityID field on the C# instance (your scripts have: public uint/int EntityID;)
			// If your field is 'int', this still works (Mono boxes by pointer size); adjust type if needed.
			uint32_t idCopy = static_cast<uint32_t>(entityID);
			se.SetFieldValue(instance, "EntityID", &idCopy);

			if (e.HasComponent<ScriptComponent>())
			{
				auto &sc = e.GetComponent<ScriptComponent>();
				sc.ScriptClassName = klass;
				sc.ScriptInstance = instance;
			}
			else
			{
				auto &sc = e.AddComponent<ScriptComponent>();
				sc.ScriptClassName = klass;
				sc.ScriptInstance = instance;
			}

			se.CallMethod(instance, "OnStart");

			LOG_INFO("[InternalCall] Attached script '", klass, "' to entity ID=", entityID);
		}

		static void Scene_DestroyEntity(uint64_t entityID)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCall] Scene_DestroyEntity: current scene is null");
				return;
			}
			auto id = s_CurrentScene->GetEntity(static_cast<entt::entity>(entityID));
			s_CurrentScene->DestroyEntity(id);
			LOG_INFO("[InternalCall] Destroyed entity ID=", entityID);
		}

		static uint64_t Prefab_Instantiate(MonoString *prefabPathStr)
		{
			if (!InternalCalls::s_CurrentScene)
			{
				LOG_ERROR("[InternalCall] Prefab_Instantiate: current scene is null");
				return 0;
			}

			if (!prefabPathStr)
			{
				LOG_ERROR("[InternalCall] Prefab_Instantiate: prefab path is null");
				return 0;
			}

			// Convert MonoString to C++ string
			char *c = mono_string_to_utf8(prefabPathStr);
			std::string prefabPath = c ? c : "";
			if (c) mono_free(c);

			if (prefabPath.empty())
			{
				LOG_ERROR("[InternalCall] Prefab_Instantiate: empty prefab path");
				return 0;
			}

			LOG_INFO("[InternalCall] Instantiating prefab: ", prefabPath);

			// Load prefab from file
			auto prefab = PrefabSerializer::LoadPrefabFromFile(prefabPath);
			if (!prefab)
			{
				LOG_ERROR("[InternalCall] Prefab_Instantiate: failed to load prefab from ", prefabPath);
				return 0;
			}

			// Register prefab
			PrefabRegistry::Get().RegisterPrefab(prefab);

			// Instantiate entity from prefab
			Entity entity = PrefabInstantiator::InstantiateEntityPrefab(
				InternalCalls::s_CurrentScene,
				prefab->GetGUID()
			);

			if (!entity)
			{
				LOG_ERROR("[InternalCall] Prefab_Instantiate: failed to instantiate entity");
				return 0;
			}

			uint64_t entityID = static_cast<uint64_t>(static_cast<uint32_t>(entity));
			LOG_INFO("[InternalCall] Successfully instantiated prefab - Entity ID: ", entityID);

			return entityID;
		}

		void Transform_Move(uint64_t entityID, float deltaX, float deltaY, float deltaZ)
		{
			if (!s_CurrentScene) return;

			auto entity = s_CurrentScene->GetEntity(static_cast<entt::entity>(entityID));
			if (!entity || !entity.HasComponent<TransformComponent>()) return;

			auto &transform = entity.GetComponent<TransformComponent>();
			transform.Position.x += deltaX;
			transform.Position.y += deltaY;
			transform.Position.z += deltaZ;
		}
		uint64_t Scene_FindEntityByName(MonoString *nameString)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCall] Scene not set!");
				return 0; // 0 = invalid entity (entt::null)
			}

			if (!nameString)
			{
				LOG_ERROR("[InternalCall] Entity name string is null!");
				return 0;
			}

			// Convert MonoString to C++ string
			char *nameStr = mono_string_to_utf8(nameString);
			if (!nameStr)
			{
				LOG_ERROR("[InternalCall] Failed to convert entity name string!");
				return 0;
			}

			std::string name(nameStr);
			mono_free(nameStr);

			if (name.empty())
			{
				LOG_WARNING("[InternalCall] Empty entity name provided!");
				return 0;
			}

			// Find entity by name
			Entity entity = s_CurrentScene->FindEntityByName(name);
			LOG_INFO("player found heres");

			// Convert Entity to uint32_t using the conversion operator
			// If entity is invalid (entt::null), this will return 0
			return static_cast<uint32_t>(entity);
		}

		void SetCurrentScene(Scene *scene)
		{
			s_CurrentScene = scene;
		}

		void SetInputSystem(Input *input)
		{
			s_InputSystem = input;
		}

		void Log(MonoString *message)
		{
			char *cStr = mono_string_to_utf8(message);
			LOG_INFO("[C#] ", cStr);
			mono_free(cStr);
		}

		void LogError(MonoString *message)
		{
			char *cStr = mono_string_to_utf8(message);
			LOG_ERROR("[C#] ", cStr);
			mono_free(cStr);
		}

		void LogWarning(MonoString *message)
		{
			char *cStr = mono_string_to_utf8(message);
			LOG_WARNING("[C#] ", cStr);  // Fixed: was LOG_ERROR
			mono_free(cStr);
		}

		uint64_t Entity_GetEntityID(MonoObject *entityObj)
		{
			(void)entityObj;  // Suppress warning

			// Entity ID is stored in the C# Entity class
			return 0; // Placeholder - implement based on your C# Entity class structure
		}

		bool Entity_HasComponent(uint64_t entityID, MonoReflectionType *componentType)
		{
			if (!s_CurrentScene) return false;
			(void)componentType;
			auto &registry = s_CurrentScene->GetRegistry();
			auto handle = static_cast<entt::entity>(entityID);

			// Check if entity exists in registry
			if (!registry.valid(handle)) return false;

			// Check component type (you'll need to map C# component types to C++ component types)
			// This is a simplified version
			return false; // Implement based on your component system
		}



		void Transform_GetPosition(uint64_t entityID, glm::vec3 *outPosition)
		{
			if (!s_CurrentScene || !outPosition) return;

			auto entity = s_CurrentScene->GetEntity(static_cast<entt::entity>(entityID));
			if (!entity || !entity.HasComponent<TransformComponent>()) return;

			auto &transform = entity.GetComponent<TransformComponent>();
			*outPosition = transform.Position;
		}

		void Transform_SetPosition(uint64_t entityID, glm::vec3 *position)
		{
			if (!s_CurrentScene || !position) return;

			auto entity = s_CurrentScene->GetEntity(static_cast<entt::entity>(entityID));
			if (!entity || !entity.HasComponent<TransformComponent>()) return;

			auto &transform = entity.GetComponent<TransformComponent>();
			transform.Position = *position;
			transform.IsDirty = true;
		}

		void Transform_GetRotation(uint64_t entityID, glm::vec3 *outRotation)
		{
			if (!s_CurrentScene || !outRotation) return;

			auto &registry = s_CurrentScene->GetRegistry();
			auto handle = static_cast<entt::entity>(entityID);

			if (!registry.valid(handle)) return;
			if (!registry.all_of<TransformComponent>(handle)) return;

			auto &transform = registry.get<TransformComponent>(handle);
			*outRotation = glm::degrees(glm::eulerAngles(transform.Rotation));  // Convert quat to euler
		}

		void Transform_SetRotation(uint64_t entityID, glm::vec3 *rotation)
		{
			if (!s_CurrentScene || !rotation) return;

			auto &registry = s_CurrentScene->GetRegistry();
			auto handle = static_cast<entt::entity>(entityID);

			if (!registry.valid(handle)) return;
			if (!registry.all_of<TransformComponent>(handle)) return;

			auto &transform = registry.get<TransformComponent>(handle);
			transform.Rotation = glm::quat(glm::radians(*rotation));  // Convert euler to quat
		}

		void Transform_GetScale(uint64_t entityID, glm::vec3 *outScale)
		{
			if (!s_CurrentScene || !outScale) return;

			auto &registry = s_CurrentScene->GetRegistry();
			auto handle = static_cast<entt::entity>(entityID);

			if (!registry.valid(handle)) return;
			if (!registry.all_of<TransformComponent>(handle)) return;

			auto &transform = registry.get<TransformComponent>(handle);
			*outScale = transform.Scale;
		}

		void Transform_SetScale(uint64_t entityID, glm::vec3 *scale)
		{
			if (!s_CurrentScene || !scale) return;

			auto &registry = s_CurrentScene->GetRegistry();
			auto handle = static_cast<entt::entity>(entityID);

			if (!registry.valid(handle)) return;
			if (!registry.all_of<TransformComponent>(handle)) return;

			auto &transform = registry.get<TransformComponent>(handle);
			transform.Scale = *scale;
		}

		bool Input_IsKeyPressed(int keyCode)
		{
			return s_InputSystem->IsKeyPressed(keyCode);
		}


		bool Input_IsMouseButtonPressed(int button)
		{

			if (!s_InputSystem)
			{
				LOG_WARNING("[InternalCall] Input system not initialized");
				return false;
			}
			return s_InputSystem->IsMouseButtonPressed(button);
		}


		void Input_GetMousePosition(glm::vec2 *outPosition)
		{
			if (!outPosition) return;
			// Access your input system
			// Example: *outPosition = Input::GetMousePosition();
		}

		// Helper to fetch entity from current scene
		static inline Entity GetEntityOrNull(uint64_t id)
		{
			if (!s_CurrentScene) return {};
			return s_CurrentScene->GetEntity(static_cast<entt::entity>(id));
		}

		// =============== Physics (bodies) ===============

		void Entity_AddRigidBody(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			if (!e) return;
			e.AddComponent<RigidbodyComponent>();
		}

		void Rigidbody_GetVelocity(uint64_t entityID, glm::vec3 *outVel)
		{
			auto e = GetEntityOrNull(entityID);
			if (!outVel) return;
			*outVel = e.GetComponent<RigidbodyComponent>().GetVelocity();
		}

		void Rigidbody_SetVelocity(uint64_t entityID, glm::vec3 *inVel)
		{
			auto e = GetEntityOrNull(entityID);
			if (!inVel) return;
			e.GetComponent<RigidbodyComponent>().SetVelocity(*inVel);
		}

		void Rigidbody_AddVelocity(uint64_t entityID, glm::vec3 *inVel)
		{
			auto e = GetEntityOrNull(entityID);
			if (!inVel) return;
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			rb.SetVelocity(rb.GetVelocity() + *inVel);
		}

		float Rigidbody_GetMass(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			return rb.GetMass();
		}

		void Rigidbody_SetMass(uint64_t entityID, float mass)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			rb.SetMass(mass);
		}

		bool Rigidbody_GetIsKinematic(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			return rb.IsKinematicBody();
		}

		void Rigidbody_SetIsKinematic(uint64_t entityID, bool isKinematic)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			rb.SetKinematic(isKinematic);
		}

		bool Rigidbody_GetUseGravity(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			return rb.IsGravityEnabled();
		}

		void Rigidbody_SetUseGravity(uint64_t entityID, bool useGravity)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			rb.SetGravityEnabled(useGravity);
		}

		float Rigidbody_GetSpeed(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			return rb.GetSpeed();
		}

		bool Rigidbody_IsMoving(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			return rb.IsMoving();
		}

		bool Rigidbody_IsStatic(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			return rb.IsStatic();
		}

		void Rigidbody_AddForce(uint64_t entityID, glm::vec3 *force)
		{
			auto e = GetEntityOrNull(entityID);
			if (!force) return;
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			rb.AddForce(*force);
		}

		void Rigidbody_Stop(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &rb{ e.GetComponent<RigidbodyComponent>() };
			rb.Stop();
		}

		// ==================================
		// Collision events (via PhysicsAPI)
		// ==================================

		void Physics_EnableCollisionEvents()
		{
			Engine::PhysicsAPI::EnableCollisionEvents();
		}

		void Physics_BeginCollisionFrame()
		{
			Engine::PhysicsAPI::BeginCollisionFrame();
		}

		int Physics_GetCollisionCount()
		{
			return (int)Engine::PhysicsAPI::GetInstance().GetCollisionEvents().size();
		}

		void Physics_GetCollisionPair(int index, uint32_t *a, uint32_t *b)
		{
			if (!a || !b) return;
			const auto &evs = Engine::PhysicsAPI::GetInstance().GetCollisionEvents();
			if (index < 0 || index >= (int)evs.size()) return;
			*a = (uint32_t)evs[index].entA;
			*b = (uint32_t)evs[index].entB;
		}

		void Entity_AddTag(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			if (!e) return;
			e.AddComponent<TagComponent>();
		}

		void Entity_AddCamera(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			if (!e) return;
			e.AddComponent<CameraComponent>();
		}

		void Entity_AddAudio(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			if (!e) return;
			e.AddComponent<AudioComponent>();
		}

		void Entity_AddMeshRenderer(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			if (!e) return;
			e.AddComponent<MeshRendererComponent>();
		}

		MonoString *Tag_GetTag(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TagComponent>())
				return mono_string_new(mono_domain_get(), "");

			auto &tagComp = e.GetComponent<TagComponent>();
			const std::string &tag = tagComp.GetTag();
			return mono_string_new(mono_domain_get(), tag.c_str());
		}

		void Tag_SetTag(uint64_t entityID, MonoString *tagStr)
		{
			auto e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TagComponent>() || !tagStr)
				return;

			char *utf8 = mono_string_to_utf8(tagStr);
			if (!utf8) return;

			e.GetComponent<TagComponent>().SetTag(utf8);
			mono_free(utf8);
		}

		bool Camera_GetEnabled(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			return cam.Enabled;
		}

		void Camera_SetEnabled(uint64_t entityID, bool enabled)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			cam.Enabled = enabled;
			cam.isDirty = true;
		}

		bool Camera_GetPrimary(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			return cam.Primary;
		}

		void Camera_SetPrimary(uint64_t entityID, bool primary)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			cam.Primary = primary;
			cam.isDirty = true;
		}

		float Camera_GetFOV(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			return cam.FOV;
		}

		void Camera_SetFOV(uint64_t entityID, float fov)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			cam.FOV = fov;
			cam.isDirty = true;
		}

		float Camera_GetNear(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			return cam.NearPlane;
		}

		void Camera_SetNear(uint64_t entityID, float nearPlane)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			cam.NearPlane = nearPlane;
			cam.isDirty = true;
		}

		float Camera_GetFar(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			return cam.FarPlane;
		}

		void Camera_SetFar(uint64_t entityID, float farPlane)
		{
			auto e = GetEntityOrNull(entityID);
			auto &cam = e.GetComponent<CameraComponent>();
			cam.FarPlane = farPlane;
			cam.isDirty = true;
		}

		void Camera_GetTarget(uint64_t entityID, glm::vec3 *outTarget)
		{
			auto e = GetEntityOrNull(entityID);
			if (!outTarget) return;
			auto &cam = e.GetComponent<CameraComponent>();
			*outTarget = cam.Target;
		}

		void Camera_SetTarget(uint64_t entityID, glm::vec3 *inTarget)
		{
			auto e = GetEntityOrNull(entityID);
			if (!inTarget) return;
			auto &cam = e.GetComponent<CameraComponent>();
			cam.SetTarget(*inTarget); // marks dirty internally
		}

		bool MeshRenderer_GetVisible(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			return mr.Visible;
		}

		void MeshRenderer_SetVisible(uint64_t entityID, bool visible)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			mr.Visible = visible;
		}

		bool MeshRenderer_GetShadowCast(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			return mr.ShadowCast;
		}

		void MeshRenderer_SetShadowCast(uint64_t entityID, bool cast)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			mr.ShadowCast = cast;
		}

		bool MeshRenderer_GetShadowReceive(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			return mr.ShadowReceive;
		}

		void MeshRenderer_SetShadowReceive(uint64_t entityID, bool receive)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			mr.ShadowReceive = receive;
		}

		bool MeshRenderer_GetGlobalIlluminate(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			return mr.GlobalIlluminate;
		}

		void MeshRenderer_SetGlobalIlluminate(uint64_t entityID, bool gi)
		{
			auto e = GetEntityOrNull(entityID);
			auto &mr = e.GetComponent<MeshRendererComponent>();
			mr.GlobalIlluminate = gi;
		}

		void Audio_Play(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.SetState(PlayState::PLAY);
		}

		void Audio_Stop(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.SetState(PlayState::STOP);
		}

		void Audio_Pause(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.SetState(PlayState::PAUSE);
		}

		float Audio_GetVolume(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			return audio.Volume;
		}

		void Audio_SetVolume(uint64_t entityID, float volume)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.SetVolume(volume);
		}

		float Audio_GetPitch(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			return audio.Pitch;
		}

		void Audio_SetPitch(uint64_t entityID, float pitch)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.SetPitch(pitch);
		}

		bool Audio_GetLoop(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			return audio.Loop;
		}

		void Audio_SetLoop(uint64_t entityID, bool loop)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.SetLoop(loop);
		}

		bool Audio_GetMute(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			return audio.Mute;
		}

		void Audio_SetMute(uint64_t entityID, bool mute)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.SetMute(mute);
		}

		bool Audio_GetIs3D(uint64_t entityID)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			return audio.Is3D;
		}

		void Audio_SetIs3D(uint64_t entityID, bool is3d)
		{
			auto e = GetEntityOrNull(entityID);
			auto &audio = e.GetComponent<AudioComponent>();
			audio.Set3D(is3d);
		}

		void Audio_SetFile(uint64_t entityID, MonoString *path)
		{
			auto e = GetEntityOrNull(entityID);
			if (!path) return;

			auto &audio = e.GetComponent<AudioComponent>();

			char *utf8 = mono_string_to_utf8(path);
			if (!utf8) return;

			audio.SetAudioFile(utf8);
			mono_free(utf8);
		}
	} // namespace internalcalls

	// Expose these functions for external use
	void SetScriptingInputSystem(Input *input)
	{
		InternalCalls::SetInputSystem(input);
	}

	void SetScriptingCurrentScene(Scene *scene)
	{
		InternalCalls::SetCurrentScene(scene);
	}

} // namespace Engine

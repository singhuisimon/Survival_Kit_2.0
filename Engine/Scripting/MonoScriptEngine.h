#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <vector>
#include <cstdint>

// Forward declarations for Mono types
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoClassField MonoClassField;

namespace Engine {

	class Scene;
	class Entity;
	class Input;
	class AudioManager;
	class Renderer;
	class Game;

	struct ManagedFieldSnapshot {
		// entity id -> (class name -> (field name -> serialized bytes/string))
		// Keep this aligned with what your engine already supports (int/float/bool/string/vec3/quat/etc).
		std::unordered_map<std::uint32_t,
			std::unordered_map<std::string,
			std::unordered_map<std::string, std::vector<std::uint8_t>>>> data;
	};

	// Functions to set context for internal calls
	void SetScriptingCurrentScene(Scene *scene);
	void SetScriptingInputSystem(Input *input);
	void SetScriptingAudioManager(AudioManager *audiomManager);
	void SetScriptingRenderer(Renderer* renderer);
	void SetScriptingGameInstance(Game* gameInstance);

	class MonoScriptEngine {
	public:
		static MonoScriptEngine &GetInstance();

		// Initialization and cleanup
		void Initialize(const std::string &assemblyPath);
		void Shutdown();

		// ============================================================
		// Script instance management (HANDLE-FIRST)
		// ============================================================
		// Preferred API:
		//  - CreateScriptInstanceHandle returns a GCHandle that you store in ScriptComponent::GCHandle.
		//  - Resolve later using GetObjectFromGCHandle(handle) or ScriptHandleUtil::Resolve(component).
		//  - Destroy using DestroyScriptHandle(handle).
		uint32_t   CreateScriptInstanceHandle(const std::string &className, MonoObject **outInstance = nullptr, bool pinned = false);
		MonoObject *GetObjectFromGCHandle(uint32_t gcHandle);
		void       DestroyScriptHandle(uint32_t gcHandle);
		uint32_t   CreateGCHandleForObject(MonoObject *obj, bool pinned = false);
		void       FreeGCHandle(uint32_t gcHandle);

		// Legacy API (NO LONGER ROOTS/OWNS LIFETIME):
		// Returns a raw MonoObject*; caller must immediately create/store a GCHandle,
		// otherwise the object can be moved/collected.
		MonoObject *CreateScriptInstance(const std::string &className);

		// Legacy: calls OnDestroy on the passed instance (does NOT free any handle).
		void DestroyScriptInstance(MonoObject *instance);

		bool IsValidMonoObject(MonoObject *instance);

		// Script method invocation
		void CallMethod(MonoObject *instance, const std::string &methodName);
		void CallMethod(MonoObject *instance, const std::string &methodName,
						void **params, int paramCount);

		// Class and method lookup
		MonoClass *GetScriptClass(const std::string &className);
		MonoMethod *GetMethod(MonoClass *klass, const std::string &methodName,
							  int paramCount = 0);

		// Field access
		void SetFieldValue(MonoObject *instance, const std::string &fieldName, void *value);
		void *GetFieldValue(MonoObject *instance, const std::string &fieldName);

		// Bind native EntityID to managed script
		void BindEntityID(MonoObject *instance, std::uint32_t entityID);

		// ============================================================
		// ScriptComponent-backed serialized field storage (editor source of truth)
		// ============================================================
		// Store a single field's current value (read from 'instance') into the owning ScriptComponent.
		// This is intended to be called by the editor when a user changes a serialized field.
		void StoreSerializedFieldToComponent(MonoObject *instance, MonoClassField *field);

		// Apply ScriptComponent::SerializedFields (if any) onto the given instance.
		// Safe to call on newly created instances (before OnStart).
		void ApplySerializedFieldsFromComponent(std::uint32_t entityID, MonoObject *instance);

		// Hot reload support
		void EnsureCorrectDomain();
		bool IsInCorrectDomain();
		// NOTE: Any native thread calling into Mono must be attached.
		void EnsureThreadAttached();

		// Editor/runtime safety: resolve a possibly-stale MonoObject* (cached raw pointer)
		// to the current instance address using the owning ScriptComponent's GCHandle.
		// Returns nullptr if it cannot be resolved safely.
		MonoObject *ResolveScriptInstance(MonoObject *maybeStale);

		// Getters
		MonoDomain *GetRootDomain() const {
			return m_RootDomain;
		}
		MonoDomain *GetDomain() const {
			return m_AppDomain ? m_AppDomain : m_RootDomain;
		}
		MonoAssembly *GetAssembly() const {
			return m_AppAssembly;
		}
		MonoImage *GetImage() const {
			return m_AppImage;
		}

		// ============================================================
		// Editor/runtime helper: ensure managed instances exist
		// ============================================================
		// Ensures a ScriptComponent has a valid managed instance and rooted GCHandle.
		// - Creates an instance if missing
		// - Clears garbage/stale handles loaded from disk
		// - Recreates instances if the type no longer matches ScriptClassName
		// - Does NOT call OnStart/OnUpdate/OnFixedUpdate
		MonoObject *EnsureScriptInstance(Scene *scene, std::uint32_t entityID, bool applySerializedFields = true);

		// Ensures every ScriptComponent in the scene has a valid managed instance.
		// Does NOT call OnStart/OnUpdate/OnFixedUpdate.
		void EnsureAllScriptInstances(Scene *scene, bool applySerializedFields = true);


		// Legacy helper kept for compilation safety (best-effort).
		// If you have a GCHandle, call GetObjectFromGCHandle(handle) instead.
		MonoObject *GetObjectFromHandle(void *instancePtr);
		bool HotReloadOnPlay(bool preserveManagedState);
		bool InstanceMatchesClass(MonoObject *instance, const std::string &expectedFullName);

	private:
		MonoScriptEngine() = default;
		~MonoScriptEngine() = default;
		MonoScriptEngine(const MonoScriptEngine &) = delete;
		MonoScriptEngine &operator=(const MonoScriptEngine &) = delete;

		std::unordered_set<MonoObject *> m_ValidInstances;

		void RegisterInternalCalls();
		void LoadAssembly(const std::string &path);
		void UnloadAssembly();

		ManagedFieldSnapshot CaptureManagedState();
		void RestoreManagedState(const ManagedFieldSnapshot &snap);

		bool BuildGameScripts();          // you already have something like this (dotnet build/msbuild)
		bool ReloadDomainAndAssembly();   // your existing unload/load flow
		void RebindAllScriptComponents(); // recreate instances + BindEntityID for all entities

		MonoDomain *m_RootDomain = nullptr;
		MonoDomain *m_AppDomain = nullptr;
		MonoAssembly *m_AppAssembly = nullptr;
		MonoImage *m_AppImage = nullptr;
		std::string   m_AssemblyPath;
		std::unordered_map<std::string, MonoClass *> m_ClassCache;
		std::unordered_set<std::uint32_t> m_LiveGCHandles;
	};

} // namespace Engine

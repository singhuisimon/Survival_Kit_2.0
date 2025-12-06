#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <cstdint>   // <-- add this

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
	class AudioManager; //newly added in - amanda

	// Functions to set context for internal calls
	void SetScriptingCurrentScene(Scene *scene);
	void SetScriptingInputSystem(Input *input);

	//newly added - amanda
	void SetScriptingAudioManager(AudioManager *audiomManager);

	class MonoScriptEngine {
	public:
		static MonoScriptEngine &GetInstance();

		// Initialization and cleanup
		void Initialize(const std::string &assemblyPath);
		void Shutdown();

		// Script instance management
		MonoObject *CreateScriptInstance(const std::string &className);
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

		// NEW: convenience helper to bind the native entity ID to a script
		void BindEntityID(MonoObject *instance, std::uint32_t entityID);

		// Hot reload support
		//void ReloadAssembly();
		void EnsureCorrectDomain();

		bool IsInCorrectDomain();
		// Getters

		MonoDomain* GetRootDomain() const { return m_RootDomain; }   //  NEW


		MonoDomain* GetAppDomain() const { return m_AppDomain; }  //  NOT GetDomain!

		MonoDomain *GetDomain() const {
			return m_AppDomain;
		}
		MonoAssembly *GetAssembly() const {
			return m_AppAssembly;
		}
		MonoImage *GetImage() const {
			return m_AppImage;
		}

		void RegisterInstance(MonoObject *instance) {
			if(instance) m_ValidInstances.insert(instance);
		}

		void UnregisterInstance(MonoObject *instance) {
			m_ValidInstances.erase(instance);
		}

		bool IsValidInstance(MonoObject *instance) {
			return m_ValidInstances.find(instance) != m_ValidInstances.end();
		}

		void ClearAllInstances() {
			m_ValidInstances.clear();
		}

		MonoObject* GetObjectFromHandle(void* instancePtr);

	private:
		MonoScriptEngine() = default;
		~MonoScriptEngine() = default;
		MonoScriptEngine(const MonoScriptEngine &) = delete;
		MonoScriptEngine &operator=(const MonoScriptEngine &) = delete;

		std::unordered_set<MonoObject *> m_ValidInstances;

		void RegisterInternalCalls();
		void LoadAssembly(const std::string &path);
		void UnloadAssembly();

	private:
		MonoDomain *m_RootDomain = nullptr;
		MonoDomain *m_AppDomain = nullptr;
		MonoAssembly *m_AppAssembly = nullptr;
		MonoImage *m_AppImage = nullptr;
		std::string   m_AssemblyPath;
		std::unordered_map<std::string, MonoClass *> m_ClassCache;
		std::unordered_map<MonoObject*, uint32_t> m_ObjectToHandle;
	};

} // namespace Engine

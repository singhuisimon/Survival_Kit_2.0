#include <windows.h>

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
#include <mono/metadata/appdomain.h>
#include <mono/metadata/mono-gc.h>

#include <filesystem>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "InternalCalls.h"

namespace
{
	enum class SnapType : uint8_t
	{
		Invalid = 0,
		Bool = 1,
		I1 = 2,
		U1 = 3,
		I2 = 4,
		U2 = 5,
		I4 = 6,
		U4 = 7,
		I8 = 8,
		U8 = 9,
		R4 = 10,
		R8 = 11,
		StringUtf8 = 12,
		RawValue = 13
	};

	static inline void AppendBytes(std::vector<uint8_t> &out, const void *p, size_t n)
	{
		const uint8_t *b = static_cast<const uint8_t *>(p);
		out.insert(out.end(), b, b + n);
	}

	static bool HasAttribute(MonoClassField *field, const char *ns, const char *name)
	{
		if (!field) return false;

		MonoClass *parent = mono_field_get_parent(field);
		if (!parent) return false;

		MonoCustomAttrInfo *attrs = mono_custom_attrs_from_field(parent, field);
		if (!attrs) return false;

		// Try find the attribute class in the current domain by name
		// (If not found, just treat as "not present".)
		MonoClass *attrKlass = nullptr;

		// First: corlib/System.* common attrs
		MonoImage *corlib = mono_get_corlib();
		if (corlib)
			attrKlass = mono_class_from_name(corlib, ns, name);

		bool has = (attrKlass && mono_custom_attrs_has_attr(attrs, attrKlass));
		mono_custom_attrs_free(attrs);
		return has;
	}

	static bool ShouldSerializeField(MonoClassField *field)
	{
		if (!field) return false;

		const uint32_t flags = mono_field_get_flags(field);

		if (flags & MONO_FIELD_ATTR_STATIC)
			return false;

		if (flags & MONO_FIELD_ATTR_NOT_SERIALIZED)
			return false;

		// [NonSerialized]
		if (HasAttribute(field, "System", "NonSerializedAttribute"))
			return false;

		// Public => ok
		if (flags & MONO_FIELD_ATTR_PUBLIC)
			return true;

		// Non-public => only if [SerializeField] (support either namespace)
		if (HasAttribute(field, "UnityEngine", "SerializeField"))
			return true;

		if (HasAttribute(field, "Engine", "SerializeField"))
			return true;

		return false;
	}

	static std::vector<uint8_t> SerializeFieldValue(MonoObject *instance, MonoClassField *field)
	{
		std::vector<uint8_t> payload;
		if (!instance || !field) return payload;

		MonoType *mt = mono_field_get_type(field);
		if (!mt) return payload;

		const MonoTypeEnum t = static_cast<MonoTypeEnum>(mono_type_get_type(mt));

#define SER_PRIM(TAG, CPP_T)                              \
		do {                                              \
			CPP_T v{};                                    \
			mono_field_get_value(instance, field, &v);     \
			payload.push_back(static_cast<uint8_t>(TAG)); \
			AppendBytes(payload, &v, sizeof(CPP_T));       \
			return payload;                               \
		} while (0)

		switch (t)
		{
		case MONO_TYPE_BOOLEAN: SER_PRIM(SnapType::Bool, uint8_t);
		case MONO_TYPE_I1:      SER_PRIM(SnapType::I1, int8_t);
		case MONO_TYPE_U1:      SER_PRIM(SnapType::U1, uint8_t);
		case MONO_TYPE_I2:      SER_PRIM(SnapType::I2, int16_t);
		case MONO_TYPE_U2:      SER_PRIM(SnapType::U2, uint16_t);
		case MONO_TYPE_I4:      SER_PRIM(SnapType::I4, int32_t);
		case MONO_TYPE_U4:      SER_PRIM(SnapType::U4, uint32_t);
		case MONO_TYPE_I8:      SER_PRIM(SnapType::I8, int64_t);
		case MONO_TYPE_U8:      SER_PRIM(SnapType::U8, uint64_t);
		case MONO_TYPE_R4:      SER_PRIM(SnapType::R4, float);
		case MONO_TYPE_R8:      SER_PRIM(SnapType::R8, double);

		case MONO_TYPE_STRING:
		{
			MonoString *ms = nullptr;
			mono_field_get_value(instance, field, &ms);

			payload.push_back(static_cast<uint8_t>(SnapType::StringUtf8));

			if (!ms)
			{
				uint32_t len = 0;
				AppendBytes(payload, &len, sizeof(len));
				return payload;
			}

			char *utf8 = mono_string_to_utf8(ms);
			const uint32_t len = utf8 ? static_cast<uint32_t>(std::strlen(utf8)) : 0;

			AppendBytes(payload, &len, sizeof(len));
			if (len && utf8)
				AppendBytes(payload, utf8, len);

			if (utf8)
				mono_free(utf8);

			return payload;
		}

		case MONO_TYPE_VALUETYPE:
		{
			MonoClass *vc = mono_type_get_class(mt);
			if (!vc) return {};

			// Enums
			if (mono_class_is_enum(vc))
			{
				int32_t v{};
				mono_field_get_value(instance, field, &v);
				payload.push_back(static_cast<uint8_t>(SnapType::I4));
				AppendBytes(payload, &v, sizeof(v));
				return payload;
			}

			uint32_t align = 0;
			const int sz = mono_class_value_size(vc, &align);
			if (sz <= 0) return {};

			payload.push_back(static_cast<uint8_t>(SnapType::RawValue));
			const uint32_t usz = static_cast<uint32_t>(sz);
			AppendBytes(payload, &usz, sizeof(usz));

			const size_t start = payload.size();
			payload.resize(start + static_cast<size_t>(sz));
			mono_field_get_value(instance, field, payload.data() + start);
			return payload;
		}

		default:
			break;
		}

#undef SER_PRIM
		return {};
	}
} // anonymous namespace

namespace Engine
{
	template <typename T, typename = void>
	struct has_gchandle : std::false_type
	{
	};
	template <typename T>
	struct has_gchandle<T, std::void_t<decltype(std::declval<T &>().GCHandle)>> : std::true_type
	{
	};

	template <typename T, typename = void>
	struct has_started : std::false_type
	{
	};
	template <typename T>
	struct has_started<T, std::void_t<decltype(std::declval<T &>().Started)>> : std::true_type
	{
	};

	template <typename T, typename = void>
	struct has_scriptinstance : std::false_type
	{
	};
	template <typename T>
	struct has_scriptinstance<T, std::void_t<decltype(std::declval<T &>().ScriptInstance)>> : std::true_type
	{
	};

	template <typename T>
	concept HasScriptInstancePtr = requires(T t)
	{
		{
			t.ScriptInstance
		};
	};

	static Scene *s_ScriptingSceneContext = nullptr;

	MonoScriptEngine &MonoScriptEngine::GetInstance()
	{
		static MonoScriptEngine instance;
		return instance;
	}

	bool MonoScriptEngine::IsValidMonoObject(MonoObject *instance)
	{
		if (!instance)
			return false;

		if ((uintptr_t)instance == 0xFFFFFFFFFFFFFFFF || (uintptr_t)instance < 0x10000)
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
				LOG_INFO("  Successfully switched to correct domain");
			else
				LOG_ERROR("   FAILED to switch domain!");
		}
	}

	bool MonoScriptEngine::IsInCorrectDomain()
	{
		if (!m_AppDomain) return false;
		return mono_domain_get() == m_AppDomain;
	}

	// Pointer to Engine.EventSystem.RaiseFromNative(string, string)
	static MonoMethod *s_EventSystemRaiseFromNative = nullptr;

	static void RefreshEventBindings(MonoImage *appImage)
	{
		s_EventSystemRaiseFromNative = nullptr;
		if (!appImage) return;

		MonoClass *eventSystemClass = mono_class_from_name(appImage, "Engine", "EventSystem");
		if (!eventSystemClass)
		{
			LOG_WARNING("[Mono] Engine.EventSystem class not found - script events will not be delivered to C#");
			return;
		}

		s_EventSystemRaiseFromNative =
			mono_class_get_method_from_name(eventSystemClass, "RaiseFromNative", 2);

		if (!s_EventSystemRaiseFromNative)
			LOG_WARNING("[Mono] Engine.EventSystem.RaiseFromNative(string,string) not found");
	}

	void MonoScriptEngine::Initialize(const std::string &assemblyPath)
	{
		static bool s_Initialized = false;
		if (s_Initialized)
		{
			LOG_WARNING("Mono Script Engine already initialized, skipping...");
			return;
		}

		LOG_INFO("Initializing Mono Script Engine...");

		m_AssemblyPath = assemblyPath;

		WCHAR exePath[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, exePath, MAX_PATH);

		std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
		std::filesystem::path monoLibPath = exeDir / "mono" / "lib";

		std::string monoLibPathStr = monoLibPath.generic_string();
		mono_set_assemblies_path(monoLibPathStr.c_str());

		LOG_INFO("[Mono] Assembly path set to: ", monoLibPathStr);

		m_RootDomain = mono_jit_init("EngineRuntime");
		if (!m_RootDomain)
		{
			LOG_ERROR("Failed to initialize Mono JIT");
			return;
		}

		LOG_INFO(" Mono JIT initialized");

		// RECOMMENDED: Use multi-domain mode for proper hot reload
#if 1  // Change to 0 to use single-domain mode
		const char *domainName = "GameScriptsDomain";
		m_AppDomain = mono_domain_create_appdomain(const_cast<char *>(domainName), nullptr);

		if (!m_AppDomain)
		{
			LOG_WARNING("Failed to create separate app domain, falling back to root domain");
			LOG_WARNING("  Hot reload will be limited in single-domain mode");
			m_AppDomain = m_RootDomain;
		}
		else
		{
			LOG_INFO("Using multi-domain mode for proper hot reload support");
			LOG_INFO("  Root domain:  ", (uintptr_t)m_RootDomain);
			LOG_INFO("  App domain:   ", (uintptr_t)m_AppDomain);
		}
#else
// Single-domain mode (limited hot reload)
		m_AppDomain = m_RootDomain;
		LOG_WARNING("Using single-domain mode (hot reload will be imperfect)");
#endif

		mono_domain_set(m_AppDomain, false);

		if (!assemblyPath.empty() && std::filesystem::exists("GameScripts.dll"))
			LoadAssembly("GameScripts.dll");
		else
		{
			LOG_WARNING("Assembly not found: ", assemblyPath);
			LOG_WARNING("Mono initialized but no scripts will be loaded");
		}

		RegisterInternalCalls();

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

		UnloadAssembly();

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
		RefreshEventBindings(m_AppImage);
	}

	void MonoScriptEngine::UnloadAssembly()
	{
		m_ClassCache.clear();
		m_AppImage = nullptr;
		m_AppAssembly = nullptr;
	}

	MonoClass *MonoScriptEngine::GetScriptClass(const std::string &className)
	{
		if (!m_AppImage)
		{
			LOG_ERROR("Cannot get script class '", className, "': Assembly not loaded");
			return nullptr;
		}

		auto it = m_ClassCache.find(className);
		if (it != m_ClassCache.end())
			return it->second;

		size_t lastDot = className.find_last_of('.');
		std::string namespaceName = lastDot != std::string::npos ? className.substr(0, lastDot) : "";
		std::string classNameOnly = lastDot != std::string::npos ? className.substr(lastDot + 1) : className;

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

		m_ClassCache[className] = klass;
		return klass;
	}

	// ------------------------------------------------------------------------
	// Legacy raw instance creation (caller must store a GCHandle immediately).
	// ------------------------------------------------------------------------
	MonoObject *MonoScriptEngine::CreateScriptInstance(const std::string &className)
	{
		if (!m_AppImage)
			return nullptr;

		EnsureCorrectDomain();

		MonoClass *klass = GetScriptClass(className);
		if (!klass)
			return nullptr;

		MonoObject *instance = mono_object_new(m_AppDomain, klass);
		if (!instance)
		{
			LOG_ERROR("Failed to create instance of: ", className);
			return nullptr;
		}

		mono_runtime_object_init(instance);
		return instance;
	}

	// ------------------------------------------------------------------------
	// Handle-first creation (what ScriptComponent should use).
	// ------------------------------------------------------------------------
	uint32_t MonoScriptEngine::CreateScriptInstanceHandle(const std::string &className, MonoObject **outInstance, bool pinned)
	{
		MonoObject *obj = CreateScriptInstance(className);
		if (!obj)
		{
			if (outInstance) *outInstance = nullptr;
			return 0;
		}

		uint32_t handle = mono_gchandle_new(obj, pinned ? 1 : 0);
		if (outInstance) *outInstance = obj;
		return handle;
	}

	MonoObject *MonoScriptEngine::GetObjectFromGCHandle(uint32_t gcHandle)
	{
		if (gcHandle == 0)
			return nullptr;

		EnsureCorrectDomain();
		return mono_gchandle_get_target(gcHandle);
	}

	void MonoScriptEngine::DestroyScriptHandle(uint32_t gcHandle)
	{
		if (gcHandle == 0)
			return;

		EnsureCorrectDomain();

		MonoObject *obj = mono_gchandle_get_target(gcHandle);
		if (obj && IsValidMonoObject(obj))
		{
			// Let CallMethod do lookup + exception printing
			CallMethod(obj, "OnDestroy");
		}

		mono_gchandle_free(gcHandle);
	}

	void MonoScriptEngine::DestroyScriptInstance(MonoObject *instance)
	{
		if (!instance)
			return;

		EnsureCorrectDomain();

		if (!IsValidMonoObject(instance))
			return;

		// Legacy: call OnDestroy only. Handle freeing is the caller's responsibility now.
		CallMethod(instance, "OnDestroy");
	}

	MonoMethod *MonoScriptEngine::GetMethod(MonoClass *klass, const std::string &methodName, int paramCount)
	{
		if (!klass) return nullptr;
		return mono_class_get_method_from_name(klass, methodName.c_str(), paramCount);
	}

	void MonoScriptEngine::CallMethod(MonoObject *instance, const std::string &methodName)
	{
		CallMethod(instance, methodName, nullptr, 0);
	}

	void MonoScriptEngine::CallMethod(MonoObject *instance, const std::string &methodName, void **params, int paramCount)
	{
		if (!instance)
			return;

		EnsureCorrectDomain();

		if (!IsValidMonoObject(instance))
		{
			LOG_WARNING("CallMethod: Instance is from unloaded domain, skipping ", methodName);
			return;
		}

		MonoClass *klass = mono_object_get_class(instance);
		MonoMethod *method = GetMethod(klass, methodName, paramCount);
		if (!method)
			return;

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
		// Best-effort legacy helper:
		// If you have a GCHandle, call GetObjectFromGCHandle(handle) instead.
		if (!instancePtr)
			return nullptr;

		MonoObject *obj = (MonoObject *)instancePtr;
		if (!IsValidMonoObject(obj))
			return nullptr;

		return obj;
	}

	// =========================
	// Field access (unchanged)
	// =========================

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

		MonoClass *klass = mono_object_get_class(instance);
		if (!klass)
		{
			LOG_ERROR("SetFieldValue: instance has no class for field: ", fieldName);
			return;
		}

		const char *className = mono_class_get_name(klass);
		const char *classNs = mono_class_get_namespace(klass);

		MonoClass *currentClass = klass;
		MonoClassField *field = nullptr;

		while (currentClass && !field)
		{
			field = mono_class_get_field_from_name(currentClass, fieldName.c_str());
			if (field)
				break;
			currentClass = mono_class_get_parent(currentClass);
		}

		if (field)
		{
			mono_field_set_value(instance, field, value);
			return;
		}

		currentClass = klass;
		MonoProperty *prop = nullptr;

		while (currentClass && !prop)
		{
			prop = mono_class_get_property_from_name(currentClass, fieldName.c_str());
			if (prop)
				break;
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

		void *args[1] = { value };

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
	}

	void *MonoScriptEngine::GetFieldValue(MonoObject *instance, const std::string &fieldName)
	{
		if (!instance)
			return nullptr;

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

		if (!IsValidMonoObject(instance))
		{
			LOG_ERROR("[BindEntityID] instance is from wrong/unloaded domain!");
			return;
		}

		std::uint32_t idCopy = entityID;
		SetFieldValue(instance, "EntityID", &idCopy);
	}

	bool MonoScriptEngine::HotReloadOnPlay(bool preserveManagedState)
	{
		ManagedFieldSnapshot snap{};
		if (preserveManagedState)
			snap = CaptureManagedState();

		if (!BuildGameScripts())
			return false;

		if (!ReloadDomainAndAssembly())
			return false;

		RebindAllScriptComponents();

		if (preserveManagedState)
			RestoreManagedState(snap);

		return true;
	}


	ManagedFieldSnapshot MonoScriptEngine::CaptureManagedState()
	{
		ManagedFieldSnapshot out{};

		Scene *scene = InternalCalls::s_CurrentScene;
		if (!scene)
			return out;

		auto &reg = scene->GetRegistry();

		auto view = reg.view<ScriptComponent>();
		for (auto ent : view)
		{
			auto &sc = view.get<ScriptComponent>(ent);
			if (!sc.ScriptInstance)
				continue;

			MonoObject *instance = static_cast<MonoObject *>(sc.ScriptInstance);
			MonoClass *klass = mono_object_get_class(instance);
			if (!klass)
				continue;

			const uint64_t eid = static_cast<uint64_t>(static_cast<uint32_t>(ent));
			const std::string classKey = sc.ScriptClassName.empty()
				? std::string(mono_class_get_name(klass))
				: sc.ScriptClassName;

			void *iter = nullptr;
			MonoClassField *field = nullptr;

			while ((field = mono_class_get_fields(klass, &iter)) != nullptr)
			{
				if (!ShouldSerializeField(field))
					continue;

				const char *fname = mono_field_get_name(field);
				if (!fname || !fname[0])
					continue;

				std::vector<uint8_t> payload = SerializeFieldValue(instance, field);
				if (payload.empty())
					continue;

				out.data[eid][classKey][fname] = std::move(payload);
			}
		}

		return out;
	}

	void MonoScriptEngine::RestoreManagedState(const ManagedFieldSnapshot &snap)
	{
		Scene *scene = InternalCalls::s_CurrentScene;
		if (!scene)
			return;

		EnsureCorrectDomain();

		auto &reg = scene->GetRegistry();
		auto view = reg.view<ScriptComponent>();

		for (auto ent : view)
		{
			const std::uint32_t eid = static_cast<std::uint32_t>(ent);

			auto itEnt = snap.data.find(eid);
			if (itEnt == snap.data.end())
				continue;

			auto &sc = view.get<ScriptComponent>(ent);

			MonoObject *instance = nullptr;

			// Prefer GCHandle if the component has it
			if constexpr (has_gchandle<ScriptComponent>::value)
			{
				if (sc.GCHandle != 0)
					instance = GetObjectFromGCHandle(sc.GCHandle);
			}

			// Fallback to raw ScriptInstance if present
			if (!instance)
			{
				if constexpr (has_scriptinstance<ScriptComponent>::value)
					instance = static_cast<MonoObject *>(sc.ScriptInstance);
			}

			if (!instance || !IsValidMonoObject(instance))
				continue;

			MonoClass *klass = mono_object_get_class(instance);
			if (!klass)
				continue;

			// Snapshot groups by class name; we only restore the block matching this component's class
			const std::string classKey = sc.ScriptClassName.empty()
				? std::string(mono_class_get_name(klass))
				: sc.ScriptClassName;

			auto itClass = itEnt->second.find(classKey);
			if (itClass == itEnt->second.end())
				continue;

			for (const auto &[fieldName, bytes] : itClass->second)
			{
				if (bytes.empty())
					continue;

				// Resolve field (walk parents like your SetFieldValue does)
				MonoClass *cur = klass;
				MonoClassField *field = nullptr;
				while (cur && !field)
				{
					field = mono_class_get_field_from_name(cur, fieldName.c_str());
					if (field) break;
					cur = mono_class_get_parent(cur);
				}
				if (!field)
					continue;

				const uint8_t tag = bytes[0];
				const uint8_t *p = bytes.data() + 1;
				const size_t n = bytes.size() - 1;

				auto need = [&](size_t k)
					{
						return n >= k;
					};

				switch (static_cast<SnapType>(tag))
				{
				case SnapType::Bool:
				{
					if (!need(sizeof(uint8_t))) break;
					uint8_t v;
					std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::I1:
				{
					if (!need(sizeof(int8_t))) break;
					int8_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::U1:
				{
					if (!need(sizeof(uint8_t))) break;
					uint8_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::I2:
				{
					if (!need(sizeof(int16_t))) break;
					int16_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::U2:
				{
					if (!need(sizeof(uint16_t))) break;
					uint16_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::I4:
				{
					if (!need(sizeof(int32_t))) break;
					int32_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::U4:
				{
					if (!need(sizeof(uint32_t))) break;
					uint32_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::I8:
				{
					if (!need(sizeof(int64_t))) break;
					int64_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::U8:
				{
					if (!need(sizeof(uint64_t))) break;
					uint64_t v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::R4:
				{
					if (!need(sizeof(float))) break;
					float v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::R8:
				{
					if (!need(sizeof(double))) break;
					double v; std::memcpy(&v, p, sizeof(v));
					mono_field_set_value(instance, field, &v);
					break;
				}
				case SnapType::StringUtf8:
				{
					if (!need(sizeof(uint32_t))) break;

					uint32_t len = 0;
					std::memcpy(&len, p, sizeof(len));
					p += sizeof(uint32_t);

					if ((bytes.data() + bytes.size()) < (p + len))
						break;

					MonoString *ms = mono_string_new_len(m_AppDomain, reinterpret_cast<const char *>(p), static_cast<uint32_t>(len));
					mono_field_set_value(instance, field, &ms); // note: pass address for ref types
					break;
				}
				case SnapType::RawValue:
				{
					if (!need(sizeof(uint32_t))) break;

					uint32_t sz = 0;
					std::memcpy(&sz, p, sizeof(sz));
					p += sizeof(uint32_t);

					if ((bytes.data() + bytes.size()) < (p + sz))
						break;

					// Set raw bytes for blittable valuetype
					std::vector<uint8_t> tmp(sz);
					std::memcpy(tmp.data(), p, sz);
					mono_field_set_value(instance, field, tmp.data());
					break;
				}
				default:
					break;
				}
			}
		}
	}

	bool MonoScriptEngine::BuildGameScripts()
	{
#if defined(_DEBUG) || defined(DEBUG)
		const char *cfg = "Debug";
#else
		const char *cfg = "Release";
#endif

		WCHAR exePathW[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, exePathW, MAX_PATH);
		std::filesystem::path exeDir = std::filesystem::path(exePathW).parent_path();

		std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();
		std::filesystem::path scriptProjectPath = projectRoot / "Scripts" / "GameScripts.csproj";
		std::filesystem::path scriptsDir = scriptProjectPath.parent_path();

		if (!std::filesystem::exists(scriptProjectPath))
		{
			LOG_ERROR("[HotReloadOnPlay] Missing csproj: ", scriptProjectPath.string());
			return false;
		}

		// Build command
		std::string cmd = "cmd.exe /C \"dotnet build \"" + scriptProjectPath.string() + "\" --configuration " + cfg + "\"";

		STARTUPINFOA si{};
		PROCESS_INFORMATION pi{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		std::vector<char> cmdBuf(cmd.begin(), cmd.end());
		cmdBuf.push_back('\0');

		const std::string workDir = projectRoot.string();

		BOOL ok = CreateProcessA(
			nullptr,
			cmdBuf.data(),
			nullptr, nullptr,
			FALSE,
			CREATE_NO_WINDOW,
			nullptr,
			workDir.c_str(),
			&si, &pi);

		if (!ok)
		{
			DWORD err = GetLastError();
			LOG_ERROR("[HotReloadOnPlay] Failed to start build. GetLastError=", (int)err);
			LOG_ERROR("[HotReloadOnPlay] Cmd: ", cmd);
			LOG_ERROR("[HotReloadOnPlay] WorkDir: ", workDir);
			return false;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode = 1;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		if (exitCode != 0)
		{
			LOG_ERROR("[HotReloadOnPlay] dotnet build failed, exit code: ", exitCode);
			return false;
		}

		std::vector<std::filesystem::path> candidates = {
			scriptsDir / "obj" / cfg / "GameScripts.dll",
			scriptsDir / "bin" / cfg / "GameScripts.dll"
		};

		std::filesystem::path builtDll;
		for (auto &c : candidates)
		{
			if (std::filesystem::exists(c))
			{
				builtDll = c;
				break;
			}
		}

		if (builtDll.empty())
		{
			// fallback: recursive search in Scripts/bin + Scripts/obj
			for (auto root : { scriptsDir / "bin", scriptsDir / "obj" })
			{
				if (!std::filesystem::exists(root)) continue;
				for (auto &e : std::filesystem::recursive_directory_iterator(root))
				{
					if (e.is_regular_file() && e.path().filename() == "GameScripts.dll")
					{
						builtDll = e.path();
						break;
					}
				}
				if (!builtDll.empty()) break;
			}
		}

		if (builtDll.empty())
		{
			LOG_ERROR("[HotReloadOnPlay] Build succeeded but GameScripts.dll not found under Scripts/bin or Scripts/obj");
			return false;
		}

		// Copy beside exe and load that
		std::filesystem::path outDll = exeDir / "GameScripts.dll";
		std::filesystem::path tmpDll = exeDir / "GameScripts.dll.tmp";

		try
		{
			std::filesystem::copy_file(builtDll, tmpDll, std::filesystem::copy_options::overwrite_existing);
			if (std::filesystem::exists(outDll))
				std::filesystem::remove(outDll);
			std::filesystem::rename(tmpDll, outDll);
		}
		catch (const std::exception &e)
		{
			LOG_ERROR("[HotReloadOnPlay] Failed to swap output DLL: ", e.what());
			return false;
		}

		m_AssemblyPath = outDll.string();

		LOG_INFO("[HotReloadOnPlay] Build OK");
		LOG_INFO("  csproj:    ", scriptProjectPath.string());
		LOG_INFO("  built:     ", builtDll.string());
		LOG_INFO("  output:    ", m_AssemblyPath);

		return true;
	}

	bool MonoScriptEngine::ReloadDomainAndAssembly()
	{
		if (!m_RootDomain)
		{
			LOG_ERROR("[HotReloadOnPlay] Mono not initialized");
			return false;
		}

		// FIXED: Handle both single-domain and multi-domain modes
		if (m_AppDomain)
		{
			// Switch to root domain before unloading
			mono_domain_set(m_RootDomain, false);

			if (m_AppDomain != m_RootDomain)
			{
				// Multi-domain mode: unload the separate app domain
				LOG_INFO("[HotReloadOnPlay] Unloading separate app domain...");
				mono_domain_unload(m_AppDomain);
			}
			else
			{
				// Single-domain mode: can't unload root domain
				// Instead, we rely on assembly hot-swapping if supported by Mono
				LOG_WARNING("[HotReloadOnPlay] Single-domain mode: cannot unload assemblies cleanly.");
				LOG_WARNING("  Recommend switching to multi-domain mode for proper hot reload.");
				LOG_WARNING("  Current reload will attempt to load new assembly but old one remains in memory.");
			}

			m_AppDomain = nullptr;
		}

		// Clear old assembly references
		UnloadAssembly();

		// Create new domain for scripts
		const char *domainName = "GameScriptsDomain";
		MonoDomain *newDomain = mono_domain_create_appdomain(const_cast<char *>(domainName), nullptr);

		if (!newDomain)
		{
			LOG_ERROR("[HotReloadOnPlay] Failed to create new app domain");
			LOG_WARNING("  Falling back to root domain (hot reload will be imperfect)");
			m_AppDomain = m_RootDomain;
		}
		else
		{
			m_AppDomain = newDomain;
			LOG_INFO("[HotReloadOnPlay] Created new app domain: ", (uintptr_t)newDomain);
		}

		// Switch to the app domain
		mono_domain_set(m_AppDomain, false);

		if (m_AssemblyPath.empty())
		{
			LOG_ERROR("[HotReloadOnPlay] m_AssemblyPath is empty; did BuildGameScripts() run?");
			return false;
		}

		// Load the new assembly
		LoadAssembly(m_AssemblyPath);

		if (!m_AppAssembly || !m_AppImage)
		{
			LOG_ERROR("[HotReloadOnPlay] Assembly load failed: ", m_AssemblyPath);
			return false;
		}

		LOG_INFO("[HotReloadOnPlay] Successfully loaded new assembly");
		return true;
	}

	void MonoScriptEngine::RebindAllScriptComponents()
	{
		Scene *scene = Engine::s_ScriptingSceneContext;
		if (!scene)
		{
			LOG_ERROR("[RebindAllScriptComponents] No scene context");
			return;
		}

		if (!m_AppDomain || !m_AppImage)
		{
			LOG_ERROR("[RebindAllScriptComponents] App domain/image not ready");
			return;
		}

		EnsureCorrectDomain();

		auto &reg = scene->GetRegistry();
		auto view = reg.view<ScriptComponent>();

		std::uint32_t rebuilt = 0;
		std::uint32_t failed = 0;

		for (auto ent : view)
		{
			auto &sc = view.get<ScriptComponent>(ent);

			if (sc.ScriptClassName.empty())
				continue;

			// FIXED: Safe cleanup of old handles
			if (sc.GCHandle != 0)
			{
				// In multi-domain mode, the old domain is already unloaded,
				// so we can't validate. Just free the handle number.
				// Mono will handle the case where the domain is gone.
				try
				{
					mono_gchandle_free(sc.GCHandle);
				}
				catch (...)
				{
					LOG_WARNING("[RebindAllScriptComponents] Exception freeing old GCHandle (expected if domain unloaded)");
				}
				sc.GCHandle = 0;
			}

			sc.ScriptInstance = nullptr;
			sc.Started = false;

			// Create new instance in the new domain
			MonoObject *created = nullptr;
			uint32_t handle = CreateScriptInstanceHandle(sc.ScriptClassName, &created, false);

			if (handle == 0 || !created)
			{
				LOG_WARNING("[RebindAllScriptComponents] Failed to create: ", sc.ScriptClassName);
				++failed;
				continue;
			}

			// Bind entity ID to the new instance
			BindEntityID(created, static_cast<std::uint32_t>(ent));

			sc.GCHandle = handle;
			sc.ScriptInstance = created;
			sc.Started = false;

			++rebuilt;
		}

		LOG_INFO("[RebindAllScriptComponents] Complete: rebuilt=", rebuilt, " failed=", failed);
	}

	// Context setters
	void SetScriptingInputSystem(Input *input)
	{
		InternalCalls::SetInputSystem(input);
	}
	void SetScriptingCurrentScene(Scene *scene)
	{
		Engine::s_ScriptingSceneContext = scene;
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
		BindInternalCall("Engine.Scene::Scene_CreateEntity",
			reinterpret_cast<void *>(InternalCalls::Scene_CreateEntity));
		BindInternalCall("Engine.Scene::Scene_DestroyEntity",
			reinterpret_cast<void *>(InternalCalls::Scene_DestroyEntity));
		BindInternalCall("Engine.Scene::Scene_FindEntityByName",
			reinterpret_cast<void *>(InternalCalls::Scene_FindEntityByName));
		BindInternalCall("Engine.Scene::Scene_FindEntitiesByTag",
			reinterpret_cast<void *>(InternalCalls::Scene_FindEntitiesByTag));
		BindInternalCall("Engine.Scene::Entity_AddScript",
			reinterpret_cast<void *>(InternalCalls::Entity_AddScript));
		BindInternalCall("Engine.Scene::Entity_AddRigidBody",
			reinterpret_cast<void *>(InternalCalls::Entity_AddRigidBody));
		BindInternalCall("Engine.Scene::Entity_AddCamera",
			reinterpret_cast<void *>(InternalCalls::Entity_AddCamera));
		BindInternalCall("Engine.Scene::Entity_AddAudio",
			reinterpret_cast<void *>(InternalCalls::Entity_AddAudio));
		BindInternalCall("Engine.Scene::Entity_AddMeshRenderer",
			reinterpret_cast<void *>(InternalCalls::Entity_AddMeshRenderer));
		BindInternalCall("Engine.Scene::Entity_AddTag",
			reinterpret_cast<void *>(InternalCalls::Entity_AddTag));

		// =====================================================================
		// Transform
		// =====================================================================
		BindInternalCall("Engine.Transform::Transform_GetParent",
			reinterpret_cast<void *>(InternalCalls::Transform_GetParent));
		BindInternalCall("Engine.Transform::Transform_GetPosition",
			reinterpret_cast<void *>(InternalCalls::Transform_GetPosition));
		BindInternalCall("Engine.Transform::Transform_SetPosition",
			reinterpret_cast<void *>(InternalCalls::Transform_SetPosition));
		BindInternalCall("Engine.Transform::Transform_GetRotation",
			reinterpret_cast<void *>(InternalCalls::Transform_GetRotation));
		BindInternalCall("Engine.Transform::Transform_SetRotation",
			reinterpret_cast<void *>(InternalCalls::Transform_SetRotation));
		BindInternalCall("Engine.Transform::Transform_GetScale",
			reinterpret_cast<void *>(InternalCalls::Transform_GetScale));
		BindInternalCall("Engine.Transform::Transform_SetScale",
			reinterpret_cast<void *>(InternalCalls::Transform_SetScale));

		// =====================================================================
		// Logging
		// =====================================================================
		BindInternalCall("Engine.Logger::LogMessage",
			reinterpret_cast<void *>(InternalCalls::LogMessage));
		BindInternalCall("Engine.Logger::LogError",
			reinterpret_cast<void *>(InternalCalls::LogError));
		BindInternalCall("Engine.Logger::LogWarning",
			reinterpret_cast<void *>(InternalCalls::LogWarning));

		// =====================================================================
		// Entity helpers
		// =====================================================================
		BindInternalCall("Engine.Entity::GetEntityID_Native",
			reinterpret_cast<void *>(InternalCalls::Entity_GetEntityID));
		BindInternalCall("Engine.Entity::HasComponent_Native",
			reinterpret_cast<void *>(InternalCalls::Entity_HasComponent));

		// =====================================================================
		// Prefab
		// =====================================================================
		BindInternalCall("Engine.Prefab::Prefab_Instantiate",
			reinterpret_cast<void *>(InternalCalls::Prefab_Instantiate));
		BindInternalCall("Engine.Prefab::Prefab_InstantiateScene",
			reinterpret_cast<void *>(InternalCalls::Prefab_InstantiateScene));
		BindInternalCall("Engine.Prefab::Prefab_InstantiateWithTransform",
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
		BindInternalCall("Engine.Input::Input_GetMouseDelta",
			reinterpret_cast<void *>(InternalCalls::Input_GetMouseDelta));

		// =====================================================================
		// Rigidbody
		// =====================================================================
		BindInternalCall("Engine.Rigidbody::Rigidbody_GetVelocity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetVelocity));
		BindInternalCall("Engine.Rigidbody::Rigidbody_SetVelocity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetVelocity));
		BindInternalCall("Engine.Rigidbody::Rigidbody_AddVelocity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_AddVelocity));

		BindInternalCall("Engine.Rigidbody::Rigidbody_GetMass",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetMass));
		BindInternalCall("Engine.Rigidbody::Rigidbody_SetMass",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetMass));
		BindInternalCall("Engine.Rigidbody::Rigidbody_GetIsKinematic",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetIsKinematic));
		BindInternalCall("Engine.Rigidbody::Rigidbody_SetIsKinematic",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetIsKinematic));
		BindInternalCall("Engine.Rigidbody::Rigidbody_GetUseGravity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetUseGravity));
		BindInternalCall("Engine.Rigidbody::Rigidbody_SetUseGravity",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_SetUseGravity));

		BindInternalCall("Engine.Rigidbody::Rigidbody_GetSpeed",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_GetSpeed));
		BindInternalCall("Engine.Rigidbody::Rigidbody_IsMoving",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_IsMoving));
		BindInternalCall("Engine.Rigidbody::Rigidbody_IsStatic",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_IsStatic));
		BindInternalCall("Engine.Rigidbody::Rigidbody_AddForce",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_AddForce));
		BindInternalCall("Engine.Rigidbody::Rigidbody_Stop",
			reinterpret_cast<void *>(InternalCalls::Rigidbody_Stop));

		// =====================================================================
		// Physics
		// =====================================================================
		BindInternalCall("Engine.Physics::Physics_EnableCollisionEvents",
			reinterpret_cast<void *>(InternalCalls::Physics_EnableCollisionEvents));
		BindInternalCall("Engine.Physics::Physics_BeginCollisionFrame",
			reinterpret_cast<void *>(InternalCalls::Physics_BeginCollisionFrame));
		BindInternalCall("Engine.Physics::Physics_GetCollisionCount",
			reinterpret_cast<void *>(InternalCalls::Physics_GetCollisionCount));
		BindInternalCall("Engine.Physics::Physics_GetCollisionPair",
			reinterpret_cast<void *>(InternalCalls::Physics_GetCollisionPair));

		// =====================================================================
		// Tag
		// =====================================================================
		BindInternalCall("Engine.Tag::Tag_GetTag",
			reinterpret_cast<void *>(InternalCalls::Tag_GetTag));
		BindInternalCall("Engine.Tag::Tag_SetTag",
			reinterpret_cast<void *>(InternalCalls::Tag_SetTag));

		// =====================================================================
		// Camera
		// =====================================================================
		BindInternalCall("Engine.Camera::Camera_GetEnabled",
			reinterpret_cast<void *>(InternalCalls::Camera_GetEnabled));
		BindInternalCall("Engine.Camera::Camera_SetEnabled",
			reinterpret_cast<void *>(InternalCalls::Camera_SetEnabled));
		BindInternalCall("Engine.Camera::Camera_GetPrimary",
			reinterpret_cast<void *>(InternalCalls::Camera_GetPrimary));
		BindInternalCall("Engine.Camera::Camera_SetPrimary",
			reinterpret_cast<void *>(InternalCalls::Camera_SetPrimary));
		BindInternalCall("Engine.Camera::Camera_GetFOV",
			reinterpret_cast<void *>(InternalCalls::Camera_GetFOV));
		BindInternalCall("Engine.Camera::Camera_SetFOV",
			reinterpret_cast<void *>(InternalCalls::Camera_SetFOV));
		BindInternalCall("Engine.Camera::Camera_GetNear",
			reinterpret_cast<void *>(InternalCalls::Camera_GetNear));
		BindInternalCall("Engine.Camera::Camera_SetNear",
			reinterpret_cast<void *>(InternalCalls::Camera_SetNear));
		BindInternalCall("Engine.Camera::Camera_GetFar",
			reinterpret_cast<void *>(InternalCalls::Camera_GetFar));
		BindInternalCall("Engine.Camera::Camera_SetFar",
			reinterpret_cast<void *>(InternalCalls::Camera_SetFar));
		BindInternalCall("Engine.Camera::Camera_GetTarget",
			reinterpret_cast<void *>(InternalCalls::Camera_GetTarget));
		BindInternalCall("Engine.Camera::Camera_SetTarget",
			reinterpret_cast<void *>(InternalCalls::Camera_SetTarget));

		// =====================================================================
		// MeshRenderer
		// =====================================================================
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_GetVisible",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetVisible));
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_SetVisible",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetVisible));
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_GetShadowCast",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetShadowCast));
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_SetShadowCast",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetShadowCast));
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_GetShadowReceive",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetShadowReceive));
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_SetShadowReceive",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetShadowReceive));
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_GetGlobalIlluminate",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_GetGlobalIlluminate));
		BindInternalCall("Engine.MeshRenderer::MeshRenderer_SetGlobalIlluminate",
			reinterpret_cast<void *>(InternalCalls::MeshRenderer_SetGlobalIlluminate));

		// =====================================================================
		// AudioComponent
		// =====================================================================
		BindInternalCall("Engine.Audio::Audio_Play",
			reinterpret_cast<void *>(InternalCalls::Audio_Play));
		BindInternalCall("Engine.Audio::Audio_Stop",
			reinterpret_cast<void *>(InternalCalls::Audio_Stop));
		BindInternalCall("Engine.Audio::Audio_Pause",
			reinterpret_cast<void *>(InternalCalls::Audio_Pause));

		BindInternalCall("Engine.Audio::Audio_GetVolume",
			reinterpret_cast<void *>(InternalCalls::Audio_GetVolume));
		BindInternalCall("Engine.Audio::Audio_SetVolume",
			reinterpret_cast<void *>(InternalCalls::Audio_SetVolume));

		BindInternalCall("Engine.Audio::Audio_GetPitch",
			reinterpret_cast<void *>(InternalCalls::Audio_GetPitch));
		BindInternalCall("Engine.Audio::Audio_SetPitch",
			reinterpret_cast<void *>(InternalCalls::Audio_SetPitch));

		BindInternalCall("Engine.Audio::Audio_GetLoop",
			reinterpret_cast<void *>(InternalCalls::Audio_GetLoop));
		BindInternalCall("Engine.Audio::Audio_SetLoop",
			reinterpret_cast<void *>(InternalCalls::Audio_SetLoop));

		BindInternalCall("Engine.Audio::Audio_GetMute",
			reinterpret_cast<void *>(InternalCalls::Audio_GetMute));
		BindInternalCall("Engine.Audio::Audio_SetMute",
			reinterpret_cast<void *>(InternalCalls::Audio_SetMute));

		BindInternalCall("Engine.Audio::Audio_GetIs3D",
			reinterpret_cast<void *>(InternalCalls::Audio_GetIs3D));
		BindInternalCall("Engine.Audio::Audio_SetIs3D",
			reinterpret_cast<void *>(InternalCalls::Audio_SetIs3D));

		BindInternalCall("Engine.Audio::Audio_SetFile",
			reinterpret_cast<void *>(InternalCalls::Audio_SetFile));

		// AudioComponent extensions
		BindInternalCall("Engine.Audio::Audio_GetMinDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_GetMinDistance));
		BindInternalCall("Engine.Audio::Audio_SetMinDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_SetMinDistance));
		BindInternalCall("Engine.Audio::Audio_GetMaxDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_GetMaxDistance));
		BindInternalCall("Engine.Audio::Audio_SetMaxDistance",
			reinterpret_cast<void *>(InternalCalls::Audio_SetMaxDistance));
		BindInternalCall("Engine.Audio::Audio_GetRolloffMode",
			reinterpret_cast<void *>(InternalCalls::Audio_GetRolloffMode));
		BindInternalCall("Engine.Audio::Audio_SetRolloffMode",
			reinterpret_cast<void *>(InternalCalls::Audio_SetRolloffMode));
		BindInternalCall("Engine.Audio::Audio_GetDopplerLevel",
			reinterpret_cast<void *>(InternalCalls::Audio_GetDopplerLevel));
		BindInternalCall("Engine.Audio::Audio_SetDopplerLevel",
			reinterpret_cast<void *>(InternalCalls::Audio_SetDopplerLevel));
		BindInternalCall("Engine.Audio::Audio_GetPan2D",
			reinterpret_cast<void *>(InternalCalls::Audio_GetPan2D));
		BindInternalCall("Engine.Audio::Audio_SetPan2D",
			reinterpret_cast<void *>(InternalCalls::Audio_SetPan2D));
		BindInternalCall("Engine.Audio::Audio_GetReverbMix",
			reinterpret_cast<void *>(InternalCalls::Audio_GetReverbMix));
		BindInternalCall("Engine.Audio::Audio_SetReverbMix",
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
		BindInternalCall("Engine.Event::Event_Publish",
			reinterpret_cast<void *>(InternalCalls::Event_Publish));

		// =====================================================================
		// Quaternion
		// =====================================================================
		BindInternalCall("Engine.QuatNative::Quat_FromAxisAngle",
			reinterpret_cast<void *>(InternalCalls::Quat_FromAxisAngle));
		BindInternalCall("Engine.QuatNative::Quat_GetForward",
			reinterpret_cast<void *>(InternalCalls::Quat_GetForward));
		BindInternalCall("Engine.QuatNative::Quat_GetRight",
			reinterpret_cast<void *>(InternalCalls::Quat_GetRight));
		BindInternalCall("Engine.QuatNative::Quat_GetUp",
			reinterpret_cast<void *>(InternalCalls::Quat_GetUp));
		BindInternalCall("Engine.QuatNative::Quat_RotateVector",
			reinterpret_cast<void *>(InternalCalls::Quat_RotateVector));
		BindInternalCall("Engine.QuatNative::Quat_Multiply",
			reinterpret_cast<void *>(InternalCalls::Quat_Multiply));
		BindInternalCall("Engine.QuatNative::Quat_Slerp",
			reinterpret_cast<void *>(InternalCalls::Quat_Slerp));
		BindInternalCall("Engine.QuatNative::Quat_Inverse",
			reinterpret_cast<void *>(InternalCalls::Quat_Inverse));
		BindInternalCall("Engine.QuatNative::Quat_ToEuler",
			reinterpret_cast<void *>(InternalCalls::Quat_ToEuler));
		BindInternalCall("Engine.QuatNative::Quat_FromEuler",
			reinterpret_cast<void *>(InternalCalls::Quat_FromEuler));
		BindInternalCall("Engine.QuatNative::Quat_Normalize",
			reinterpret_cast<void *>(InternalCalls::Quat_Normalize));
		BindInternalCall("Engine.QuatNative::Quat_Length",
			reinterpret_cast<void *>(InternalCalls::Quat_Length));
		BindInternalCall("Engine.QuatNative::Quat_Dot",
			reinterpret_cast<void *>(InternalCalls::Quat_Dot));

		LOG_INFO("Internal calls registered");
	}
} // namespace Engine

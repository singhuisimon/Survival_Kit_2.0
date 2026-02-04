/**************************************************************************
 * @file
 * InternalCalls.cpp
 * @author
 * Varying amounts by team
 * @date
 * 2026/01/06 (YYYY/MM/DD)
 * @brief
 * This source file defines the native InternalCalls bridge used by the Mono
 * scripting layer to invoke engine functionality (ECS, input, audio, physics,
 * prefabs, events, math helpers, etc.).
 *
 * Each function in this file is registered with Mono and can be invoked from
 * managed C# code. Functions generally assume the ScriptSystem has already
 * provided valid engine context pointers via SetCurrentScene / SetInputSystem /
 * SetAudioManager.
***************************************************************************/

#include "InternalCalls.h"
#include "MonoScriptEngine.h"

#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"

#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include "../ECS/Components.h"

#include "../Core/input.h"
#include "../Audio/AudioManager.h"
#include "../Event/EventSystem.h"

#include "../Serialization/PrefabSerializer.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Prefab/PrefabRegistry.h"

#include "../Physics/PhysicsAPI.h"

#include "../Physics/CollisionSystem2D.h"

// Mono
#include <mono/jit/jit.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>

// GLM extras (translate/scale, quat ops)
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/matrix_decompose.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#ifdef _WIN32
#include <direct.h>   // For _mkdir on Windows
#else
#include <sys/stat.h> // For mkdir on Unix/Linux
#endif


namespace Engine
{
	namespace
	{
		static inline bool IsFinite(float v)
		{
			return std::isfinite(v) != 0;
		}

		static inline bool IsFiniteVec3(const glm::vec3 &v)
		{
			return IsFinite(v.x) && IsFinite(v.y) && IsFinite(v.z);
		}

		static inline bool IsFiniteQuat(const glm::quat &q)
		{
			return IsFinite(q.x) && IsFinite(q.y) && IsFinite(q.z) && IsFinite(q.w);
		}

		static inline glm::quat SafeNormalizeQuat(const glm::quat &q)
		{
			// Reject NaNs/Infs early
			if (!IsFiniteQuat(q))
				return glm::quat(1.f, 0.f, 0.f, 0.f); // identity (w,x,y,z) ctor in glm

			const float len2 = glm::length2(q);
			// If too small, return identity to avoid divide-by-zero / blow-ups
			if (!(len2 > 1e-12f))
				return glm::quat(1.f, 0.f, 0.f, 0.f);

			return glm::normalize(q);
		}

		static inline glm::vec3 SafeVec3OrZero(const glm::vec3 &v)
		{
			return IsFiniteVec3(v) ? v : glm::vec3(0.f);
		}

		static inline glm::vec3 SafeScale(const glm::vec3 &s)
		{
			// If you want to preserve negative scale (mirroring), keep it.
			// Only clean NaN/Inf. Optionally clamp away from 0 to avoid singular matrices.
			glm::vec3 out = s;
			if (!IsFinite(out.x)) out.x = 1.f;
			if (!IsFinite(out.y)) out.y = 1.f;
			if (!IsFinite(out.z)) out.z = 1.f;

			// Optional: avoid exact zero scale if your TRS->matrix code divides by scale
			// const float eps = 1e-6f;
			// if (std::abs(out.x) < eps) out.x = (out.x < 0.f ? -eps : eps);
			// if (std::abs(out.y) < eps) out.y = (out.y < 0.f ? -eps : eps);
			// if (std::abs(out.z) < eps) out.z = (out.z < 0.f ? -eps : eps);

			return out;
		}
	}

	namespace InternalCalls
	{
		/**************************************************************************
		 * @brief
		 * Sets the scene context used by InternalCalls.
		 * @param scene
		 * Pointer to the active scene used for subsequent internal calls.
		***************************************************************************/
		void SetCurrentScene(Scene *scene)
		{
			s_CurrentScene = scene;
		}

		/**************************************************************************
		 * @brief
		 * Sets the input system context used by InternalCalls.
		 * @param input
		 * Pointer to the engine input system used for subsequent internal calls.
		***************************************************************************/
		void SetInputSystem(Input *input)
		{
			s_InputSystem = input;
		}

		/**************************************************************************
		 * @brief
		 * Sets the audio manager context used by InternalCalls.
		 * @param audioManager
		 * Pointer to the engine audio manager used for subsequent internal calls.
		***************************************************************************/
		void SetAudioManager(AudioManager *audioManager)
		{
			s_AudioManager = audioManager;
		}

		/**************************************************************************
		 * @brief
		 * Gets the audio manager context used by InternalCalls.
		 * @return
		 * Pointer to the engine audio manager used for subsequent internal calls.
		***************************************************************************/
		AudioManager *GetAudioManager()
		{
			return s_AudioManager;
		}

		// =====================================================================
		// Helpers
		// =====================================================================
		static inline Entity GetEntityOrNull(uint64_t id)
		{
			if (!s_CurrentScene)
				return {};

			Entity e = s_CurrentScene->GetEntity(static_cast<entt::entity>(id));
			return e;
		}

		/**************************************************************************
		 * @brief
		 * Initializes and binds the ScriptComponent instance for an entity.
		 * @param entity
		 * Entity handle in the current scene.
		***************************************************************************/
		static void InitializeScriptComponentForEntity(Entity entity)
		{
			if (!entity)
				return;

			if (!entity.HasComponent<ScriptComponent>())
				return;

			auto &sc = entity.GetComponent<ScriptComponent>();
			if (sc.ScriptClassName.empty())
				return;

			auto &se = MonoScriptEngine::GetInstance();
			uint64_t eid = static_cast<uint32_t>(entity);

			// If prefab clone already contains a handle, just rebind to the resolved object.
			if (sc.GCHandle != 0)
			{
				MonoObject *obj = se.GetObjectFromGCHandle(sc.GCHandle);
				sc.ScriptInstance = obj; // keep cache synced

				if (!obj)
				{
					// handle is stale/invalid -> clear and recreate next
					se.DestroyScriptHandle(sc.GCHandle);
					sc.GCHandle = 0;
					sc.ScriptInstance = nullptr;
				}
				else
				{
					se.BindEntityID(obj, static_cast<std::uint32_t>(eid));
					sc.Started = false;
					return;
				}
			}

			// Transitional: if old data has ScriptInstance but no handle, adopt it.
			// (Optional, but helps when cloning prefabs that copied ScriptInstance pointer.)
			if (sc.ScriptInstance && sc.GCHandle == 0)
			{
				MonoObject *legacy = static_cast<MonoObject *>(sc.ScriptInstance);
				sc.GCHandle = mono_gchandle_new(legacy, /*pinned*/ false);
				MonoObject *obj = se.GetObjectFromGCHandle(sc.GCHandle);
				sc.ScriptInstance = obj;

				if (obj)
				{
					se.BindEntityID(obj, static_cast<std::uint32_t>(eid));
					sc.Started = false;
					return;
				}

				// If legacy pointer was bad, clean it up and fall through to recreate
				se.DestroyScriptHandle(sc.GCHandle);
				sc.GCHandle = 0;
				sc.ScriptInstance = nullptr;
			}

			// Create a NEW managed instance and store handle (handle-first)
			MonoObject *instance = nullptr;
			uint32_t handle = se.CreateScriptInstanceHandle(sc.ScriptClassName, &instance, /*pinned*/ false);
			if (handle == 0 || !instance)
			{
				LOG_ERROR("[InternalCall] Prefab script init failed for class '", sc.ScriptClassName, "' on entity ", eid);
				return;
			}

			se.BindEntityID(instance, static_cast<std::uint32_t>(eid));
			sc.GCHandle = handle;
			sc.ScriptInstance = instance; // cache only
			sc.Started = false;
		}

		// =====================================================================
		// Component presence helpers (registered as internal calls)
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Internal engine <-> scripting bridge call.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool EntityHasCamera(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			return e && e.HasComponent<CameraComponent>();
		}

		/**************************************************************************
		 * @brief
		 * Internal engine <-> scripting bridge call.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool EntityHasRigidBody(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			return e && e.HasComponent<RigidbodyComponent>();
		}

		// =====================================================================
		// Scene / Entity lifecycle
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Creates a new entity in the current scene.
		 * @param nameStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Entity identifier (0 if not found / invalid).
		***************************************************************************/
		uint64_t Scene_CreateEntity(MonoString *nameStr)
		{
			if (!s_CurrentScene)
				return 0;

			std::string name = "Entity";
			if (nameStr)
			{
				char *c = mono_string_to_utf8(nameStr);
				if (c)
				{
					name = c;
					mono_free(c);
				}
			}

			Entity e = s_CurrentScene->CreateEntity(name.c_str());
			return static_cast<uint32_t>(e);
		}

		/**************************************************************************
		 * @brief
		 * Destroys an entity in the current scene.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Scene_DestroyEntity(uint64_t entityID)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCall] Scene_DestroyEntity: current scene is null");
				return;
			}

			Entity e = s_CurrentScene->GetEntity(static_cast<entt::entity>(entityID));
			if (!e)
				return;

			s_CurrentScene->DestroyEntity(e);
		}

		/**************************************************************************
		 * @brief
		 * Attaches a managed script class to the specified entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param classFullNameStr
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void Entity_AddScript(uint64_t entityID, MonoString *classFullNameStr)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: current scene is null");
				return;
			}
			if (!classFullNameStr)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: class name is null");
				return;
			}

			Entity e = s_CurrentScene->GetEntity(static_cast<entt::entity>(entityID));
			if (!e)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: entity ID=", entityID, " is invalid");
				return;
			}

			char *c = mono_string_to_utf8(classFullNameStr);
			std::string klass = c ? c : "";
			if (c) mono_free(c);

			if (klass.empty())
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: empty class name");
				return;
			}

			auto &se = MonoScriptEngine::GetInstance();

			// If entity already has a script, destroy the old handle cleanly
			if (e.HasComponent<ScriptComponent>())
			{
				auto &scOld = e.GetComponent<ScriptComponent>();
				if (scOld.GCHandle != 0)
				{
					se.DestroyScriptHandle(scOld.GCHandle);
					scOld.GCHandle = 0;
				}
				scOld.ScriptInstance = nullptr;
				scOld.Started = false;
			}

			MonoObject *instance = nullptr;
			uint32_t handle = se.CreateScriptInstanceHandle(klass, &instance, false);
			if (handle == 0 || !instance)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: failed to create instance of ", klass);
				return;
			}

			se.BindEntityID(instance, static_cast<std::uint32_t>(entityID));

			// Attach/update component with handle as source of truth
			if (e.HasComponent<ScriptComponent>())
			{
				auto &sc = e.GetComponent<ScriptComponent>();
				sc.ScriptClassName = klass;
				sc.GCHandle = handle;
				sc.ScriptInstance = instance; // cache
				sc.Started = false;
			}
			else
			{
				auto &sc = e.AddComponent<ScriptComponent>();
				sc.ScriptClassName = klass;
				sc.GCHandle = handle;
				sc.ScriptInstance = instance; // cache
				sc.Started = false;
			}
		}

		/**************************************************************************
		 * @brief
		 * Finds an entity by name in the current scene.
		 * @param nameString
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Entity identifier (0 if not found / invalid).
		***************************************************************************/
		uint64_t Scene_FindEntityByName(MonoString *nameString)
		{
			if (!s_CurrentScene || !nameString)
				return 0;

			char *nameStr = mono_string_to_utf8(nameString);
			if (!nameStr)
				return 0;

			std::string name(nameStr);
			mono_free(nameStr);

			if (name.empty())
				return 0;

			Entity entity = s_CurrentScene->FindEntityByName(name);
			return static_cast<uint32_t>(entity);
		}

		uint64_t Scene_FindEntityByTag(MonoString* tagString) {
			if (!s_CurrentScene || !tagString)
				return 0;

			char* tagStr = mono_string_to_utf8(tagString);
			if (!tagStr)
				return 0;

			std::string tag(tagStr);
			mono_free(tagStr);

			if (tag.empty())
				return 0;

			Entity entity = s_CurrentScene->FindEntityByTag(tag);
			return static_cast<uint32_t>(entity);
		}
		// =====================================================================
		// Logging
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Writes a message to the engine logging system.
		 * @param message
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void LogMessage(MonoString *message)
		{
#ifdef DEBUG
			if (!message) return;
			char *cStr = mono_string_to_utf8(message);
			LOG_INFO("[C#] ", cStr ? cStr : "<null>");
			if (cStr) mono_free(cStr);
#else
			(void *)message;
#endif
		}

		/**************************************************************************
		 * @brief
		 * Writes a message to the engine logging system.
		 * @param message
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void LogError(MonoString *message)
		{
#ifdef DEBUG
			if (!message) return;
			char *cStr = mono_string_to_utf8(message);
			LOG_ERROR("[C#] ", cStr ? cStr : "<null>");
			if (cStr) mono_free(cStr);
#else
			(void *)message;
#endif	
		}

		/**************************************************************************
		 * @brief
		 * Writes a message to the engine logging system.
		 * @param message
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void LogWarning(MonoString *message)
		{
#ifdef DEBUG
			if (!message) return;
			char *cStr = mono_string_to_utf8(message);
			LOG_WARNING("[C#] ", cStr ? cStr : "<null>");
			if (cStr) mono_free(cStr);
#else
			(void *)message;
#endif	
		}

		// =====================================================================
		// Entity utilities
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Returns the native entity ID associated with a managed Entity wrapper.
		 * @param entityObj
		 * Managed object provided by the scripting runtime (MonoObject*).
		 * @return
		 * Unsigned integer result.
		***************************************************************************/
		uint64_t Entity_GetEntityID(MonoObject *entityObj)
		{
			if (!entityObj)
				return 0;

			MonoClass *entityClass = mono_object_get_class(entityObj);
			if (!entityClass)
				return 0;

			MonoMethod *getIdMethod = mono_class_get_method_from_name(entityClass, "get_EntityID", 0);
			if (!getIdMethod)
				return 0;

			MonoObject *exception = nullptr;
			MonoObject *result = mono_runtime_invoke(getIdMethod, entityObj, nullptr, &exception);
			if (exception || !result)
				return 0;

			void *unboxed = mono_object_unbox(result);
			if (!unboxed)
				return 0;

			return *reinterpret_cast<uint64_t *>(unboxed);
		}

		/**************************************************************************
		 * @brief
		 * Checks whether an entity has a component identified by its short type name.
		 * @param e
		 * Entity handle in the current scene.
		 * @param shortName
		 * Short type name string.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		static bool HasComponentByShortName(Entity e, const std::string &shortName)
		{
			if (!e) return false;

			if (shortName == "TransformComponent")    return e.HasComponent<TransformComponent>();
			if (shortName == "RigidbodyComponent")    return e.HasComponent<RigidbodyComponent>();
			if (shortName == "TagComponent")          return e.HasComponent<TagComponent>();
			if (shortName == "CameraComponent")       return e.HasComponent<CameraComponent>();
			if (shortName == "AudioComponent")        return e.HasComponent<AudioComponent>();
			if (shortName == "MeshRendererComponent") return e.HasComponent<MeshRendererComponent>();
			if (shortName == "ScriptComponent")       return e.HasComponent<ScriptComponent>();

			return false;
		}

		/**************************************************************************
		 * @brief
		 * Checks whether the entity has a component of the specified managed type.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param componentType
		 * Managed reflection type used to resolve a native component type.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Entity_HasComponent(uint64_t entityID, MonoReflectionType *componentType)
		{
			if (!s_CurrentScene || !componentType)
				return false;

			Entity e = GetEntityOrNull(entityID);
			if (!e)
				return false;

			MonoType *monoType = mono_reflection_type_get_type(componentType);
			if (!monoType)
				return false;

			char *typeNameC = mono_type_get_name(monoType);
			if (!typeNameC)
				return false;

			std::string typeName(typeNameC);
			mono_free(typeNameC);

			// Reduce to short name (after last '.')
			std::string shortName = typeName;
			const size_t dot = shortName.find_last_of('.');
			if (dot != std::string::npos && dot + 1 < shortName.size())
				shortName = shortName.substr(dot + 1);

			return HasComponentByShortName(e, shortName);
		}

		// =====================================================================
		// Transform
		// =====================================================================
		void Transform_GetPosition(uint64_t entityID, glm::vec3 *outPosition)
		{
			if (!outPosition) return;
			*outPosition = glm::vec3(0.f); // deterministic default

			// RequireMainThread();

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			const auto &t = e.GetComponent<TransformComponent>();
			*outPosition = SafeVec3OrZero(t.Position);
		}

		void Transform_SetPosition(uint64_t entityID, glm::vec3 *position)
		{
			if (!position) return;

			// RequireMainThread();

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			glm::vec3 p = SafeVec3OrZero(*position);

			auto &t = e.GetComponent<TransformComponent>();
			t.Position = p;
			t.IsDirty = true;
		}

		// ---- Rotation -----------------------------------------------------------

		void Transform_GetRotation(uint64_t entityID, glm::quat *outRotation)
		{
			if (!outRotation) return;
			*outRotation = glm::quat(1.f, 0.f, 0.f, 0.f); // identity default

			// RequireMainThread();

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			const auto &t = e.GetComponent<TransformComponent>();
			// Optionally normalize on get to keep scripts safe even if native code set bad values
			*outRotation = SafeNormalizeQuat(t.Rotation);
		}

		void Transform_SetRotation(uint64_t entityID, glm::quat *rotation)
		{
			if (!rotation) return;

			// RequireMainThread();

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			glm::quat r = SafeNormalizeQuat(*rotation);

			auto &t = e.GetComponent<TransformComponent>();
			t.Rotation = r;
			t.IsDirty = true;
		}

		// ---- Scale --------------------------------------------------------------

		void Transform_GetScale(uint64_t entityID, glm::vec3 *outScale)
		{
			if (!outScale) return;
			*outScale = glm::vec3(1.f); // deterministic default

			// RequireMainThread();

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			const auto &t = e.GetComponent<TransformComponent>();
			*outScale = SafeScale(t.Scale);
		}

		void Transform_SetScale(uint64_t entityID, glm::vec3 *scale)
		{
			if (!scale) return;

			// RequireMainThread();

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			glm::vec3 s = SafeScale(*scale);

			auto &t = e.GetComponent<TransformComponent>();
			t.Scale = s;
			t.IsDirty = true;
		}

		/**************************************************************************
		 * @brief
		 * Retrieves a transform property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested integer value.
		***************************************************************************/
		int Transform_GetParent(uint64_t entityID)
		{
			if (!s_CurrentScene)
				return 0;

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity handle = static_cast<entt::entity>(entityID);

			if (!registry.valid(handle) || !registry.all_of<TransformComponent>(handle))
				return 0;

			auto &transform = registry.get<TransformComponent>(handle);
			return transform.GetParentEntity();
		}

		// =====================================================================
		// Input
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param keyCode
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Input_IsKeyPressed(int keyCode)
		{
			if (!s_InputSystem) return false;
			return s_InputSystem->IsKeyPressed(keyCode);
		}

		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param keyCode
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Input_IsKeyReleased(int keyCode)
		{
			if (!s_InputSystem) return false;
			return s_InputSystem->IsKeyJustReleased(keyCode);
		}

		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param button
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Input_IsMouseButtonPressed(int button)
		{
			if (!s_InputSystem) return false;
			return s_InputSystem->IsMouseButtonPressed(button);
		}

		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param outPosition
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Input_GetMousePosition(glm::vec2 *outPosition)
		{
			if (!outPosition) return;
			if (!s_InputSystem)
			{
				*outPosition = { 0.0f, 0.0f }; return;
			}
			*outPosition = s_InputSystem->GetMousePosition();
		}

		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param outX
		 * Output parameter that receives the requested value.
		 * @param outY
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Input_GetMouseDelta(float *outX, float *outY)
		{
			if (outX) *outX = 0.0f;
			if (outY) *outY = 0.0f;
			if (!s_InputSystem) return;

			glm::vec2 d = s_InputSystem->GetMouseDelta();
			if (outX) *outX = d.x;
			if (outY) *outY = d.y;
		}

		/**************************************************************************
		 * @brief
		 * Sets the visibility of the system cursor.
		 * @param visible
		 * Input parameter.
		 **************************************************************************/
		void Input_SetCursorVisible(bool visible)
		{
			if (!s_InputSystem) return;
			s_InputSystem->SetCursorVisible(visible);
		}

		/**************************************************************************
		 * @brief
		 * Gets the visibility of the system cursor.
		 * @return
		 * True if the cursor is visible; otherwise false.
		 **************************************************************************/
		bool Input_GetCursorVisible()
		{
			if (!s_InputSystem) return true;
			return s_InputSystem->IsCursorVisible();
		}

		// =====================================================================
		// Prefabs
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Instantiates a prefab into the current scene.
		 * @param prefabPathStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Unsigned integer result.
		***************************************************************************/
		uint64_t Prefab_Instantiate(MonoString *prefabPathStr)
		{
			if (!s_CurrentScene || !prefabPathStr)
				return 0;

			char *c = mono_string_to_utf8(prefabPathStr);
			std::string prefabPath = c ? c : "";
			if (c) mono_free(c);

			if (prefabPath.empty())
				return 0;

			std::string prefabFullPath = getAssetFilePath(prefabPath);

			//auto prefab = PrefabSerializer::LoadPrefabFromFile(prefabFullPath);
			/*if (!prefab)
				return 0;*/

				//PrefabRegistry::Get().RegisterPrefab(prefab);

			Entity entity = PrefabInstantiator::InstantiatePrefabFromFile(
				s_CurrentScene,
				prefabFullPath,
				Entity{}  // No parent
			);
			if (!entity)
				return 0;

			InitializeScriptComponentForEntity(entity);
			entity.GetComponent<TransformComponent>().IsDirty = true;
			return static_cast<uint64_t>(static_cast<uint32_t>(entity));
		}

		/**************************************************************************
		 * @brief
		 * Instantiates a prefab into the current scene.
		 * @param prefabPathStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Unsigned integer result.
		***************************************************************************/
		uint64_t Prefab_InstantiateScene(MonoString *prefabPathStr)
		{
			if (!s_CurrentScene || !prefabPathStr)
				return 0;

			char *c = mono_string_to_utf8(prefabPathStr);
			std::string prefabPath = c ? c : "";
			if (c) mono_free(c);

			if (prefabPath.empty())
				return 0;

			std::string prefabFullPath = getAssetFilePath(prefabPath);

			/*auto prefab = PrefabSerializer::LoadPrefabFromFile(prefabFullPath);
			if (!prefab)
				return 0;

			PrefabRegistry::Get().RegisterPrefab(prefab);

			Entity entity = PrefabInstantiator::InstantiateScenePrefab(s_CurrentScene, prefab->GetGUID());*/

			Entity entity = PrefabInstantiator::InstantiatePrefabFromFile(
				s_CurrentScene,
				prefabFullPath,
				Entity{}  // No parent
			);
			if (!entity)
				return 0;

			InitializeScriptComponentForEntity(entity);
			return static_cast<uint64_t>(static_cast<uint32_t>(entity));
		}

		/**************************************************************************
		 * @brief
		 * Instantiates a prefab into the current scene.
		 * @param prefabPathStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @param position
		 * Pointer/reference to a vector value.
		 * @param rotation
		 * Pointer/reference to a quaternion value.
		 * @param scale
		 * Pointer/reference to a vector value.
		 * @param isScenePrefab
		 * Input parameter.
		 * @return
		 * Unsigned integer result.
		***************************************************************************/
		uint64_t Prefab_InstantiateWithTransform(
			MonoString *prefabPathStr,
			glm::vec3 *position,
			glm::quat *rotation,
			glm::vec3 *scale,
			bool isScenePrefab)
		{
			if (!s_CurrentScene || !prefabPathStr)
				return 0;

			char *c = mono_string_to_utf8(prefabPathStr);
			std::string prefabPath = c ? c : "";
			if (c) mono_free(c);

			if (prefabPath.empty())
				return 0;

			std::string prefabFullPath = getAssetFilePath(prefabPath);

			//auto prefab = PrefabSerializer::LoadPrefabFromFile(prefabFullPath);
		/*	if (!prefab)
				return 0;

			*/
			Entity entity = PrefabInstantiator::InstantiatePrefabFromFile(
				s_CurrentScene,
				prefabFullPath,
				Entity{}  // No parent
			);
			/*if (isScenePrefab)
				entity = PrefabInstantiator::InstantiateScenePrefab(s_CurrentScene, prefab->GetGUID());
			else
				entity = PrefabInstantiator::InstantiateEntityPrefab(s_CurrentScene, prefab->GetGUID());*/


			if (!entity)
				return 0;

			if (position && rotation && scale && entity.HasComponent<TransformComponent>())
			{
				auto &t = entity.GetComponent<TransformComponent>();
				t.Position = *position;
				t.Rotation = *rotation;
				t.Scale = *scale;
				t.IsDirty = true;

				glm::mat4 T = glm::translate(glm::mat4(1.0f), t.Position);
				glm::mat4 R = glm::toMat4(t.Rotation);
				glm::mat4 S = glm::scale(glm::mat4(1.0f), t.Scale);
				glm::mat4 M = T * R * S;

				t.LocalTransform = M;
				t.WorldTransform = M;
			}

			InitializeScriptComponentForEntity(entity);
			return static_cast<uint64_t>(static_cast<uint32_t>(entity));
		}

		// =====================================================================
		// Physics / Rigidbody
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddRigidBody(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<RigidbodyComponent>())
				e.AddComponent<RigidbodyComponent>();
		}

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Result value.
		***************************************************************************/
		glm::vec3 Rigidbody_GetVelocity(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return glm::vec3{};
			return e.GetComponent<RigidbodyComponent>().GetVelocity();
		}

		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param inVel
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Rigidbody_SetVelocity(uint64_t entityID, glm::vec3 *inVel)
		{
			if (!inVel) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetVelocity(*inVel);
		}

		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param delta
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Rigidbody_AddVelocity(uint64_t entityID, glm::vec3 *delta)
		{
			if (!delta) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			auto &rb = e.GetComponent<RigidbodyComponent>();
			rb.SetVelocity(rb.GetVelocity() + *delta);
		}

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Result value.
		***************************************************************************/
		glm::vec3 Rigidbody_GetAngularVelocity(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return glm::vec3{};
			return e.GetComponent<RigidbodyComponent>().AngularVelocity;
		}

		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param inVel
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Rigidbody_SetAngularVelocity(uint64_t entityID, glm::vec3 *inVel)
		{
			if (!inVel) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().AngularVelocity = *inVel;
		}

		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param delta
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Rigidbody_AddAngularVelocity(uint64_t entityID, glm::vec3 *delta)
		{
			if (!delta) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			auto &rb = e.GetComponent<RigidbodyComponent>().AngularVelocity += *delta;
		}

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Rigidbody_GetMass(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return 0.0f;
			return e.GetComponent<RigidbodyComponent>().GetMass();
		}

		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mass
		 * Input parameter.
		***************************************************************************/
		void Rigidbody_SetMass(uint64_t entityID, float mass)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetMass(mass);
		}

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Rigidbody_GetIsKinematic(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsKinematicBody();
		}

		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param isKinematic
		 * Input parameter.
		***************************************************************************/
		void Rigidbody_SetIsKinematic(uint64_t entityID, bool isKinematic)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetKinematic(isKinematic);
		}

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Rigidbody_GetUseGravity(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsGravityEnabled();
		}

		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param useGravity
		 * Input parameter.
		***************************************************************************/
		void Rigidbody_SetUseGravity(uint64_t entityID, bool useGravity)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetGravityEnabled(useGravity);
		}

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Rigidbody_GetSpeed(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return 0.0f;
			return e.GetComponent<RigidbodyComponent>().GetSpeed();
		}

		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Rigidbody_IsMoving(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsMoving();
		}

		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Rigidbody_IsStatic(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsStatic();
		}

		/**************************************************************************
		 * @brief
		 * Applies a force/impulse to the entity's rigidbody.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param force
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Rigidbody_AddForce(uint64_t entityID, glm::vec3 *force)
		{
			if (!force) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().AddForce(*force);
		}

		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Rigidbody_Stop(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().Stop();
		}

		void Rigidbody_SetBoxHalfExtent(uint64_t entityID, glm::vec3 *newBoxHalfExtents)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;

			e.GetComponent<RigidbodyComponent>().BoxHalfExtents = *newBoxHalfExtents;
		}

		glm::vec3 Rigidbody_GetBoxHalfExtent(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return glm::vec3{};
			return e.GetComponent<RigidbodyComponent>().BoxHalfExtents;
		}

		// =====================================================================
		// Physics collisions (PhysicsAPI only)
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Queries global physics system state or collision results.
		***************************************************************************/
		void Physics_EnableCollisionEvents()
		{
			PhysicsAPI::EnableCollisionEvents();
		}

		/**************************************************************************
		 * @brief
		 * Queries global physics system state or collision results.
		***************************************************************************/
		void Physics_BeginCollisionFrame()
		{
			PhysicsAPI::BeginCollisionFrame();
		}

		/**************************************************************************
		 * @brief
		 * Queries global physics system state or collision results.
		 * @return
		 * Requested integer value.
		***************************************************************************/
		int Physics_GetCollisionCount()
		{
			return (int)PhysicsAPI::GetInstance().GetCollisionEvents().size();
		}

		/**************************************************************************
		 * @brief
		 * Queries global physics system state or collision results.
		 * @param index
		 * Zero-based index into the requested collection.
		 * @param a
		 * Output parameter receiving an entity identifier involved in a collision
		 * pair.
		 * @param b
		 * Output parameter receiving an entity identifier involved in a collision
		 * pair.
		***************************************************************************/
		void Physics_GetCollisionPair(int index, uint32_t *a, uint32_t *b)
		{
			if (!a || !b) return;

			const auto &evs = PhysicsAPI::GetInstance().GetCollisionEvents();
			if (index < 0 || index >= (int)evs.size()) return;

			*a = (uint32_t)evs[index].entA;
			*b = (uint32_t)evs[index].entB;
		}

		// =====================================================================
		// Component adders
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddTag(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<TagComponent>())
				e.AddComponent<TagComponent>();
		}

		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddCamera(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<CameraComponent>())
				e.AddComponent<CameraComponent>();
		}

		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddAudio(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<AudioComponent>())
				e.AddComponent<AudioComponent>();
		}

		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddMeshRenderer(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<MeshRendererComponent>())
				e.AddComponent<MeshRendererComponent>();
		}

		// =====================================================================
		// Tag
		// =====================================================================
		MonoString *Tag_GetTag(uint64_t entityID)
		{
			MonoDomain *domain = mono_domain_get();
			if (!domain) return nullptr;

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TagComponent>())
				return mono_string_new(domain, "");

			const std::string &tag = e.GetComponent<TagComponent>().GetTag();
			return mono_string_new(domain, tag.c_str());
		}

		/**************************************************************************
		 * @brief
		 * Gets or sets the entity tag.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param tag
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void Tag_SetTag(uint64_t entityID, MonoString *tagStr)
		{
			if (!tagStr) return;

			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TagComponent>()) return;

			char *utf8 = mono_string_to_utf8(tagStr);
			if (!utf8) return;

			e.GetComponent<TagComponent>().SetTag(utf8);
			mono_free(utf8);
		}

		/***************************************************************************
		 * @brief Finds all entities in the current scene whose TagComponent matches
		 * 		the provided tag string, and returns their entity IDs as a managed
		 * 		Mono array.
		 *
		 * @details
		 * Converts the incoming \p tagString (MonoString) to a UTF-8 std::string.
		 * Iterates the registry view<TagComponent> and collects matching entities.
		 * Returns a MonoArray of uint32 entity IDs.
		 * If the Mono domain is unavailable, returns nullptr.
		 * If the scene/tag is unavailable or conversion fails, returns an empty array.
		 *
		 * @param tagString
		 * 		The tag to search for (managed string). If null, returns an empty array.
		 *
		 * @return A managed MonoArray (System.UInt32[]) containing matching entity IDs,
		 * 		or nullptr if the current Mono domain/class lookup fails.
		***************************************************************************/
		MonoArray *Scene_FindEntitiesByTag(MonoString *tagString)
		{
			MonoDomain *domain = mono_domain_get();
			if (!domain) return nullptr;

			MonoClass *uintClass = mono_get_uint32_class();
			if (!uintClass) return nullptr;

			auto make_empty = [&]() -> MonoArray *
				{
					return mono_array_new(domain, uintClass, 0);
				};

			if (!s_CurrentScene || !tagString)
				return make_empty();

			char *tagCStr = mono_string_to_utf8(tagString);
			if (!tagCStr)
				return make_empty();

			std::string tag(tagCStr);
			mono_free(tagCStr);

			auto &registry = s_CurrentScene->GetRegistry();
			std::vector<uint32_t> results;

			auto view = registry.view<TagComponent>();
			for (auto ent : view)
			{
				const auto &tagComp = view.get<TagComponent>(ent);
				if (tagComp.GetTag() == tag)
					results.push_back(static_cast<uint32_t>(ent));
			}

			MonoArray *arr = mono_array_new(domain, uintClass, (uintptr_t)results.size());
			for (uintptr_t i = 0; i < results.size(); ++i)
				mono_array_set(arr, uint32_t, i, results[i]);

			return arr;
		}

		// =====================================================================
		// Camera
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Camera_GetEnabled(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return false;
			return e.GetComponent<CameraComponent>().Enabled;
		}

		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param enabled
		 * Input parameter.
		***************************************************************************/
		void Camera_SetEnabled(uint64_t entityID, bool enabled)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().Enabled = enabled;
		}

		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Camera_GetPrimary(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return false;
			return e.GetComponent<CameraComponent>().Primary;
		}

		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param primary
		 * Input parameter.
		***************************************************************************/
		void Camera_SetPrimary(uint64_t entityID, bool primary)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().Primary = primary;
		}

		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Camera_GetFOV(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return 0.0f;
			return e.GetComponent<CameraComponent>().FOV;
		}

		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param fov
		 * Input parameter.
		***************************************************************************/
		void Camera_SetFOV(uint64_t entityID, float fov)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().FOV = fov;
		}

		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Camera_GetNear(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return 0.0f;
			return e.GetComponent<CameraComponent>().NearPlane;
		}

		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param nearPlane
		 * Input parameter.
		***************************************************************************/
		void Camera_SetNear(uint64_t entityID, float nearPlane)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().NearPlane = nearPlane;
		}

		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Camera_GetFar(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return 0.0f;
			return e.GetComponent<CameraComponent>().FarPlane;
		}

		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param farPlane
		 * Input parameter.
		***************************************************************************/
		void Camera_SetFar(uint64_t entityID, float farPlane)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().FarPlane = farPlane;
		}

		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param outTarget
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Camera_GetTarget(uint64_t entityID, glm::vec3 *outTarget)
		{
			if (!outTarget) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			*outTarget = e.GetComponent<CameraComponent>().Target;
		}

		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param inTarget
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Camera_SetTarget(uint64_t entityID, glm::vec3 *inTarget)
		{
			if (!inTarget) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().SetTarget(*inTarget);
		}

		// =====================================================================
		// MeshRenderer
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Gets a mesh renderer property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool MeshRenderer_GetVisible(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return false;
			return e.GetComponent<MeshRendererComponent>().Visible;
		}

		/**************************************************************************
		 * @brief
		 * Sets a mesh renderer property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param visible
		 * Input parameter.
		***************************************************************************/
		void MeshRenderer_SetVisible(uint64_t entityID, bool visible)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return;
			e.GetComponent<MeshRendererComponent>().Visible = visible;
		}

		/**************************************************************************
		 * @brief
		 * Gets a mesh renderer property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool MeshRenderer_GetShadowReceive(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return false;
			return e.GetComponent<MeshRendererComponent>().ShadowReceive;
		}

		/**************************************************************************
		 * @brief
		 * Sets a mesh renderer property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param receive
		 * Input parameter.
		***************************************************************************/
		void MeshRenderer_SetShadowReceive(uint64_t entityID, bool receive)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return;
			e.GetComponent<MeshRendererComponent>().ShadowReceive = receive;
		}

		/**************************************************************************
		 * @brief
		 * Gets a mesh renderer property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool MeshRenderer_GetGlobalIlluminate(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return false;
			return e.GetComponent<MeshRendererComponent>().GlobalIlluminate;
		}

		/**************************************************************************
		 * @brief
		 * Sets a mesh renderer property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param gi
		 * Input parameter.
		***************************************************************************/
		void MeshRenderer_SetGlobalIlluminate(uint64_t entityID, bool gi)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return;
			e.GetComponent<MeshRendererComponent>().GlobalIlluminate = gi;
		}

		// =====================================================================
		// AudioComponent
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Starts playback of the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Audio_Play(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetState(PlayState::PLAY);
		}

		/**************************************************************************
		 * @brief
		 * Stops playback of the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Audio_Stop(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetState(PlayState::STOP);
		}

		/**************************************************************************
		 * @brief
		 * Pauses playback of the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Audio_Pause(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetState(PlayState::PAUSE);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetVolume(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0.0f;
			return e.GetComponent<AudioComponent>().Volume;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param volume
		 * New value to apply.
		***************************************************************************/
		void Audio_SetVolume(uint64_t entityID, float volume)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetVolume(volume);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetPitch(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 1.0f;
			return e.GetComponent<AudioComponent>().Pitch;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param pitch
		 * New value to apply.
		***************************************************************************/
		void Audio_SetPitch(uint64_t entityID, float pitch)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetPitch(pitch);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Audio_GetLoop(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return false;
			return e.GetComponent<AudioComponent>().Loop;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param loop
		 * New boolean value to apply.
		***************************************************************************/
		void Audio_SetLoop(uint64_t entityID, bool loop)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetLoop(loop);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Audio_GetMute(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return false;
			return e.GetComponent<AudioComponent>().Mute;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mute
		 * New boolean value to apply.
		***************************************************************************/
		void Audio_SetMute(uint64_t entityID, bool mute)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetMute(mute);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Audio_GetIs3D(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return false;
			return e.GetComponent<AudioComponent>().Is3D;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param is3d
		 * New boolean value to apply.
		***************************************************************************/
		void Audio_SetIs3D(uint64_t entityID, bool is3d)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().Set3D(is3d);
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param path
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void Audio_SetFile(uint64_t entityID, MonoString *path)
		{
			if (!path) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;

			char *utf8 = mono_string_to_utf8(path);
			if (!utf8) return;

			e.GetComponent<AudioComponent>().SetAudioFile(utf8);
			mono_free(utf8);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetMinDistance(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 1.0f;
			return e.GetComponent<AudioComponent>().MinDistance;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param minDist
		 * New value to apply.
		***************************************************************************/
		void Audio_SetMinDistance(uint64_t entityID, float minDist)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetMinDistance(minDist);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetMaxDistance(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 10000.0f;
			return e.GetComponent<AudioComponent>().MaxDistance;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param maxDist
		 * New value to apply.
		***************************************************************************/
		void Audio_SetMaxDistance(uint64_t entityID, float maxDist)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetMaxDistance(maxDist);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested integer value.
		***************************************************************************/
		int Audio_GetRolloffMode(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0;
			return static_cast<int>(e.GetComponent<AudioComponent>().RolloffMode);
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mode
		 * New mode value to apply.
		***************************************************************************/
		void Audio_SetRolloffMode(uint64_t entityID, int mode)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetRolloffMode(static_cast<AudioRolloffMode>(mode));
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetDopplerLevel(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 1.0f;
			return e.GetComponent<AudioComponent>().DopplerLevel;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param level
		 * New value to apply.
		***************************************************************************/
		void Audio_SetDopplerLevel(uint64_t entityID, float level)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetDopplerLevel(level);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetPan2D(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0.0f;
			return e.GetComponent<AudioComponent>().Pan2D;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param pan
		 * New value to apply.
		***************************************************************************/
		void Audio_SetPan2D(uint64_t entityID, float pan)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetPan(pan);
		}

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetReverbMix(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0.0f;
			return e.GetComponent<AudioComponent>().ReverbProperties;
		}

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mix
		 * New value to apply.
		***************************************************************************/
		void Audio_SetReverbMix(uint64_t entityID, float mix)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetReverbProperties(mix);
		}

		// =====================================================================
		// AudioManager
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param volume
		 * New value to apply.
		***************************************************************************/
		void AudioManager_SetGroupVolume(int groupType, float volume)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->SetGroupVolume(static_cast<AudioType>(groupType), volume);
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float AudioManager_GetGroupVolume(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return 0.0f;
			float vol = 0.0f;
			am->GetGroupVolume(static_cast<AudioType>(groupType), vol);
			return vol;
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param pitch
		 * New value to apply.
		***************************************************************************/
		void AudioManager_SetGroupPitch(int groupType, float pitch)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->SetGroupPitch(static_cast<AudioType>(groupType), pitch);
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float AudioManager_GetGroupPitch(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return 1.0f;
			float pitch = 1.0f;
			am->GetGroupPitch(static_cast<AudioType>(groupType), pitch);
			return pitch;
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param mute
		 * New boolean value to apply.
		***************************************************************************/
		void AudioManager_SetGroupMute(int groupType, bool mute)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->MuteGroup(static_cast<AudioType>(groupType), mute);
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool AudioManager_IsGroupMuted(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return false;
			return am->IsGroupMuted(static_cast<AudioType>(groupType));
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param pause
		 * Input parameter.
		***************************************************************************/
		void AudioManager_PauseGroup(int groupType, bool pause)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->PauseGroup(static_cast<AudioType>(groupType), pause);
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param pause
		 * Input parameter.
		***************************************************************************/
		void AudioManager_PauseAll(bool pause)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->PauseAll(pause);
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		***************************************************************************/
		void AudioManager_StopByType(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->StopByType(static_cast<AudioType>(groupType));
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		***************************************************************************/
		void AudioManager_StopAll()
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->StopAll();
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param effectType
		 * Input parameter.
		***************************************************************************/
		void AudioManager_CreateDSP(int groupType, int effectType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->CreateDSP(static_cast<DSPEffectType>(effectType), static_cast<AudioType>(groupType));
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param effectType
		 * Input parameter.
		 * @param enable
		 * Input parameter.
		***************************************************************************/
		void AudioManager_EnableDSP(int groupType, int effectType, bool enable)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->EnableDSP(static_cast<AudioType>(groupType), static_cast<DSPEffectType>(effectType), enable);
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param effectType
		 * Input parameter.
		 * @param paramIndex
		 * Input parameter.
		 * @param value
		 * Input parameter.
		***************************************************************************/
		void AudioManager_SetDSPParameter(int groupType, int effectType, int paramIndex, float value)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->SetDSPParameter(static_cast<AudioType>(groupType),
				static_cast<DSPEffectType>(effectType),
				paramIndex, value);
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param effectType
		 * Input parameter.
		***************************************************************************/
		void AudioManager_ReleaseSpecificDSPinGroup(int groupType, int effectType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->ReleaseSpecificDSPinGroup(static_cast<AudioType>(groupType),
				static_cast<DSPEffectType>(effectType));
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		***************************************************************************/
		void AudioManager_ReleaseDSPByGroup(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->ReleaseDSPByGroup(static_cast<AudioType>(groupType));
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		***************************************************************************/
		void AudioManager_ReleaseAllDSPs()
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->ReleaseAllDSPs();
		}

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param position
		 * Pointer/reference to a vector value.
		 * @param forward
		 * Pointer to a vector that receives the computed result.
		 * @param up
		 * Pointer to a vector that receives the computed result.
		 * @param velocity
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void AudioManager_SetListenerAttributes(glm::vec3 *position, glm::vec3 *forward,
			glm::vec3 *up, glm::vec3 *velocity)
		{
			auto *am = GetAudioManager();
			if (!am || !position || !forward || !up || !velocity) return;
			am->SetListenerAttributes(*position, *forward, *up, *velocity);
		}

		// =====================================================================
		// Event publish
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Publishes an event into the engine event system.
		 * @param nameStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @param payloadStr
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void Event_Publish(MonoString *nameStr, MonoString *payloadStr)
		{
			ScriptEvent ev;

			if (nameStr)
			{
				char *cName = mono_string_to_utf8(nameStr);
				if (cName)
				{
					ev.name = cName; mono_free(cName);
				}
			}

			if (payloadStr)
			{
				char *cPayload = mono_string_to_utf8(payloadStr);
				if (cPayload)
				{
					ev.payload = cPayload; mono_free(cPayload);
				}
			}

			EventSystem::Instance().Queue(ev);
		}

		// =====================================================================
		// Quaternion helpers
		// =====================================================================
		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param axis
		 * Pointer/reference to a vector value.
		 * @param angleRadians
		 * Input parameter.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_FromAxisAngle(glm::vec3 *axis, float angleRadians, glm::quat *outQuat)
		{
			if (!axis || !outQuat) return;
			*outQuat = glm::angleAxis(angleRadians, glm::normalize(*axis));
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outForward
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_GetForward(glm::quat *quat, glm::vec3 *outForward)
		{
			if (!quat || !outForward) return;
			*outForward = glm::rotate(*quat, glm::vec3(0.0f, 0.0f, -1.0f));
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outRight
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_GetRight(glm::quat *quat, glm::vec3 *outRight)
		{
			if (!quat || !outRight) return;
			*outRight = glm::rotate(*quat, glm::vec3(1.0f, 0.0f, 0.0f));
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outUp
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_GetUp(glm::quat *quat, glm::vec3 *outUp)
		{
			if (!quat || !outUp) return;
			*outUp = glm::rotate(*quat, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param vec
		 * Pointer/reference to a vector value.
		 * @param outVec
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_RotateVector(glm::quat *quat, glm::vec3 *vec, glm::vec3 *outVec)
		{
			if (!quat || !vec || !outVec) return;
			*outVec = glm::rotate(*quat, *vec);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param q1
		 * Pointer/reference to a quaternion value.
		 * @param q2
		 * Pointer/reference to a quaternion value.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_Multiply(glm::quat *q1, glm::quat *q2, glm::quat *outQuat)
		{
			if (!q1 || !q2 || !outQuat) return;
			*outQuat = (*q1) * (*q2);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param q1
		 * Pointer/reference to a quaternion value.
		 * @param q2
		 * Pointer/reference to a quaternion value.
		 * @param t
		 * Interpolation parameter in [0, 1].
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_Slerp(glm::quat *q1, glm::quat *q2, float t, glm::quat *outQuat)
		{
			if (!q1 || !q2 || !outQuat) return;
			*outQuat = glm::slerp(*q1, *q2, t);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_Inverse(glm::quat *quat, glm::quat *outQuat)
		{
			if (!quat || !outQuat) return;
			*outQuat = glm::inverse(*quat);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outEuler
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_ToEuler(glm::quat *quat, glm::vec3 *outEuler)
		{
			if (!quat || !outEuler) return;
			*outEuler = glm::eulerAngles(*quat);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param euler
		 * Pointer/reference to a vector value.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_FromEuler(glm::vec3 *euler, glm::quat *outQuat)
		{
			if (!euler || !outQuat) return;
			*outQuat = glm::quat(*euler);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Quat_Normalize(glm::quat *quat, glm::quat *outQuat)
		{
			if (!quat || !outQuat) return;
			*outQuat = glm::normalize(*quat);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Quat_Length(glm::quat *quat)
		{
			if (!quat) return 0.0f;
			return glm::length(*quat);
		}

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param q1
		 * Pointer/reference to a quaternion value.
		 * @param q2
		 * Pointer/reference to a quaternion value.
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Quat_Dot(glm::quat *q1, glm::quat *q2)
		{
			if (!q1 || !q2) return 0.0f;
			return glm::dot(*q1, *q2);
		}

		// ========================================
// File I/O
// ========================================

/**
 * @brief Checks if a file exists at the given path.
 * @param pathStr Managed string provided by the scripting runtime (MonoString*).
 * @return True if the file exists, otherwise false.
 */
		bool FileExists(MonoString *pathStr)
		{
			if (!pathStr)
				return false;

			char *pathCStr = mono_string_to_utf8(pathStr);
			if (!pathCStr)
				return false;

			std::string path(pathCStr);
			mono_free(pathCStr);

			// Use fopen for reliable cross-platform existence check
			FILE *file = fopen(path.c_str(), "r");
			if (file)
			{
				fclose(file);
				return true;
			}
			return false;
		}

		/**
		 * @brief Reads the entire content of a text file.
		 * @param pathStr Managed string provided by the scripting runtime (MonoString*).
		 * @return Managed string containing file content, or empty string on failure.
		 */
		MonoString *FileReadAllText(MonoString *pathStr)
		{
			MonoDomain *domain = mono_domain_get();
			if (!domain)
				return nullptr;

			if (!pathStr)
				return mono_string_new(domain, "");

			char *pathCStr = mono_string_to_utf8(pathStr);
			if (!pathCStr)
				return mono_string_new(domain, "");

			std::string path(pathCStr);
			mono_free(pathCStr);

			// Open file for reading
			std::ifstream file(path);
			if (!file.is_open())
			{
				LOG_ERROR("[FileIO] Failed to open file for reading: {0}", path);
				return mono_string_new(domain, "");
			}

			// Read entire file into string
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();

			std::string content = buffer.str();
			return mono_string_new(domain, content.c_str());
		}

		/**
		 * @brief Helper function to create directory recursively (cross-platform).
		 * @param path Directory path to create.
		 */
		static void CreateDirectoriesRecursive(const std::string &path)
		{
			if (path.empty())
				return;

			// Find parent directory
			size_t pos = path.find_last_of("/\\");
			if (pos != std::string::npos)
			{
				std::string parent = path.substr(0, pos);
				CreateDirectoriesRecursive(parent);
			}

			// Try to create this directory (ignore errors if it already exists)
#ifdef _WIN32
			_mkdir(path.c_str());
#else
			mkdir(path.c_str(), 0755);
#endif
		}

		/**
		 * @brief Writes text content to a file, creating parent directories if needed.
		 * @param pathStr Managed string provided by the scripting runtime (MonoString*).
		 * @param contentStr Managed string provided by the scripting runtime (MonoString*).
		 * @return True if write succeeded, otherwise false.
		 */
		bool FileWriteAllText(MonoString *pathStr, MonoString *contentStr)
		{
			if (!pathStr || !contentStr)
				return false;

			// Convert path
			char *pathCStr = mono_string_to_utf8(pathStr);
			if (!pathCStr)
				return false;

			std::string path(pathCStr);
			mono_free(pathCStr);

			// Convert content
			char *contentCStr = mono_string_to_utf8(contentStr);
			if (!contentCStr)
				return false;

			std::string content(contentCStr);
			mono_free(contentCStr);

			// Create parent directories if needed
			size_t lastSlash = path.find_last_of("/\\");
			if (lastSlash != std::string::npos)
			{
				std::string parentDir = path.substr(0, lastSlash);
				CreateDirectoriesRecursive(parentDir);
			}

			// Write file
			std::ofstream file(path);
			if (!file.is_open())
			{
				LOG_ERROR("[FileIO] Failed to open file for writing: {0}", path);
				return false;
			}

			file << content;
			file.close();

			LOG_INFO("[FileIO] File written successfully: {0}", path);
			return true;
		}





		static std::uint32_t g_rngState = 0x6D2B79F5u; // non-zero default

		static inline std::uint32_t NextU32()
		{
			// xorshift32
			std::uint32_t x = g_rngState;
			if (x == 0u) x = 0x6D2B79F5u; // never allow 0 state

			x ^= (x << 13);
			x ^= (x >> 17);
			x ^= (x << 5);

			g_rngState = x;
			return x;
		}

		void RNG_Seed(std::uint32_t seed)
		{
			g_rngState = (seed == 0u) ? 0x6D2B79F5u : seed;
			(void)NextU32(); // diffuse a bit
		}

		int RNG_RandInt(int min, int max)
		{
			if (min == max) return min;
			if (min > max)
			{
				int tmp = min; min = max; max = tmp;
			}

			// Inclusive range [min, max]
			std::uint32_t span = static_cast<std::uint32_t>(max - min) + 1u;

			// Simple modulo (tiny bias, fine for games)
			std::uint32_t r = NextU32();
			int val = min + static_cast<int>(r % span);
			return val;
		}

		float RNG_RandFloat(float min, float max)
		{
			if (min == max) return min;
			if (min > max)
			{
				float tmp = min; min = max; max = tmp;
			}

			// Convert to [0,1) using 24 bits
			std::uint32_t r = NextU32();
			r >>= 8; // 24 bits
			float t = static_cast<float>(r) * (1.0f / 16777216.0f); // 2^24

			return min + (max - min) * t; // [min, max)
		}

		bool RNG_RandBool()
		{
			return (NextU32() & 1u) != 0u;
		}

		bool CollisionSystem2D_IsPointInEntity(uint64_t entityId, glm::vec2 *point)
		{
			if (!s_CurrentScene) return false;

			auto sys = s_CurrentScene->GetSystem<CollisionSystem2D>();
			return sys->IsPointInEntity(static_cast<entt::entity>(entityId), *point);
		}

		// =====================================================================
		// SpriteRenderer
		// =====================================================================

		void SpriteRenderer_SetIsVisible(uint32_t entityID, bool visible)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_SetIsVisible: No scene set");
				return;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_SetIsVisible: Invalid entity ID ", entityID);
				return;
			}

			if (registry.all_of<SpriteRendererComponent>(entity))
			{
				auto &sprite = registry.get<SpriteRendererComponent>(entity);
				sprite.SetIsVisible(visible);
			}
			else
			{
				LOG_WARNING("[InternalCalls] Entity ", entityID, " has no SpriteRendererComponent");
			}
		}

		bool SpriteRenderer_GetIsVisible(uint32_t entityID)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_GetIsVisible: No scene set");
				return false;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_GetIsVisible: Invalid entity ID ", entityID);
				return false;
			}

			if (registry.all_of<SpriteRendererComponent>(entity))
			{
				auto &sprite = registry.get<SpriteRendererComponent>(entity);
				return sprite.GetIsVisible();
			}

			return false;
		}

		void SpriteRenderer_SetColor(uint32_t entityID, float r, float g, float b, float a)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_SetColor: No scene set");
				return;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_SetColor: Invalid entity ID ", entityID);
				return;
			}

			if (registry.all_of<SpriteRendererComponent>(entity))
			{
				auto &sprite = registry.get<SpriteRendererComponent>(entity);
				sprite.SetColor(glm::vec4(r, g, b, a));
			}
			else
			{
				LOG_WARNING("[InternalCalls] Entity ", entityID, " has no SpriteRendererComponent");
			}
		}

		void SpriteRenderer_GetColor(uint32_t entityID, float *r, float *g, float *b, float *a)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_GetColor: No scene set");
				return;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] SpriteRenderer_GetColor: Invalid entity ID ", entityID);
				return;
			}

			if (registry.all_of<SpriteRendererComponent>(entity))
			{
				auto &sprite = registry.get<SpriteRendererComponent>(entity);
				glm::vec4 const &color = sprite.GetColor();
				*r = color.r;
				*g = color.g;
				*b = color.b;
				*a = color.a;
			}
		}

		void ParticleSystem_SetEmitterVelocity(uint64_t entityID, glm::vec3 *vel)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<ParticleComponent>()) return;
			e.GetComponent<ParticleComponent>().InitialVelocity = (*vel);
		}

		void ParticleSystem_SetEmissionRate(uint64_t entityID, float rate)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<ParticleComponent>()) return;
			e.GetComponent<ParticleComponent>().EmissionRate = rate;
		}

		void Text_SetText(uint32_t entityID, MonoString *text)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] Text_SetText: No scene set");
				return;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] Text_SetText: Invalid entity ID ", entityID);
				return;
			}

			if (registry.all_of<TextComponent>(entity))
			{
				auto &textComp = registry.get<TextComponent>(entity);

				// Convert MonoString to C++ string
				char *utf8 = mono_string_to_utf8(text);
				if (utf8)
				{
					textComp.setText(std::string(utf8));
					mono_free(utf8);
				}
			}
			else
			{
				LOG_WARNING("[InternalCalls] Entity ", entityID, " has no TextComponent");
			}
		}

		MonoString *Text_GetText(uint32_t entityID)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] Text_GetText: No scene set");
				return nullptr;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] Text_GetText: Invalid entity ID ", entityID);
				return nullptr;
			}

			if (registry.all_of<TextComponent>(entity))
			{
				auto &textComp = registry.get<TextComponent>(entity);

				MonoDomain *domain = mono_domain_get();
				return mono_string_new(domain, textComp.getText().c_str());
			}

			return nullptr;
		}

		void Text_SetFontSize(uint32_t entityID, float size)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] Text_SetFontSize: No scene set");
				return;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] Text_SetFontSize: Invalid entity ID ", entityID);
				return;
			}

			if (registry.all_of<TextComponent>(entity))
			{
				auto &textComp = registry.get<TextComponent>(entity);
				textComp.setFontSize(size);
			}
		}

		float Text_GetFontSize(uint32_t entityID)
		{
			if (!s_CurrentScene)
			{
				LOG_ERROR("[InternalCalls] Text_GetFontSize: No scene set");
				return 0.0f;
			}

			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);

			if (!registry.valid(entity))
			{
				LOG_ERROR("[InternalCalls] Text_GetFontSize: Invalid entity ID ", entityID);
				return 0.0f;
			}

			if (registry.all_of<TextComponent>(entity))
			{
				auto &textComp = registry.get<TextComponent>(entity);
				return textComp.getFontSize();
			}

			return 0.0f;
		}

		void Text_SetIsVisible(uint32_t entityID, bool visible)
		{
			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);
			if (registry.all_of<TextComponent>(entity))
			{
				auto &text = registry.get<TextComponent>(entity);
				text.setVisible(visible);
			}
		}

		bool Text_GetIsVisible(uint32_t entityID)
		{
			auto &registry = s_CurrentScene->GetRegistry();
			entt::entity entity = static_cast<entt::entity>(entityID);
			if (registry.all_of<TextComponent>(entity))
			{
				auto &text = registry.get<TextComponent>(entity);
				return text.isShown();
			}
			return false;
		}
	} // namespace InternalCalls
} // namespace Engine

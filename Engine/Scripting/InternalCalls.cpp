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

// Mono
#include <mono/jit/jit.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>

// GLM extras (translate/scale, quat ops)
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <vector>

namespace Engine
{
	namespace InternalCalls
	{
		// =====================================================================
		// Global context (set from ScriptSystem via MonoScriptEngine helpers)
		// =====================================================================
		static Scene *s_CurrentScene = nullptr;
		static Input *s_InputSystem = nullptr;
		static AudioManager *s_AudioManager = nullptr;

		void SetCurrentScene(Scene *scene) { s_CurrentScene = scene; }
		void SetInputSystem(Input *input) { s_InputSystem = input; }
		void SetAudioManager(AudioManager *audioManager) { s_AudioManager = audioManager; }
		AudioManager *GetAudioManager() { return s_AudioManager; }

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

			// If prefab clone already contains a managed instance, just rebind.
			if (sc.ScriptInstance)
			{
				se.BindEntityID(static_cast<MonoObject *>(sc.ScriptInstance), static_cast<std::uint32_t>(eid));
				sc.Started = false;
				return;
			}

			MonoObject *instance = se.CreateScriptInstance(sc.ScriptClassName);
			if (!instance)
			{
				LOG_ERROR("[InternalCall] Prefab script init failed for class '", sc.ScriptClassName, "' on entity ", eid);
				return;
			}

			se.BindEntityID(instance, static_cast<std::uint32_t>(eid));
			sc.ScriptInstance = instance;
			sc.Started = false;
		}

		// =====================================================================
		// Component presence helpers (registered as internal calls)
		// =====================================================================
		bool EntityHasCamera(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			return e && e.HasComponent<CameraComponent>();
		}

		bool EntityHasRigidBody(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			return e && e.HasComponent<RigidbodyComponent>();
		}

		// =====================================================================
		// Scene / Entity lifecycle
		// =====================================================================
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
			MonoObject *instance = se.CreateScriptInstance(klass);
			if (!instance)
			{
				LOG_ERROR("[InternalCall] Entity_AddScript: failed to create instance of ", klass);
				return;
			}

			se.BindEntityID(instance, static_cast<std::uint32_t>(entityID));

			if (e.HasComponent<ScriptComponent>())
			{
				auto &sc = e.GetComponent<ScriptComponent>();
				sc.ScriptClassName = klass;
				sc.ScriptInstance = instance;
				sc.Started = false;
			}
			else
			{
				auto &sc = e.AddComponent<ScriptComponent>();
				sc.ScriptClassName = klass;
				sc.ScriptInstance = instance;
				sc.Started = false;
			}
		}

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

		// =====================================================================
		// Logging
		// =====================================================================
		void Log(MonoString *message)
		{
			if (!message) return;
			char *cStr = mono_string_to_utf8(message);
			LOG_INFO("[C#] ", cStr ? cStr : "<null>");
			if (cStr) mono_free(cStr);
		}

		void LogError(MonoString *message)
		{
			if (!message) return;
			char *cStr = mono_string_to_utf8(message);
			LOG_ERROR("[C#] ", cStr ? cStr : "<null>");
			if (cStr) mono_free(cStr);
		}

		void LogWarning(MonoString *message)
		{
			if (!message) return;
			char *cStr = mono_string_to_utf8(message);
			LOG_WARNING("[C#] ", cStr ? cStr : "<null>");
			if (cStr) mono_free(cStr);
		}

		// =====================================================================
		// Entity utilities
		// =====================================================================
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
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			*outPosition = e.GetComponent<TransformComponent>().Position;
		}

		void Transform_SetPosition(uint64_t entityID, glm::vec3 *position)
		{
			if (!position) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			auto &t = e.GetComponent<TransformComponent>();
			t.Position = *position;
			t.IsDirty = true;
		}

		void Transform_GetRotation(uint64_t entityID, glm::quat *outRotation)
		{
			if (!outRotation) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			*outRotation = e.GetComponent<TransformComponent>().Rotation;
		}

		void Transform_SetRotation(uint64_t entityID, glm::quat *rotation)
		{
			if (!rotation) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			auto &t = e.GetComponent<TransformComponent>();
			t.Rotation = *rotation;
			t.IsDirty = true;
		}

		void Transform_GetScale(uint64_t entityID, glm::vec3 *outScale)
		{
			if (!outScale) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			*outScale = e.GetComponent<TransformComponent>().Scale;
		}

		void Transform_SetScale(uint64_t entityID, glm::vec3 *scale)
		{
			if (!scale) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<TransformComponent>()) return;

			auto &t = e.GetComponent<TransformComponent>();
			t.Scale = *scale;
			t.IsDirty = true;
		}

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
		bool Input_IsKeyPressed(int keyCode)
		{
			if (!s_InputSystem) return false;
			return s_InputSystem->IsKeyPressed(keyCode);
		}

		bool Input_IsKeyReleased(int keyCode)
		{
			if (!s_InputSystem) return false;
			return s_InputSystem->IsKeyJustReleased(keyCode);
		}

		bool Input_IsMouseButtonPressed(int button)
		{
			if (!s_InputSystem) return false;
			return s_InputSystem->IsMouseButtonPressed(button);
		}

		void Input_GetMousePosition(glm::vec2 *outPosition)
		{
			if (!outPosition) return;
			if (!s_InputSystem) { *outPosition = { 0.0f, 0.0f }; return; }
			*outPosition = s_InputSystem->GetMousePosition();
		}

		void Input_GetMouseDelta(float *outX, float *outY)
		{
			if (outX) *outX = 0.0f;
			if (outY) *outY = 0.0f;
			if (!s_InputSystem) return;

			glm::vec2 d = s_InputSystem->GetMouseDelta();
			if (outX) *outX = d.x;
			if (outY) *outY = d.y;
		}

		// =====================================================================
		// Prefabs
		// =====================================================================
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

			auto prefab = PrefabSerializer::LoadPrefabFromFile(prefabFullPath);
			if (!prefab)
				return 0;

			PrefabRegistry::Get().RegisterPrefab(prefab);

			Entity entity = PrefabInstantiator::InstantiateEntityPrefab(s_CurrentScene, prefab->GetGUID());
			if (!entity)
				return 0;

			InitializeScriptComponentForEntity(entity);
			return static_cast<uint64_t>(static_cast<uint32_t>(entity));
		}

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

			auto prefab = PrefabSerializer::LoadPrefabFromFile(prefabFullPath);
			if (!prefab)
				return 0;

			PrefabRegistry::Get().RegisterPrefab(prefab);

			Entity entity = PrefabInstantiator::InstantiateScenePrefab(s_CurrentScene, prefab->GetGUID());
			if (!entity)
				return 0;

			InitializeScriptComponentForEntity(entity);
			return static_cast<uint64_t>(static_cast<uint32_t>(entity));
		}

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

			auto prefab = PrefabSerializer::LoadPrefabFromFile(prefabFullPath);
			if (!prefab)
				return 0;

			PrefabRegistry::Get().RegisterPrefab(prefab);

			Entity entity;
			if (isScenePrefab)
				entity = PrefabInstantiator::InstantiateScenePrefab(s_CurrentScene, prefab->GetGUID());
			else
				entity = PrefabInstantiator::InstantiateEntityPrefab(s_CurrentScene, prefab->GetGUID());

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
		void Entity_AddRigidBody(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<RigidbodyComponent>())
				e.AddComponent<RigidbodyComponent>();
		}

		void Rigidbody_GetVelocity(uint64_t entityID, glm::vec3 *outVel)
		{
			if (!outVel) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			*outVel = e.GetComponent<RigidbodyComponent>().GetVelocity();
		}

		void Rigidbody_SetVelocity(uint64_t entityID, glm::vec3 *inVel)
		{
			if (!inVel) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetVelocity(*inVel);
		}

		void Rigidbody_AddVelocity(uint64_t entityID, glm::vec3 *delta)
		{
			if (!delta) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			auto &rb = e.GetComponent<RigidbodyComponent>();
			rb.SetVelocity(rb.GetVelocity() + *delta);
		}

		float Rigidbody_GetMass(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return 0.0f;
			return e.GetComponent<RigidbodyComponent>().GetMass();
		}

		void Rigidbody_SetMass(uint64_t entityID, float mass)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetMass(mass);
		}

		bool Rigidbody_GetIsKinematic(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsKinematicBody();
		}

		void Rigidbody_SetIsKinematic(uint64_t entityID, bool isKinematic)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetKinematic(isKinematic);
		}

		bool Rigidbody_GetUseGravity(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsGravityEnabled();
		}

		void Rigidbody_SetUseGravity(uint64_t entityID, bool useGravity)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().SetGravityEnabled(useGravity);
		}

		float Rigidbody_GetSpeed(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return 0.0f;
			return e.GetComponent<RigidbodyComponent>().GetSpeed();
		}

		bool Rigidbody_IsMoving(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsMoving();
		}

		bool Rigidbody_IsStatic(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return false;
			return e.GetComponent<RigidbodyComponent>().IsStatic();
		}

		void Rigidbody_AddForce(uint64_t entityID, glm::vec3 *force)
		{
			if (!force) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().AddForce(*force);
		}

		void Rigidbody_Stop(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<RigidbodyComponent>()) return;
			e.GetComponent<RigidbodyComponent>().Stop();
		}

		// =====================================================================
		// Physics collisions (PhysicsAPI only)
		// =====================================================================
		void Physics_EnableCollisionEvents()
		{
			PhysicsAPI::EnableCollisionEvents();
		}

		void Physics_BeginCollisionFrame()
		{
			PhysicsAPI::BeginCollisionFrame();
		}

		int Physics_GetCollisionCount()
		{
			return (int)PhysicsAPI::GetInstance().GetCollisionEvents().size();
		}

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
		void Entity_AddTag(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<TagComponent>())
				e.AddComponent<TagComponent>();
		}

		void Entity_AddCamera(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<CameraComponent>())
				e.AddComponent<CameraComponent>();
		}

		void Entity_AddAudio(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e) return;
			if (!e.HasComponent<AudioComponent>())
				e.AddComponent<AudioComponent>();
		}

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
		bool Camera_GetEnabled(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return false;
			return e.GetComponent<CameraComponent>().Enabled;
		}

		void Camera_SetEnabled(uint64_t entityID, bool enabled)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().Enabled = enabled;
		}

		bool Camera_GetPrimary(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return false;
			return e.GetComponent<CameraComponent>().Primary;
		}

		void Camera_SetPrimary(uint64_t entityID, bool primary)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().Primary = primary;
		}

		float Camera_GetFOV(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return 0.0f;
			return e.GetComponent<CameraComponent>().FOV;
		}

		void Camera_SetFOV(uint64_t entityID, float fov)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().FOV = fov;
		}

		float Camera_GetNear(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return 0.0f;
			return e.GetComponent<CameraComponent>().NearPlane;
		}

		void Camera_SetNear(uint64_t entityID, float nearPlane)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().NearPlane = nearPlane;
		}

		float Camera_GetFar(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return 0.0f;
			return e.GetComponent<CameraComponent>().FarPlane;
		}

		void Camera_SetFar(uint64_t entityID, float farPlane)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			e.GetComponent<CameraComponent>().FarPlane = farPlane;
		}

		void Camera_GetTarget(uint64_t entityID, glm::vec3 *outTarget)
		{
			if (!outTarget) return;
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<CameraComponent>()) return;
			*outTarget = e.GetComponent<CameraComponent>().Target;
		}

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
		bool MeshRenderer_GetVisible(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return false;
			return e.GetComponent<MeshRendererComponent>().Visible;
		}

		void MeshRenderer_SetVisible(uint64_t entityID, bool visible)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return;
			e.GetComponent<MeshRendererComponent>().Visible = visible;
		}

		bool MeshRenderer_GetShadowCast(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return false;
			return e.GetComponent<MeshRendererComponent>().ShadowCast;
		}

		void MeshRenderer_SetShadowCast(uint64_t entityID, bool cast)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return;
			e.GetComponent<MeshRendererComponent>().ShadowCast = cast;
		}

		bool MeshRenderer_GetShadowReceive(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return false;
			return e.GetComponent<MeshRendererComponent>().ShadowReceive;
		}

		void MeshRenderer_SetShadowReceive(uint64_t entityID, bool receive)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return;
			e.GetComponent<MeshRendererComponent>().ShadowReceive = receive;
		}

		bool MeshRenderer_GetGlobalIlluminate(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return false;
			return e.GetComponent<MeshRendererComponent>().GlobalIlluminate;
		}

		void MeshRenderer_SetGlobalIlluminate(uint64_t entityID, bool gi)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<MeshRendererComponent>()) return;
			e.GetComponent<MeshRendererComponent>().GlobalIlluminate = gi;
		}

		// =====================================================================
		// AudioComponent
		// =====================================================================
		void Audio_Play(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetState(PlayState::PLAY);
		}

		void Audio_Stop(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetState(PlayState::STOP);
		}

		void Audio_Pause(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetState(PlayState::PAUSE);
		}

		float Audio_GetVolume(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0.0f;
			return e.GetComponent<AudioComponent>().Volume;
		}

		void Audio_SetVolume(uint64_t entityID, float volume)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetVolume(volume);
		}

		float Audio_GetPitch(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 1.0f;
			return e.GetComponent<AudioComponent>().Pitch;
		}

		void Audio_SetPitch(uint64_t entityID, float pitch)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetPitch(pitch);
		}

		bool Audio_GetLoop(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return false;
			return e.GetComponent<AudioComponent>().Loop;
		}

		void Audio_SetLoop(uint64_t entityID, bool loop)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetLoop(loop);
		}

		bool Audio_GetMute(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return false;
			return e.GetComponent<AudioComponent>().Mute;
		}

		void Audio_SetMute(uint64_t entityID, bool mute)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetMute(mute);
		}

		bool Audio_GetIs3D(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return false;
			return e.GetComponent<AudioComponent>().Is3D;
		}

		void Audio_SetIs3D(uint64_t entityID, bool is3d)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().Set3D(is3d);
		}

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

		float Audio_GetMinDistance(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 1.0f;
			return e.GetComponent<AudioComponent>().MinDistance;
		}

		void Audio_SetMinDistance(uint64_t entityID, float minDist)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetMinDistance(minDist);
		}

		float Audio_GetMaxDistance(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 10000.0f;
			return e.GetComponent<AudioComponent>().MaxDistance;
		}

		void Audio_SetMaxDistance(uint64_t entityID, float maxDist)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetMaxDistance(maxDist);
		}

		int Audio_GetRolloffMode(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0;
			return static_cast<int>(e.GetComponent<AudioComponent>().RolloffMode);
		}

		void Audio_SetRolloffMode(uint64_t entityID, int mode)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetRolloffMode(static_cast<AudioRolloffMode>(mode));
		}

		float Audio_GetDopplerLevel(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 1.0f;
			return e.GetComponent<AudioComponent>().DopplerLevel;
		}

		void Audio_SetDopplerLevel(uint64_t entityID, float level)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetDopplerLevel(level);
		}

		float Audio_GetPan2D(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0.0f;
			return e.GetComponent<AudioComponent>().Pan2D;
		}

		void Audio_SetPan2D(uint64_t entityID, float pan)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetPan(pan);
		}

		float Audio_GetReverbMix(uint64_t entityID)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return 0.0f;
			return e.GetComponent<AudioComponent>().ReverbProperties;
		}

		void Audio_SetReverbMix(uint64_t entityID, float mix)
		{
			Entity e = GetEntityOrNull(entityID);
			if (!e || !e.HasComponent<AudioComponent>()) return;
			e.GetComponent<AudioComponent>().SetReverbProperties(mix);
		}

		// =====================================================================
		// AudioManager
		// =====================================================================
		void AudioManager_SetGroupVolume(int groupType, float volume)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->SetGroupVolume(static_cast<AudioType>(groupType), volume);
		}

		float AudioManager_GetGroupVolume(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return 0.0f;
			float vol = 0.0f;
			am->GetGroupVolume(static_cast<AudioType>(groupType), vol);
			return vol;
		}

		void AudioManager_SetGroupPitch(int groupType, float pitch)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->SetGroupPitch(static_cast<AudioType>(groupType), pitch);
		}

		float AudioManager_GetGroupPitch(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return 1.0f;
			float pitch = 1.0f;
			am->GetGroupPitch(static_cast<AudioType>(groupType), pitch);
			return pitch;
		}

		void AudioManager_SetGroupMute(int groupType, bool mute)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->MuteGroup(static_cast<AudioType>(groupType), mute);
		}

		bool AudioManager_IsGroupMuted(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return false;
			return am->IsGroupMuted(static_cast<AudioType>(groupType));
		}

		void AudioManager_PauseGroup(int groupType, bool pause)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->PauseGroup(static_cast<AudioType>(groupType), pause);
		}

		void AudioManager_PauseAll(bool pause)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->PauseAll(pause);
		}

		void AudioManager_StopByType(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->StopByType(static_cast<AudioType>(groupType));
		}

		void AudioManager_StopAll()
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->StopAll();
		}

		void AudioManager_CreateDSP(int groupType, int effectType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->CreateDSP(static_cast<DSPEffectType>(effectType), static_cast<AudioType>(groupType));
		}

		void AudioManager_EnableDSP(int groupType, int effectType, bool enable)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->EnableDSP(static_cast<AudioType>(groupType), static_cast<DSPEffectType>(effectType), enable);
		}

		void AudioManager_SetDSPParameter(int groupType, int effectType, int paramIndex, float value)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->SetDSPParameter(static_cast<AudioType>(groupType),
				static_cast<DSPEffectType>(effectType),
				paramIndex, value);
		}

		void AudioManager_ReleaseSpecificDSPinGroup(int groupType, int effectType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->ReleaseSpecificDSPinGroup(static_cast<AudioType>(groupType),
				static_cast<DSPEffectType>(effectType));
		}

		void AudioManager_ReleaseDSPByGroup(int groupType)
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->ReleaseDSPByGroup(static_cast<AudioType>(groupType));
		}

		void AudioManager_ReleaseAllDSPs()
		{
			auto *am = GetAudioManager();
			if (!am) return;
			am->ReleaseAllDSPs();
		}

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
		void Event_Publish(MonoString *nameStr, MonoString *payloadStr)
		{
			ScriptEvent ev;

			if (nameStr)
			{
				char *cName = mono_string_to_utf8(nameStr);
				if (cName) { ev.name = cName; mono_free(cName); }
			}

			if (payloadStr)
			{
				char *cPayload = mono_string_to_utf8(payloadStr);
				if (cPayload) { ev.payload = cPayload; mono_free(cPayload); }
			}

			EventSystem::Instance().Queue(ev);
		}

		// =====================================================================
		// Quaternion helpers
		// =====================================================================
		void Quat_FromAxisAngle(glm::vec3 *axis, float angleRadians, glm::quat *outQuat)
		{
			if (!axis || !outQuat) return;
			*outQuat = glm::angleAxis(angleRadians, glm::normalize(*axis));
		}

		void Quat_GetForward(glm::quat *quat, glm::vec3 *outForward)
		{
			if (!quat || !outForward) return;
			*outForward = glm::rotate(*quat, glm::vec3(0.0f, 0.0f, -1.0f));
		}

		void Quat_GetRight(glm::quat *quat, glm::vec3 *outRight)
		{
			if (!quat || !outRight) return;
			*outRight = glm::rotate(*quat, glm::vec3(1.0f, 0.0f, 0.0f));
		}

		void Quat_GetUp(glm::quat *quat, glm::vec3 *outUp)
		{
			if (!quat || !outUp) return;
			*outUp = glm::rotate(*quat, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		void Quat_RotateVector(glm::quat *quat, glm::vec3 *vec, glm::vec3 *outVec)
		{
			if (!quat || !vec || !outVec) return;
			*outVec = glm::rotate(*quat, *vec);
		}

		void Quat_Multiply(glm::quat *q1, glm::quat *q2, glm::quat *outQuat)
		{
			if (!q1 || !q2 || !outQuat) return;
			*outQuat = (*q1) * (*q2);
		}

		void Quat_Slerp(glm::quat *q1, glm::quat *q2, float t, glm::quat *outQuat)
		{
			if (!q1 || !q2 || !outQuat) return;
			*outQuat = glm::slerp(*q1, *q2, t);
		}

		void Quat_Inverse(glm::quat *quat, glm::quat *outQuat)
		{
			if (!quat || !outQuat) return;
			*outQuat = glm::inverse(*quat);
		}

		void Quat_ToEuler(glm::quat *quat, glm::vec3 *outEuler)
		{
			if (!quat || !outEuler) return;
			*outEuler = glm::eulerAngles(*quat);
		}

		void Quat_FromEuler(glm::vec3 *euler, glm::quat *outQuat)
		{
			if (!euler || !outQuat) return;
			*outQuat = glm::quat(*euler);
		}

		void Quat_Normalize(glm::quat *quat, glm::quat *outQuat)
		{
			if (!quat || !outQuat) return;
			*outQuat = glm::normalize(*quat);
		}

		float Quat_Length(glm::quat *quat)
		{
			if (!quat) return 0.0f;
			return glm::length(*quat);
		}

		float Quat_Dot(glm::quat *q1, glm::quat *q2)
		{
			if (!q1 || !q2) return 0.0f;
			return glm::dot(*q1, *q2);
		}
	} // namespace InternalCalls
} // namespace Engine

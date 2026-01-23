/**************************************************************************
 * @file
 * InternalCalls.h
 * @author
 * Varying amounts by team
 * @date
 * 2026/01/06 (YYYY/MM/DD)
 * @brief
 * This header declares the native internal-call bridge used by the Mono
 * scripting layer to invoke engine functionality.
 *
 * InternalCalls are registered with Mono and exposed to C# scripts. Most
 * functions assume a valid "current scene" context has been set by the engine
 * before any calls are made.
***************************************************************************/

#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

namespace Engine
{
	// Forward declarations for context setters (avoid heavy includes in the header)
	class Scene;
	class Input;
	class AudioManager;

	namespace InternalCalls
	{
		// ===== Scripting context (set by ScriptSystem / MonoScriptEngine) =====
		/**************************************************************************
		 * @brief
		 * Sets the scene context used by InternalCalls.
		 * @param scene
		 * Pointer to the active scene used for subsequent internal calls.
		***************************************************************************/
		void         SetCurrentScene(Scene *scene);
		/**************************************************************************
		 * @brief
		 * Sets the input system context used by InternalCalls.
		 * @param input
		 * Pointer to the engine input system used for subsequent internal calls.
		***************************************************************************/
		void         SetInputSystem(Input *input);
		/**************************************************************************
		 * @brief
		 * Sets the audio manager context used by InternalCalls.
		 * @param audioManager
		 * Pointer to the engine audio manager used for subsequent internal calls.
		***************************************************************************/
		void         SetAudioManager(AudioManager *audioManager);
		AudioManager *GetAudioManager();

		// ===== Scene / Entity lifecycle =====
		/**************************************************************************
		 * @brief
		 * Creates a new entity in the current scene.
		 * @param nameStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Entity identifier (0 if not found / invalid).
		***************************************************************************/
		uint64_t Scene_CreateEntity(MonoString *nameStr);
		/**************************************************************************
		 * @brief
		 * Destroys an entity in the current scene.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void     Scene_DestroyEntity(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Attaches a managed script class to the specified entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param classFullNameStr
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void     Entity_AddScript(uint64_t entityID, MonoString *classFullNameStr);

		// ===== Logging =====
		/**************************************************************************
		 * @brief
		 * Writes a message to the engine logging system.
		 * @param message
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void LogMessage(MonoString *message);
		/**************************************************************************
		 * @brief
		 * Writes a message to the engine logging system.
		 * @param message
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void LogError(MonoString *message);
		/**************************************************************************
		 * @brief
		 * Writes a message to the engine logging system.
		 * @param message
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void LogWarning(MonoString *message);

		// ===== Entity utilities =====
		/**************************************************************************
		 * @brief
		 * Finds an entity by name in the current scene.
		 * @param nameString
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Entity identifier (0 if not found / invalid).
		***************************************************************************/
		uint64_t Scene_FindEntityByName(MonoString *nameString);
		/**************************************************************************
		 * @brief
		 * Returns the native entity ID associated with a managed Entity wrapper.
		 * @param entityObj
		 * Managed object provided by the scripting runtime (MonoObject*).
		 * @return
		 * Unsigned integer result.
		***************************************************************************/
		uint64_t Entity_GetEntityID(MonoObject *entityObj);
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
		bool     Entity_HasComponent(uint64_t entityID, MonoReflectionType *componentType);

		// ===== Transform =====
		/**************************************************************************
		 * @brief
		 * Retrieves a transform property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param outPosition
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Transform_GetPosition(uint64_t entityID, glm::vec3 *outPosition);
		/**************************************************************************
		 * @brief
		 * Sets a transform property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param position
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Transform_SetPosition(uint64_t entityID, glm::vec3 *position);

		/**************************************************************************
		 * @brief
		 * Retrieves a transform property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param outRotation
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Transform_GetRotation(uint64_t entityID, glm::quat *outRotation);
		/**************************************************************************
		 * @brief
		 * Sets a transform property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param rotation
		 * Pointer/reference to a quaternion value.
		***************************************************************************/
		void Transform_SetRotation(uint64_t entityID, glm::quat *rotation);

		/**************************************************************************
		 * @brief
		 * Retrieves a transform property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param outScale
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Transform_GetScale(uint64_t entityID, glm::vec3 *outScale);
		/**************************************************************************
		 * @brief
		 * Sets a transform property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param scale
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Transform_SetScale(uint64_t entityID, glm::vec3 *scale);

		/**************************************************************************
		 * @brief
		 * Retrieves a transform property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested integer value.
		***************************************************************************/
		int  Transform_GetParent(uint64_t entityID);

		// ===== Input =====
		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param keyCode
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Input_IsKeyPressed(int keyCode);
		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param keyCode
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Input_IsKeyReleased(int keyCode);
		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param button
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool Input_IsMouseButtonPressed(int button);

		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param outPosition
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Input_GetMousePosition(glm::vec2 *outPosition);
		/**************************************************************************
		 * @brief
		 * Queries the current input state from the engine input system.
		 * @param outX
		 * Output parameter that receives the requested value.
		 * @param outY
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void Input_GetMouseDelta(float *outX, float *outY);

		/**************************************************************************
		 * @brief
		 * Sets the visibility of the system cursor.
		 * @param visible
		 * Input parameter.
		 **************************************************************************/
		void Input_SetCursorVisible(bool visible);

		/**************************************************************************
		 * @brief
		 * Gets the visibility of the system cursor.
		 * @return
		 * True if the cursor is visible; otherwise false.
		 **************************************************************************/
		bool Input_GetCursorVisible();

		// ===== Event System =====
		/**************************************************************************
		 * @brief
		 * Publishes an event into the engine event system.
		 * @param nameStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @param payloadStr
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void Event_Publish(MonoString *nameStr, MonoString *payloadStr);

		// ===== Prefab instantiation =====
		/**************************************************************************
		 * @brief
		 * Instantiates a prefab into the current scene.
		 * @param prefabPathStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Unsigned integer result.
		***************************************************************************/
		uint64_t Prefab_Instantiate(MonoString *prefabPathStr);
		/**************************************************************************
		 * @brief
		 * Instantiates a prefab into the current scene.
		 * @param prefabPathStr
		 * Managed string provided by the scripting runtime (MonoString*).
		 * @return
		 * Unsigned integer result.
		***************************************************************************/
		uint64_t Prefab_InstantiateScene(MonoString *prefabPathStr);
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
			bool        isScenePrefab
		);

		// ===== Physics bindings =====
		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddRigidBody(uint64_t entityID);

		// ---- Rigidbody velocity access (ECS-level) ----
		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Result value.
		***************************************************************************/
		glm::vec3 Rigidbody_GetVelocity(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param inVel
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Rigidbody_SetVelocity(uint64_t entityID, glm::vec3 *inVel);
		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param delta
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void Rigidbody_AddVelocity(uint64_t entityID, glm::vec3 *delta);

		// ---- Rigidbody scalar properties ----
		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Rigidbody_GetMass(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mass
		 * Input parameter.
		***************************************************************************/
		void  Rigidbody_SetMass(uint64_t entityID, float mass);

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Rigidbody_GetIsKinematic(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param isKinematic
		 * Input parameter.
		***************************************************************************/
		void  Rigidbody_SetIsKinematic(uint64_t entityID, bool isKinematic);

		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Rigidbody_GetUseGravity(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a rigidbody property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param useGravity
		 * Input parameter.
		***************************************************************************/
		void  Rigidbody_SetUseGravity(uint64_t entityID, bool useGravity);

		// ---- Rigidbody helpers ----
		/**************************************************************************
		 * @brief
		 * Gets a rigidbody property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Rigidbody_GetSpeed(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Rigidbody_IsMoving(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Rigidbody_IsStatic(uint64_t entityID);

		// ---- Rigidbody forces ----
		/**************************************************************************
		 * @brief
		 * Applies a force/impulse to the entity's rigidbody.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param force
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void  Rigidbody_AddForce(uint64_t entityID, glm::vec3 *force);
		/**************************************************************************
		 * @brief
		 * Invokes a rigidbody/physics operation on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void  Rigidbody_Stop(uint64_t entityID);

		// ---- Collision events (via PhysicsAPI only) ----
		/**************************************************************************
		 * @brief
		 * Queries global physics system state or collision results.
		***************************************************************************/
		void Physics_EnableCollisionEvents();
		/**************************************************************************
		 * @brief
		 * Queries global physics system state or collision results.
		***************************************************************************/
		void Physics_BeginCollisionFrame();
		/**************************************************************************
		 * @brief
		 * Queries global physics system state or collision results.
		 * @return
		 * Requested integer value.
		***************************************************************************/
		int  Physics_GetCollisionCount();
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
		void Physics_GetCollisionPair(int index, uint32_t *a, uint32_t *b);

		// ===== Entity / component adders =====
		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddTag(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddCamera(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddAudio(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Adds the specified component to the entity if it does not already exist.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void Entity_AddMeshRenderer(uint64_t entityID);

		// ===== TagComponent =====
		MonoString *Tag_GetTag(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Gets or sets the entity tag.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param tag
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void        Tag_SetTag(uint64_t entityID, MonoString *tag);
		MonoArray *Scene_FindEntitiesByTag(MonoString *tagString);

		// ===== CameraComponent =====
		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Camera_GetEnabled(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param enabled
		 * Input parameter.
		***************************************************************************/
		void  Camera_SetEnabled(uint64_t entityID, bool enabled);
		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Camera_GetPrimary(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param primary
		 * Input parameter.
		***************************************************************************/
		void  Camera_SetPrimary(uint64_t entityID, bool primary);
		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Camera_GetFOV(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param fov
		 * Input parameter.
		***************************************************************************/
		void  Camera_SetFOV(uint64_t entityID, float fov);
		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Camera_GetNear(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param nearPlane
		 * Input parameter.
		***************************************************************************/
		void  Camera_SetNear(uint64_t entityID, float nearPlane);
		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Camera_GetFar(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param farPlane
		 * Input parameter.
		***************************************************************************/
		void  Camera_SetFar(uint64_t entityID, float farPlane);
		/**************************************************************************
		 * @brief
		 * Gets a camera property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param outTarget
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Camera_GetTarget(uint64_t entityID, glm::vec3 *outTarget);
		/**************************************************************************
		 * @brief
		 * Sets a camera property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param inTarget
		 * Pointer/reference to a vector value.
		***************************************************************************/
		void  Camera_SetTarget(uint64_t entityID, glm::vec3 *inTarget);

		// ===== MeshRendererComponent =====
		/**************************************************************************
		 * @brief
		 * Gets a mesh renderer property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool MeshRenderer_GetVisible(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a mesh renderer property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param visible
		 * Input parameter.
		***************************************************************************/
		void MeshRenderer_SetVisible(uint64_t entityID, bool visible);
		/**************************************************************************
		 * @brief
		 * Gets a mesh renderer property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool MeshRenderer_GetShadowCast(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a mesh renderer property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param cast
		 * Input parameter.
		***************************************************************************/
		void MeshRenderer_SetShadowCast(uint64_t entityID, bool cast);
		/**************************************************************************
		 * @brief
		 * Gets a mesh renderer property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool MeshRenderer_GetShadowReceive(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a mesh renderer property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param receive
		 * Input parameter.
		***************************************************************************/
		void MeshRenderer_SetShadowReceive(uint64_t entityID, bool receive);
		/**************************************************************************
		 * @brief
		 * Gets a mesh renderer property from the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool MeshRenderer_GetGlobalIlluminate(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets a mesh renderer property on the entity.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param gi
		 * Input parameter.
		***************************************************************************/
		void MeshRenderer_SetGlobalIlluminate(uint64_t entityID, bool gi);

		// ===== AudioComponent =====
		/**************************************************************************
		 * @brief
		 * Starts playback of the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void  Audio_Play(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Stops playback of the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void  Audio_Stop(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Pauses playback of the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		***************************************************************************/
		void  Audio_Pause(uint64_t entityID);

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetVolume(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param volume
		 * New value to apply.
		***************************************************************************/
		void  Audio_SetVolume(uint64_t entityID, float volume);

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetPitch(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param pitch
		 * New value to apply.
		***************************************************************************/
		void  Audio_SetPitch(uint64_t entityID, float pitch);

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Audio_GetLoop(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param loop
		 * New boolean value to apply.
		***************************************************************************/
		void  Audio_SetLoop(uint64_t entityID, bool loop);

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Audio_GetMute(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mute
		 * New boolean value to apply.
		***************************************************************************/
		void  Audio_SetMute(uint64_t entityID, bool mute);

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  Audio_GetIs3D(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param is3d
		 * New boolean value to apply.
		***************************************************************************/
		void  Audio_SetIs3D(uint64_t entityID, bool is3d);

		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param path
		 * Managed string provided by the scripting runtime (MonoString*).
		***************************************************************************/
		void  Audio_SetFile(uint64_t entityID, MonoString *path);

		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetMinDistance(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param minDist
		 * New value to apply.
		***************************************************************************/
		void  Audio_SetMinDistance(uint64_t entityID, float minDist);
		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetMaxDistance(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param maxDist
		 * New value to apply.
		***************************************************************************/
		void  Audio_SetMaxDistance(uint64_t entityID, float maxDist);
		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested integer value.
		***************************************************************************/
		int   Audio_GetRolloffMode(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mode
		 * New mode value to apply.
		***************************************************************************/
		void  Audio_SetRolloffMode(uint64_t entityID, int mode);
		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetDopplerLevel(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param level
		 * New value to apply.
		***************************************************************************/
		void  Audio_SetDopplerLevel(uint64_t entityID, float level);
		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetPan2D(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param pan
		 * New value to apply.
		***************************************************************************/
		void  Audio_SetPan2D(uint64_t entityID, float pan);
		/**************************************************************************
		 * @brief
		 * Gets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Audio_GetReverbMix(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Sets an audio property for the entity's audio component.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @param mix
		 * New value to apply.
		***************************************************************************/
		void  Audio_SetReverbMix(uint64_t entityID, float mix);

		// ===== AudioManager =====
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param volume
		 * New value to apply.
		***************************************************************************/
		void  AudioManager_SetGroupVolume(int groupType, float volume);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float AudioManager_GetGroupVolume(int groupType);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param pitch
		 * New value to apply.
		***************************************************************************/
		void  AudioManager_SetGroupPitch(int groupType, float pitch);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float AudioManager_GetGroupPitch(int groupType);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param mute
		 * New boolean value to apply.
		***************************************************************************/
		void  AudioManager_SetGroupMute(int groupType, bool mute);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool  AudioManager_IsGroupMuted(int groupType);

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param pause
		 * Input parameter.
		***************************************************************************/
		void  AudioManager_PauseGroup(int groupType, bool pause);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param pause
		 * Input parameter.
		***************************************************************************/
		void  AudioManager_PauseAll(bool pause);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		***************************************************************************/
		void  AudioManager_StopByType(int groupType);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		***************************************************************************/
		void  AudioManager_StopAll();

		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param effectType
		 * Input parameter.
		***************************************************************************/
		void  AudioManager_CreateDSP(int groupType, int effectType);
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
		void  AudioManager_EnableDSP(int groupType, int effectType, bool enable);
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
		void  AudioManager_SetDSPParameter(int groupType, int effectType, int paramIndex, float value);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		 * @param effectType
		 * Input parameter.
		***************************************************************************/
		void  AudioManager_ReleaseSpecificDSPinGroup(int groupType, int effectType);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		 * @param groupType
		 * Input parameter.
		***************************************************************************/
		void  AudioManager_ReleaseDSPByGroup(int groupType);
		/**************************************************************************
		 * @brief
		 * Invokes an AudioManager operation.
		***************************************************************************/
		void  AudioManager_ReleaseAllDSPs();

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
		void  AudioManager_SetListenerAttributes(glm::vec3 *position, glm::vec3 *forward,
			glm::vec3 *up, glm::vec3 *velocity);

		// ===== Component presence helpers =====
		/**************************************************************************
		 * @brief
		 * Internal engine <-> scripting bridge call.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool EntityHasCamera(uint64_t entityID);
		/**************************************************************************
		 * @brief
		 * Internal engine <-> scripting bridge call.
		 * @param entityID
		 * Entity identifier (stored as uint64_t; corresponds to an entt::entity).
		 * @return
		 * True if the condition is met; otherwise false.
		***************************************************************************/
		bool EntityHasRigidBody(uint64_t entityID);

		// ===== Quaternion helpers (GLM exposed) =====
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
		void  Quat_FromAxisAngle(glm::vec3 *axis, float angleRadians, glm::quat *outQuat);
		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outForward
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Quat_GetForward(glm::quat *quat, glm::vec3 *outForward);
		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outRight
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Quat_GetRight(glm::quat *quat, glm::vec3 *outRight);
		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outUp
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Quat_GetUp(glm::quat *quat, glm::vec3 *outUp);
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
		void  Quat_RotateVector(glm::quat *quat, glm::vec3 *vec, glm::vec3 *outVec);

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
		void  Quat_Multiply(glm::quat *q1, glm::quat *q2, glm::quat *outQuat);
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
		void  Quat_Slerp(glm::quat *q1, glm::quat *q2, float t, glm::quat *outQuat);
		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Quat_Inverse(glm::quat *quat, glm::quat *outQuat);

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outEuler
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Quat_ToEuler(glm::quat *quat, glm::vec3 *outEuler);
		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param euler
		 * Pointer/reference to a vector value.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Quat_FromEuler(glm::vec3 *euler, glm::quat *outQuat);

		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @param outQuat
		 * Output parameter that receives the requested value.
		***************************************************************************/
		void  Quat_Normalize(glm::quat *quat, glm::quat *outQuat);
		/**************************************************************************
		 * @brief
		 * Performs a quaternion math operation.
		 * @param quat
		 * Pointer/reference to a quaternion value.
		 * @return
		 * Requested floating-point value.
		***************************************************************************/
		float Quat_Length(glm::quat *quat);
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
		float Quat_Dot(glm::quat *q1, glm::quat *q2);

		/**************************************************************************
		 * @brief
		 * Checks if a point lies within a game object entity.
		 * @param entityId
		 * The entity id of the game object to check.
		 * @param point
		 * The point of interest.
		 * @return
		 * True if point lies within game object, else false.
		***************************************************************************/
		bool CollisionSystem2D_IsPointInEntity(uint64_t entityId, glm::vec2* point);

		void  RNG_Seed(std::uint32_t seed);
		int   RNG_RandInt(int min, int max);
		float RNG_RandFloat(float min, float max);
		bool  RNG_RandBool();

		// =====================================================================
		// Global context (set from ScriptSystem via MonoScriptEngine helpers)
		// =====================================================================
		static Scene *s_CurrentScene = nullptr;
		static Input *s_InputSystem = nullptr;
		static AudioManager *s_AudioManager = nullptr;


		// ========================================
// File I/O
// ========================================

/**
 * @brief Checks if a file exists at the given path.
 * @param pathStr Managed string provided by the scripting runtime (MonoString*).
 * @return True if the file exists, otherwise false.
 */
		bool FileExists(MonoString* pathStr);

		/**
		 * @brief Reads the entire content of a text file.
		 * @param pathStr Managed string provided by the scripting runtime (MonoString*).
		 * @return Managed string containing file content, or empty string on failure.
		 */
		MonoString* FileReadAllText(MonoString* pathStr);

		/**
		 * @brief Writes text content to a file, creating directories if needed.
		 * @param pathStr Managed string provided by the scripting runtime (MonoString*).
		 * @param contentStr Managed string provided by the scripting runtime (MonoString*).
		 * @return True if write succeeded, otherwise false.
		 */
		bool FileWriteAllText(MonoString* pathStr, MonoString* contentStr);


	} // namespace InternalCalls
} // namespace Engine

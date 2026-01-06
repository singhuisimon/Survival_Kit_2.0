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
		void         SetCurrentScene(Scene *scene);
		void         SetInputSystem(Input *input);
		void         SetAudioManager(AudioManager *audioManager);
		AudioManager *GetAudioManager();

		// ===== Scene / Entity lifecycle =====
		uint64_t Scene_CreateEntity(MonoString *nameStr);
		void     Scene_DestroyEntity(uint64_t entityID);
		void     Entity_AddScript(uint64_t entityID, MonoString *classFullNameStr);

		// ===== Logging =====
		void LogMessage(MonoString *message);
		void LogError(MonoString *message);
		void LogWarning(MonoString *message);

		// ===== Entity utilities =====
		uint64_t Scene_FindEntityByName(MonoString *nameString);
		uint64_t Entity_GetEntityID(MonoObject *entityObj);
		bool     Entity_HasComponent(uint64_t entityID, MonoReflectionType *componentType);

		// ===== Transform =====
		void Transform_GetPosition(uint64_t entityID, glm::vec3 *outPosition);
		void Transform_SetPosition(uint64_t entityID, glm::vec3 *position);

		void Transform_GetRotation(uint64_t entityID, glm::quat *outRotation);
		void Transform_SetRotation(uint64_t entityID, glm::quat *rotation);

		void Transform_GetScale(uint64_t entityID, glm::vec3 *outScale);
		void Transform_SetScale(uint64_t entityID, glm::vec3 *scale);

		int  Transform_GetParent(uint64_t entityID);

		// ===== Input =====
		bool Input_IsKeyPressed(int keyCode);
		bool Input_IsKeyReleased(int keyCode);
		bool Input_IsMouseButtonPressed(int button);

		void Input_GetMousePosition(glm::vec2 *outPosition);
		void Input_GetMouseDelta(float *outX, float *outY);

		// ===== Event System =====
		void Event_Publish(MonoString *nameStr, MonoString *payloadStr);

		// ===== Prefab instantiation =====
		uint64_t Prefab_Instantiate(MonoString *prefabPathStr);
		uint64_t Prefab_InstantiateScene(MonoString *prefabPathStr);
		uint64_t Prefab_InstantiateWithTransform(
			MonoString *prefabPathStr,
			glm::vec3 *position,
			glm::quat *rotation,
			glm::vec3 *scale,
			bool        isScenePrefab
		);

		// ===== Physics bindings =====
		void Entity_AddRigidBody(uint64_t entityID);

		// ---- Rigidbody velocity access (ECS-level) ----
		glm::vec3 Rigidbody_GetVelocity(uint64_t entityID);
		void Rigidbody_SetVelocity(uint64_t entityID, glm::vec3 *inVel);
		void Rigidbody_AddVelocity(uint64_t entityID, glm::vec3 *delta);

		// ---- Rigidbody scalar properties ----
		float Rigidbody_GetMass(uint64_t entityID);
		void  Rigidbody_SetMass(uint64_t entityID, float mass);

		bool  Rigidbody_GetIsKinematic(uint64_t entityID);
		void  Rigidbody_SetIsKinematic(uint64_t entityID, bool isKinematic);

		bool  Rigidbody_GetUseGravity(uint64_t entityID);
		void  Rigidbody_SetUseGravity(uint64_t entityID, bool useGravity);

		// ---- Rigidbody helpers ----
		float Rigidbody_GetSpeed(uint64_t entityID);
		bool  Rigidbody_IsMoving(uint64_t entityID);
		bool  Rigidbody_IsStatic(uint64_t entityID);

		// ---- Rigidbody forces ----
		void  Rigidbody_AddForce(uint64_t entityID, glm::vec3 *force);
		void  Rigidbody_Stop(uint64_t entityID);

		// ---- Collision events (via PhysicsAPI only) ----
		void Physics_EnableCollisionEvents();
		void Physics_BeginCollisionFrame();
		int  Physics_GetCollisionCount();
		void Physics_GetCollisionPair(int index, uint32_t *a, uint32_t *b);

		// ===== Entity / component adders =====
		void Entity_AddTag(uint64_t entityID);
		void Entity_AddCamera(uint64_t entityID);
		void Entity_AddAudio(uint64_t entityID);
		void Entity_AddMeshRenderer(uint64_t entityID);

		// ===== TagComponent =====
		MonoString *Tag_GetTag(uint64_t entityID);
		void        Tag_SetTag(uint64_t entityID, MonoString *tag);
		MonoArray *Scene_FindEntitiesByTag(MonoString *tagString);

		// ===== CameraComponent =====
		bool  Camera_GetEnabled(uint64_t entityID);
		void  Camera_SetEnabled(uint64_t entityID, bool enabled);
		bool  Camera_GetPrimary(uint64_t entityID);
		void  Camera_SetPrimary(uint64_t entityID, bool primary);
		float Camera_GetFOV(uint64_t entityID);
		void  Camera_SetFOV(uint64_t entityID, float fov);
		float Camera_GetNear(uint64_t entityID);
		void  Camera_SetNear(uint64_t entityID, float nearPlane);
		float Camera_GetFar(uint64_t entityID);
		void  Camera_SetFar(uint64_t entityID, float farPlane);
		void  Camera_GetTarget(uint64_t entityID, glm::vec3 *outTarget);
		void  Camera_SetTarget(uint64_t entityID, glm::vec3 *inTarget);

		// ===== MeshRendererComponent =====
		bool MeshRenderer_GetVisible(uint64_t entityID);
		void MeshRenderer_SetVisible(uint64_t entityID, bool visible);
		bool MeshRenderer_GetShadowCast(uint64_t entityID);
		void MeshRenderer_SetShadowCast(uint64_t entityID, bool cast);
		bool MeshRenderer_GetShadowReceive(uint64_t entityID);
		void MeshRenderer_SetShadowReceive(uint64_t entityID, bool receive);
		bool MeshRenderer_GetGlobalIlluminate(uint64_t entityID);
		void MeshRenderer_SetGlobalIlluminate(uint64_t entityID, bool gi);

		// ===== AudioComponent =====
		void  Audio_Play(uint64_t entityID);
		void  Audio_Stop(uint64_t entityID);
		void  Audio_Pause(uint64_t entityID);

		float Audio_GetVolume(uint64_t entityID);
		void  Audio_SetVolume(uint64_t entityID, float volume);

		float Audio_GetPitch(uint64_t entityID);
		void  Audio_SetPitch(uint64_t entityID, float pitch);

		bool  Audio_GetLoop(uint64_t entityID);
		void  Audio_SetLoop(uint64_t entityID, bool loop);

		bool  Audio_GetMute(uint64_t entityID);
		void  Audio_SetMute(uint64_t entityID, bool mute);

		bool  Audio_GetIs3D(uint64_t entityID);
		void  Audio_SetIs3D(uint64_t entityID, bool is3d);

		void  Audio_SetFile(uint64_t entityID, MonoString *path);

		float Audio_GetMinDistance(uint64_t entityID);
		void  Audio_SetMinDistance(uint64_t entityID, float minDist);
		float Audio_GetMaxDistance(uint64_t entityID);
		void  Audio_SetMaxDistance(uint64_t entityID, float maxDist);
		int   Audio_GetRolloffMode(uint64_t entityID);
		void  Audio_SetRolloffMode(uint64_t entityID, int mode);
		float Audio_GetDopplerLevel(uint64_t entityID);
		void  Audio_SetDopplerLevel(uint64_t entityID, float level);
		float Audio_GetPan2D(uint64_t entityID);
		void  Audio_SetPan2D(uint64_t entityID, float pan);
		float Audio_GetReverbMix(uint64_t entityID);
		void  Audio_SetReverbMix(uint64_t entityID, float mix);

		// ===== AudioManager =====
		void  AudioManager_SetGroupVolume(int groupType, float volume);
		float AudioManager_GetGroupVolume(int groupType);
		void  AudioManager_SetGroupPitch(int groupType, float pitch);
		float AudioManager_GetGroupPitch(int groupType);
		void  AudioManager_SetGroupMute(int groupType, bool mute);
		bool  AudioManager_IsGroupMuted(int groupType);

		void  AudioManager_PauseGroup(int groupType, bool pause);
		void  AudioManager_PauseAll(bool pause);
		void  AudioManager_StopByType(int groupType);
		void  AudioManager_StopAll();

		void  AudioManager_CreateDSP(int groupType, int effectType);
		void  AudioManager_EnableDSP(int groupType, int effectType, bool enable);
		void  AudioManager_SetDSPParameter(int groupType, int effectType, int paramIndex, float value);
		void  AudioManager_ReleaseSpecificDSPinGroup(int groupType, int effectType);
		void  AudioManager_ReleaseDSPByGroup(int groupType);
		void  AudioManager_ReleaseAllDSPs();

		void  AudioManager_SetListenerAttributes(glm::vec3 *position, glm::vec3 *forward,
			glm::vec3 *up, glm::vec3 *velocity);

		// ===== Component presence helpers =====
		bool EntityHasCamera(uint64_t entityID);
		bool EntityHasRigidBody(uint64_t entityID);

		// ===== Quaternion helpers (GLM exposed) =====
		void  Quat_FromAxisAngle(glm::vec3 *axis, float angleRadians, glm::quat *outQuat);
		void  Quat_GetForward(glm::quat *quat, glm::vec3 *outForward);
		void  Quat_GetRight(glm::quat *quat, glm::vec3 *outRight);
		void  Quat_GetUp(glm::quat *quat, glm::vec3 *outUp);
		void  Quat_RotateVector(glm::quat *quat, glm::vec3 *vec, glm::vec3 *outVec);

		void  Quat_Multiply(glm::quat *q1, glm::quat *q2, glm::quat *outQuat);
		void  Quat_Slerp(glm::quat *q1, glm::quat *q2, float t, glm::quat *outQuat);
		void  Quat_Inverse(glm::quat *quat, glm::quat *outQuat);

		void  Quat_ToEuler(glm::quat *quat, glm::vec3 *outEuler);
		void  Quat_FromEuler(glm::vec3 *euler, glm::quat *outQuat);

		void  Quat_Normalize(glm::quat *quat, glm::quat *outQuat);
		float Quat_Length(glm::quat *quat);
		float Quat_Dot(glm::quat *q1, glm::quat *q2);
	} // namespace InternalCalls
} // namespace Engine

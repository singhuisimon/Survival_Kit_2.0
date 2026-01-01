using System;
using System.Runtime.CompilerServices;

namespace Engine
{
    public static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LogError(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LogWarning(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Scene_FindEntityByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Transform_GetParent(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint[] Scene_FindEntitiesByTag(string tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetPosition(uint entityID, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetPosition(uint entityID, ref Vector3 position);

        // Rotation now uses Quat as the marshalled type
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetRotation(uint entityID, out Quat rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetRotation(uint entityID, ref Quat rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetScale(uint entityID, out Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetScale(uint entityID, ref Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Scene_CreateEntity(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Scene_DestroyEntity(uint entity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddScript(uint entity, string managedClassFullName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddRigidBody(uint entityID);

        // Prefabs
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Prefab_Instantiate(string prefabPath);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Prefab_InstantiateScene(string prefabPath);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint Prefab_InstantiateWithTransform(string prefabPath, ref Vector3 position, ref Quat rotation, ref Vector3 scale, bool isScenePrefab);

        // Rigidbody core properties
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_GetVelocity(uint entityID, out Vector3 vel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetVelocity(uint entityID, ref Vector3 vel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_AddVelocity(uint entityID, ref Vector3 delta);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Rigidbody_GetMass(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetMass(uint entityID, float mass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_GetIsKinematic(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetIsKinematic(uint entityID, bool isKinematic);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_GetUseGravity(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetUseGravity(uint entityID, bool useGravity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Rigidbody_GetSpeed(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_IsMoving(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_IsStatic(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_AddForce(uint entityID, ref Vector3 force);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_Stop(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_EnableCollisionEvents();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_BeginCollisionFrame();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Physics_GetCollisionCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_GetCollisionPair(int index, out uint a, out uint b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddTag(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddCamera(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddAudio(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddMeshRenderer(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string Tag_GetTag(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Tag_SetTag(uint entityID, string tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Camera_GetEnabled(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetEnabled(uint entityID, bool enabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Camera_GetPrimary(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetPrimary(uint entityID, bool primary);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Camera_GetFOV(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetFOV(uint entityID, float fov);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Camera_GetNear(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetNear(uint entityID, float nearPlane);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Camera_GetFar(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetFar(uint entityID, float farPlane);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_GetTarget(uint entityID, out Vector3 target);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetTarget(uint entityID, ref Vector3 target);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetVisible(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetVisible(uint entityID, bool visible);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetShadowCast(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetShadowCast(uint entityID, bool cast);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetShadowReceive(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetShadowReceive(uint entityID, bool receive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetGlobalIlluminate(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetGlobalIlluminate(uint entityID, bool gi);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_Play(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_Stop(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_Pause(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetVolume(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetVolume(uint entityID, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetPitch(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetPitch(uint entityID, float pitch);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Audio_GetLoop(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetLoop(uint entityID, bool loop);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Audio_GetMute(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetMute(uint entityID, bool mute);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Audio_GetIs3D(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetIs3D(uint entityID, bool is3d);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetFile(uint entityID, string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetMinDistance(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetMinDistance(uint entityID, float minDist);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetMaxDistance(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetMaxDistance(uint entityID, float maxDist);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Audio_GetRolloffMode(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetRolloffMode(uint entityID, int mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetDopplerLevel(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetDopplerLevel(uint entityID, float level);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetPan2D(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetPan2D(uint entityID, float pan);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetReverbMix(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetReverbMix(uint entityID, float mix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool EntityHasCamera(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool EntityHasRigidBody(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float AudioManager_GetGroupVolume(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_SetGroupVolume(int groupType, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float AudioManager_GetGroupPitch(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_SetGroupPitch(int groupType, float pitch);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioManager_IsGroupMuted(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_SetGroupMute(int groupType, bool mute);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_PauseGroup(int groupType, bool pause);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_PauseAll(bool pause);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_StopByType(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_StopAll();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_CreateDSP(int groupType, int effectType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_EnableDSP(int groupType, int effectType, bool enable);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_SetDSPParameter(int groupType, int effectType, int paramIndex, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioMaster_ReleaseSpecificDSPinGroup(int groupType, int effectType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_ReleaseDSPByGroup(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_ReleaseAllDSPs();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AudioManager_SetListenerAttributes(ref Vector3 position, ref Vector3 forward, ref Vector3 up, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Event_Publish(string name, string payload);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_GetForward(ref Quat quat, out Vector3 forward);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_GetRight(ref Quat quat, out Vector3 right);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_GetUp(ref Quat quat, out Vector3 up);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_RotateVector(ref Quat quat, ref Vector3 vec, out Vector3 outVec);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_Multiply(ref Quat q1, ref Quat q2, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_FromAxisAngle(ref Vector3 axis, float angleRadians, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_Slerp(ref Quat q1, ref Quat q2, float t, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_Inverse(ref Quat quat, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_ToEuler(ref Quat quat, out Vector3 euler);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_FromEuler(ref Vector3 euler, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quat_Normalize(ref Quat quat, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Quat_Length(ref Quat quat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Quat_Dot(ref Quat q1, ref Quat q2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Input_GetMouseDelta(out float deltaX, out float deltaY);
    }

    // ======================
        // Logging
        // ======================
        public static void Log(string message) => InternalCalls.Log(message);
        public static void LogError(string message) => InternalCalls.LogError(message);
        public static void LogWarning(string message) => InternalCalls.LogWarning(message);

        // ======================
        // Scene
        // ======================
        public static uint SceneFindEntityByName(string name) => InternalCalls.Scene_FindEntityByName(name);
        public static uint SceneCreateEntity(string name) => InternalCalls.Scene_CreateEntity(name);
        public static void SceneDestroyEntity(uint entity) => InternalCalls.Scene_DestroyEntity(entity);
        public static uint[] SceneFindEntitiesByTag(string tag) => InternalCalls.Scene_FindEntitiesByTag(tag);

        // ======================
        // Entity / Script
        // ======================
        public static void EntityAddScript(uint entity, string managedClassFullName)
            => InternalCalls.Entity_AddScript(entity, managedClassFullName);

        public static bool EntityHasCamera(uint entityID) => InternalCalls.EntityHasCamera(entityID);
        public static bool EntityHasRigidBody(uint entityID) => InternalCalls.EntityHasRigidBody(entityID);

        // ======================
        // Transform
        // ======================
        public static uint TransformGetParent(uint entityID) => InternalCalls.Transform_GetParent(entityID);

        public static Vector3 TransformGetPosition(uint entityID)
        {
            InternalCalls.Transform_GetPosition(entityID, out Vector3 p);
            return p;
        }

        public static void TransformSetPosition(uint entityID, Vector3 position)
        {
            InternalCalls.Transform_SetPosition(entityID, ref position);
        }

        public static Quat TransformGetRotation(uint entityID)
        {
            InternalCalls.Transform_GetRotation(entityID, out Quat q);
            return q;
        }

        public static void TransformSetRotation(uint entityID, Quat rotation)
        {
            InternalCalls.Transform_SetRotation(entityID, ref rotation);
        }

        public static Vector3 TransformGetScale(uint entityID)
        {
            InternalCalls.Transform_GetScale(entityID, out Vector3 s);
            return s;
        }

        public static void TransformSetScale(uint entityID, Vector3 scale)
        {
            InternalCalls.Transform_SetScale(entityID, ref scale);
        }

        // ======================
        // Input
        // ======================
        public static void InputGetMouseDelta(out float deltaX, out float deltaY) 
            => InternalCalls.Input_GetMouseDelta(out deltaX, out deltaY);

        // ======================
        // Tags / Components
        // ======================
        public static void EntityAddTag(uint entityID) => InternalCalls.Entity_AddTag(entityID);
        public static void EntityAddCamera(uint entityID) => InternalCalls.Entity_AddCamera(entityID);
        public static void EntityAddAudio(uint entityID) => InternalCalls.Entity_AddAudio(entityID);
        public static void EntityAddMeshRenderer(uint entityID) => InternalCalls.Entity_AddMeshRenderer(entityID);

        public static string TagGetTag(uint entityID) => InternalCalls.Tag_GetTag(entityID);
        public static void TagSetTag(uint entityID, string tag) => InternalCalls.Tag_SetTag(entityID, tag);

        // ======================
        // Prefabs
        // ======================
        public static uint PrefabInstantiate(string prefabPath) => InternalCalls.Prefab_Instantiate(prefabPath);
        public static uint PrefabInstantiateScene(string prefabPath) => InternalCalls.Prefab_InstantiateScene(prefabPath);

        public static uint PrefabInstantiateWithTransform(
            string prefabPath,
            Vector3 position,
            Quat rotation,
            Vector3 scale,
            bool isScenePrefab)
        {
            return InternalCalls.Prefab_InstantiateWithTransform(prefabPath, ref position, ref rotation, ref scale, isScenePrefab);
        }

        // ======================
        // Rigidbody
        // ======================
        public static void EntityAddRigidBody(uint entityID) => InternalCalls.Entity_AddRigidBody(entityID);

        public static Vector3 RigidbodyGetVelocity(uint entityID)
        {
            InternalCalls.Rigidbody_GetVelocity(entityID, out Vector3 v);
            return v;
        }

        public static void RigidbodySetVelocity(uint entityID, Vector3 vel)
        {
            InternalCalls.Rigidbody_SetVelocity(entityID, ref vel);
        }

        public static void RigidbodyAddVelocity(uint entityID, Vector3 delta)
        {
            InternalCalls.Rigidbody_AddVelocity(entityID, ref delta);
        }

        public static float RigidbodyGetMass(uint entityID) => InternalCalls.Rigidbody_GetMass(entityID);
        public static void RigidbodySetMass(uint entityID, float mass) => InternalCalls.Rigidbody_SetMass(entityID, mass);

        public static bool RigidbodyGetIsKinematic(uint entityID) => InternalCalls.Rigidbody_GetIsKinematic(entityID);
        public static void RigidbodySetIsKinematic(uint entityID, bool isKinematic) => InternalCalls.Rigidbody_SetIsKinematic(entityID, isKinematic);

        public static bool RigidbodyGetUseGravity(uint entityID) => InternalCalls.Rigidbody_GetUseGravity(entityID);
        public static void RigidbodySetUseGravity(uint entityID, bool useGravity) => InternalCalls.Rigidbody_SetUseGravity(entityID, useGravity);

        public static float RigidbodyGetSpeed(uint entityID) => InternalCalls.Rigidbody_GetSpeed(entityID);
        public static bool RigidbodyIsMoving(uint entityID) => InternalCalls.Rigidbody_IsMoving(entityID);
        public static bool RigidbodyIsStatic(uint entityID) => InternalCalls.Rigidbody_IsStatic(entityID);

        public static void RigidbodyAddForce(uint entityID, Vector3 force)
        {
            InternalCalls.Rigidbody_AddForce(entityID, ref force);
        }

        public static void RigidbodyStop(uint entityID) => InternalCalls.Rigidbody_Stop(entityID);

        // ======================
        // Physics collision events
        // ======================
        public static void PhysicsEnableCollisionEvents() => InternalCalls.Physics_EnableCollisionEvents();
        public static void PhysicsBeginCollisionFrame() => InternalCalls.Physics_BeginCollisionFrame();
        public static int PhysicsGetCollisionCount() => InternalCalls.Physics_GetCollisionCount();

        public static void PhysicsGetCollisionPair(int index, out uint a, out uint b)
            => InternalCalls.Physics_GetCollisionPair(index, out a, out b);

        // ======================
        // Camera
        // ======================
        public static bool CameraGetEnabled(uint entityID) => InternalCalls.Camera_GetEnabled(entityID);
        public static void CameraSetEnabled(uint entityID, bool enabled) => InternalCalls.Camera_SetEnabled(entityID, enabled);

        public static bool CameraGetPrimary(uint entityID) => InternalCalls.Camera_GetPrimary(entityID);
        public static void CameraSetPrimary(uint entityID, bool primary) => InternalCalls.Camera_SetPrimary(entityID, primary);

        public static float CameraGetFOV(uint entityID) => InternalCalls.Camera_GetFOV(entityID);
        public static void CameraSetFOV(uint entityID, float fov) => InternalCalls.Camera_SetFOV(entityID, fov);

        public static float CameraGetNear(uint entityID) => InternalCalls.Camera_GetNear(entityID);
        public static void CameraSetNear(uint entityID, float nearPlane) => InternalCalls.Camera_SetNear(entityID, nearPlane);

        public static float CameraGetFar(uint entityID) => InternalCalls.Camera_GetFar(entityID);
        public static void CameraSetFar(uint entityID, float farPlane) => InternalCalls.Camera_SetFar(entityID, farPlane);

        public static Vector3 CameraGetTarget(uint entityID)
        {
            InternalCalls.Camera_GetTarget(entityID, out Vector3 t);
            return t;
        }

        public static void CameraSetTarget(uint entityID, Vector3 target)
        {
            InternalCalls.Camera_SetTarget(entityID, ref target);
        }

        // ======================
        // MeshRenderer
        // ======================
        public static bool MeshRendererGetVisible(uint entityID) => InternalCalls.MeshRenderer_GetVisible(entityID);
        public static void MeshRendererSetVisible(uint entityID, bool visible) => InternalCalls.MeshRenderer_SetVisible(entityID, visible);

        public static bool MeshRendererGetShadowCast(uint entityID) => InternalCalls.MeshRenderer_GetShadowCast(entityID);
        public static void MeshRendererSetShadowCast(uint entityID, bool cast) => InternalCalls.MeshRenderer_SetShadowCast(entityID, cast);

        public static bool MeshRendererGetShadowReceive(uint entityID) => InternalCalls.MeshRenderer_GetShadowReceive(entityID);
        public static void MeshRendererSetShadowReceive(uint entityID, bool receive) => InternalCalls.MeshRenderer_SetShadowReceive(entityID, receive);

        public static bool MeshRendererGetGlobalIlluminate(uint entityID) => InternalCalls.MeshRenderer_GetGlobalIlluminate(entityID);
        public static void MeshRendererSetGlobalIlluminate(uint entityID, bool gi) => InternalCalls.MeshRenderer_SetGlobalIlluminate(entityID, gi);

        // ======================
        // AudioComponent
        // ======================
        public static void AudioPlay(uint entityID) => InternalCalls.Audio_Play(entityID);
        public static void AudioStop(uint entityID) => InternalCalls.Audio_Stop(entityID);
        public static void AudioPause(uint entityID) => InternalCalls.Audio_Pause(entityID);

        public static float AudioGetVolume(uint entityID) => InternalCalls.Audio_GetVolume(entityID);
        public static void AudioSetVolume(uint entityID, float volume) => InternalCalls.Audio_SetVolume(entityID, volume);

        public static float AudioGetPitch(uint entityID) => InternalCalls.Audio_GetPitch(entityID);
        public static void AudioSetPitch(uint entityID, float pitch) => InternalCalls.Audio_SetPitch(entityID, pitch);

        public static bool AudioGetLoop(uint entityID) => InternalCalls.Audio_GetLoop(entityID);
        public static void AudioSetLoop(uint entityID, bool loop) => InternalCalls.Audio_SetLoop(entityID, loop);

        public static bool AudioGetMute(uint entityID) => InternalCalls.Audio_GetMute(entityID);
        public static void AudioSetMute(uint entityID, bool mute) => InternalCalls.Audio_SetMute(entityID, mute);

        public static bool AudioGetIs3D(uint entityID) => InternalCalls.Audio_GetIs3D(entityID);
        public static void AudioSetIs3D(uint entityID, bool is3d) => InternalCalls.Audio_SetIs3D(entityID, is3d);

        public static void AudioSetFile(uint entityID, string path) => InternalCalls.Audio_SetFile(entityID, path);

        public static float AudioGetMinDistance(uint entityID) => InternalCalls.Audio_GetMinDistance(entityID);
        public static void AudioSetMinDistance(uint entityID, float minDist) => InternalCalls.Audio_SetMinDistance(entityID, minDist);

        public static float AudioGetMaxDistance(uint entityID) => InternalCalls.Audio_GetMaxDistance(entityID);
        public static void AudioSetMaxDistance(uint entityID, float maxDist) => InternalCalls.Audio_SetMaxDistance(entityID, maxDist);

        public static int AudioGetRolloffMode(uint entityID) => InternalCalls.Audio_GetRolloffMode(entityID);
        public static void AudioSetRolloffMode(uint entityID, int mode) => InternalCalls.Audio_SetRolloffMode(entityID, mode);

        public static float AudioGetDopplerLevel(uint entityID) => InternalCalls.Audio_GetDopplerLevel(entityID);
        public static void AudioSetDopplerLevel(uint entityID, float level) => InternalCalls.Audio_SetDopplerLevel(entityID, level);

        public static float AudioGetPan2D(uint entityID) => InternalCalls.Audio_GetPan2D(entityID);
        public static void AudioSetPan2D(uint entityID, float pan) => InternalCalls.Audio_SetPan2D(entityID, pan);

        public static float AudioGetReverbMix(uint entityID) => InternalCalls.Audio_GetReverbMix(entityID);
        public static void AudioSetReverbMix(uint entityID, float mix) => InternalCalls.Audio_SetReverbMix(entityID, mix);

        // ======================
        // AudioManager
        // ======================
        public static float AudioManagerGetGroupVolume(int groupType) => InternalCalls.AudioManager_GetGroupVolume(groupType);
        public static void AudioManagerSetGroupVolume(int groupType, float volume) => InternalCalls.AudioManager_SetGroupVolume(groupType, volume);

        public static float AudioManagerGetGroupPitch(int groupType) => InternalCalls.AudioManager_GetGroupPitch(groupType);
        public static void AudioManagerSetGroupPitch(int groupType, float pitch) => InternalCalls.AudioManager_SetGroupPitch(groupType, pitch);

        public static bool AudioManagerIsGroupMuted(int groupType) => InternalCalls.AudioManager_IsGroupMuted(groupType);
        public static void AudioManagerSetGroupMute(int groupType, bool mute) => InternalCalls.AudioManager_SetGroupMute(groupType, mute);

        public static void AudioManagerPauseGroup(int groupType, bool pause) => InternalCalls.AudioManager_PauseGroup(groupType, pause);
        public static void AudioManagerPauseAll(bool pause) => InternalCalls.AudioManager_PauseAll(pause);

        public static void AudioManagerStopByType(int groupType) => InternalCalls.AudioManager_StopByType(groupType);
        public static void AudioManagerStopAll() => InternalCalls.AudioManager_StopAll();

        public static void AudioManagerCreateDSP(int groupType, int effectType) => InternalCalls.AudioManager_CreateDSP(groupType, effectType);
        public static void AudioManagerEnableDSP(int groupType, int effectType, bool enable) => InternalCalls.AudioManager_EnableDSP(groupType, effectType, enable);
        public static void AudioManagerSetDSPParameter(int groupType, int effectType, int paramIndex, float value)
            => InternalCalls.AudioManager_SetDSPParameter(groupType, effectType, paramIndex, value);

        // NOTE: This wraps your current extern name (AudioMaster_ReleaseSpecificDSPinGroup).
        public static void AudioMasterReleaseSpecificDSPinGroup(int groupType, int effectType)
            => InternalCalls.AudioMaster_ReleaseSpecificDSPinGroup(groupType, effectType);

        public static void AudioManagerReleaseDSPByGroup(int groupType) => InternalCalls.AudioManager_ReleaseDSPByGroup(groupType);
        public static void AudioManagerReleaseAllDSPs() => InternalCalls.AudioManager_ReleaseAllDSPs();

        public static void AudioManagerSetListenerAttributes(Vector3 position, Vector3 forward, Vector3 up, Vector3 velocity)
            => InternalCalls.AudioManager_SetListenerAttributes(ref position, ref forward, ref up, ref velocity);

        // ======================
        // Events
        // ======================
        public static void EventPublish(string name, string payload) => InternalCalls.Event_Publish(name, payload);

        // ======================
        // Quat / Math helpers
        // ======================
        public static Vector3 QuatGetForward(Quat quat)
        {
            InternalCalls.Quat_GetForward(ref quat, out Vector3 v);
            return v;
        }

        public static Vector3 QuatGetRight(Quat quat)
        {
            InternalCalls.Quat_GetRight(ref quat, out Vector3 v);
            return v;
        }

        public static Vector3 QuatGetUp(Quat quat)
        {
            InternalCalls.Quat_GetUp(ref quat, out Vector3 v);
            return v;
        }

        public static Vector3 QuatRotateVector(Quat quat, Vector3 vec)
        {
            InternalCalls.Quat_RotateVector(ref quat, ref vec, out Vector3 outVec);
            return outVec;
        }

        public static Quat QuatMultiply(Quat q1, Quat q2)
        {
            InternalCalls.Quat_Multiply(ref q1, ref q2, out Quat outQuat);
            return outQuat;
        }

        public static Quat QuatFromAxisAngle(Vector3 axis, float angleRadians)
        {
            InternalCalls.Quat_FromAxisAngle(ref axis, angleRadians, out Quat outQuat);
            return outQuat;
        }

        public static Quat QuatSlerp(Quat q1, Quat q2, float t)
        {
            InternalCalls.Quat_Slerp(ref q1, ref q2, t, out Quat outQuat);
            return outQuat;
        }

        public static Quat QuatInverse(Quat quat)
        {
            InternalCalls.Quat_Inverse(ref quat, out Quat outQuat);
            return outQuat;
        }

        public static Vector3 QuatToEuler(Quat quat)
        {
            InternalCalls.Quat_ToEuler(ref quat, out Vector3 euler);
            return euler;
        }

        public static Quat QuatFromEuler(Vector3 euler)
        {
            InternalCalls.Quat_FromEuler(ref euler, out Quat outQuat);
            return outQuat;
        }

        public static Quat QuatNormalize(Quat quat)
        {
            InternalCalls.Quat_Normalize(ref quat, out Quat outQuat);
            return outQuat;
        }

        public static float QuatLength(Quat quat) => InternalCalls.Quat_Length(ref quat);
        public static float QuatDot(Quat q1, Quat q2) => InternalCalls.Quat_Dot(ref q1, ref q2);
    }

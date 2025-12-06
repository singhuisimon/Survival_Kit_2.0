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
        public static extern ulong Scene_FindEntityByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong Transform_GetParent(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong[] Scene_FindEntitiesByTag(string tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetPosition(ulong entityID, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetPosition(ulong entityID, ref Vector3 position);

        // Rotation now uses Quat as the marshalled type
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetRotation(ulong entityID, out Quat rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetRotation(ulong entityID, ref Quat rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetScale(ulong entityID, out Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetScale(ulong entityID, ref Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong Scene_CreateEntity(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Scene_DestroyEntity(ulong entity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddScript(ulong entity, string managedClassFullName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddRigidBody(ulong entityID);

        // Prefabs
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong Prefab_Instantiate(string prefabPath);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong Prefab_InstantiateScene(string prefabPath);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Prefab_InstantiateWithTransform(string prefabPath, ref Vector3 position, ref Quat rotation, ref Vector3 scale, bool isScenePrefab);

        // Rigidbody core properties
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_GetVelocity(ulong entityID, out Vector3 vel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetVelocity(ulong entityID, ref Vector3 vel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_AddVelocity(ulong entityID, ref Vector3 delta);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Rigidbody_GetMass(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetMass(ulong entityID, float mass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_GetIsKinematic(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetIsKinematic(ulong entityID, bool isKinematic);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_GetUseGravity(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_SetUseGravity(ulong entityID, bool useGravity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Rigidbody_GetSpeed(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_IsMoving(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Rigidbody_IsStatic(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_AddForce(ulong entityID, ref Vector3 force);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Rigidbody_Stop(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_EnableCollisionEvents();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_BeginCollisionFrame();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Physics_GetCollisionCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_GetCollisionPair(int index, out ulong a, out ulong b);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddTag(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddCamera(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddAudio(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddMeshRenderer(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string Tag_GetTag(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Tag_SetTag(ulong entityID, string tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Camera_GetEnabled(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetEnabled(ulong entityID, bool enabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Camera_GetPrimary(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetPrimary(ulong entityID, bool primary);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Camera_GetFOV(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetFOV(ulong entityID, float fov);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Camera_GetNear(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetNear(ulong entityID, float nearPlane);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Camera_GetFar(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetFar(ulong entityID, float farPlane);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_GetTarget(ulong entityID, out Vector3 target);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Camera_SetTarget(ulong entityID, ref Vector3 target);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetVisible(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetVisible(ulong entityID, bool visible);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetShadowCast(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetShadowCast(ulong entityID, bool cast);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetShadowReceive(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetShadowReceive(ulong entityID, bool receive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool MeshRenderer_GetGlobalIlluminate(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void MeshRenderer_SetGlobalIlluminate(ulong entityID, bool gi);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_Play(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_Stop(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_Pause(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetVolume(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetVolume(ulong entityID, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetPitch(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetPitch(ulong entityID, float pitch);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Audio_GetLoop(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetLoop(ulong entityID, bool loop);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Audio_GetMute(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetMute(ulong entityID, bool mute);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Audio_GetIs3D(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetIs3D(ulong entityID, bool is3d);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetFile(ulong entityID, string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetMinDistance(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetMinDistance(ulong entityID, float minDist);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetMaxDistance(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetMaxDistance(ulong entityID, float maxDist);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Audio_GetRolloffMode(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetRolloffMode(ulong entityID, int mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetDopplerLevel(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetDopplerLevel(ulong entityID, float level);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetPan2D(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetPan2D(ulong entityID, float pan);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Audio_GetReverbMix(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Audio_SetReverbMix(ulong entityID, float mix);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool EntityHasCamera(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool EntityHasRigidBody(ulong entityID);

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
        public static extern float Input_GetMouseDelta(out float deltaX, out float deltaY);
    }
}

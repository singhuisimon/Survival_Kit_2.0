using System;
using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Internal calls to C++ engine functions
    /// These are implemented in C++ and registered via mono_add_internal_call
    /// </summary>
    public static class InternalCalls
    {
        // Logging
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LogError(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LogWarning(string message);

        // Scene / entities
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Scene_FindEntityByName(string name);

        // NEW: list of entities matching a tag
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint[] Scene_FindEntitiesByTag(string tag);

        // CORRECT - GetPosition uses out, SetPosition uses ref
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetPosition(uint entityID, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetPosition(uint entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetRotation(uint entityID, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetRotation(uint entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetScale(uint entityID, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetScale(uint entityID, ref Vector3 position);

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

        // ---- Collision events ----

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

        //Added new functions -amanda
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

        //Audio Manager implementation - new amanda
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

    }
}

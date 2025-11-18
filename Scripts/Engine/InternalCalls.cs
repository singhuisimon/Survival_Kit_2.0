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

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetPosition(uint entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetPosition(uint entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_Move(uint entityID, float deltaX, float deltaY, float deltaZ);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Scene_CreateEntity(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Scene_DestroyEntity(uint entity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddScript(uint entity, string managedClassFullName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_AddRigidBody(ulong entityID);

        // Prefabs
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Prefab_Instantiate(string prefabPath);

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

        // ---- Collision events ----

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_EnableCollisionEvents();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_BeginCollisionFrame();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Physics_GetCollisionCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Physics_GetCollisionPair(int index, out uint a, out uint b);

        //[MethodImpl(MethodImplOptions.InternalCall)]
        //public static extern void Rigidbody_SetVelocity(uint entity, float x, float y, float z);
    }
}

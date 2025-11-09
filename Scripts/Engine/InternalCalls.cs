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

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Prefab_Instantiate(string prefabPath);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        public static extern void Rigidbody_GetVelocity(ulong entityID, out Vector3 vel);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        public static extern void Rigidbody_SetVelocity(ulong entityID, ref Vector3 vel);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        public static extern void Rigidbody_AddVelocity(ulong entityID, ref Vector3 delta);

        // ---- Collision events

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

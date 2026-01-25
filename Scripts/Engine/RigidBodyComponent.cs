using System.Runtime.CompilerServices;

namespace Engine
{
    public class Rigidbody : Component
    {
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern Vector3 Rigidbody_GetVelocity(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_SetVelocity(uint entityID, ref Vector3 vel);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_AddVelocity(uint entityID, ref Vector3 delta);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Rigidbody_GetMass(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_SetMass(uint entityID, float mass);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Rigidbody_GetIsKinematic(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_SetIsKinematic(uint entityID, bool isKinematic);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Rigidbody_GetUseGravity(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_SetUseGravity(uint entityID, bool useGravity);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Rigidbody_GetSpeed(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Rigidbody_IsMoving(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Rigidbody_IsStatic(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_AddForce(uint entityID, ref Vector3 force);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_Stop(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Rigidbody_SetBoxHalfExtent(uint entityID, ref Vector3 boxHalf);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern Vector3 Rigidbody_GetBoxHalfExtent(uint entityID);

        public static Vector3 RigidbodyGetVelocity(uint entityID) => Rigidbody_GetVelocity(entityID);

        public static void RigidbodySetVelocity(uint entityID, ref Vector3 vel) => Rigidbody_SetVelocity(entityID, ref vel);
        public static void RigidbodyAddVelocity(uint entityID, ref Vector3 delta) => Rigidbody_AddVelocity(entityID, ref delta);

        public static float RigidbodyGetMass(uint entityID) => Rigidbody_GetMass(entityID);
        public static void RigidbodySetMass(uint entityID, float mass) => Rigidbody_SetMass(entityID, mass);

        public static bool RigidbodyGetIsKinematic(uint entityID) => Rigidbody_GetIsKinematic(entityID);
        public static void RigidbodySetIsKinematic(uint entityID, bool isKinematic) => Rigidbody_SetIsKinematic(entityID, isKinematic);

        public static bool RigidbodyGetUseGravity(uint entityID) => Rigidbody_GetUseGravity(entityID);
        public static void RigidbodySetUseGravity(uint entityID, bool useGravity) => Rigidbody_SetUseGravity(entityID, useGravity);

        public static float RigidbodyGetSpeed(uint entityID) => Rigidbody_GetSpeed(entityID);
        public static bool RigidbodyIsMoving(uint entityID) => Rigidbody_IsMoving(entityID);
        public static bool RigidbodyIsStatic(uint entityID) => Rigidbody_IsStatic(entityID);

        public static void RigidbodyAddForce(uint entityID, ref Vector3 force) => Rigidbody_AddForce(entityID, ref force);
        public static void RigidbodyStop(uint entityID) => Rigidbody_Stop(entityID);

        public static void RigidbodySetBoxHalfExtents(uint entityID, ref Vector3 boxHalf) => Rigidbody_SetBoxHalfExtent(entityID, ref boxHalf);
        public static Vector3 RigidbodyGetBoxHalfExtents(uint entityID) => Rigidbody_GetBoxHalfExtent(entityID);

        // Instance-style API still fine to keep
        private uint ID => Entity.EntityID;

        public float Mass { get => Rigidbody_GetMass(ID); set => Rigidbody_SetMass(ID, value); }
        public bool IsKinematic { get => Rigidbody_GetIsKinematic(ID); set => Rigidbody_SetIsKinematic(ID, value); }
        public bool UseGravity { get => Rigidbody_GetUseGravity(ID); set => Rigidbody_SetUseGravity(ID, value); }

        public Vector3 Velocity
        {
            get { return Rigidbody_GetVelocity(ID); }
            set { Rigidbody_SetVelocity(ID, ref value); }
        }

        public float Speed => Rigidbody_GetSpeed(ID);
        public bool IsMoving => Rigidbody_IsMoving(ID);
        public bool IsStatic => Rigidbody_IsStatic(ID);

        public void AddForce(Vector3 force) => Rigidbody_AddForce(ID, ref force);
        public void Stop() => Rigidbody_Stop(ID);
        public Vector3 BoxHalfExtents
        {
            get { return Rigidbody_GetBoxHalfExtent(ID); }
            set { Rigidbody_SetBoxHalfExtent(ID, ref value);}
        }
    }
}

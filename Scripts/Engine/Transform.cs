using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over native TransformComponent.
    /// Exposes position, rotation and scale via internal calls.
    /// </summary>
    public class Transform : Component
    {
        // =========================
        // Native bindings (private)
        // =========================
        // Register as: "Engine.Transform::Transform_*"
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint Transform_GetParent(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_GetPosition(uint entityID, out Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_SetPosition(uint entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_GetRotation(uint entityID, out Quat rotation);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_SetRotation(uint entityID, ref Quat rotation);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_GetScale(uint entityID, out Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_SetScale(uint entityID, ref Vector3 scale);

        // =========================
        // Public static wrappers
        // (so scripts can: using static Engine.Transform;)
        // =========================

        public static uint TransformGetParent(uint entityID) => Transform_GetParent(entityID);

        public static Vector3 TransformGetPosition(uint entityID)
        {
            Transform_GetPosition(entityID, out Vector3 pos);
            return pos;
        }

        public static void TransformSetPosition(uint entityID, ref Vector3 position) => Transform_SetPosition(entityID, ref position);

        public static Quat TransformGetRotation(uint entityID)
        {
            Transform_GetRotation(entityID, out Quat rot);
            return rot;
        }

        public static void TransformSetRotation(uint entityID, ref Quat rotation) => Transform_SetRotation(entityID, ref rotation);

        public static Vector3 TransformGetScale(uint entityID)
        {
            Transform_GetScale(entityID, out Vector3 s);
            return s;
        }

        public static void TransformSetScale(uint entityID, ref Vector3 scale) => Transform_SetScale(entityID, ref scale);
        public static Vector3 GetPosition(uint entityID) => TransformGetPosition(entityID);
        public static void SetPosition(uint entityID, ref Vector3 position) => TransformSetPosition(entityID, ref position);
        public static Quat GetRotation(uint entityID) => TransformGetRotation(entityID);
        public static void SetRotation(uint entityID, ref Quat rotation) => TransformSetRotation(entityID, ref rotation);
        public static Vector3 GetScale(uint entityID) => TransformGetScale(entityID);
        public static void SetScale(uint entityID, ref Vector3 scale) => TransformSetScale(entityID, ref scale);

        // =========================
        // Instance API (Component)
        // =========================

        private uint ID => Entity.EntityID;

        public uint ParentID => Transform_GetParent(ID);

        /// <summary>World position of this entity.</summary>
        public Vector3 Position
        {
            get => TransformGetPosition(ID);
            set => Transform_SetPosition(ID, ref value);
        }

        /// <summary>World rotation as a quaternion.</summary>
        public Quat Rotation
        {
            get => TransformGetRotation(ID);
            set => Transform_SetRotation(ID, ref value);
        }

        /// <summary>Local scale of this entity.</summary>
        public Vector3 Scale
        {
            get => TransformGetScale(ID);
            set => Transform_SetScale(ID, ref value);
        }

        public void Rotate(Quat rotation)
        {
            Quat r = Rotation;
            r = r * rotation;
            Transform_SetRotation(ID, ref r);
        }

        public void RotateAxisAngle(Vector3 axis, float angleRadians)
        {
            Quat delta = Quat.FromAxisAngle(axis, angleRadians);
            Quat r = Rotation;
            r = r * delta;
            Transform_SetRotation(ID, ref r);
        }

        // =========================
        // LookAt helper (pure math)
        // =========================
        public static void LookAt(uint entityID, Vector3 target)
        {
            Vector3 myPos = TransformGetPosition(entityID);
            Vector3 direction = new Vector3(
                target.X - myPos.X,
                target.Y - myPos.Y,
                target.Z - myPos.Z
            );

            float lenSq = direction.X * direction.X +
                          direction.Y * direction.Y +
                          direction.Z * direction.Z;
            if (lenSq <= 0.0001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            direction.X *= invLen;
            direction.Y *= invLen;
            direction.Z *= invLen;

            // Engine convention: identity faces +Z
            Vector3 forward = new Vector3(0.0f, 0.0f, 1.0f);

            float dot = forward.X * direction.X +
                        forward.Y * direction.Y +
                        forward.Z * direction.Z;

            Quat rotation;

            if (dot < -0.9999f)
            {
                rotation = new Quat(0.0f, 1.0f, 0.0f, 0.0f);
            }
            else
            {
                Vector3 cross = new Vector3(
                    forward.Y * direction.Z - forward.Z * direction.Y,
                    forward.Z * direction.X - forward.X * direction.Z,
                    forward.X * direction.Y - forward.Y * direction.X
                );

                float s = SimpleMath.Sqrt((1.0f + dot) * 2.0f);
                float invS = 1.0f / s;

                rotation = new Quat(
                    cross.X * invS,
                    cross.Y * invS,
                    cross.Z * invS,
                    0.5f * s
                );
            }

            Transform_SetRotation(entityID, ref rotation);
        }
    }
}

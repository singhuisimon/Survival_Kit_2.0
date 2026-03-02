using System;
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

        // World
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_GetWorldPosition(uint entityID, out Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_SetWorldPosition(uint entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_GetWorldRotation(uint entityID, out Quat rotation);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_SetWorldRotation(uint entityID, ref Quat rotation);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_GetWorldScale(uint entityID, out Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Transform_SetWorldScale(uint entityID, ref Vector3 scale);

        // =========================
        // Public static wrappers
        // (so scripts can: using static Engine.Transform;)
        // =========================

        private uint _entityID;

        internal void __Bind(uint entityID) => _entityID = entityID;

        public uint EntityID => _entityID;

        // ------------------------------------------------------------
        // Hierarchy
        // ------------------------------------------------------------

        /// <summary>
        /// Returns parent entity ID (0 if none, depending on your engine convention).
        /// </summary>
        public uint Parent => Transform_GetParent(_entityID);

        // ------------------------------------------------------------
        // TRS
        // ------------------------------------------------------------

        public Vector3 Position
        {
            get
            {
                Transform_GetPosition(_entityID, out var p);
                return p;
            }
            set
            {
                Transform_SetPosition(_entityID, ref value);
            }
        }

        public Quat Rotation
        {
            get
            {
                Transform_GetRotation(_entityID, out var r);
                return r;
            }
            set
            {
                Transform_SetRotation(_entityID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                Transform_GetScale(_entityID, out var s);
                return s;
            }
            set
            {
                Transform_SetScale(_entityID, ref value);
            }
        }

        // ------------------------------------------------------------
        // TRS (World)
        // ------------------------------------------------------------

        public Vector3 WorldPosition
        {
            get
            {
                Transform_GetWorldPosition(_entityID, out var p);
                return p;
            }
            set
            {
                Transform_SetWorldPosition(_entityID, ref value);
            }
        }

        public Quat WorldRotation
        {
            get
            {
                Transform_GetWorldRotation(_entityID, out var r);
                return r;
            }
            set
            {
                Transform_SetWorldRotation(_entityID, ref value);
            }
        }

        public Vector3 WorldScale
        {
            get
            {
                Transform_GetWorldScale(_entityID, out var s);
                return s;
            }
            set
            {
                Transform_SetWorldScale(_entityID, ref value);
            }
        }

        // ------------------------------------------------------------
        // Convenience (derived from Rotation)
        // ------------------------------------------------------------

        public Vector3 Forward => Rotation.Forward;
        public Vector3 Right => Rotation.Right;
        public Vector3 Up => Rotation.Up;

        // World space convenience vectors
        public Vector3 WorldForward => WorldRotation.Forward;
        public Vector3 WorldRight => WorldRotation.Right;
        public Vector3 WorldUp => WorldRotation.Up;

        public static uint TransformGetParent(uint entityID) => Transform_GetParent(entityID);

        public static Vector3 GetPosition(uint entityID)
        {
            Transform_GetPosition(entityID, out var p);
            return p;
        }

        public static void SetPosition(uint entityID, ref Vector3 position) => Transform_SetPosition(entityID, ref position);

        public static Quat GetRotation(uint entityID)
        {
            Transform_GetRotation(entityID, out var r);
            return r;
        }

        public static void SetRotation(uint entityID, ref Quat rotation) => Transform_SetRotation(entityID, ref rotation);

        public static Vector3 GetScale(uint entityID)
        {
            Transform_GetScale(entityID, out var s);
            return s;
        }

        public static void SetScale(uint entityID, ref Vector3 scale) => Transform_SetScale(entityID, ref scale);

        // --- World Static Wrappers ---
        public static Vector3 GetWorldPosition(uint entityID)
        {
            Transform_GetWorldPosition(entityID, out var p);
            return p;
        }

        public static void SetWorldPosition(uint entityID, ref Vector3 position) => Transform_SetWorldPosition(entityID, ref position);

        public static Quat GetWorldRotation(uint entityID)
        {
            Transform_GetWorldRotation(entityID, out var r);
            return r;
        }

        public static void SetWorldRotation(uint entityID, ref Quat rotation) => Transform_SetWorldRotation(entityID, ref rotation);

        public static Vector3 GetWorldScale(uint entityID)
        {
            Transform_GetWorldScale(entityID, out var s);
            return s;
        }

        public static void SetWorldScale(uint entityID, ref Vector3 scale) => Transform_SetWorldScale(entityID, ref scale);

        // ------------------------------------------------------------
        // Transform helpers
        // ------------------------------------------------------------
        /// <summary>
        /// Returns a quaternion representing a rotation of <paramref name="angleRadians"/>
        /// around <paramref name="axis"/>.
        /// </summary>
        public static Quat RotateAxisAngle(Vector3 axis, float angleRadians)
        {
            // If you want safety, normalize axis here (optional):
            // axis = axis.Normalized();

            return Quat.FromAxisAngle(axis, angleRadians);
        }

        /// <summary>
        /// Rotates an existing quaternion by an axis-angle delta.
        /// If <paramref name="localSpace"/> is true, applies delta in local space (q * delta),
        /// else world space (delta * q).
        /// </summary>
        public static Quat RotateAxisAngle(in Quat q, Vector3 axis, float angleRadians, bool localSpace = true)
        {
            Quat delta = Quat.FromAxisAngle(axis, angleRadians);
            return localSpace ? (q * delta) : (delta * q);
        }

        /// <summary>
        /// In-place version: modifies <paramref name="q"/>.
        /// </summary>
        public static void RotateAxisAngleInPlace(ref Quat q, Vector3 axis, float angleRadians, bool localSpace = true)
        {
            Quat delta = Quat.FromAxisAngle(axis, angleRadians);
            q = localSpace ? (q * delta) : (delta * q);
        }
    }
}

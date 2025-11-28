using System.Runtime.CompilerServices;

namespace Engine
{
    public class Transform : Component
    {
        // Internal calls to C++
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPosition_Native(ulong entityID, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetPosition_Native(ulong entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetRotation_Native(ulong entityID, out Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetRotation_Native(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetScale_Native(ulong entityID, out Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetScale_Native(ulong entityID, ref Vector3 scale);

        // Instance properties (component-style)
        public Vector3 Position
        {
            get
            {
                GetPosition_Native(Entity.EntityID, out Vector3 position);
                return position;
            }
            set
            {
                SetPosition_Native(Entity.EntityID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                GetRotation_Native(Entity.EntityID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                SetRotation_Native(Entity.EntityID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                GetScale_Native(Entity.EntityID, out Vector3 scale);
                return scale;
            }
            set
            {
                SetScale_Native(Entity.EntityID, ref value);
            }
        }

        // Methods
        public void Translate(Vector3 translation)
        {
            Position = Position + translation;
        }

        public void Rotate(Vector3 rotation)
        {
            Rotation = Rotation + rotation;
        }

        public void LookAt(Vector3 target)
        {
            // Simple look-at implementation
            Vector3 direction = (target - Position).Normalized;

            float yaw = (float)System.Math.Atan2(direction.X, direction.Z) * (180.0f / (float)System.Math.PI);
            float pitch = (float)System.Math.Asin(-direction.Y) * (180.0f / (float)System.Math.PI);

            Rotation = new Vector3(pitch, yaw, 0);
        }

        // ===== Static helpers for scripts that only have an entityID =====

        public static Vector3 GetPosition(ulong entityID)
        {
            GetPosition_Native(entityID, out Vector3 position);
            return position;
        }

        public static void SetPosition(ulong entityID, ref Vector3 position)
        {
            SetPosition_Native(entityID, ref position);
        }

        public static Vector3 GetRotation(ulong entityID)
        {
            GetRotation_Native(entityID, out Vector3 rotation);
            return rotation;
        }

        public static void SetRotation(ulong entityID, ref Vector3 rotation)
        {
            SetRotation_Native(entityID, ref rotation);
        }

        public static Vector3 GetScale(ulong entityID)
        {
            GetScale_Native(entityID, out Vector3 scale);
            return scale;
        }

        public static void SetScale(ulong entityID, ref Vector3 scale)
        {
            SetScale_Native(entityID, ref scale);
        }
    }
}

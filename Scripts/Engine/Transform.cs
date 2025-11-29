using System;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over native TransformComponent.
    /// Exposes position, rotation and scale via InternalCalls.
    /// </summary>
    public class Transform : Component
    {
        /// <summary>
        /// World position of this entity.
        /// </summary>
        public Vector3 Position
        {
            get
            {
                Vector3 pos;
                InternalCalls.Transform_GetPosition(Entity.EntityID, out pos);
                return pos;
            }
            set
            {
                InternalCalls.Transform_SetPosition(Entity.EntityID, ref value);
            }
        }

        /// <summary>
        /// World rotation as Euler angles in degrees (pitch, yaw, roll).
        /// </summary>
        public Vector3 Rotation
        {
            get
            {
                Vector3 rot;
                InternalCalls.Transform_GetRotation(Entity.EntityID, out rot);
                return rot;
            }
            set
            {
                InternalCalls.Transform_SetRotation(Entity.EntityID, ref value);
            }
        }

        /// <summary>
        /// Local scale of this entity.
        /// </summary>
        public Vector3 Scale
        {
            get
            {
                Vector3 scale;
                InternalCalls.Transform_GetScale(Entity.EntityID, out scale);
                return scale;
            }
            set
            {
                InternalCalls.Transform_SetScale(Entity.EntityID, ref value);
            }
        }

        // ---------------------------------
        // Instance helpers
        // ---------------------------------

        /// <summary>
        /// Adds to the current rotation (Euler degrees).
        /// </summary>
        public void Rotate(Vector3 rotation)
        {
            Rotation = Rotation + rotation;
        }

        /// <summary>
        /// Rotates this transform so its forward faces the target position.
        /// </summary>
        public void LookAt(Vector3 target)
        {
            Vector3 direction = (target - Position).Normalized;

            float yaw = (float)System.Math.Atan2(direction.X, direction.Z) *
                        (180.0f / (float)System.Math.PI);
            float pitch = (float)System.Math.Asin(-direction.Y) *
                          (180.0f / (float)System.Math.PI);

            Rotation = new Vector3(pitch, yaw, 0.0f);
        }

        // ---------------------------------
        // Static helpers (when you only have an ID)
        // ---------------------------------

        public static Vector3 GetPosition(uint entityID)
        {
            Vector3 pos;
            InternalCalls.Transform_GetPosition(entityID, out pos);
            return pos;
        }

        public static void SetPosition(uint entityID, ref Vector3 position)
        {
            InternalCalls.Transform_SetPosition(entityID, ref position);
        }

        public static Vector3 GetRotation(uint entityID)
        {
            Vector3 rot;
            InternalCalls.Transform_GetRotation(entityID, out rot);
            return rot;
        }

        public static void SetRotation(uint entityID, ref Vector3 rotation)
        {
            InternalCalls.Transform_SetRotation(entityID, ref rotation);
        }

        public static Vector3 GetScale(uint entityID)
        {
            Vector3 scale;
            InternalCalls.Transform_GetScale(entityID, out scale);
            return scale;
        }

        public static void SetScale(uint entityID, ref Vector3 scale)
        {
            InternalCalls.Transform_SetScale(entityID, ref scale);
        }
    }
}

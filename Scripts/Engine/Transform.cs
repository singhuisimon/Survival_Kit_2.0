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
        /// World rotation as a quaternion (primary representation).
        /// </summary>
        public Quat Rotation
        {
            get
            {
                Quat rot;
                InternalCalls.Transform_GetRotation(Entity.EntityID, out rot);
                return rot;
            }
            set
            {
                InternalCalls.Transform_SetRotation(Entity.EntityID, ref value);
            }
        }

        /// <summary>
        /// World rotation as Euler angles in radians.
        /// Wraps the quaternion Rotation property.
        /// </summary>
        public Vector3 RotationEulerRadians
        {
            get
            {
                return Rotation.ToEuler();
            }
            set
            {
                Rotation = Quat.FromEuler(value);
            }
        }

        /// <summary>
        /// World rotation as Euler angles in degrees.
        /// Convenience wrapper for designers who prefer degrees.
        /// </summary>
        public Vector3 RotationEulerDegrees
        {
            get
            {
                Vector3 eulerRad = Rotation.ToEuler();
                return new Vector3(
                    eulerRad.X * SimpleMath.RAD_TO_DEG,
                    eulerRad.Y * SimpleMath.RAD_TO_DEG,
                    eulerRad.Z * SimpleMath.RAD_TO_DEG
                );
            }
            set
            {
                Vector3 eulerRad = new Vector3(
                    value.X * SimpleMath.DEG_TO_RAD,
                    value.Y * SimpleMath.DEG_TO_RAD,
                    value.Z * SimpleMath.DEG_TO_RAD
                );
                Rotation = Quat.FromEuler(eulerRad);
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
        /// Rotates by multiplying the current rotation with the given quaternion.
        /// </summary>
        public void Rotate(Quat rotation)
        {
            Rotation = Rotation * rotation;
        }

        /// <summary>
        /// Rotates by a given axis and angle (in radians).
        /// </summary>
        public void RotateAxisAngle(Vector3 axis, float angleRadians)
        {
            Quat deltaRotation = Quat.FromAxisAngle(axis, angleRadians);
            Rotation = Rotation * deltaRotation;
        }

        /// <summary>
        /// Rotates this transform so its forward faces the target position.
        /// </summary>
        public void LookAt(Vector3 target)
        {
            Vector3 direction = (target - Position).Normalized;

            float yaw = SimpleMath.Atan2(direction.X, direction.Z);
            float pitch = SimpleMath.Asin(-direction.Y);

            Vector3 eulerRadians = new Vector3(pitch, yaw, 0.0f);
            Rotation = Quat.FromEuler(eulerRadians);
        }

        /// <summary>
        /// Static version: Rotates entity to look at target position.
        /// </summary>
        public static void LookAt(uint entityID, Vector3 target)
        {
            Vector3 myPos = GetPosition(entityID);
            Vector3 direction = target - myPos;

            float lenSq = direction.X * direction.X + direction.Y * direction.Y + direction.Z * direction.Z;
            if (lenSq <= 0.0001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            direction.X *= invLen;
            direction.Y *= invLen;
            direction.Z *= invLen;

            float yaw = SimpleMath.Atan2(direction.X, direction.Z);
            float pitch = SimpleMath.Asin(-direction.Y);

            Vector3 eulerRadians = new Vector3(pitch, yaw, 0.0f);
            Quat rotation = Quat.FromEuler(eulerRadians);
            SetRotation(entityID, ref rotation);
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

        public static Quat GetRotation(uint entityID)
        {
            Quat rot;
            InternalCalls.Transform_GetRotation(entityID, out rot);
            return rot;
        }

        public static void SetRotation(uint entityID, ref Quat rotation)
        {
            InternalCalls.Transform_SetRotation(entityID, ref rotation);
        }

        /// <summary>
        /// Gets world rotation as Euler angles in radians for an entity ID.
        /// </summary>
        public static Vector3 GetRotationEulerRadians(uint entityID)
        {
            return GetRotation(entityID).ToEuler();
        }

        /// <summary>
        /// Sets world rotation from Euler angles in radians for an entity ID.
        /// </summary>
        public static void SetRotationEulerRadians(uint entityID, ref Vector3 eulerRadians)
        {
            Quat rotation = Quat.FromEuler(eulerRadians);
            SetRotation(entityID, ref rotation);
        }

        /// <summary>
        /// Gets world rotation as Euler angles in degrees for an entity ID.
        /// </summary>
        public static Vector3 GetRotationEulerDegrees(uint entityID)
        {
            Vector3 eulerRadians = GetRotationEulerRadians(entityID);
            return new Vector3(
                eulerRadians.X * SimpleMath.RAD_TO_DEG,
                eulerRadians.Y * SimpleMath.RAD_TO_DEG,
                eulerRadians.Z * SimpleMath.RAD_TO_DEG
            );
        }

        /// <summary>
        /// Sets world rotation from Euler angles in degrees for an entity ID.
        /// </summary>
        public static void SetRotationEulerDegrees(uint entityID, ref Vector3 eulerDegrees)
        {
            Vector3 eulerRadians = new Vector3(
                eulerDegrees.X * SimpleMath.DEG_TO_RAD,
                eulerDegrees.Y * SimpleMath.DEG_TO_RAD,
                eulerDegrees.Z * SimpleMath.DEG_TO_RAD
            );
            SetRotationEulerRadians(entityID, ref eulerRadians);
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

    /// <summary>
    /// Simple math utilities that do not rely on System.Math
    /// </summary>
    public static class SimpleMath
    {
        public const float PI = 3.14159265359f;
        public const float DEG_TO_RAD = 0.0174532924f;
        public const float RAD_TO_DEG = 57.2957795131f;

        public static float Sqrt(float value)
        {
            if (value <= 0.0f) return 0.0f;
            if (value == 1.0f) return 1.0f;

            float x = value;
            float y = 1.0f;
            const float epsilon = 0.0001f;

            for (int i = 0; i < 4; i++)
            {
                y = 0.5f * (x + value / x);
                float diff = x - y;
                if (diff < epsilon && diff > -epsilon)
                    break;
                x = y;
            }

            return y;
        }

        public static float Atan2(float y, float x)
        {
            if (x == 0.0f)
            {
                if (y > 0.0f) return PI * 0.5f;
                if (y < 0.0f) return -PI * 0.5f;
                return 0.0f;
            }

            float z = y / x;
            float absZ = z < 0.0f ? -z : z;

            float atan;
            if (absZ <= 1.0f)
            {
                atan = AtanApprox(z);
            }
            else
            {
                atan = (PI * 0.5f) - AtanApprox(1.0f / z);
                if (z < 0.0f)
                    atan = -atan;
            }

            if (x < 0.0f)
            {
                if (y >= 0.0f)
                    atan = atan + PI;
                else
                    atan = atan - PI;
            }

            return atan;
        }

        private static float AtanApprox(float x)
        {
            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            float x7 = x5 * x2;
            float x9 = x7 * x2;

            return x - (x3 / 3.0f) + (x5 / 5.0f) - (x7 / 7.0f) + (x9 / 9.0f);
        }

        public static float Asin(float x)
        {
            if (x <= -1.0f) return -PI * 0.5f;
            if (x >= 1.0f) return PI * 0.5f;

            if (x < -0.5f || x > 0.5f)
            {
                float sign = x < 0.0f ? -1.0f : 1.0f;
                float absX = x < 0.0f ? -x : x;
                float sqrtTerm = Sqrt(1.0f - absX * absX);
                return sign * (PI * 0.5f - AsinApprox(sqrtTerm));
            }

            return AsinApprox(x);
        }

        private static float AsinApprox(float x)
        {
            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            float x7 = x5 * x2;
            float x9 = x7 * x2;

            return x - (x3 / 3.0f) + (x5 / 5.0f) - (x7 / 7.0f) + (x9 / 9.0f);
        }

        public static float Lerp(float a, float b, float t)
        {
            if (t <= 0.0f) return a;
            if (t >= 1.0f) return b;
            return a + (b - a) * t;
        }

        public static float Clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
    }
}

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
        /// Uses custom math functions to avoid System.Math dependency.
        /// </summary>
        public void LookAt(Vector3 target)
        {
            Vector3 direction = (target - Position).Normalized;

            // Calculate yaw (rotation around Y axis)
            float yaw = SimpleMath.Atan2(direction.X, direction.Z) * SimpleMath.RAD_TO_DEG;

            // Calculate pitch (rotation around X axis)
            float pitch = SimpleMath.Asin(-direction.Y) * SimpleMath.RAD_TO_DEG;

            Rotation = new Vector3(pitch, yaw, 0.0f);
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

            float yaw = SimpleMath.Atan2(direction.X, direction.Z) * SimpleMath.RAD_TO_DEG;
            float pitch = SimpleMath.Asin(-direction.Y) * SimpleMath.RAD_TO_DEG;

            Vector3 rotation = new Vector3(pitch, yaw, 0.0f);
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

    /// <summary>
    /// Simple math utilities that don't rely on System.Math
    /// </summary>
    public static class SimpleMath
    {
        public const float PI = 3.14159265359f;
        public const float DEG_TO_RAD = 0.0174532924f;
        public const float RAD_TO_DEG = 57.2957795131f;

        /// <summary>
        /// Square root using Newton-Raphson method
        /// </summary>
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

        /// <summary>
        /// Arctangent of y/x using Taylor series
        /// </summary>
        public static float Atan2(float y, float x)
        {
            // Handle special cases
            if (x == 0.0f)
            {
                if (y > 0.0f) return PI * 0.5f;
                if (y < 0.0f) return -PI * 0.5f;
                return 0.0f;
            }

            float z = y / x;
            float absZ = z < 0.0f ? -z : z;

            // Use atan approximation for |z| <= 1
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

            // Adjust for quadrant
            if (x < 0.0f)
            {
                if (y >= 0.0f)
                    atan = atan + PI;
                else
                    atan = atan - PI;
            }

            return atan;
        }

        /// <summary>
        /// Arctangent approximation for |x| <= 1
        /// </summary>
        private static float AtanApprox(float x)
        {
            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            float x7 = x5 * x2;
            float x9 = x7 * x2;

            return x - (x3 / 3.0f) + (x5 / 5.0f) - (x7 / 7.0f) + (x9 / 9.0f);
        }

        /// <summary>
        /// Arcsine using Taylor series (valid for |x| <= 1)
        /// </summary>
        public static float Asin(float x)
        {
            // Clamp to valid range
            if (x <= -1.0f) return -PI * 0.5f;
            if (x >= 1.0f) return PI * 0.5f;

            if (x > 0.7f || x < -0.7f)
            {
                float sign = x < 0.0f ? -1.0f : 1.0f;
                float absX = x < 0.0f ? -x : x;
                float sqrtTerm = Sqrt(1.0f - absX * absX);
                return sign * (PI * 0.5f - AsinApprox(sqrtTerm));
            }

            return AsinApprox(x);
        }

        /// <summary>
        /// Arcsine approximation using Taylor series
        /// </summary>
        private static float AsinApprox(float x)
        {
            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            float x7 = x5 * x2;

            return x + (x3 / 6.0f) + (3.0f * x5 / 40.0f) + (5.0f * x7 / 112.0f);
        }

        /// <summary>
        /// Linear interpolation
        /// </summary>
        public static float Lerp(float a, float b, float t)
        {
            if (t <= 0.0f) return a;
            if (t >= 1.0f) return b;
            return a + (b - a) * t;
        }

        /// <summary>
        /// Clamp value between min and max
        /// </summary>
        public static float Clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
    }
}
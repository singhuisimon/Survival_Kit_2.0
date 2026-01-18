/// <summary>
/// Simple math utilities that do not rely on System.Math
/// </summary>
namespace Engine
{
    public static class SimpleMath
    {
        public const float PI = 3.14159265359f;
        public const float TWO_PI = 6.28318530718f;
        public const float HALF_PI = 1.57079632679f;
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

        /// <summary>
        /// Cosine function using Taylor series.
        /// cos(x) = 1 - x²/2! + x⁴/4! - x⁶/6! + x⁸/8! - ...
        /// </summary>
        public static float Cos(float angleRadians)
        {
            // Reduce angle to [0, 2π] range
            angleRadians = angleRadians % TWO_PI;
            if (angleRadians < 0.0f)
                angleRadians += TWO_PI;

            // Use symmetry to reduce to [0, π/2] for better convergence
            int quadrant = 0;
            if (angleRadians > PI)
            {
                angleRadians -= PI;
                quadrant = 2;
            }
            if (angleRadians > HALF_PI)
            {
                angleRadians = PI - angleRadians;
                quadrant += 1;
            }

            // Taylor series for cos(x) in range [0, π/2]
            float x2 = angleRadians * angleRadians;
            float x4 = x2 * x2;
            float x6 = x4 * x2;
            float x8 = x6 * x2;

            float result = 1.0f 
                - (x2 / 2.0f)           // x²/2!
                + (x4 / 24.0f)          // x⁴/4!
                - (x6 / 720.0f)         // x⁶/6!
                + (x8 / 40320.0f);      // x⁸/8!

            // Apply sign based on quadrant
            // Quadrant 0 (0 to π/2): positive
            // Quadrant 1 (π/2 to π): negative
            // Quadrant 2 (π to 3π/2): negative
            // Quadrant 3 (3π/2 to 2π): positive
            if (quadrant == 1 || quadrant == 2)
                result = -result;

            return result;
        }

        /// <summary>
        /// Sine function using Taylor series.
        /// sin(x) = x - x³/3! + x⁵/5! - x⁷/7! + x⁹/9! - ...
        /// </summary>
        public static float Sin(float angleRadians)
        {
            // Reduce angle to [0, 2π] range
            angleRadians = angleRadians % TWO_PI;
            if (angleRadians < 0.0f)
                angleRadians += TWO_PI;

            // Use symmetry to reduce to [0, π/2] for better convergence
            bool negate = false;
            if (angleRadians > PI)
            {
                angleRadians -= PI;
                negate = true;
            }
            if (angleRadians > HALF_PI)
            {
                angleRadians = PI - angleRadians;
            }

            // Taylor series for sin(x) in range [0, π/2]
            float x2 = angleRadians * angleRadians;
            float x3 = x2 * angleRadians;
            float x5 = x3 * x2;
            float x7 = x5 * x2;
            float x9 = x7 * x2;

            float result = angleRadians 
                - (x3 / 6.0f)           // x³/3!
                + (x5 / 120.0f)         // x⁵/5!
                - (x7 / 5040.0f)        // x⁷/7!
                + (x9 / 362880.0f);     // x⁹/9!

            // Apply sign based on original quadrant
            if (negate)
                result = -result;

            return result;
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

        public static float Min(float x, float y)
        {
            return (x > y) ? y : x;
        }

        public static float Max(float x, float y)
        {
            return (x > y) ? x : y;
        }

        public static Quat QuatFromBasis(Vector3 right, Vector3 up, Vector3 forward)
        {
            float m00 = right.X, m01 = up.X, m02 = forward.X;
            float m10 = right.Y, m11 = up.Y, m12 = forward.Y;
            float m20 = right.Z, m21 = up.Z, m22 = forward.Z;

            float trace = m00 + m11 + m22;

            if (trace > 0.0f)
            {
                float s = SimpleMath.Sqrt(trace + 1.0f) * 2f;
                return new Quat(
                    (m21 - m12) / s,
                    (m02 - m20) / s,
                    (m10 - m01) / s,
                    0.25f * s
                );
            }
            else if (m00 > m11 && m00 > m22)
            {
                float s = SimpleMath.Sqrt(1.0f + m00 - m11 - m22) * 2f;
                return new Quat(
                    0.25f * s,
                    (m01 + m10) / s,
                    (m02 + m20) / s,
                    (m21 - m12) / s
                );
            }
            else if (m11 > m22)
            {
                float s = SimpleMath.Sqrt(1.0f + m11 - m00 - m22) * 2f;
                return new Quat(
                    (m01 + m10) / s,
                    0.25f * s,
                    (m12 + m21) / s,
                    (m02 - m20) / s
                );
            }
            else
            {
                float s = SimpleMath.Sqrt(1.0f + m22 - m00 - m11) * 2f;
                return new Quat(
                    (m02 + m20) / s,
                    (m12 + m21) / s,
                    0.25f * s,
                    (m10 - m01) / s
                );
            }
        }
    }
}
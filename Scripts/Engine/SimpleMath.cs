/// <summary>
/// Simple math utilities that do not rely on System.Math
/// </summary>
namespace Engine
{
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
using System.Runtime.InteropServices;

namespace Engine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quat
    {
        public float X, Y, Z, W;

        public Quat(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        // Static identity quaternion
        public static Quat Identity => new Quat(0f, 0f, 0f, 1f);

        // Squared length (no sqrt)
        public float SqrLength => X * X + Y * Y + Z * Z + W * W;

        // Length using custom sqrt (no System.Math)
        public float Length
        {
            get
            {
                float sq = SqrLength;
                if (sq <= 0.0f)
                    return 0.0f;
                return FastSqrt(sq);
            }
        }

        // Normalized quaternion (no System.Math)
        public Quat Normalized
        {
            get
            {
                float sqLen = SqrLength;
                if (sqLen <= 0.0000001f)
                    return Identity;

                float invLen = 1.0f / FastSqrt(sqLen);
                return new Quat(
                    X * invLen,
                    Y * invLen,
                    Z * invLen,
                    W * invLen
                );
            }
        }

        // Direction vector properties (implemented via internal calls)
        public Vector3 Forward
        {
            get
            {
                InternalCalls.Quat_GetForward(ref this, out Vector3 result);
                return result;
            }
        }

        public Vector3 Right
        {
            get
            {
                InternalCalls.Quat_GetRight(ref this, out Vector3 result);
                return result;
            }
        }

        public Vector3 Up
        {
            get
            {
                InternalCalls.Quat_GetUp(ref this, out Vector3 result);
                return result;
            }
        }

        // =================
        // Operators
        // =================

        public static Quat operator *(Quat lhs, Quat rhs)
        {
            InternalCalls.Quat_Multiply(ref lhs, ref rhs, out Quat result);
            return result;
        }

        // =================
        // Static factories
        // =================

        public static Quat FromAxisAngle(Vector3 axis, float angleRadians)
        {
            InternalCalls.Quat_FromAxisAngle(ref axis, angleRadians, out Quat result);
            return result;
        }

        public static Quat FromEuler(Vector3 euler)
        {
            InternalCalls.Quat_FromEuler(ref euler, out Quat result);
            return result;
        }

        public static float Dot(Quat a, Quat b)
        {
            return InternalCalls.Quat_Dot(ref a, ref b);
        }

        public static Quat Slerp(Quat from, Quat to, float t)
        {
            // Clamp t manually (no System.Math)
            if (t < 0.0f) t = 0.0f;
            else if (t > 1.0f) t = 1.0f;

            InternalCalls.Quat_Slerp(ref from, ref to, t, out Quat result);
            return result;
        }

        // =================
        // Instance methods
        // =================

        public Vector3 Rotate(Vector3 vec)
        {
            InternalCalls.Quat_RotateVector(ref this, ref vec, out Vector3 result);
            return result;
        }

        public Quat Inverse()
        {
            InternalCalls.Quat_Inverse(ref this, out Quat result);
            return result;
        }

        public Vector3 ToEuler()
        {
            InternalCalls.Quat_ToEuler(ref this, out Vector3 result);
            return result;
        }

        public override string ToString()
        {
            return $"Quat({X:F2}, {Y:F2}, {Z:F2}, {W:F2})";
        }

        // =================
        // Internal helpers
        // =================

        /// <summary>
        /// Simple Newton–Raphson sqrt approximation.
        /// No System.Math usage, only basic float ops.
        /// Good enough for normalization & length checks.
        /// </summary>
        private static float FastSqrt(float value)
        {
            if (value <= 0.0f)
                return 0.0f;

            // Initial guess
            float x = value;

            // A few NR iterations
            // x_{n+1} = 0.5 * (x_n + value / x_n)
            for (int i = 0; i < 5; ++i)
            {
                x = 0.5f * (x + value / x);
            }

            return x;
        }
    }
}

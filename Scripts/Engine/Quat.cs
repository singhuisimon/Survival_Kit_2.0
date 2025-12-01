using System;
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

        // Static properties
        public static Quat Identity => new Quat(0f, 0f, 0f, 1f);

        // Properties
        public float Length => (float)Math.Sqrt(X * X + Y * Y + Z * Z + W * W);
        public float SqrLength => X * X + Y * Y + Z * Z + W * W;

        public Quat Normalized
        {
            get
            {
                float len = Length;
                if (len > 0.00001f)
                    return new Quat(X / len, Y / len, Z / len, W / len);
                return Identity;
            }
        }

        // Direction vector properties
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

        // Operators
        public static Quat operator *(Quat lhs, Quat rhs)
        {
            InternalCalls.Quat_Multiply(ref lhs, ref rhs, out Quat result);
            return result;
        }

        // Static methods
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
            t = Math.Max(0f, Math.Min(1f, t));
            InternalCalls.Quat_Slerp(ref from, ref to, t, out Quat result);
            return result;
        }

        // Instance methods
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

        public override string ToString() => $"Quat({X:F2}, {Y:F2}, {Z:F2}, {W:F2})";
    }
}
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Engine
{
    // Native bindings for quaternion operations.
    // Register as "Engine.QuatNative::Quat_*" in mono_add_internal_call.
    public static class QuatNative
    {
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_GetForward(ref Quat quat, out Vector3 forward);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_GetRight(ref Quat quat, out Vector3 right);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_GetUp(ref Quat quat, out Vector3 up);

        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_RotateVector(ref Quat quat, ref Vector3 vec, out Vector3 outVec);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_Multiply(ref Quat q1, ref Quat q2, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_FromAxisAngle(ref Vector3 axis, float angleRadians, out Quat outQuat);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_Slerp(ref Quat q1, ref Quat q2, float t, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_Inverse(ref Quat quat, out Quat outQuat);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_ToEuler(ref Quat quat, out Vector3 euler);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_FromEuler(ref Vector3 euler, out Quat outQuat);

        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Quat_Normalize(ref Quat quat, out Quat outQuat);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern float Quat_Length(ref Quat quat);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern float Quat_Dot(ref Quat q1, ref Quat q2);
    }

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

        public static Quat Identity => new Quat(0f, 0f, 0f, 1f);

        public float SqrLength => X * X + Y * Y + Z * Z + W * W;

        // Prefer native if you have it (consistent + fast)
        public float Length => QuatNative.Quat_Length(ref this);

        public Quat Normalized
        {
            get
            {
                QuatNative.Quat_Normalize(ref this, out Quat outQ);
                return outQ;
            }
        }

        public Vector3 Forward
        {
            get
            {
                QuatNative.Quat_GetForward(ref this, out Vector3 v);
                return v;
            }
        }

        public Vector3 Right
        {
            get
            {
                QuatNative.Quat_GetRight(ref this, out Vector3 v);
                return v;
            }
        }

        public Vector3 Up
        {
            get
            {
                QuatNative.Quat_GetUp(ref this, out Vector3 v);
                return v;
            }
        }

        public static Quat operator *(Quat lhs, Quat rhs)
        {
            QuatNative.Quat_Multiply(ref lhs, ref rhs, out Quat outQ);
            return outQ;
        }

        public static Quat FromAxisAngle(Vector3 axis, float angleRadians)
        {
            QuatNative.Quat_FromAxisAngle(ref axis, angleRadians, out Quat outQ);
            return outQ;
        }

        public static Quat FromEuler(Vector3 euler)
        {
            QuatNative.Quat_FromEuler(ref euler, out Quat outQ);
            return outQ;
        }

        public static float Dot(Quat a, Quat b)
        {
            return QuatNative.Quat_Dot(ref a, ref b);
        }

        public static Quat Slerp(Quat from, Quat to, float t)
        {
            if (t < 0.0f) t = 0.0f;
            else if (t > 1.0f) t = 1.0f;

            QuatNative.Quat_Slerp(ref from, ref to, t, out Quat outQ);
            return outQ;
        }

        public Vector3 Rotate(Vector3 vec)
        {
            QuatNative.Quat_RotateVector(ref this, ref vec, out Vector3 outV);
            return outV;
        }

        public Quat Inverse()
        {
            QuatNative.Quat_Inverse(ref this, out Quat outQ);
            return outQ;
        }

        public Vector3 ToEuler()
        {
            QuatNative.Quat_ToEuler(ref this, out Vector3 euler);
            return euler;
        }

        public override string ToString()
        {
            return $"Quat({X:F2}, {Y:F2}, {Z:F2}, {W:F2})";
        }
    }
}

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

    /// <summary>
    /// Blittable quaternion value type (X,Y,Z,W) that works with internal calls using ref/out.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Quat
    {
        public float X;
        public float Y;
        public float Z;
        public float W;


        public Quat(float x, float y, float z, float w)
        {
            X = x; Y = y; Z = z; W = w;
        }

        /// <summary>Identity quaternion (no rotation).</summary>
        public static Quat Identity => new Quat(0f, 0f, 0f, 1f);

        // --------------------------------------------------------------------
        // Direction vectors (computed via native calls)
        // NOTE: avoid `ref this` in getters (treats `this` as readonly)
        // --------------------------------------------------------------------

        public Vector3 Forward
        {
            get
            {
                var q = this;
                QuatNative.Quat_GetForward(ref q, out var v);
                return v;
            }
        }

        public Vector3 Right
        {
            get
            {
                var q = this;
                QuatNative.Quat_GetRight(ref q, out var v);
                return v;
            }
        }

        public Vector3 Up
        {
            get
            {
                var q = this;
                QuatNative.Quat_GetUp(ref q, out var v);
                return v;
            }
        }

        // --------------------------------------------------------------------
        // Core ops (routed to native)
        // --------------------------------------------------------------------


        public float Length()
        {
            var q = this;
            return QuatNative.Quat_Length(ref q);
        }


        public Quat Normalized()
        {
            var q = this;
            QuatNative.Quat_Normalize(ref q, out var outQ);
            return outQ;
        }


        public void NormalizeInPlace()
        {
            var q = this;
            QuatNative.Quat_Normalize(ref q, out var outQ);
            this = outQ;
        }


        public Quat Inverse()
        {
            var q = this;
            QuatNative.Quat_Inverse(ref q, out var outQ);
            return outQ;
        }


        public Vector3 ToEuler()
        {
            var q = this;
            QuatNative.Quat_ToEuler(ref q, out var e);
            return e;
        }


        public Vector3 RotateVector(Vector3 v)
        {
            var q = this;
            QuatNative.Quat_RotateVector(ref q, ref v, out var outV);
            return outV;
        }


        public static float Dot(Quat a, Quat b)
            => QuatNative.Quat_Dot(ref a, ref b);


        public static Quat operator *(Quat a, Quat b)
        {
            QuatNative.Quat_Multiply(ref a, ref b, out var outQ);
            return outQ;
        }


        public static Quat Multiply(Quat a, Quat b)
        {
            QuatNative.Quat_Multiply(ref a, ref b, out var outQ);
            return outQ;
        }


        public static Quat Slerp(Quat a, Quat b, float t)
        {
            QuatNative.Quat_Slerp(ref a, ref b, t, out var outQ);
            return outQ;
        }


        public static Quat FromAxisAngle(Vector3 axis, float angleRadians)
        {
            QuatNative.Quat_FromAxisAngle(ref axis, angleRadians, out var outQ);
            return outQ;
        }


        public static Quat FromEuler(Vector3 eulerRadians)
        {
            QuatNative.Quat_FromEuler(ref eulerRadians, out var outQ);
            return outQ;
        }

        // --------------------------------------------------------------------
        // Equality / Hash (no System.HashCode dependency)
        // --------------------------------------------------------------------

        public bool Equals(Quat other)
            => X == other.X && Y == other.Y && Z == other.Z && W == other.W;

        public override bool Equals(object obj)
            => obj is Quat q && Equals(q);

        public override int GetHashCode()
        {
            unchecked
            {
                int h = 17;
                h = (h * 31) + X.GetHashCode();
                h = (h * 31) + Y.GetHashCode();
                h = (h * 31) + Z.GetHashCode();
                h = (h * 31) + W.GetHashCode();
                return h;
            }
        }

        public static bool operator ==(Quat a, Quat b) => a.Equals(b);
        public static bool operator !=(Quat a, Quat b) => !a.Equals(b);

        public override string ToString() => $"Quat({X}, {Y}, {Z}, {W})";
    }
}

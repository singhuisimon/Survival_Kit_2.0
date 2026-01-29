using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Engine.Transform;

namespace Engine
{
    /// <summary>
    /// Simple math utilities intended to avoid System.Math usage.
    /// Includes scalar helpers, Vector2/3/4, and Quat + native bindings.
    /// </summary>
    public static class SimpleMath
    {
        public const float PI = 3.14159265359f;
        public const float TWO_PI = 6.28318530718f;
        public const float HALF_PI = 1.57079632679f;
        public const float DEG_TO_RAD = 0.0174532924f;
        public const float RAD_TO_DEG = 57.2957795131f;

        // Tunables
        public const float EPSILON = 1e-6f;

        // -------------------------
        // Basic helpers
        // -------------------------

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Abs(float v) => (v < 0f) ? -v : v;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int Abs(int v) => (v < 0) ? -v : v;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Min(float a, float b) => (a < b) ? a : b;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Max(float a, float b) => (a > b) ? a : b;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Clamp(float v, float min, float max)
        {
            if (v < min) return min;
            if (v > max) return max;
            return v;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Clamp01(float v) => Clamp(v, 0f, 1f);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Sign(float v) => (v < 0f) ? -1f : 1f;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool Approximately(float a, float b, float eps = 1e-5f)
            => Abs(a - b) <= eps;

        // -------------------------
        // Integer-ish float ops (no System.Math)
        // -------------------------

        public static int FloorToInt(float v)
        {
            int i = (int)v;
            if (v < 0f && v != i) i -= 1;
            return i;
        }

        public static int CeilToInt(float v)
        {
            int i = (int)v;
            if (v > 0f && v != i) i += 1;
            return i;
        }

        public static int RoundToInt(float v)
        {
            // Banker's rounding not needed; simple .5 up.
            return (v >= 0f) ? (int)(v + 0.5f) : (int)(v - 0.5f);
        }

        public static float Repeat(float t, float length)
        {
            if (length <= EPSILON) return 0f;
            float x = t - FloorToInt(t / length) * length;
            return Clamp(x, 0f, length);
        }

        public static float DeltaAngle(float currentDeg, float targetDeg)
        {
            float delta = Repeat((targetDeg - currentDeg), 360f);
            if (delta > 180f) delta -= 360f;
            return delta;
        }

        // -------------------------
        // Lerp / Smooth
        // -------------------------

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Lerp(float a, float b, float t)
        {
            t = Clamp01(t);
            return a + (b - a) * t;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float LerpUnclamped(float a, float b, float t)
            => a + (b - a) * t;

        public static float SmoothStep(float a, float b, float t)
        {
            t = Clamp01(t);
            // Hermite: 3t^2 - 2t^3
            t = t * t * (3f - 2f * t);
            return a + (b - a) * t;
        }

        public static float MoveTowards(float current, float target, float maxDelta)
        {
            float delta = target - current;
            float ad = Abs(delta);
            if (ad <= maxDelta || ad < EPSILON) return target;
            return current + Sign(delta) * maxDelta;
        }

        // -------------------------
        // Sqrt (Newton) - from your existing code
        // -------------------------

        public static float Sqrt(float value)
        {
            if (value <= 0.0f) return 0.0f;
            if (value == 1.0f) return 1.0f;

            float x = value;
            float y = 1.0f;
            const float epsilon = 0.0001f;

            for (int i = 0; i < 6; i++)
            {
                y = 0.5f * (x + value / x);
                float diff = x - y;
                if (diff < epsilon && diff > -epsilon)
                    break;
                x = y;
            }

            return y;
        }

        // -------------------------
        // Trig (fast approximations)
        // -------------------------
        // These are lightweight approximations good enough for gameplay math.
        // If you want perfect parity with GLM, route these to native later.

        public static float WrapPi(float rad)
        {
            // Map to [-pi, pi]
            rad = rad % TWO_PI;
            if (rad > PI) rad -= TWO_PI;
            if (rad < -PI) rad += TWO_PI;
            return rad;
        }

        public static float Sin(float rad)
        {
            // Fast sine approximation using a minimax-ish polynomial on [-pi, pi]
            rad = WrapPi(rad);

            float x = rad;
            float x2 = x * x;

            // sin(x) ~ x - x^3/6 + x^5/120 - x^7/5040
            float x3 = x * x2;
            float x5 = x3 * x2;
            float x7 = x5 * x2;
            return x - (x3 * 0.16666667f) + (x5 * 0.008333333f) - (x7 * 0.0001984127f);
        }

        public static float Cos(float rad)
        {
            // cos(x) = sin(x + pi/2)
            return Sin(rad + HALF_PI);
        }

        public static float Tan(float rad)
        {
            float c = Cos(rad);
            if (Abs(c) < 1e-5f) return (Sin(rad) >= 0f) ? float.PositiveInfinity : float.NegativeInfinity;
            return Sin(rad) / c;
        }

        // -------------------------
        // Atan / Atan2 / Asin / Acos - from your existing code (+ Acos)
        // -------------------------

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

        public static float Atan(float x) => Atan2(x, 1.0f);

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

        public static float Acos(float x)
        {
            // acos(x) = pi/2 - asin(x)
            return HALF_PI - Asin(x);
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

        public static Vector3 LocalChildtoWorld (uint childID){
            Vector3 childPosition = GetPosition((uint)childID);
            Vector3 parentPosition = GetPosition(TransformGetParent((uint)childID));
            return parentPosition + childPosition;
        }

        public static Vector3 QuatMultiplyVec3 (Quat q, Vector3 v)
        {
            float x = q.X;
            float y = q.Y;
            float z = q.Z;
            float w = q.W;

            // Cross product:
            float crossX = 2.0f * (y * v.Z - z * v.Y);
            float crossY = 2.0f * (z * v.X - x * v.Z);
            float crossZ = 2.0f * (x * v.Y - y * v.X);

            // Final rotated vector: v + w*t + cross(q.xyz, t)
            float rotatedX = v.X + w * crossX + (y * crossZ - z * crossY);
            float rotatedY = v.Y + w * crossY + (z * crossX - x * crossZ);
            float rotatedZ = v.Z + w * crossZ + (x * crossY - y * crossX);

            return new Engine.Vector3(rotatedX, rotatedY, rotatedZ);
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

        /// <summary>
        /// Creates a rotation quaternion that looks in the specified direction
        /// </summary>
        public static Quat LookRotation(Vector3 forward, Vector3 up)
        {
            forward = forward.Normalized;
            
            // Calculate right vector
            Vector3 right = Vector3.Cross(up, forward).Normalized;
            
            // Recalculate up to ensure orthogonality
            up = Vector3.Cross(forward, right).Normalized;
            
            // Build quaternion from rotation matrix
            return FromMatrix(right, up, forward);
        }

        /// <summary>
        /// Creates a quaternion from three orthonormal basis vectors
        /// </summary>
        private static Quat FromMatrix(Vector3 right, Vector3 up, Vector3 forward)
        {
            // This is the standard matrix-to-quaternion conversion
            // Matrix layout:
            // [ right.x   up.x   forward.x ]
            // [ right.y   up.y   forward.y ]
            // [ right.z   up.z   forward.z ]
            
            float trace = right.X + up.Y + forward.Z;
            
            if (trace > 0.0f)
            {
                float s = Sqrt(trace + 1.0f) * 2.0f;
                float invS = 1.0f / s;
                
                return new Quat(
                    (up.Z - forward.Y) * invS,      // x
                    (forward.X - right.Z) * invS,   // y
                    (right.Y - up.X) * invS,        // z
                    s * 0.25f                       // w
                );
            }
            else if (right.X > up.Y && right.X > forward.Z)
            {
                float s = Sqrt(1.0f + right.X - up.Y - forward.Z) * 2.0f;
                float invS = 1.0f / s;
                
                return new Quat(
                    s * 0.25f,                      // x
                    (right.Y + up.X) * invS,        // y
                    (forward.X + right.Z) * invS,   // z
                    (up.Z - forward.Y) * invS       // w
                );
            }
            else if (up.Y > forward.Z)
            {
                float s = Sqrt(1.0f + up.Y - right.X - forward.Z) * 2.0f;
                float invS = 1.0f / s;
                
                return new Quat(
                    (right.Y + up.X) * invS,        // x
                    s * 0.25f,                      // y
                    (up.Z + forward.Y) * invS,      // z
                    (forward.X - right.Z) * invS    // w
                );
            }
            else
            {
                float s = Sqrt(1.0f + forward.Z - right.X - up.Y) * 2.0f;
                float invS = 1.0f / s;
                
                return new Quat(
                    (forward.X + right.Z) * invS,   // x
                    (up.Z + forward.Y) * invS,      // y
                    s * 0.25f,                      // z
                    (right.Y - up.X) * invS         // w
                );
            }
        }


    }

    // =====================================================================
    // VECTORS
    // =====================================================================

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2
    {
        public float X, Y;

        public Vector2(float x, float y) { X = x; Y = y; }
        public Vector2(float scalar) : this(scalar, scalar) { }

        public static Vector2 Zero => new Vector2(0, 0);
        public static Vector2 One => new Vector2(1, 1);
        public static Vector2 Up => new Vector2(0, 1);
        public static Vector2 Down => new Vector2(0, -1);
        public static Vector2 Left => new Vector2(-1, 0);
        public static Vector2 Right => new Vector2(1, 0);

        public float SqrMagnitude => X * X + Y * Y;
        public float Magnitude => SimpleMath.Sqrt(SqrMagnitude);

        public Vector2 Normalized
        {
            get
            {
                float m = Magnitude;
                if (m > 1e-6f) return this / m;
                return Zero;
            }
        }

        public static Vector2 operator +(Vector2 a, Vector2 b) => new Vector2(a.X + b.X, a.Y + b.Y);
        public static Vector2 operator -(Vector2 a, Vector2 b) => new Vector2(a.X - b.X, a.Y - b.Y);
        public static Vector2 operator *(Vector2 v, float s) => new Vector2(v.X * s, v.Y * s);
        public static Vector2 operator *(float s, Vector2 v) => new Vector2(v.X * s, v.Y * s);
        public static Vector2 operator /(Vector2 v, float s) => new Vector2(v.X / s, v.Y / s);
        public static Vector2 operator -(Vector2 v) => new Vector2(-v.X, -v.Y);

        public static float Dot(Vector2 a, Vector2 b) => a.X * b.X + a.Y * b.Y;

        public static float Distance(Vector2 a, Vector2 b) => (b - a).Magnitude;

        public static Vector2 Lerp(Vector2 a, Vector2 b, float t)
        {
            t = SimpleMath.Clamp01(t);
            return a + (b - a) * t;
        }

        public override string ToString() => $"Vector2({X}, {Y})";
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float X, Y, Z;

        public Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }
        public Vector3(float scalar) : this(scalar, scalar, scalar) { }
        public Vector3(Vector3 other) : this(other.X, other.Y, other.Z) { }

        public static Vector3 Zero => new Vector3(0, 0, 0);
        public static Vector3 One => new Vector3(1, 1, 1);

        // Unity-ish basis: Forward = -Z (matches your existing file)
        public static Vector3 Up => new Vector3(0, 1, 0);
        public static Vector3 Down => new Vector3(0, -1, 0);
        public static Vector3 Left => new Vector3(-1, 0, 0);
        public static Vector3 Right => new Vector3(1, 0, 0);
        public static Vector3 Forward => new Vector3(0, 0, -1);
        public static Vector3 Back => new Vector3(0, 0, 1);

        public float SqrMagnitude => X * X + Y * Y + Z * Z;
        public float Magnitude => SimpleMath.Sqrt(SqrMagnitude);

        public Vector3 Normalized
        {
            get
            {
                float m = Magnitude;
                if (m > 1e-6f) return this / m;
                return Zero;
            }
        }

        public static Vector3 operator +(Vector3 a, Vector3 b) => new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        public static Vector3 operator -(Vector3 a, Vector3 b) => new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        public static Vector3 operator *(Vector3 v, float s) => new Vector3(v.X * s, v.Y * s, v.Z * s);
        public static Vector3 operator *(float s, Vector3 v) => new Vector3(v.X * s, v.Y * s, v.Z * s);
        public static Vector3 operator /(Vector3 v, float s) => new Vector3(v.X / s, v.Y / s, v.Z / s);
        public static Vector3 operator -(Vector3 v) => new Vector3(-v.X, -v.Y, -v.Z);

        public static float Dot(Vector3 a, Vector3 b) => a.X * b.X + a.Y * b.Y + a.Z * b.Z;

        public static Vector3 Cross(Vector3 a, Vector3 b) => new Vector3(
            a.Y * b.Z - a.Z * b.Y,
            a.Z * b.X - a.X * b.Z,
            a.X * b.Y - a.Y * b.X
        );

        public static float Distance(Vector3 a, Vector3 b) => (b - a).Magnitude;

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            t = SimpleMath.Clamp01(t);
            return a + (b - a) * t;
        }

        public static Vector3 Project(Vector3 v, Vector3 onNormal)
        {
            float denom = Dot(onNormal, onNormal);
            if (denom < 1e-8f) return Zero;
            return onNormal * (Dot(v, onNormal) / denom);
        }

        public static Vector3 Reflect(Vector3 inDir, Vector3 inNormal)
        {
            // r = v - 2*dot(v,n)*n
            float d = Dot(inDir, inNormal);
            return inDir - 2f * d * inNormal;
        }

        public static float Angle(Vector3 a, Vector3 b)
        {
            float denom = a.Magnitude * b.Magnitude;
            if (denom < 1e-8f) return 0f;
            float c = Dot(a, b) / denom;
            c = SimpleMath.Clamp(c, -1f, 1f);
            return SimpleMath.Acos(c) * SimpleMath.RAD_TO_DEG;
        }

        public override string ToString() => $"Vector3({X}, {Y}, {Z})";
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector4
    {
        public float X, Y, Z, W;

        public Vector4(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }
        public Vector4(float scalar) : this(scalar, scalar, scalar, scalar) { }

        public static Vector4 operator +(Vector4 a, Vector4 b) => new Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        public static Vector4 operator -(Vector4 a, Vector4 b) => new Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        public static Vector4 operator *(Vector4 v, float s) => new Vector4(v.X * s, v.Y * s, v.Z * s, v.W * s);
        public static Vector4 operator /(Vector4 v, float s) => new Vector4(v.X / s, v.Y / s, v.Z / s, v.W / s);
        public static Vector4 operator -(Vector4 v) => new Vector4(-v.X, -v.Y, -v.Z, -v.W);

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            t = SimpleMath.Clamp01(t);
            return a + (b - a) * t;
        }

        public override string ToString() => $"Vector4({X}, {Y}, {Z}, {W})";
    }

    // =====================================================================
    // QUATERNIONS (native-backed)
    // =====================================================================

    /// <summary>
    /// Native bindings for quaternion operations.
    /// Register as "Engine.QuatNative::Quat_*" in mono_add_internal_call.
    /// </summary>
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
        public float W;
        public float X;
        public float Y;
        public float Z;

        public Quat(float w, float x, float y, float z)
        {
            X = x; Y = y; Z = z; W = w;
        }

        public static Quat Identity => new Quat(0f, 0f, 0f, 1f);

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

        public bool Equals(Quat other)
            => X == other.X && Y == other.Y && Z == other.Z && W == other.W;

        public override bool Equals(object obj)
            => obj is Quat q && Equals(q);

        public static bool operator ==(Quat a, Quat b) => a.Equals(b);
        public static bool operator !=(Quat a, Quat b) => !a.Equals(b);

        public override string ToString() => $"Quat({X}, {Y}, {Z}, {W})";
    }

    public static class RNG
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Seed(uint seed);

        // Inclusive: [min, max]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int RandInt(int min, int max);

        // Range: [min, max) (typical for floats)
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float RandFloat(float min, float max);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool RandBool();
    }
}

using System;

namespace Engine
{
    public struct Vector3
    {
        public float X, Y, Z;

        public Vector3(float x, float y, float z)
        {
            X = x; Y = y; Z = z;
        }

        public Vector3(float scalar) : this(scalar, scalar, scalar) { }

        public Vector3(Vector3 other) : this(other.X, other.Y, other.Z) { }

        // Static properties
        public static Vector3 Zero => new Vector3(0, 0, 0);
        public static Vector3 One => new Vector3(1, 1, 1);
        public static Vector3 Up => new Vector3(0, 1, 0);
        public static Vector3 Down => new Vector3(0, -1, 0);
        public static Vector3 Left => new Vector3(-1, 0, 0);
        public static Vector3 Right => new Vector3(1, 0, 0);
        public static Vector3 Forward => new Vector3(0, 0, 1);
        public static Vector3 Back => new Vector3(0, 0, -1);

        // Properties
        public float Magnitude => (float)SimpleMath.Sqrt(X * X + Y * Y + Z * Z);
        public float SqrMagnitude => X * X + Y * Y + Z * Z;
        public Vector3 Normalized
        {
            get
            {
                float mag = Magnitude;
                if (mag > 0.00001f)
                    return new Vector3(X / mag, Y / mag, Z / mag);
                return Zero;
            }
        }

        // Operators
        public static Vector3 operator +(Vector3 a, Vector3 b) =>
            new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        public static Vector3 operator -(Vector3 a, Vector3 b) =>
            new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        public static Vector3 operator *(Vector3 v, float scalar) =>
            new Vector3(v.X * scalar, v.Y * scalar, v.Z * scalar);
        public static Vector3 operator /(Vector3 v, float scalar) =>
            new Vector3(v.X / scalar, v.Y / scalar, v.Z / scalar);
        public static Vector3 operator -(Vector3 v) =>
            new Vector3(-v.X, -v.Y, -v.Z);

        // Static methods
        public static float Dot(Vector3 a, Vector3 b) =>
            a.X * b.X + a.Y * b.Y + a.Z * b.Z;

        public static Vector3 Cross(Vector3 a, Vector3 b) => new Vector3(
            a.Y * b.Z - a.Z * b.Y,
            a.Z * b.X - a.X * b.Z,
            a.X * b.Y - a.Y * b.X
        );

        public static float Distance(Vector3 a, Vector3 b) => (b - a).Magnitude;

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            t = SimpleMath.Max(0, SimpleMath.Min(1, t));
            return a + (b - a) * t;
        }
    }

    public struct Vector4
    {
        public float X, Y, Z, W;

        public Vector4(float x, float y, float z, float w)
        {
            X = x; Y = y; Z = z; W = w;
        }

        public Vector4(float scalar) : this(scalar, scalar, scalar, scalar) { }

        // Operators
        public static Vector4 operator +(Vector4 a, Vector4 b) => new Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        public static Vector4 operator -(Vector4 a, Vector4 b) => new Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        public static Vector4 operator *(Vector4 v, float scalar) => new Vector4(v.X * scalar, v.Y * scalar, v.Z * scalar, v.W * scalar);
        public static Vector4 operator /(Vector4 v, float scalar) => new Vector4(v.X / scalar, v.Y / scalar, v.Z / scalar, v.W / scalar);
        public static Vector4 operator -(Vector4 v) => new Vector4(-v.X, -v.Y, -v.Z, -v.W);

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            t = SimpleMath.Max(0, SimpleMath.Min(1, t));
            return a + (b - a) * t;
        }
    }

    public struct Vector2
    {
        public float X, Y;

        public Vector2(float x, float y) { X = x; Y = y; }

        public static float Distance(Vector2 a, Vector2 b) =>
            (float)SimpleMath.Sqrt((b.X - a.X) * (b.X - a.X) + (b.Y - a.Y) * (b.Y - a.Y));
    }
}
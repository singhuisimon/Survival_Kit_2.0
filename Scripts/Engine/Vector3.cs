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
        public float Magnitude => (float)Math.Sqrt(X * X + Y * Y + Z * Z);
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
            t = Math.Max(0, Math.Min(1, t));
            return a + (b - a) * t;
        }

        public override string ToString() => $"({X:F2}, {Y:F2}, {Z:F2})";
    }

    public struct Vector2
    {
        public float X, Y;

        public Vector2(float x, float y) { X = x; Y = y; }

        public static float Distance(Vector2 a, Vector2 b) =>
            (float)Math.Sqrt((b.X - a.X) * (b.X - a.X) + (b.Y - a.Y) * (b.Y - a.Y));
    }
}
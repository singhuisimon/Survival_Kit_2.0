using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// BeamRenderer component bindings for controlling beam visual properties from C#.
    /// </summary>
    public static class BeamRenderer
    {
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetActive(ulong entityID, bool active);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Beam_GetActive(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetStartColor(ulong entityID, ref Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_GetStartColor(ulong entityID, out Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetEndColor(ulong entityID, ref Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_GetEndColor(ulong entityID, out Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetStartWidth(ulong entityID, float width);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Beam_GetStartWidth(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetEndWidth(ulong entityID, float width);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Beam_GetEndWidth(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetNoiseAmplitude(ulong entityID, float amplitude);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Beam_GetNoiseAmplitude(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetNoiseSpeed(ulong entityID, float speed);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Beam_GetNoiseSpeed(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetUVScrollSpeed(ulong entityID, float speed);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Beam_GetUVScrollSpeed(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetEndPointOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_GetEndPointOffset(ulong entityID, out Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetStartOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_GetStartOffset(ulong entityID, out Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Beam_SetTargetEntity(ulong entityID, ulong targetID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern ulong Beam_GetTargetEntity(ulong entityID);

        public static void SetActive(ulong entityID, bool active) => Beam_SetActive(entityID, active);
        public static bool GetActive(ulong entityID) => Beam_GetActive(entityID);
        public static void SetStartColor(ulong entityID, Vector4 color) => Beam_SetStartColor(entityID, ref color);
        public static Vector4 GetStartColor(ulong entityID) { Beam_GetStartColor(entityID, out var v); return v; }
        public static void SetEndColor(ulong entityID, Vector4 color) => Beam_SetEndColor(entityID, ref color);
        public static Vector4 GetEndColor(ulong entityID) { Beam_GetEndColor(entityID, out var v); return v; }
        public static void SetStartWidth(ulong entityID, float width) => Beam_SetStartWidth(entityID, width);
        public static float GetStartWidth(ulong entityID) => Beam_GetStartWidth(entityID);
        public static void SetEndWidth(ulong entityID, float width) => Beam_SetEndWidth(entityID, width);
        public static float GetEndWidth(ulong entityID) => Beam_GetEndWidth(entityID);
        public static void SetNoiseAmplitude(ulong entityID, float amplitude) => Beam_SetNoiseAmplitude(entityID, amplitude);
        public static float GetNoiseAmplitude(ulong entityID) => Beam_GetNoiseAmplitude(entityID);
        public static void SetNoiseSpeed(ulong entityID, float speed) => Beam_SetNoiseSpeed(entityID, speed);
        public static float GetNoiseSpeed(ulong entityID) => Beam_GetNoiseSpeed(entityID);
        public static void SetUVScrollSpeed(ulong entityID, float speed) => Beam_SetUVScrollSpeed(entityID, speed);
        public static float GetUVScrollSpeed(ulong entityID) => Beam_GetUVScrollSpeed(entityID);
        public static void SetEndPointOffset(ulong entityID, Vector3 offset) => Beam_SetEndPointOffset(entityID, ref offset);
        public static Vector3 GetEndPointOffset(ulong entityID) { Beam_GetEndPointOffset(entityID, out var v); return v; }
        public static void SetStartOffset(ulong entityID, Vector3 offset) => Beam_SetStartOffset(entityID, ref offset);
        public static Vector3 GetStartOffset(ulong entityID) { Beam_GetStartOffset(entityID, out var v); return v; }

        /// <summary>Pass 0 to clear the target and fall back to EndPointOffset.</summary>
        public static void SetTargetEntity(ulong entityID, ulong targetID) => Beam_SetTargetEntity(entityID, targetID);
        public static ulong GetTargetEntity(ulong entityID) => Beam_GetTargetEntity(entityID);
    }
}
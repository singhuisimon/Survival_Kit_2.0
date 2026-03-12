using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Global render settings bindings exposed to C#.
    /// </summary>
    public static class RenderSettings
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Renderer_GetGamma();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Renderer_SetGamma(float gamma);

        public static float GetGamma() => Renderer_GetGamma();

        public static void SetGamma(float gamma) => Renderer_SetGamma(gamma);
    }
}
using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// SpriteRenderer component bindings for controlling 2D sprite visibility and color
    /// </summary>
    public static class SpriteRenderer
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SpriteRenderer_SetIsVisible(uint entityID, bool visible);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool SpriteRenderer_GetIsVisible(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SpriteRenderer_SetColor(uint entityID, float r, float g, float b, float a);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SpriteRenderer_GetColor(uint entityID, out float r, out float g, out float b, out float a);

        /// <summary>
        /// Set sprite visibility (show/hide)
        /// </summary>
        public static void SetIsVisible(uint entityID, bool visible)
        {
            SpriteRenderer_SetIsVisible(entityID, visible);
        }

        /// <summary>
        /// Get sprite visibility status
        /// </summary>
        public static bool GetIsVisible(uint entityID)
        {
            return SpriteRenderer_GetIsVisible(entityID);
        }

        /// <summary>
        /// Set sprite color (RGBA)
        /// </summary>
        public static void SetColor(uint entityID, float r, float g, float b, float a)
        {
            SpriteRenderer_SetColor(entityID, r, g, b, a);
        }

        /// <summary>
        /// Get sprite color (RGBA)
        /// </summary>
        public static void GetColor(uint entityID, out float r, out float g, out float b, out float a)
        {
            SpriteRenderer_GetColor(entityID, out r, out g, out b, out a);
        }
    }
}
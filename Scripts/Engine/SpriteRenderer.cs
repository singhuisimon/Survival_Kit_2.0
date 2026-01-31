using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// SpriteRenderer component bindings for controlling 2D sprite visibility
    /// </summary>
    public static class SpriteRenderer
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SpriteRenderer_SetIsVisible(uint entityID, bool visible);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool SpriteRenderer_GetIsVisible(uint entityID);

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
    }
}
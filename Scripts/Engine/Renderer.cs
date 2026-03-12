using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over MeshRendererComponent.
    /// Lets scripts toggle visibility and basic lighting flags.
    /// </summary>
    public class MeshRenderer : Component
    {
        // Native bindings (register as: "Engine.MeshRenderer::MeshRenderer_*")
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool MeshRenderer_GetVisible(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void MeshRenderer_SetVisible(uint entityID, bool visible);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool MeshRenderer_GetShadowCast(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void MeshRenderer_SetShadowCast(uint entityID, bool cast);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool MeshRenderer_GetShadowReceive(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void MeshRenderer_SetShadowReceive(uint entityID, bool receive);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool MeshRenderer_GetGlobalIlluminate(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void MeshRenderer_SetGlobalIlluminate(uint entityID, bool gi);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void MeshRenderer_SetEmissiveTexture(uint entityID, string textureName);

        public static bool GetVisible(uint entityID) => MeshRenderer_GetVisible(entityID);
        public static void SetVisible(uint entityID, bool visible) => MeshRenderer_SetVisible(entityID, visible);

        public static bool GetShadowCast(uint entityID) => MeshRenderer_GetShadowCast(entityID);
        public static void SetShadowCast(uint entityID, bool cast) => MeshRenderer_SetShadowCast(entityID, cast);

        public static bool GetShadowReceive(uint entityID) => MeshRenderer_GetShadowReceive(entityID);
        public static void SetShadowReceive(uint entityID, bool receive) => MeshRenderer_SetShadowReceive(entityID, receive);

        public static bool GetGlobalIlluminate(uint entityID) => MeshRenderer_GetGlobalIlluminate(entityID);
        public static void SetGlobalIlluminate(uint entityID, bool gi) => MeshRenderer_SetGlobalIlluminate(entityID, gi);

        public static void SetEmissiveTexture(uint entityID, string textureName) => MeshRenderer_SetEmissiveTexture(entityID, textureName);

        private uint ID => Entity.EntityID;

        public bool Visible
        {
            get => MeshRenderer_GetVisible(ID);
            set => MeshRenderer_SetVisible(ID, value);
        }

        public bool ShadowCast
        {
            get => MeshRenderer_GetShadowCast(ID);
            set => MeshRenderer_SetShadowCast(ID, value);
        }

        public bool ShadowReceive
        {
            get => MeshRenderer_GetShadowReceive(ID);
            set => MeshRenderer_SetShadowReceive(ID, value);
        }

        public bool GlobalIlluminate
        {
            get => MeshRenderer_GetGlobalIlluminate(ID);
            set => MeshRenderer_SetGlobalIlluminate(ID, value);
        }
    }
}

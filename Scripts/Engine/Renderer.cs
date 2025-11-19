using System;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over MeshRendererComponent.
    /// Lets scripts toggle visibility and basic lighting flags.
    /// </summary>
    public class MeshRenderer : Component
    {
        public bool Visible
        {
            get { return InternalCalls.MeshRenderer_GetVisible(Entity.EntityID); }
            set { InternalCalls.MeshRenderer_SetVisible(Entity.EntityID, value); }
        }

        public bool ShadowCast
        {
            get { return InternalCalls.MeshRenderer_GetShadowCast(Entity.EntityID); }
            set { InternalCalls.MeshRenderer_SetShadowCast(Entity.EntityID, value); }
        }

        public bool ShadowReceive
        {
            get { return InternalCalls.MeshRenderer_GetShadowReceive(Entity.EntityID); }
            set { InternalCalls.MeshRenderer_SetShadowReceive(Entity.EntityID, value); }
        }

        public bool GlobalIlluminate
        {
            get { return InternalCalls.MeshRenderer_GetGlobalIlluminate(Entity.EntityID); }
            set { InternalCalls.MeshRenderer_SetGlobalIlluminate(Entity.EntityID, value); }
        }
    }
}

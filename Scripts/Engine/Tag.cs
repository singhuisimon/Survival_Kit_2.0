using System;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over TagComponent – human-readable name for an entity.
    /// </summary>
    public class Tag : Component
    {
        /// <summary>
        /// The tag / name string of this entity.
        /// </summary>
        public string Value
        {
            get { return InternalCalls.Tag_GetTag(Entity.EntityID); }
            set { InternalCalls.Tag_SetTag(Entity.EntityID, value); }
        }

        public override string ToString()
        {
            return Value;
        }
    }
}

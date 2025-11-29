using System;
using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Represents an entity in the game world
    /// This is a wrapper around the native C++ Entity
    /// </summary>
    public class Entity
    {
        /// <summary>
        /// The native entity ID (entt::entity)
        /// </summary>
        public uint EntityID { get; internal set; }

        public Entity(uint entityID)
        {
            EntityID = entityID;
        }

        // Entity methods
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint GetEntityID_Native(Entity entity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(uint entityID, Type componentType);

        public bool HasComponent<T>() where T : Component
        {
            return HasComponent_Native(EntityID, typeof(T));
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T();
            component.Entity = this;
            return component;
        }
    }
}

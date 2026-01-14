namespace Engine
{
    /// <summary>
    /// Base class for all game scripts
    /// Inherit from this to create gameplay logic in C#
    /// </summary>
    public abstract class ScriptBehaviour
    {
        /// <summary>
        /// The entity this script is attached to
        /// Set automatically by the engine
        /// </summary>
        public uint EntityID { get; set; }

        public virtual void OnStart() { }
        public virtual void OnUpdate(float deltaTime) { }
        public virtual void OnDestroy() { }

        // Helper properties
        public Entity Entity => new Entity(EntityID);
        public Transform Transform => Entity.GetComponent<Transform>();
    }
}

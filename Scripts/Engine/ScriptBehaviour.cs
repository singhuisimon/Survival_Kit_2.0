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

        /// <summary>
        /// Called once when the script is first created
        /// Use this for initialization
        /// </summary>
        public virtual void OnStart() { }

        /// <summary>
        /// Called every frame
        /// </summary>
        /// <param name="deltaTime">Time since last frame in seconds</param>
        public virtual void OnUpdate(float deltaTime) { }

        /// <summary>
        /// Called when the entity is destroyed
        /// Use this for cleanup
        /// </summary>
        public virtual void OnDestroy() { }

        // Helper properties
        public Entity Entity => new Entity(EntityID);
        public Transform Transform => Entity.GetComponent<Transform>();

        // Logging helpers
        protected void Log(string message) => InternalCalls.Log(message);
        protected void LogError(string message) => InternalCalls.LogError(message);
        protected void LogWarning(string message) => InternalCalls.LogWarning(message);
    }
}

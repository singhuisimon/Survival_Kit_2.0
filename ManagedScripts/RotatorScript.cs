using ScriptAPI;

namespace Scripts
{
    public class RotatorScript : ScriptBase
    {
        public override void OnUpdate(float deltaTime)
        {
            var transform = new ScriptTransform(GetEntityID());
            // Example: rotate entity
            float x = transform.GetX();
            float y = transform.GetY();
            float z = transform.GetZ();
            // ... rotation logic
        }
    }
}

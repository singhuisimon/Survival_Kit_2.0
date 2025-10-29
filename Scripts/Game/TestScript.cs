using Engine;

namespace Game
{
    public class TestScript : ScriptBehaviour
    {
        private float timer = 0;

        public override void OnStart()
        {
            Log("TestScript started! EntityID: " + EntityID);
        }

        public override void OnUpdate(float deltaTime)
        {
            timer += deltaTime;
            if (timer >= 1.0f)
            {
                Log($"TestScript running! Position: {Transform.Position}");
                timer = 0;
            }
        }
    }
}
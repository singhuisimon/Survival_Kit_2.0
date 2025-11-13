using Engine;

namespace Game
{
    public class Projectile
    {
        public uint EntityID;
        public float speed = 1400f;  // units/second
        public float lifetime = 2f;

        private float t = 0f;

        public void OnUpdate(float dt)
        {
            t += dt;
            if (t >= lifetime)
            {
                //InternalCalls.Scene_DestroyEntity(EntityID);
                return;
            }
        }
    }
}

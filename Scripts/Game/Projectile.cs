using Engine;

namespace Game
{
    public class Projectile
    {
        public uint EntityID;
        public float speed = 1f;  // units/second
        public float lifetime = 5f;

        private float t = 0f;

        public void OnUpdate(float dt)
        {
            t += dt;
            if (t >= lifetime)
            {
                //InternalCalls.Scene_DestroyEntity(EntityID);
                return;
            }

            // Move forward (-Z) each frame
            InternalCalls.Transform_Move(EntityID, 0f, 0f, -speed * dt);
        }
    }
}

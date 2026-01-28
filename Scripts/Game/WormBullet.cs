using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Physics;
using static Engine.Tag;
using static Engine.Event;

namespace Game
{
    public class WormBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileLifetime = 2.0f;

        [SerializeField]
        public float Damage = 10.0f;

        [SerializeField] private float targetX;
        [SerializeField] private float targetY;
        [SerializeField] private float targetZ;

        [SerializeField] private float directionX;
        [SerializeField] private float directionY;
        [SerializeField] private float directionZ;

        private float elapsedTime = 0.0f;

        public override void OnStart()
        {
        }

        public override void OnUpdate(float deltaTime)
        {
            // Lifetime
            elapsedTime += deltaTime;
            if (elapsedTime >= ProjectileLifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            // Collisions (for ALL bullets, decided by tag)
            CheckCollisions();
        }

        // -------- COLLISION HANDLING (TAG-BASED, FOR ALL BULLETS) --------

        private void CheckCollisions()
        {
            ;
        }

        private void OnBulletHit(uint bulletEntityID, uint targetEntityID)
        {
            // // Publish event
            // Publish("BulletHit", targetEntityID.ToString());
            // Publish("BulletHitEnemy", true.ToString());
            // LogMessage("Event Published! BulletHit: target=" + targetEntityID + " from bullet=" + bulletEntityID);

            // // Destroy the bullet that actually hit
            SceneDestroyEntity(bulletEntityID);
        }

        public override void OnDestroy()
        {
            // Optional cleanup hook
        }
    }
}

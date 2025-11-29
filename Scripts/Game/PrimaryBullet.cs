using Engine;
using System;

namespace Game
{
    public class PrimaryBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileSpeed = 1000.0f;

        [SerializeField]
        public float ProjectileLifetime = 2.0f;

        [SerializeField]
        public float Damage = 10.0f;

        private float elapsedTime = 0.0f;
        private int frameCount = 0;

        public override void OnStart()
        {
            Log("=== PrimaryBullet OnStart ===");
            Log("EntityID: " + EntityID);
            Log("ProjectileLifetime: " + ProjectileLifetime);
            Log("elapsedTime: " + elapsedTime);
            Log("=== PrimaryBullet OnStart COMPLETE ===");
        }

        public override void OnUpdate(float deltaTime)
        {
            frameCount++;

            // Log every frame for the first 5 frames
            if (frameCount <= 5)
            {
                Log("=== PrimaryBullet Frame " + frameCount + " ===");
                Log("EntityID: " + EntityID);
                Log("deltaTime: " + deltaTime);
                Log("elapsedTime BEFORE: " + elapsedTime);
            }

            elapsedTime += deltaTime;

            if (frameCount <= 5)
            {
                Log("elapsedTime AFTER: " + elapsedTime);
                Log("ProjectileLifetime: " + ProjectileLifetime);
                Log("Will expire? " + (elapsedTime >= ProjectileLifetime));
            }

            // Check lifetime
            if (elapsedTime >= ProjectileLifetime)
            {
                Log("=== PrimaryBullet LIFETIME EXPIRED ===");
                Log("Frame: " + frameCount);
                Log("elapsedTime: " + elapsedTime);
                Log("ProjectileLifetime: " + ProjectileLifetime);
                Log("Destroying EntityID: " + EntityID);
                InternalCalls.Scene_DestroyEntity((uint)EntityID);
                return;
            }

            // Check collisions
            CheckCollisions();
        }

        private void CheckCollisions()
        {
            int collisionCount = InternalCalls.Physics_GetCollisionCount();

            if (frameCount <= 5 && collisionCount > 0)
            {
                Log("=== COLLISION DETECTED ===");
                Log("Collision count: " + collisionCount);
            }

            for (int i = 0; i < collisionCount; i++)
            {
                InternalCalls.Physics_GetCollisionPair(i, out uint entityA, out uint entityB);

                if (entityA == EntityID || entityB == EntityID)
                {
                    uint otherEntity = (entityA == EntityID) ? entityB : entityA;

                    Log("=== PrimaryBullet HIT SOMETHING ===");
                    Log("Frame: " + frameCount);
                    Log("Hit entity: " + otherEntity);
                    Log("Bullet EntityID: " + EntityID);

                    OnHitEnemy(otherEntity);
                    return;
                }
            }
        }

        public void OnHitEnemy(ulong targetEntityID)
        {
            Log("=== OnHitEnemy Called ===");
            Log("Target: " + targetEntityID);
            Log("Destroying bullet EntityID: " + EntityID);
            InternalCalls.Scene_DestroyEntity((uint)EntityID);
        }

        public override void OnDestroy()
        {
            Log("=== PrimaryBullet OnDestroy ===");
            Log("EntityID: " + EntityID);
            Log("Total frames alive: " + frameCount);
            Log("Final elapsedTime: " + elapsedTime);
        }
    }
}
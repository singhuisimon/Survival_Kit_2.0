using Engine;
using System;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// Primary weapon bullet - updated to use CollisionManager.
    /// Queries CollisionManager for hits instead of manually checking all collisions.
    /// </summary>
    public class PrimaryBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileLifetime = 2.0f;

        [SerializeField]
        public float Damage = 10.0f;

        [SerializeField]
        public int UltRecharged = 1;

        private float elapsedTime = 0.0f;

        public override void OnStart()
        {
        }

        public override void OnUpdate(float deltaTime)
        {
            // Lifetime check
            elapsedTime += deltaTime;
            if (elapsedTime >= ProjectileLifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            // Check collisions using CollisionManager
            CheckCollisions();
        }

        // ========================================================================
        // COLLISION HANDLING - Using CollisionManager (OPTIMIZED!)
        // ========================================================================

        private void CheckCollisions()
        {
            // Query the CollisionManager for hits
            List<uint> hits = CollisionManager.GetPlayerProjectileHits((uint)EntityID);
            
            if (hits != null && hits.Count > 0)
            {
                // Process all hits (in case bullet passed through multiple enemies in one frame)
                foreach (uint targetId in hits)
                {
                    OnBulletHit((uint)EntityID, targetId);
                }
                
                // Note: OnBulletHit destroys the bullet, so we only process first frame of hits
                // The bullet won't exist next frame to check again
            }
        }

        // ========================================================================
        // WHEN A VALID TARGET IS HIT
        // ========================================================================

        private void OnBulletHit(uint bulletEntityID, uint targetEntityID)
        {
            // Deal damage using DamageSystem
            DamageSystem.DealDamage(targetEntityID, Damage, bulletEntityID);
            
            // Publish events
            Publish("BulletHit", targetEntityID.ToString());
            Publish("BulletHitEnemy", true.ToString());
            Publish("GainUlt", UltRecharged.ToString());
            
            LogMessage("BulletHit: target=" + targetEntityID + " from bullet=" + bulletEntityID + " damage=" + Damage);

            // Destroy the bullet
            SceneDestroyEntity(bulletEntityID);
        }

        public override void OnDestroy()
        {
            // Optional cleanup hook
        }
    }
}
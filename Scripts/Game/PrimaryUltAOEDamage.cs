using Engine;
using System;
using System.Collections.Generic;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;

namespace Game
{
    /// <summary>
    /// AOE damage zone spawned by the primary ult.
    /// Damages all enemies within range that collide with it.
    /// Updated to use CollisionManager for efficient collision detection.
    /// </summary>
    public class PrimaryUltAOEDamage : ScriptBehaviour
    {
        [SerializeField] private float aoeDamage = 30.0f;
        [SerializeField] private float projectileLifetime = 2.0f;

        private float elapsedTime = 0.0f;
        
        // Track which entities we've already damaged (prevent double-damage)
        private HashSet<uint> damagedEntities = new HashSet<uint>();

        public override void OnStart()
        {
            LogMessage("[PrimaryUltAOEDamage] AOE zone spawned: " + EntityID);
        }
        
        public override void OnUpdate(float deltaTime)
        {
            // Update lifetime
            elapsedTime += deltaTime;
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            // Check lifetime first
            if (elapsedTime >= projectileLifetime)
            {
                LogMessage("[PrimaryUltAOEDamage] Lifetime expired. Damaged " + damagedEntities.Count + " entities total");
                SceneDestroyEntity((uint)EntityID);
                return;
            }
            
            // Check collisions using CollisionManager
            CheckCollisions();
        }

        public override void OnDestroy()
        {
        }

        // ========================================================================
        // COLLISION HANDLING - Using CollisionManager (OPTIMIZED!)
        // ========================================================================

        private void CheckCollisions()
        {
            // Query the CollisionManager for hits
            // AOE uses PLAYER_PROJECTILE category (same as bullets)
            List<uint> hits = CollisionManager.GetPlayerProjectileHits((uint)EntityID);
            
            if (hits != null && hits.Count > 0)
            {
                // AOE can hit multiple enemies over its lifetime
                foreach (uint targetId in hits)
                {
                    // Only damage each enemy once
                    if (!damagedEntities.Contains(targetId))
                    {
                        OnTargetHit(targetId);
                        damagedEntities.Add(targetId);
                    }
                }
            }
        }

        // ========================================================================
        // WHEN A VALID TARGET IS HIT
        // ========================================================================

        private void OnTargetHit(uint targetEntityID)
        {
            // Deal AOE damage using DamageSystem
            DamageSystem.DealDamage(targetEntityID, aoeDamage, (uint)EntityID);

            LogMessage("[PrimaryUltAOEDamage] AOE hit target " + targetEntityID + " for " + aoeDamage + " damage");
            
            // Optional: Spawn hit effect per enemy
            // SpawnAOEHitEffect(targetEntityID);
        }
    }
}
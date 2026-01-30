using Engine;
using System;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Rigidbody;
using static Engine.Prefab;

namespace Game
{
    /// <summary>
    /// Primary Ult projectile that spawns an AOE explosion when it hits an enemy.
    /// Updated to use CollisionManager for efficient collision detection.
    /// </summary>
    public class PrimaryUltFire : ScriptBehaviour 
    {
        [SerializeField] private float aoeRadius = 10.0f;
        [SerializeField] private float projectileLifetime = 2.0f;
        [SerializeField] private float damage = 20.0f;  // Damage dealt by direct hit
        //[SerializeField] 
        private string ultExplosionPrefab = "Sources/Prefabs/PrimaryUltExplosion.prefab";

        private float elapsedTime = 0.0f;

        public override void OnStart()
        {
            LogMessage("[PrimaryUltFire] Ult bullet spawned: " + EntityID);
        }

        public override void OnUpdate(float deltaTime)
        {
            // Just update elapsed time in Update
            elapsedTime += deltaTime;

            // Lifetime check
            if (elapsedTime >= projectileLifetime)
            {
                LogMessage("[PrimaryUltFire] Lifetime expired, destroying: " + EntityID);
                SceneDestroyEntity((uint)EntityID);
                return;
            }
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            // Check collisions using CollisionManager (efficient!)
            CheckCollisions();
        }

        public override void OnDestroy()
        {
        }

        // ========================================================================
        // COLLISION HANDLING - Using CollisionManager (NEW!)
        // ========================================================================

        private void CheckCollisions()
        {
            // Query the CollisionManager for hits
            List<uint> hits = CollisionManager.GetPlayerProjectileHits((uint)EntityID);
            
            if (hits != null && hits.Count > 0)
            {
                // Ult bullet explodes on FIRST hit (not multiple)
                uint firstTarget = hits[0];
                
                LogMessage("[PrimaryUltFire] Ult bullet " + EntityID + " hit enemy " + firstTarget);
                
                // Spawn AOE explosion and destroy bullet
                OnBulletHit((uint)EntityID, firstTarget);
            }
        }

        // ========================================================================
        // WHEN A VALID TARGET IS HIT
        // ========================================================================

        private void OnBulletHit(uint bulletEntityID, uint targetEntityID)
        {
            // Deal damage to the direct hit target
            DamageSystem.DealDamage(targetEntityID, damage, bulletEntityID);
            LogMessage("[PrimaryUltFire] Direct hit! Bullet " + bulletEntityID + " dealt " + damage + " damage to " + targetEntityID);
            
            // Get impact position for AOE spawn
            Vector3 Position = GetPosition(bulletEntityID);
            Quat Rot = GetRotation(bulletEntityID);
            Vector3 Scale = new Vector3(aoeRadius, aoeRadius, aoeRadius);
            
            // Instantiate AOE explosion at impact point
            uint aoeID = PrefabInstantiateWithTransform(ultExplosionPrefab, ref Position, ref Rot, ref Scale, false);
            
            if (aoeID == 0)
            {
                LogMessage("[PrimaryUltFire] ERROR: Failed to instantiate AOE explosion!");
            }
            else
            {
                LogMessage("[PrimaryUltFire] AOE explosion spawned! ID: " + aoeID + " at impact point");
            }

            // Destroy the ult bullet
            SceneDestroyEntity(bulletEntityID);
        }
    }
}
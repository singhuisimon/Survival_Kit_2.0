using Engine;
using System;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Rigidbody;
using static Engine.Audio;
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
        [SerializeField] private float damage = 60.0f;  // Damage dealt by direct hit
        //[SerializeField] 
        private string ultExplosionPrefab = "Sources/Prefabs/PrimaryUltExplosion.prefab";
        string MainExplosionPrefabPath = "Sources/Prefabs/MainExplosion1.prefab";

        private float elapsedTime = 0.0f;
        private Vector3 savedVelocity = Vector3.Zero;
        private bool wasPaused = false;

        private float lifetime = 0.0f;
        private bool hit = false;

        private bool audioplayed = false;

        private bool instantiated = false;

        public override void OnStart()
        {
            LogMessage("[PrimaryUltFire] Ult bullet spawned: " + EntityID);
        }

        public override void OnUpdate(float deltaTime)
        {

            // Handle pause - save/restore velocity
            if (GameState.IsPaused)
            {
                if (!wasPaused)
                {
                    // Just paused - save velocity and stop
                    savedVelocity = RigidbodyGetVelocity((uint)EntityID);
                    Vector3 zero = Vector3.Zero;
                    RigidbodySetVelocity((uint)EntityID, ref zero);
                    wasPaused = true;
                }
                return;
            }
            else if (wasPaused)
            {
                // Just unpaused - restore velocity
                RigidbodySetVelocity((uint)EntityID, ref savedVelocity);
                wasPaused = false;
            }

            if(!audioplayed){
                AudioPlay((uint)EntityID);
                audioplayed = true;
            }

            // Just update elapsed time in Update
            elapsedTime += deltaTime;

            // Lifetime check
            if (elapsedTime >= projectileLifetime && !hit)
            {
                LogMessage("[PrimaryUltFire] Lifetime expired, destroying: " + EntityID);
                SceneDestroyEntity((uint)EntityID);
                return;
            } else if (hit && elapsedTime >= lifetime){
                SceneDestroyEntity((uint)EntityID);
                return;
            }
        }

        public override void OnFixedUpdate(float deltaTime)
        {

            // Don't update when game is paused
            if (GameState.IsPaused)
                return;
            
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
            
            if(!instantiated){
                // Get impact position for AOE spawn
                Vector3 Position = GetPosition(bulletEntityID);
                Quat Rot = GetRotation(bulletEntityID);
                Vector3 Scale = new Vector3(0.1f, 0.1f, 0.1f);
                
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

                Vector3 myPos = Transform.GetPosition(EntityID);
                uint vfxID = PrefabInstantiate(MainExplosionPrefabPath);
                Transform.SetPosition(vfxID, ref myPos);

                Vector3 newScale = new Vector3(20.0f, 20.0f, 20.0f);
                Transform.SetScale(vfxID, ref newScale);
                
                if (vfxID == 0)
                {
                    LogMessage("[PrimaryUltFire] ERROR: Failed to instantiate AOE explosion!");
                }
                else
                {
                    LogMessage("[PrimaryUltFire] AOE explosion spawned! ID: " + vfxID + " at impact point");
                }
            }

            AudioStop((uint)EntityID);

            hit = true;
            lifetime += 0.01f;

            // Destroy the ult bullet
            //SceneDestroyEntity(bulletEntityID);
        }
    }
}
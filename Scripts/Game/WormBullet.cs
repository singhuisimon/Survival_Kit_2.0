using Engine;
using System;
using System.Collections.Generic;
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
        public float ProjectileLifetime = 6.0f;

        [SerializeField]
        public float Damage = 1.0f;

        // Target tags - entities this bullet can damage
        private string[] TargetTags = { "Player" };

        // This bullet's own tag
        private string[] BulletTags = { "WormBullet", "EnemyTurretBullet" };

        private float elapsedTime = 0.0f;

        // DEBUG: Toggle debug messages on/off
        [SerializeField]
        private bool enableDebug = true;

        public override void OnStart()
        {
            if (enableDebug)
            {
                LogMessage("===== WormBullet STARTED =====");
                LogMessage("WormBullet EntityID: " + EntityID);
                LogMessage("Checking WormBullet components...");
            }
        }

        public override void OnUpdate(float deltaTime)
        {
            // Lifetime - bullet destroys itself after ProjectileLifetime seconds
            elapsedTime += deltaTime;
            if (elapsedTime >= ProjectileLifetime)
            {
                if (enableDebug)
                    LogMessage("WormBullet " + EntityID + " lifetime expired, destroying...");
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            // Check for collisions using CollisionManager (same as Botnet)
            List<uint> collisions = CollisionManager.GetEnemyProjectileHits((uint)EntityID);

            if (collisions == null || collisions.Count == 0)
                return;

            if (enableDebug)
            {
                LogMessage("===== COLLISION DETECTED! Collision count: " + collisions.Count + " =====");
            }

            // Check all collisions
            CheckCollisions(collisions);

            // DEBUG: Find and track player position
            if (enableDebug)
                DebugFindPlayer();
        }

        // -------- COLLISION HANDLING (TAG-BASED, SAME AS PRIMARYBULLET) --------

        private void CheckCollisions(List<uint> collisions)
        {
            if (collisions == null || collisions.Count == 0)
                return;

            uint self = (uint)EntityID;

            foreach (uint other in collisions)
            {
                // DEBUG: Log collision
                if (enableDebug)
                {
                    LogMessage("!!!!! WORM BULLET " + self + " COLLIDED WITH ENTITY " + other + " !!!!!");
                }

                // Get other entity's tag
                string otherTag = TagGetTag(other);

                // DEBUG: Log tag
                if (enableDebug)
                {
                    LogMessage("Other entity tag: '" + otherTag + "'");
                }

                // Check if it's a valid target
                if (IsTargetTag(otherTag))
                {
                    OnBulletHitTarget(self, other);
                    return; // Exit after first hit
                }
                else if (enableDebug)
                {
                    LogMessage("Tag mismatch - not a valid target");
                }
            }
        }

        private bool IsBulletTag(string tag)
        {
            if (string.IsNullOrEmpty(tag) || BulletTags == null)
                return false;

            for (int i = 0; i < BulletTags.Length; i++)
            {
                string bulletTag = BulletTags[i];
                if (!string.IsNullOrEmpty(bulletTag) &&
                    string.Equals(tag, bulletTag, StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }

            return false;
        }

        private bool IsTargetTag(string tag)
        {
            if (string.IsNullOrEmpty(tag) || TargetTags == null)
                return false;

            for (int i = 0; i < TargetTags.Length; i++)
            {
                string target = TargetTags[i];
                if (!string.IsNullOrEmpty(target) &&
                    string.Equals(tag, target, StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }

            return false;
        }

        // -------- WHEN BULLET HITS VALID TARGET (PLAYER) --------

        private void OnBulletHitTarget(uint bulletEntityID, uint targetEntityID)
        {
            // Always log the hit (even if debug is off)
            LogMessage("===== WORM BULLET HIT PLAYER! =====");
            LogMessage("Bullet EntityID: " + bulletEntityID + " hit Player EntityID: " + targetEntityID);
            LogMessage("Damage dealt: " + Damage);

            //Publish(EVENT_PLAYER_HEALTHCHAGE, playerHP.ToString());
            DamageSystem.DealDamage(targetEntityID, Damage, bulletEntityID);
            // Destroy the bullet after hitting
            SceneDestroyEntity(bulletEntityID);
            
            if (enableDebug)
                LogMessage("Worm bullet destroyed after player collision");
        }

        // -------- DEBUG FUNCTIONS --------

        // DEBUG: Try to find the player entity and show distance
        private void DebugFindPlayer()
        {
            uint playerID = SceneFindEntityByName("Player");
            
            if (playerID == 0xffffffffu || playerID == 0)
            {
                LogMessage("WARNING: Cannot find player entity with name 'Player'");
            }
            else
            {
                // Only log this once when bullet is created
                if (elapsedTime < 0.1f)
                {
                    LogMessage("Player found! PlayerID: " + playerID);
                    
                    // Check player's position
                    Vector3 playerPos = Transform.GetPosition(playerID);
                    LogMessage("Player position: X=" + playerPos.X + " Y=" + playerPos.Y + " Z=" + playerPos.Z);
                    
                    // Check bullet's position
                    Vector3 bulletPos = Transform.GetPosition((uint)EntityID);
                    LogMessage("Bullet position: X=" + bulletPos.X + " Y=" + bulletPos.Y + " Z=" + bulletPos.Z);
                    
                    // Check distance
                    float dx = playerPos.X - bulletPos.X;
                    float dy = playerPos.Y - bulletPos.Y;
                    float dz = playerPos.Z - bulletPos.Z;
                    float distance = SimpleMath.Sqrt(dx*dx + dy*dy + dz*dz);
                    LogMessage("Distance to player: " + distance);
                }
            }
        }

        public override void OnDestroy()
        {
            if (enableDebug)
                LogMessage("WormBullet " + EntityID + " destroyed");
        }
    }
}
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

        // Target tags - entities this bullet can damage
        [SerializeField]
        private string[] TargetTags = { "Player" };

        // This bullet's own tag
        [SerializeField]
        private string[] BulletTags = { "WormBullet" };

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

            // Check for collisions
            int collisionCount = PhysicsGetCollisionCount();
            
            // DEBUG: Log if any collisions detected
            if (enableDebug && collisionCount > 0)
            {
                LogMessage("===== COLLISION DETECTED! Total collisions in scene: " + collisionCount + " =====");
            }

            // Check all collisions
            CheckCollisions(collisionCount);

            // DEBUG: Find and track player position
            if (enableDebug)
                DebugFindPlayer();
        }

        // -------- COLLISION HANDLING (TAG-BASED, SAME AS PRIMARYBULLET) --------

        private void CheckCollisions(int collisionCount)
        {
            if (collisionCount <= 0)
                return;

            uint self = (uint)EntityID;

            for (int i = 0; i < collisionCount; i++)
            {
                uint entityA, entityB;
                PhysicsGetCollisionPair(i, out entityA, out entityB);

                // DEBUG: Print ALL collision pairs if debug enabled
                if (enableDebug)
                {
                    LogMessage("Collision pair " + i + ": EntityA=" + entityA + " vs EntityB=" + entityB);
                }

                // Skip if this collision doesn't involve our bullet
                if (entityA != self && entityB != self)
                    continue;

                // Get the other entity
                uint other = (entityA == self) ? entityB : entityA;

                // DEBUG: Log when our bullet is involved
                if (enableDebug)
                {
                    LogMessage("!!!!! WORM BULLET " + self + " COLLIDED WITH ENTITY " + other + " !!!!!");
                }

                // Get tags
                string selfTag = TagGetTag(self);
                string otherTag = TagGetTag(other);

                // DEBUG: Log tags
                if (enableDebug)
                {
                    LogMessage("Self tag: '" + selfTag + "' | Other tag: '" + otherTag + "'");
                }

                // Case 1: We are bullet, other is valid target
                if (IsBulletTag(selfTag) && IsTargetTag(otherTag))
                {
                    OnBulletHitTarget(self, other);
                    return; // Exit after first hit
                }
                // Case 2: Other is bullet (shouldn't happen, but for safety)
                else if (IsBulletTag(otherTag) && IsTargetTag(selfTag))
                {
                    OnBulletHitTarget(other, self);
                    return;
                }
                else if (enableDebug)
                {
                    LogMessage("Tag mismatch - no valid hit detected");
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
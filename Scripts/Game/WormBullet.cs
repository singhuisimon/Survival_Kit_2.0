using Engine;
using System;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Physics;
using static Engine.Rigidbody;
using static Engine.Tag;
using static Engine.Event;
using static Engine.Prefab;

namespace Game
{
    public class WormBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileLifetime = 2.0f;

        [SerializeField]
        public float Damage = 1.0f;

        // Target tags - entities this bullet can damage
        //private string[] TargetTags = { "Player", "Gunship" };
        // Rio - Even though technically, Gunship is sufficient, adding GunshipHelper is extra caution
        private string[] TargetTags = { "Player", "Gunship", "GunshipHelper" };

        // This bullet's own tag
        private string[] BulletTags = { "WormBullet", "EnemyTurretBullet" };

        private float elapsedTime = 0.0f;
        private Vector3 savedVelocity = Vector3.Zero;
        private Vector3 savedAngularVelocity = Vector3.Zero;
        private bool wasPaused = false;

        private string DAMAGEPLAYERAUDIOPREFAB = "Sources/Prefabs/Audio_EmemyDamage.prefab";

        // DEBUG: Toggle debug messages on/off
        [SerializeField]
        private bool enableDebug = true;

        // Game lose / win condition
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        public override void OnStart()
        {
            TagSetTag((uint)EntityID, "WormBullet");
            if (enableDebug)
            {
                LogMessage("===== WormBullet STARTED =====");
                LogMessage("WormBullet EntityID: " + EntityID);
                LogMessage("Checking WormBullet components...");
            }

            Subscribe(GAMEOVER, OnGameOver);
            Subscribe(GAMEWIN, OnGameOver);
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
                    savedAngularVelocity = RigidbodyGetAngularVelocity((uint)EntityID);

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
                RigidbodySetAngularVelocity((uint)EntityID, ref savedAngularVelocity);
                wasPaused = false;
            }

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
            // Get the target's tag to determine what we hit
            string targetTag = TagGetTag(targetEntityID);

            // Always log the hit (even if debug is off)
            LogMessage("===== WORM BULLET HIT PLAYER! =====");
            LogMessage("Bullet EntityID: " + bulletEntityID + " hit Player EntityID: " + targetEntityID);
            LogMessage("Damage dealt: " + Damage);

            // Play damage audio if we hit the player
            if (targetTag == "Player")
            {
                PrefabInstantiate(DAMAGEPLAYERAUDIOPREFAB);
                if (enableDebug)
                    LogMessage("Playing player damage audio");
            }

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
            Unsubscribe(GAMEOVER, OnGameOver);
            Unsubscribe(GAMEWIN, OnGameOver);
        }

        private void OnGameOver(string eventName, string payload)
        {
            SceneDestroyEntity(EntityID);
        }
    }
}
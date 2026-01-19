using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Prefab;
using static Engine.Rigidbody;
using static Engine.Tag;

namespace Game 
{
    /// <summary>
    /// This file contains the script and behaviour of Gunship
    /// </summary>
    
    public class GunshipScript : ScriptBehaviour 
    {

        // === NAME OF ENTITY ===
        [SerializeField] private string gunshipEntity = "ALLIES";
        private uint gunshipEntityID = 0;

        // === HEALTH INFORMATION ===
        [SerializeField] private float maxHealth = 75f;
        private float currentHealth = 75f;
        private bool isDead = false;

        // === WEAPON INFORMATION ===
        [SerializeField] private float fireRate = 0.1f;
        [SerializeField] private float turretRange = 600f;
        [SerializeField] private float turretRotationSpeed = 5f;
        [SerializeField] private float bulletSpeed = 3000f;
        [SerializeField] private string bulletPrefabPath = "Sources/Prefabs/NormalTurretBullet.prefab";

        // === PRIVATE STATES ===
        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint currentTarget = INVALID_ENTITY;    // enemy to shoot at
        private float fireCooldown = 0f;               
        private Quat turretRotation;  

        // Tags of enemies to shoot
        private readonly string[] enemyTags = { "botnet", "loveletter", "adware" };

        // Start up
        public override void OnStart() 
        {
            // Message to indicate Gunship starting (there are multiple Gunships)
            // LogMessage("=== Gunship Started (ID: " + EntityID + ") ===");
            
            // // Initialize health
            // currentHealth = maxHealth;
            // isDead = false;
            
            // // Set initial turret rotation to match gunship
            // turretRotation = GetRotation((uint)EntityID);
            
            // // Subscribe to damage events
            // Subscribe("GunshipDamage:" + EntityID, OnTakeDamage);
            
            // LogMessage("Gunship ready! Health: " + currentHealth + "/" + maxHealth);

            //new code to try and see if it can detect Gunship
            LogMessage("==== Gunship Started ====");
            gunshipEntityID = SceneFindEntityByName(gunshipEntity);
            EntityID = gunshipEntityID; 

            LogMessage("Gunship ID: " + gunshipEntityID);

            if (EntityID == 0 || EntityID == INVALID_ENTITY)
            {
                LogError("FAILED to find gunship: " + gunshipEntity);
                isDead = true;
                return;
            }

            // Verify position works
            Vector3 pos = GetPosition((uint)EntityID);
            LogMessage("Gunship position: " + pos.X + ", " + pos.Y + ", " + pos.Z);
            
            // Initialize health
            currentHealth = maxHealth;
            isDead = false;
            
            // Set initial turret rotation
            turretRotation = GetRotation((uint)EntityID);

            LogMessage("Initial rotation: X=" + turretRotation.X + ", Y=" + turretRotation.Y + 
           ", Z=" + turretRotation.Z + ", W=" + turretRotation.W);
            Vector3 testForward = turretRotation.Forward;
            LogMessage("Model's Forward direction: " + testForward.X + ", " + testForward.Y + ", " + testForward.Z);
            Vector3 testRight = turretRotation.Right;
            LogMessage("Model's Right direction: " + testRight.X + ", " + testRight.Y + ", " + testRight.Z);
            Vector3 testUp = turretRotation.Up;
            LogMessage("Model's Up direction: " + testUp.X + ", " + testUp.Y + ", " + testUp.Z);
            
            // Subscribe to damage events
            Subscribe("GunshipDamage:" + EntityID, OnTakeDamage);
            
            LogMessage("Gunship ready! Health: " + currentHealth + "/" + maxHealth);
        }

        // update loop
        public override void OnUpdate(float deltaTime) 
        {
            // does not do anything is Gunship is dead
            if (isDead)
                return;
            
            // Update fire cooldown
            if (fireCooldown > 0f)
                fireCooldown -= deltaTime;
            
            // find Enemy targets
            UpdateTargeting();
            
            // if there is a target, rotate towards it to shoot
            if (currentTarget != INVALID_ENTITY)
            {
                RotateTowardsTarget(deltaTime);
                TryShoot();
            }
        }

        // Gunship dies lol
        public override void OnDestroy()
        {
            Unsubscribe("GunshipDamage:" + EntityID, OnTakeDamage);
            LogMessage("Gunship destroyed");
        }

        // Targeting System
        private void UpdateTargeting()
        {
            // Check if current target is still there
            if (currentTarget != INVALID_ENTITY)
            {
                // if target is still alive
                string targetTag = TagGetTag(currentTarget);
                if (string.IsNullOrEmpty(targetTag))
                {
                    // finding new target
                    currentTarget = INVALID_ENTITY;
                }
                else
                {
                    // check if target is within range
                    Vector3 myPos = GetPosition((uint)EntityID);
                    Vector3 targetPos = GetPosition(currentTarget);
                    float distSq = DistanceSquared(myPos, targetPos);
                    
                    if (distSq > turretRange * turretRange)
                    {
                        // Too far away
                        currentTarget = INVALID_ENTITY;
                    }
                    else
                    {
                        return;
                    }
                }
            }
            
            // finding new target
            currentTarget = FindNearestEnemy();
            
            if (currentTarget != INVALID_ENTITY)
            {
                LogMessage("Gunship locked onto target: " + currentTarget);
            }
        }

        private uint FindNearestEnemy()
        {
            Vector3 myPos = GetPosition((uint)EntityID);
            float rangeSq = turretRange * turretRange;
            
            uint nearestEnemy = INVALID_ENTITY;
            float nearestDistSq = float.MaxValue;
            
            // Search through all Enemy types
            foreach (string enemyTag in enemyTags)
            {
                uint[] enemies = SceneFindEntitiesByTag(enemyTag);
                
                if (enemies == null || enemies.Length == 0)
                    continue;
                
                // Check each enemy
                for (int i = 0; i < enemies.Length; i++)
                {
                    uint enemyID = enemies[i];

                    // Trying to fix entity identiy returning 0
                    if (enemyID == 0 || enemyID == INVALID_ENTITY)
                    continue;

                    Vector3 enemyPos = GetPosition(enemyID);
                    float distSq = DistanceSquared(myPos, enemyPos);
                    
                    // Checking if this enemy is in range or more nearer than the other enemy
                    if (distSq <= rangeSq && distSq < nearestDistSq)
                    {
                        nearestDistSq = distSq;
                        nearestEnemy = enemyID;
                    }
                }
            }
            
            return nearestEnemy;
        }

        // Turret Rotation

        // Move the turret towards the enemy
        private void RotateTowardsTarget(float deltaTime)
        {
            if (currentTarget == INVALID_ENTITY)
                return;
            
            // Get Positons of the Target (Enemies)
            Vector3 myPos = GetPosition((uint)EntityID);
            Vector3 targetPos = GetPosition(currentTarget);
            
            // Calculate direction to target
            Vector3 toTarget = new Vector3(
                targetPos.X - myPos.X,
                targetPos.Y - myPos.Y,
                targetPos.Z - myPos.Z
            );
            
            // Normalize the direction
            float lenSq = toTarget.X * toTarget.X + toTarget.Y * toTarget.Y + toTarget.Z * toTarget.Z;
            if (lenSq <= 0.0001f)
                return;
            
            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            toTarget.X *= invLen;
            toTarget.Y *= invLen;
            toTarget.Z *= invLen;
            
            // Calculate rotation needed to face target
            Vector3 forward = new Vector3(0f, 0f, -1f);  // Forward Direction
            Quat targetRotation = QuaternionFromTo(forward, toTarget);
            
            // Rotate towards Enemy Target
            float t = SimpleMath.Clamp(turretRotationSpeed * deltaTime, 0f, 1f);
            turretRotation = Nlerp(turretRotation, targetRotation, t);
            
            // Apply rotation to gunship entity
            SetRotation((uint)EntityID, ref turretRotation);
        }

        // Shooting Mechanism

        private void TryShoot()
        {
            // To test the bullet shooting immediately
            // FireBullet();

            //On Cooldown
            if (fireCooldown > 0f)
                return;
            
            // Cannot shoot without Target (Enemy)
            if (currentTarget == INVALID_ENTITY)
                return;
            
            // Check if it is aimed at Target
            Vector3 myPos = GetPosition((uint)EntityID);
            Vector3 targetPos = GetPosition(currentTarget);
            Vector3 forward = turretRotation.Forward;

            // Debug Msg
            LogMessage("Turret forward: " + forward.X + ", " + forward.Y + ", " + forward.Z);
            
            Vector3 toTarget = new Vector3(
                targetPos.X - myPos.X,
                targetPos.Y - myPos.Y,
                targetPos.Z - myPos.Z
            );
            
            float lenSq = toTarget.X * toTarget.X + toTarget.Y * toTarget.Y + toTarget.Z * toTarget.Z;
            if (lenSq > 0.0001f)
            {
                float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
                toTarget.X *= invLen;
                toTarget.Y *= invLen;
                toTarget.Z *= invLen;
                
                // Check if aimed (dot product > 0.95 = ~18 degrees)
                float dot = forward.X * toTarget.X + forward.Y * toTarget.Y + forward.Z * toTarget.Z;
                LogMessage("Dot product: " + dot + " (need > 0.95 to shoot)");

                if (dot > -0.3f)
                {
                    // Shoot
                    LogMessage("SHOOTING NOW!");
                    FireBullet();
                } else {
                    LogMessage("Not aimed yet, rotating...");
                }
            }
        }
        
        private void FireBullet()
        {
            // Reset cooldown
            fireCooldown = fireRate;

            // calculate spawn position (in front of turret)
            Vector3 myPos = GetPosition((uint)EntityID);
            //debug msg
            LogMessage("Gunship position: " + myPos.X + ", " + myPos.Y + ", " + myPos.Z);
            Vector3 forward = turretRotation.Forward;
            //debug msg
            LogMessage("Forward direction: " + forward.X + ", " + forward.Y + ", " + forward.Z);

            // Spawn bullet infront of Gunship
            Vector3 spawnPos = new Vector3(
                myPos.X + forward.X * 1.5f,
                myPos.Y + forward.Y * 1.5f,
                myPos.Z + forward.Z * 1.5f
            );

            //debug msg
            LogMessage("Bullet spawn position: " + spawnPos.X + ", " + spawnPos.Y + ", " + spawnPos.Z);

            Vector3 bulletScale = new Vector3(0.3f, 0.2f, 0.15f);

            // Spawn the bullet prefab
            uint bulletID = PrefabInstantiateWithTransform(
                bulletPrefabPath,
                ref spawnPos,
                ref turretRotation,
                ref bulletScale,
                false
            );

            if (bulletID != 0) 
            {
                Vector3 verifyPos = GetPosition(bulletID);
                LogMessage("Bullet actual position after spawn: " + verifyPos.X + ", " + verifyPos.Y + ", " + verifyPos.Z);

                SetPosition(bulletID, ref spawnPos);

                // Give bullet velocity
                Vector3 velocity = new Vector3(
                    forward.X * bulletSpeed,
                    forward.Y * bulletSpeed,
                    forward.Z * bulletSpeed
                );

                RigidbodySetVelocity(bulletID, ref velocity);

                LogMessage("Gunship fired at target " + currentTarget);
            } 
            else 
            {
                LogError("Failed to spawn bullet!");
            }
        }

        // Damage System (This is for when the Gunship takes Damage)

        private void OnTakeDamage(string eventName, string payload)
        {
            if (isDead)
                return;
            
            // Parse damage amount
            if (!float.TryParse(payload, out float damage))
                return;
            
            // Apply damage
            currentHealth -= damage;

            // To confirm that Gunship did take damage
            LogMessage("Gunship took " + damage + " damage! Health: " + currentHealth + "/" + maxHealth);
            
            // Check if dead
            if (currentHealth <= 0f)
            {
                Die();
            }
        }

        // Gunship Dies due to too much Damage Taken
        private void Die()
        {
            if (isDead)
                return;
            
            isDead = true;
            LogMessage("Gunship destroyed!");
            
            // Publish death event 
            Publish("GunshipDeath", EntityID.ToString());
            
            
            // Destroy the gunship
            SceneDestroyEntity((uint)EntityID);
        }

        // Math Helpers
                
        // Calculate distance squared (faster than actual distance)
        private float DistanceSquared(Vector3 a, Vector3 b)
        {
            float dx = b.X - a.X;
            float dy = b.Y - a.Y;
            float dz = b.Z - a.Z;
            return dx * dx + dy * dy + dz * dz;
        }
        
        // Create quaternion rotation from one direction to another
        private Quat QuaternionFromTo(Vector3 from, Vector3 to)
        {
            float dot = from.X * to.X + from.Y * to.Y + from.Z * to.Z;
            
            // Vectors pointing opposite directions
            if (dot < -0.9999f)
            {
                return new Quat(0f, 1f, 0f, 0f);
            }
            
            // Cross product for rotation axis
            Vector3 cross = new Vector3(
                from.Y * to.Z - from.Z * to.Y,
                from.Z * to.X - from.X * to.Z,
                from.X * to.Y - from.Y * to.X
            );
            
            float s = SimpleMath.Sqrt((1f + dot) * 2f);
            float invS = 1f / s;
            
            return new Quat(
                cross.X * invS,
                cross.Y * invS,
                cross.Z * invS,
                0.5f * s
            );
        }
        
        // Normalized lerp between quaternions (smooth rotation)
        private Quat Nlerp(Quat a, Quat b, float t)
        {
            // Ensure shortest path
            float dot = a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
            if (dot < 0f)
            {
                b = new Quat(-b.X, -b.Y, -b.Z, -b.W);
            }
            
            float invT = 1f - t;
            
            Quat result = new Quat(
                a.X * invT + b.X * t,
                a.Y * invT + b.Y * t,
                a.Z * invT + b.Z * t,
                a.W * invT + b.W * t
            );
            
            // Normalize
            float lenSq = result.X * result.X + result.Y * result.Y + 
                         result.Z * result.Z + result.W * result.W;
            
            if (lenSq > 0.000001f)
            {
                float invLen = 1f / SimpleMath.Sqrt(lenSq);
                result.X *= invLen;
                result.Y *= invLen;
                result.Z *= invLen;
                result.W *= invLen;
            }
            
            return result;
        }
    }   // end of public class GunshipScript
}   // end of namespace Game
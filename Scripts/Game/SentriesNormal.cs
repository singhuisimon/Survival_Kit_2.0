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
    
    public class SentriesNormal : ScriptBehaviour 
    {

        // === TIME INFORMATION ===
        [SerializeField] private float maxDuration = 90.0f;
        private float countdown = 90.0f;
        private bool timeUp = false;

        // === WEAPON INFORMATION ===
        [SerializeField] private float fireRate = 0.1f;
        [SerializeField] private float turretRange = 600f;
        [SerializeField] private float turretRotationSpeed = 5f;
        [SerializeField] private float bulletSpeed = 6000f;
        [SerializeField] private string bulletPrefabPath = "Sources/Prefabs/NormalTurretBullet.prefab";
        [SerializeField] private string sentryDespawn = "Sources/Prefabs/SentryDespawn.prefab";

        // === PRIVATE STATES ===
        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint currentTarget = INVALID_ENTITY;    // enemy to shoot at
        private float fireCooldown = 5.0f;               
        private Quat turretRotation;  

        private uint sentryID = INVALID_ENTITY; // getting the ID of the read sentry entity
        private bool initialized = false;

        // List of enemies to verify and target
        private readonly string[] enemyTags = { "botnet" , "WormHost" , "loveletter" };
        
        private string GAMEOVEREVENT = "GameOver";
        private string GAMEWINEVENT = "GameWin";
        private bool disableshooting = false;

        // Start up
        public override void OnStart() 
        {
            LogMessage("====Sentry Helper Started====");
            LogMessage("Helper EntityID: " + EntityID);

            TagSetTag((uint)EntityID, "SentryHelper");

            Subscribe(GAMEOVEREVENT, OnGameEnd);
            Subscribe(GAMEWINEVENT, OnGameEnd);

            // not detecting Sentry here (might not be ready on frame 0)
            initialized = false;
            timeUp = false;
            disableshooting = false;
        }

        // update loop
        public override void OnUpdate(float deltaTime) 
        {
            if (!initialized) {
                uint self = (uint)EntityID;
                sentryID = TransformGetParent(self);

                // Parent is not ready yet
                if (sentryID == 0 || sentryID == INVALID_ENTITY)
                    return;

                LogMessage("Sentry Helper is bound to parent sentryID (sentry identity) " + sentryID);

                countdown = maxDuration;
                timeUp = false;

                turretRotation = GetRotation(sentryID);
                
                initialized = true;
            }

            // Don't update when game is paused
            if (GameState.IsPaused)
                return;

            countdown -= deltaTime;

            if(countdown < 0){
                Despawn();
            }

            // does not do anything is Sentry is dead
            if (timeUp)
                return;

            if(disableshooting){
                return;
            }
            
            // Update fire cooldown
            if (fireCooldown > 0f) 
            {
                fireCooldown -= deltaTime;
                if (fireCooldown < 0f) fireCooldown = 0f;
            }
            
            // find Enemy targets
            UpdateTargeting();
            
            // if there is a target, rotate towards it to shoot
            if (currentTarget != INVALID_ENTITY)
            {
                RotateTowardsTarget(deltaTime);
                TryShoot();
            }
        }

        // Sentry despawn lol
        public override void OnDestroy()
        {
            Unsubscribe(GAMEOVEREVENT, OnGameEnd);
            Unsubscribe(GAMEWINEVENT, OnGameEnd);

        }

        private void OnGameEnd(string eventName, string payload){
            LogMessage("[SentriesNormal] detect game over disabling shooting");
            disableshooting = true;
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
                    Vector3 myPos = GetPosition(sentryID);
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
                LogMessage("Sentry locked onto target: " + currentTarget);
            }
        }

        private uint FindNearestEnemy()
        {
            Vector3 myPos = GetPosition(sentryID);
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

        // Rotates the PARENT (sentryID). Script lives on the child helper.
        private void RotateTowardsTarget(float deltaTime)
        {
            if (currentTarget == INVALID_ENTITY)
                return;

            //1. Build a normalised direction from parent to target
            Vector3 myPos     = GetPosition(sentryID);
            Vector3 targetPos = GetPosition(currentTarget);

            float dx = targetPos.X - myPos.X;
            float dy = targetPos.Y - myPos.Y;
            float dz = targetPos.Z - myPos.Z;

            float lenSq = dx * dx + dy * dy + dz * dz;
            if (lenSq <= 0.0001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            Vector3 toTarget = new Vector3(dx * invLen, dy * invLen, dz * invLen);

            //  2. Build target rotation via LookRotation 
            // LookRotation(forward, worldUp) gives correct yaw AND pitch with
            // zero roll, and is stable even when the enemy is directly above or
            // below (no Atan2 singularity).
            Quat targetRotation = SimpleMath.LookRotation(toTarget, Vector3.Up);

            // 3. Smooth / dampen towards that rotation via Nlerp 
            // turretRotationSpeed is your dampen knob (editor-tweakable):
            //   ~2  = slow, tank-like tracking
            //   ~5  = default, responsive
            //   ~15 = near-instant snap
            float t = SimpleMath.Clamp(turretRotationSpeed * deltaTime, 0f, 1f);
            turretRotation = Quat.Slerp(turretRotation, targetRotation, t);

            // 4. Apply to the PARENT (sentryID) only 
            // The child helper (EntityID) is never rotated it stays fixed
            // inside the parent so positions/muzzle offsets remain correct.
            SetRotation(sentryID, ref turretRotation);
        }
        // Shooting Mechanism

        private void TryShoot()
        {
            // Cooldown Check
            if (fireCooldown > 0f) 
                return;

            // Must have a target
            if (currentTarget == INVALID_ENTITY)
                return;

            // Fire Immediately (No need for aiming checks as there are no rotation)
            FireBullet();
        }
        
        private void FireBullet()
        {
           fireCooldown = fireRate;

           // Shoot from helper position (hidden inside the Sentry)
           uint self = (uint)EntityID;
           Vector3 myPos = GetPosition(sentryID);

           // Enemy Position
           Vector3 targetPos = GetPosition(currentTarget);

           // Direction to enemy
           Vector3 dir = new Vector3(
                targetPos.X - myPos.X,
                targetPos.Y - myPos.Y,
                targetPos.Z - myPos.Z
           );

           float lenSq = dir.X * dir.X + dir.Y * dir.Y + dir.Z * dir.Z;
           if (lenSq <= 0.0001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            dir.X *= invLen;
            dir.Y *= invLen;
            dir.Z *= invLen;

            //Spawn infront of helper so it dosen't collide instantly
            float muzzleDist = 5.0f;
            Vector3 spawnPos = new Vector3 (
                myPos.X + dir.X * muzzleDist,
                myPos.Y + dir.Y * muzzleDist,
                myPos.Z + dir.Z * muzzleDist
            );

            // Rotate bullet to face direction
            Quat bulletRot = SimpleMath.LookRotation(dir, Vector3.Up);


            Vector3 bulletScale = new Vector3 (0.1f, 0.1f, 0.1f);

            uint bulletID = PrefabInstantiateWithTransform(
                bulletPrefabPath,
                ref spawnPos,
                ref bulletRot,
                ref bulletScale,
                false
            );

            if (bulletID == 0 || bulletID == INVALID_ENTITY)
            {
                LogError("Failed to spawn PrimaryBullet!");
                return;
            }

            Vector3 velocity = new Vector3(
                dir.X * bulletSpeed,
                dir.Y * bulletSpeed,
                dir.Z * bulletSpeed
            );

            RigidbodySetVelocity(bulletID, ref velocity);

            LogMessage("Sentry fired PrimaryBullet at target " + currentTarget);
        }

        // Sentry despawn due to time up
        private void Despawn()
        {
            if (timeUp)
                return;
            
            timeUp = true;
            LogMessage("Sentry destroyed!");

            uint deathID = 0;

            Vector3 spawnPos = GetPosition((uint)EntityID);
            Quat spawnRot = GetRotation((uint)EntityID);
            Vector3 scale = new Vector3(0.0f, 0.0f, 0.0f); 

            deathID = PrefabInstantiate(sentryDespawn);
            
            // Publish death event 
            
            // Destroy the gunship
            SceneDestroyEntity(sentryID);
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
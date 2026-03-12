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

        // === HEALTH INFORMATION ===
        [SerializeField] private float maxHealth = 75f;
        private float currentHealth = 75f;
        private bool isDead = false;

        // === WEAPON INFORMATION ===
        [SerializeField] private float fireRate = 0.2f;
        [SerializeField] private float turretRange = 600f;
        [SerializeField] private float turretRotationSpeed = 5f;
        [SerializeField] private float bulletSpeed = 3000f;
        [SerializeField] private string bulletPrefabPath = "Sources/Prefabs/NormalTurretBullet.prefab";
        [SerializeField] private string gunshipDeathPath = "Sources/Prefabs/GunshipDeath.prefab";
        [SerializeField] private int allyIndex = 1;

        // === PRIVATE STATES ===
        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint currentTarget = INVALID_ENTITY;    // enemy to shoot at
        private float fireCooldown = 5.0f;               
        private Quat turretRotation;  

        private uint gunshipID = INVALID_ENTITY; // getting the ID of the read gunship entity
        private bool initialized = false;

        private string healthChangeEventName = "";

        // List of enemies to verify and target
        private readonly string[] enemyTags = { "botnet" , "WormHost" , "wormchild" , "loveletter" };
        
        private string GAMEOVEREVENT = "GameOver";
        private string GAMEWINEVENT = "GameWin";
        private bool disableshooting = false;

        // Start up
        public override void OnStart() 
        {
            LogMessage("====Gunship Helper Started====");
            LogMessage("Helper EntityID: " + EntityID);

            TagSetTag((uint)EntityID, "GunshipHelper");


            healthChangeEventName = "GunshipHealthChanged:" + allyIndex;

            healthChangeEventName = "GunshipHealthChanged:" + allyIndex;
            Subscribe("DebugDamageGunship:" + allyIndex, OnTakeDamage); // DEBUG

            Subscribe(GAMEOVEREVENT, OnGameEnd);
            Subscribe(GAMEWINEVENT, OnGameEnd);

            // not detecting Gunship here (might not be ready on frame 0)
            initialized = false;
            isDead = false;
            disableshooting = false;
        }

        // update loop
        public override void OnUpdate(float deltaTime) 
        {
            if (!initialized) {
                uint self = (uint)EntityID;
                gunshipID = TransformGetParent(self);

                // Parent is not ready yet
                if (gunshipID == 0 || gunshipID == INVALID_ENTITY)
                    return;

                LogMessage("Gunship Helper is bound to parent gunshipID (gunship identity) " + gunshipID);

                currentHealth = maxHealth;
                isDead = false;

                turretRotation = GetRotation(gunshipID);

                //Subscribe("GunshipDamage:" + gunshipID, OnTakeDamage);
                Subscribe("Damage:" + gunshipID, OnTakeDamage);
                Subscribe("Damage:" + (uint)EntityID, OnTakeDamage);
                
                initialized = true;
            }

            if(disableshooting){
                return;
            }

            // Don't update when game is paused
            if (GameState.IsPaused)
                return;

            //DebugFireOnKey();

            // does not do anything is Gunship is dead
            if (isDead)
                return;
            
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
                TryShoot();
            }
        }

        // Gunship dies lol
        public override void OnDestroy()
        {
            Unsubscribe(GAMEOVEREVENT, OnGameEnd);
            Unsubscribe(GAMEWINEVENT, OnGameEnd);
            if (gunshipID != INVALID_ENTITY) 
                //Unsubscribe("GunshipDamage:" + gunshipID, OnTakeDamage);
                Unsubscribe("Damage:" + gunshipID, OnTakeDamage);
                Unsubscribe("Damage:" + (uint)EntityID, OnTakeDamage);
            Unsubscribe("DebugDamageGunship:" + allyIndex, OnTakeDamage); // DEBUG


        }

        private void OnGameEnd(string eventName, string payload){
            LogMessage("[Gunship] detect game over disabling shooting");
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
                    Vector3 myPos = GetPosition(gunshipID);
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
            Vector3 myPos = GetPosition(gunshipID);
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

        // Press K to force the gunship to fire one bullet (This is used for Debugging)
        private void DebugFireOnKey() 
        {
            if (!Input.IsKeyReleased(KeyCode.K)) 
                return;

            // Must be initalized
            if (gunshipID == 0 || gunshipID == INVALID_ENTITY)
            {
                LogWarning("[GunshipDebug] Can't fire yet: gunshipID not ready.");
                return;
            }

            fireCooldown = 0.0f;

            Vector3 gunPos = GetPosition(gunshipID);
            Vector3 helperPos = GetPosition((uint)EntityID);

            Vector3 dir;
            if (currentTarget != INVALID_ENTITY)
            {
                Vector3 targetPos = GetPosition(currentTarget);
                dir = new Vector3(targetPos.X - gunPos.X, targetPos.Y - gunPos.Y, targetPos.Z - gunPos.Z);
            }
            else
            {
                Quat gunRot = GetRotation(gunshipID);
                dir = gunRot.Forward;
            }

            float lenSq = dir.X * dir.X + dir.Y * dir.Y + dir.Z * dir.Z;
            if (lenSq < 0.0001f)
            {
                LogWarning("[GunshipDebug] Direction too small, not firing.");
                return;
            }
            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            dir.X *= invLen; dir.Y *= invLen; dir.Z *= invLen;

            float muzzleDist = 1.0f;
            Vector3 spawnPos = new Vector3 (
                gunPos.X + dir.X * muzzleDist,
                gunPos.Y + dir.Y * muzzleDist,
                gunPos.Z + dir.Z * muzzleDist
            );

            Quat bulletRot = SimpleMath.LookRotation(dir, Vector3.Up);
            Vector3 bulletScale = new Vector3(0.3f, 0.2f, 0.15f);

            uint bulletID = PrefabInstantiateWithTransform (
                bulletPrefabPath,
                ref spawnPos,
                ref bulletRot,
                ref bulletScale,
                false
            );

            if (bulletID == 0 || bulletID == INVALID_ENTITY) 
            {
                LogError("[GunshipDebug] Failed to spawn bullet.");
                return;
            }

            Vector3 vel = new Vector3(dir.X * bulletSpeed, dir.Y * bulletSpeed, dir.Z * bulletSpeed);
            RigidbodySetVelocity(bulletID, ref vel);

            // LogMessage($"[GunshipDebug] gunPos=({gunPos.X},{gunPos.Y},{gunPos.Z}) helperPos=({helperPos.X},{helperPos.Y},{helperPos.Z})");
            // LogMessage($"[GunshipDebug] spawnPos=({spawnPos.X},{spawnPos.Y},{spawnPos.Z}) dir=({dir.X},{dir.Y},{dir.Z}) bulletID={bulletID}");
        }

        // Turret Rotation

        // ===== THIS FUNCTION IS CURRENTLY NOT BEING USED AS OF RIGHT NOW ======
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

           // Shoot from helper position (hidden inside the Gunship)
           uint self = (uint)EntityID;
           Vector3 myPos = GetPosition(gunshipID);

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

            LogMessage("Gunship fired PrimaryBullet at target " + currentTarget);
        }

        // Damage System (This is for when the Gunship takes Damage)

        private void OnTakeDamage(string eventName, string payload)
        {
            LogMessage("[Gunship] OnTakeDamage fired! payload=" + payload + " event=" + eventName);
            LogMessage("Gunship Health:" + currentHealth);
            LogMessage("[Gunship] eventName = " + eventName + " helper=" + (uint)EntityID + " parent=" + gunshipID);

            if (isDead)
                return;
            
            // Parse damage amount
            float damage = ExtractFirstFloat(payload);
            if (damage <= 0f)
            {
                LogWarning("[Gunship] Could not parse damage from payload: " + payload);
                return;
            }
            
            // Apply damage
            currentHealth -= damage;

            // To confirm that Gunship did take damage
            LogMessage("Gunship took " + damage + " damage! Health: " + currentHealth + "/" + maxHealth);


            if (!string.IsNullOrEmpty(healthChangeEventName))
            {
                string healthPayload = currentHealth.ToString() + "|" + maxHealth.ToString();
                Publish(healthChangeEventName, healthPayload);
            }

            // Check if dead
            if (currentHealth <= 0f)
            {
                LogMessage("Gunship is dead, currentHealth:" + currentHealth);
                Die();
            }
        }

                // So that Gunship can take damage
        private float ExtractFirstFloat(string payload) 
        {
            if (string.IsNullOrEmpty(payload)) 
                return 0f;

            // common seperators used in event payloads
            char[] seps = new char [] { '|', ',', ';', ':', ' ' };
            string[] parts = payload.Split(seps, StringSplitOptions.RemoveEmptyEntries);

            for (int i = 0; i < parts.Length; i++)
            {
                // Strip common key=value formats
                string token = parts[i];
                int eq = token.IndexOf('=');
                if (eq >= 0 && eq < token.Length - 1)
                    token = token.Substring(eq + 1);

            if (float.TryParse(token, out float val))
                return val;
            }

            return 0f;
        }

        // Gunship Dies due to too much Damage Taken
        private void Die()
        {
            if (isDead)
                return;
            
            isDead = true;
            LogMessage("Gunship destroyed!");

            uint deathID = 0;

            Vector3 spawnPos = GetPosition((uint)EntityID);
            Quat spawnRot = GetRotation((uint)EntityID);
            Vector3 scale = new Vector3(0.0f, 0.0f, 0.0f); 

            deathID = PrefabInstantiate(gunshipDeathPath);
            
            // Publish death event 
            Publish("GunshipDeath", gunshipID.ToString());
            
            // Destroy the gunship
            SceneDestroyEntity(gunshipID);
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
using Engine;
using System;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Scene;
using static Engine.Transform;

namespace Game
{
    /// <summary>
    /// LogicBomb health script - attach to each LogicBomb sub-entity
    /// 
    /// Prefab Setup Required:
    /// 1. RigidBody: IsKinematic = true, IsTrigger = true
    /// 2. Tag: "LogicBomb" (for the LogicBomb itself)
    /// 3. Bullets must have Tag: "PrimaryBullet"
    /// 4. Parent entity must be named "loveletter" (or change parentLoveLetterName)
    /// </summary>
    public class LogicBombHealth : ScriptBehaviour
    {
        // ===== ENTITY NAMES =====
        [SerializeField] private string parentLoveLetterName = "loveletter";
        private uint parentLoveLetterID = 0;
        
        private uint logicBombEntityID = 0;
        
        // ===== HEALTH SETTINGS =====
        [SerializeField] private float maxHealth = 100.0f;
        private float currentHealth = 100.0f;
        
        [SerializeField] private float damagePerHit = 50.0f;
        
        // ===== STATE =====
        private bool isDead = false;
        
        // ===== SPAWN INVULNERABILITY =====
        [SerializeField] private float spawnGraceTime = 0.5f;
        private float spawnTimer = 0.0f;
        private bool isInvulnerable = true;

        // ===== BULLET DETECTION =====
        [SerializeField] private string bulletTag = "PrimaryBullet";
        [SerializeField] private float detectionRadius = 0.5f;
        [SerializeField] private float checkInterval = 0.05f;
        private float checkTimer = 0.0f;

        public override void OnStart()
        {
            // Find entity IDs
            logicBombEntityID = (uint)EntityID;
            parentLoveLetterID = SceneFindEntityByName(parentLoveLetterName);
            
            if (parentLoveLetterID == 0)
            {
                LogWarning("LogicBomb " + logicBombEntityID + " could not find parent: " + parentLoveLetterName);
            }
            
            // Initialize health
            currentHealth = maxHealth;
            isDead = false;
            isInvulnerable = true;
            spawnTimer = spawnGraceTime;

            LogMessage("LogicBomb " + logicBombEntityID + " initialized with " + maxHealth + " health");
            LogMessage("  Parent LoveLetter ID: " + parentLoveLetterID);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead) return;

            // Handle spawn invulnerability
            if (isInvulnerable)
            {
                spawnTimer -= deltaTime;
                if (spawnTimer <= 0.0f)
                {
                    isInvulnerable = false;
                    LogMessage("LogicBomb " + logicBombEntityID + " is now vulnerable!");
                }
                return;
            }

            // Check for bullet collisions
            checkTimer += deltaTime;
            if (checkTimer >= checkInterval)
            {
                checkTimer = 0.0f;
                CheckForBulletCollisions();
            }
        }

        private void CheckForBulletCollisions()
        {
            // Find all bullets by tag
            uint[] bullets = SceneFindEntitiesByTag(bulletTag);
            
            if (bullets == null || bullets.Length == 0)
                return;

            Engine.Vector3 myPosition = GetPosition(logicBombEntityID);

            foreach (uint bulletID in bullets)
            {
                if (bulletID == 0) continue;

                Engine.Vector3 bulletPos = GetPosition(bulletID);
                
                // Calculate distance
                float dx = bulletPos.X - myPosition.X;
                float dy = bulletPos.Y - myPosition.Y;
                float dz = bulletPos.Z - myPosition.Z;
                float distanceSq = dx * dx + dy * dy + dz * dz;
                float radiusSq = detectionRadius * detectionRadius;

                // If bullet is within detection radius
                if (distanceSq <= radiusSq)
                {
                    LogMessage("LogicBomb " + logicBombEntityID + " hit by bullet " + bulletID);
                    TakeDamage();
                    
                    // Destroy the bullet
                    SceneDestroyEntity(bulletID);
                    
                    // Only process one bullet per check
                    break;
                }
            }
        }

        private void TakeDamage()
        {
            currentHealth -= damagePerHit;
            LogMessage("LogicBomb " + logicBombEntityID + " damaged! Health: " + currentHealth + "/" + maxHealth);

            if (currentHealth <= 0.0f)
            {
                Die();
            }
        }

        private void Die()
        {
            if (isDead) return;
            isDead = true;

            LogMessage("LogicBomb " + logicBombEntityID + " destroyed!");

            // Verify parent LoveLetter still exists
            if (parentLoveLetterID != 0)
            {
                // Notify parent that a LogicBomb was destroyed
                Publish("LogicBombDestroyed", parentLoveLetterID.ToString());
                LogMessage("Notified LoveLetter " + parentLoveLetterID + " of LogicBomb destruction");
            }
            else
            {
                LogWarning("Parent LoveLetter ID is 0, cannot notify");
            }

            // TODO: Add destruction VFX/SFX here
            
            // Destroy this LogicBomb
            SceneDestroyEntity(logicBombEntityID);
        }

        public override void OnDestroy()
        {
            LogMessage("LogicBomb " + logicBombEntityID + " cleanup complete");
        }
    }
}
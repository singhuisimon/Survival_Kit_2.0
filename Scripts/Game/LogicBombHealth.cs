using Engine;
using System;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Scene;
using static Engine.Transform;

namespace Game
{
    public class LogicBombHealth : ScriptBehaviour
    {
        // ===== ENTITY REFERENCES =====
        [SerializeField] private string parentLoveLetterName = "loveletterv3";
        private uint logicBombEntityID = 0;
        private uint parentLoveLetterID = 0;
        
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
        [SerializeField] private string[] bulletTags = { "PrimaryBullet", "PrimaryBullet" };
        [SerializeField] private float detectionRadius = 8.0f;
        [SerializeField] private float checkInterval = 0.05f;
        private float checkTimer = 0.0f;

        // ===== PARENT TRACKING =====
        private bool parentWasDestroyed = false;

        public override void OnStart()
        {
            // Get this LogicBomb's entity ID
            logicBombEntityID = (uint)EntityID;
            
            // Find parent LoveLetter by name
            parentLoveLetterID = SceneFindEntityByName(parentLoveLetterName);
            
            if (parentLoveLetterID == 0)
            {
                LogError("LogicBomb " + logicBombEntityID + " could not find parent: " + parentLoveLetterName);
                LogError("Make sure the parent entity exists and is named exactly: " + parentLoveLetterName);
            }
            else
            {
                LogMessage("LogicBomb " + logicBombEntityID + " found parent ID: " + parentLoveLetterID);
            }
            
            // Initialize health
            currentHealth = maxHealth;
            isDead = false;
            isInvulnerable = true;
            spawnTimer = spawnGraceTime;
            parentWasDestroyed = false;

            // Log position for debugging
            Engine.Vector3 pos = GetPosition(logicBombEntityID);
            LogMessage("=== LogicBomb Initialized ===");
            LogMessage("  Entity ID: " + logicBombEntityID);
            LogMessage("  Health: " + maxHealth);
            LogMessage("  World Position: (" + pos.X.ToString("F1") + ", " + pos.Y.ToString("F1") + ", " + pos.Z.ToString("F1") + ")");
            LogMessage("  Parent LoveLetter ID: " + parentLoveLetterID);
            LogMessage("  Detection Radius: " + detectionRadius);
            LogMessage("  Damage Per Hit: " + damagePerHit);

            // Subscribe to parent destruction event
            Subscribe("LoveLetterDestroyed", OnParentDestroyed);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead) return;

            // Check if parent was destroyed
            if (parentWasDestroyed)
            {
                LogMessage("LogicBomb " + logicBombEntityID + " parent was destroyed, cleaning up self");
                Die();
                return;
            }

            // Verify parent still exists (in case it was destroyed without event)
            if (parentLoveLetterID != 0)
            {
                Engine.Vector3 parentPos = GetPosition(parentLoveLetterID);
                // If position is NaN or very far, parent might be destroyed
                if (float.IsNaN(parentPos.X) || float.IsNaN(parentPos.Y) || float.IsNaN(parentPos.Z))
                {
                    LogWarning("LogicBomb " + logicBombEntityID + " detected parent destruction via NaN position");
                    parentWasDestroyed = true;
                    Die();
                    return;
                }
            }

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
            // Get LogicBomb's WORLD position
            Engine.Vector3 myWorldPosition = GetPosition(logicBombEntityID);

            // Check each bullet tag variant
            foreach (string bulletTag in bulletTags)
            {
                if (string.IsNullOrEmpty(bulletTag)) continue;

                // Find all bullets with this tag
                uint[] bullets = SceneFindEntitiesByTag(bulletTag);
                
                if (bullets == null || bullets.Length == 0)
                    continue;

                foreach (uint bulletID in bullets)
                {
                    if (bulletID == 0) continue;

                    // Get bullet's WORLD position
                    Engine.Vector3 bulletWorldPos = GetPosition(bulletID);
                    
                    // Calculate distance in world space
                    float dx = bulletWorldPos.X - myWorldPosition.X;
                    float dy = bulletWorldPos.Y - myWorldPosition.Y;
                    float dz = bulletWorldPos.Z - myWorldPosition.Z;
                    float distance = SimpleMath.Sqrt(dx * dx + dy * dy + dz * dz);

                    // If bullet is within detection radius
                    if (distance <= detectionRadius)
                    {
                        LogMessage("=== BULLET HIT LOGICBOMB ===");
                        LogMessage("  LogicBomb ID: " + logicBombEntityID);
                        LogMessage("  Bullet ID: " + bulletID);
                        LogMessage("  Distance: " + distance.ToString("F1"));
                        LogMessage("  LogicBomb world pos: (" + myWorldPosition.X.ToString("F1") + ", " + myWorldPosition.Y.ToString("F1") + ", " + myWorldPosition.Z.ToString("F1") + ")");
                        LogMessage("  Bullet world pos: (" + bulletWorldPos.X.ToString("F1") + ", " + bulletWorldPos.Y.ToString("F1") + ", " + bulletWorldPos.Z.ToString("F1") + ")");
                        
                        TakeDamage();
                        
                        // Destroy the bullet
                        SceneDestroyEntity(bulletID);
                        
                        // Only process one bullet per check to avoid multiple hits
                        return;
                    }
                }
            }
        }

        private void TakeDamage()
        {
            currentHealth -= damagePerHit;
            LogMessage("=== LOGICBOMB DAMAGED ===");
            LogMessage("  LogicBomb ID: " + logicBombEntityID);
            LogMessage("  Health: " + currentHealth.ToString("F1") + "/" + maxHealth.ToString("F1"));

            if (currentHealth <= 0.0f)
            {
                Die();
            }
        }

        private void Die()
        {
            if (isDead) return;
            isDead = true;

            LogMessage("=== LOGICBOMB DESTROYED ===");
            LogMessage("  LogicBomb ID: " + logicBombEntityID);
            LogMessage("  Parent LoveLetter ID: " + parentLoveLetterID);

            // Verify parent LoveLetter still exists before notifying
            if (parentLoveLetterID != 0 && !parentWasDestroyed)
            {
                // Notify parent LoveLetter that one of its LogicBombs was destroyed
                // The payload contains the parent's ID so LoveLetter can verify it's for them
                Publish("LogicBombDestroyed", parentLoveLetterID.ToString());
                LogMessage("  Event published: LogicBombDestroyed for LoveLetter " + parentLoveLetterID);
            }
            else if (parentWasDestroyed)
            {
                LogMessage("  Parent was destroyed - no event needed");
            }
            else
            {
                LogWarning("  Cannot notify parent - parent LoveLetter ID is 0!");
            }

            // TODO: Add destruction VFX/SFX here
            // Example: Publish("PlayVFX", "logicbomb_explosion:" + logicBombEntityID);
            
            // Destroy this LogicBomb entity
            SceneDestroyEntity(logicBombEntityID);
        }

        private void OnParentDestroyed(string eventName, string payload)
        {
            // Check if this event is for our parent
            if (uint.TryParse(payload, out uint destroyedParentID))
            {
                if (destroyedParentID == parentLoveLetterID)
                {
                    LogMessage("LogicBomb " + logicBombEntityID + " received parent destruction event");
                    parentWasDestroyed = true;
                }
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe("LoveLetterDestroyed", OnParentDestroyed);
            LogMessage("LogicBomb " + logicBombEntityID + " cleanup complete");
        }
    }
}
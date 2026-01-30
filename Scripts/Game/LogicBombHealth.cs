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
        [SerializeField] private string parentLoveLetterName = "LoveLetter";
        private uint logicBombEntityID = 0;
        private uint parentLoveLetterID = 0;
        
        // ===== HEALTH SETTINGS =====
        [SerializeField] private float maxHealth = 30.0f;
        private float currentHealth = 30.0f;
        
        // Damage is now received from the bullet's Damage property via DamageSystem
        
        // ===== STATE =====
        private bool isDead = false;
        
        // ===== SPAWN INVULNERABILITY =====
        [SerializeField] private float spawnGraceTime = 0.5f;
        private float spawnTimer = 0.0f;
        private bool isInvulnerable = true;

        // ===== PARENT TRACKING =====
        private bool parentWasDestroyed = false;

        // ===== DEBUG =====
        [SerializeField] private bool enableDebug = true;
        [SerializeField] private float debugInterval = 1.0f;
        private float debugTimer = 0.0f;

        // ===== EVENTS =====
        private const string EVENT_BULLET_HIT = "BulletHit";
        private const string EVENT_LOVELETTER_DESTROYED = "LoveLetterDestroyed";
        private const string EVENT_LOGICBOMB_DESTROYED = "LogicBombDestroyed";

        public override void OnStart()
        {
            // Get this LogicBomb's entity ID
            logicBombEntityID = (uint)EntityID;
            
            // METHOD 1: Get parent directly from hierarchy (RECOMMENDED)
            parentLoveLetterID = TransformGetParent(logicBombEntityID);
            
            // METHOD 2: Find by name (fallback if Method 1 fails)
            if (parentLoveLetterID == 0 || parentLoveLetterID == 0xffffffffu)
            {
                LogWarning("Could not get parent via hierarchy, trying to find by name...");
                parentLoveLetterID = SceneFindEntityByName(parentLoveLetterName);
            }
            
            if (parentLoveLetterID == 0 || parentLoveLetterID == 0xffffffffu)
            {
                LogError("LogicBomb " + logicBombEntityID + " could not find parent!");
                LogError("Tried hierarchy and name: " + parentLoveLetterName);
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
            Engine.Vector3 scale = Transform.GetScale(logicBombEntityID);
            
            // LogMessage("=== LogicBomb Initialized ===");
            // LogMessage("  Entity ID: " + logicBombEntityID);
            // LogMessage("  Health: " + maxHealth);
            // LogMessage("  WORLD Position: (" + pos.X.ToString("F2") + ", " + pos.Y.ToString("F2") + ", " + pos.Z.ToString("F2") + ")");
            // LogMessage("  WORLD Scale: (" + scale.X.ToString("F4") + ", " + scale.Y.ToString("F4") + ", " + scale.Z.ToString("F4") + ")");
            // LogMessage("  Parent LoveLetter ID: " + parentLoveLetterID);

            // Debug parent's transform
            if (parentLoveLetterID != 0)
            {
                Engine.Vector3 parentPos = GetPosition(parentLoveLetterID);
                Engine.Vector3 parentScale = Transform.GetScale(parentLoveLetterID);
                LogMessage("  Parent WORLD Position: (" + parentPos.X.ToString("F2") + ", " + parentPos.Y.ToString("F2") + ", " + parentPos.Z.ToString("F2") + ")");
                LogMessage("  Parent WORLD Scale: (" + parentScale.X.ToString("F4") + ", " + parentScale.Y.ToString("F4") + ", " + parentScale.Z.ToString("F4") + ")");
            }

            // Subscribe to bullet hit event (same as Botnet does)
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            
            // Subscribe to parent destruction event
            Subscribe(EVENT_LOVELETTER_DESTROYED, OnParentDestroyed);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead) return;

            // DEBUG: Periodic transform logging
            if (enableDebug)
            {
                debugTimer += deltaTime;
                if (debugTimer >= debugInterval)
                {
                    debugTimer = 0.0f;
                    // DebugPrintTransform();
                }
            }

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
                    
                    if (enableDebug)
                    {
                        LogMessage("=== VULNERABILITY CHECK - Final Transform ===");
                        
                    }
                }
                return;
            }
        }

        // ===== EVENT HANDLERS (SAME AS BOTNET) =====

        private void OnBulletHit(string eventName, string payload)
        {
            if (isDead || eventName != EVENT_BULLET_HIT)
                return;

            // Skip if still invulnerable
            if (isInvulnerable)
            {
                LogMessage("LogicBomb " + logicBombEntityID + " is invulnerable, ignoring bullet hit");
                return;
            }

            // Parse the entity ID that was hit
            if (!uint.TryParse(payload, out uint hitId))
                return;

            // Check if this LogicBomb was the one hit
            if (hitId != logicBombEntityID)
            {
                // This bullet hit a different entity, not us
                return;
            }

            LogMessage("=== BULLET HIT LOGICBOMB ===");
            LogMessage("  LogicBomb ID: " + logicBombEntityID);
            LogMessage("  Hit confirmed by bullet event");
            
            if (enableDebug)
            {
                LogMessage("=== TRANSFORM AT HIT TIME ===");
                //DebugPrintTransform();
            }

            // Take damage (PrimaryBullet deals 10 damage per hit)
            // You can adjust this or get damage from DamageSystem if needed
            TakeDamage(10.0f);
        }

        private void TakeDamage(float damage)
        {
            currentHealth -= damage;
            LogMessage("=== LOGICBOMB DAMAGED ===");
            LogMessage("  LogicBomb ID: " + logicBombEntityID);
            LogMessage("  Damage Taken: " + damage.ToString("F1"));
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
                Publish(EVENT_LOGICBOMB_DESTROYED, parentLoveLetterID.ToString());
                LogMessage("  Event published: LogicBombDestroyed for LoveLetter " + parentLoveLetterID);
                LogMessage("  DEBUG: Published EVENT_LOGICBOMB_DESTROYED with payload: " + parentLoveLetterID);
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
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
            Unsubscribe(EVENT_LOVELETTER_DESTROYED, OnParentDestroyed);
            LogMessage("LogicBomb " + logicBombEntityID + " cleanup complete");
        }

       
    }
}
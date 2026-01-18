using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Scene;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// HealthBar - Visual representation of player health
    /// Uses 2D sprite renderer in screen-space (0-1280 x 0-720)
    /// Listens for damage events and updates visual accordingly
    /// </summary>
    public class HealthBar : ScriptBehaviour
    {
        // ===== Health Settings =====
        [SerializeField]
        private float maxHealth = 100.0f;

        private float currentHealth = 100.0f;

        // ===== Visual Settings =====
        // Screen position in pixels (Junrui's coordinate system)
        [SerializeField]
        private float screenX = 50.0f;  // Distance from left edge

        [SerializeField]
        private float screenY = 50.0f;  // Distance from top edge

        [SerializeField]
        private float barMaxWidth = 200.0f;  // Width when at 100% health

        [SerializeField]
        private float barHeight = 20.0f;  // Height of the bar

        // ===== State =====
        private bool isDead = false;
        private bool initialized = false;

        // ===== Event Names =====
        private const string EVENT_BOTNET_ATTACK = "BotnetAttackedPlayer";
        private const string EVENT_PLAYER_DAMAGED = "PlayerDamaged";
        private const string EVENT_PLAYER_HEALED = "PlayerHealed";
        private const string EVENT_SM_ACTIVATED = "SMActivated";
        private const string EVENT_SM_DEACTIVATED = "SMDeactivated";

        // ===== Damage Values =====
        private const float BOTNET_DAMAGE = 20.0f;

        public override void OnStart()
        {
            LogMessage("=== HealthBar OnStart ===");
            LogMessage("HealthBar EntityID: " + EntityID);

            // Initialize health to full
            currentHealth = maxHealth;
            isDead = false;

            // Subscribe to damage/heal events
            Subscribe(EVENT_BOTNET_ATTACK, OnBotnetAttack);
            Subscribe(EVENT_PLAYER_DAMAGED, OnPlayerDamaged);
            Subscribe(EVENT_PLAYER_HEALED, OnPlayerHealed);
            Subscribe(EVENT_SM_ACTIVATED, OnGameStart);
            Subscribe(EVENT_SM_DEACTIVATED, OnGameEnd);

            // Set initial position and scale
            UpdateHealthBarVisual();

            initialized = true;

            LogMessage("HealthBar initialized:");
            LogMessage("  Position: (" + screenX + ", " + screenY + ")");
            LogMessage("  Size: " + barMaxWidth + " x " + barHeight);
            LogMessage("  Health: " + currentHealth + "/" + maxHealth);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // Check for death
            if (currentHealth <= 0.0f && !isDead)
            {
                Die();
            }
        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Unsubscribe(EVENT_BOTNET_ATTACK, OnBotnetAttack);
            Unsubscribe(EVENT_PLAYER_DAMAGED, OnPlayerDamaged);
            Unsubscribe(EVENT_PLAYER_HEALED, OnPlayerHealed);
            Unsubscribe(EVENT_SM_ACTIVATED, OnGameStart);
            Unsubscribe(EVENT_SM_DEACTIVATED, OnGameEnd);

            LogMessage("=== HealthBar Destroyed ===");
        }

        // ===== Event Handlers =====

        private void OnBotnetAttack(string eventName, string payload)
        {
            if (isDead)
            {
                LogMessage("HealthBar: Ignoring attack - player is dead");
                return;
            }

            // Parse botnet ID from payload (optional, for logging)
            if (ulong.TryParse(payload, out ulong botnetID))
            {
                LogMessage("HealthBar: Botnet " + botnetID + " attacked player!");
            }
            else
            {
                LogMessage("HealthBar: Botnet attacked player!");
            }

            // Apply botnet damage
            TakeDamage(BOTNET_DAMAGE);
        }

        private void OnPlayerDamaged(string eventName, string payload)
        {
            if (isDead)
            {
                LogMessage("HealthBar: Ignoring damage - player is dead");
                return;
            }

            // Parse damage amount
            if (!float.TryParse(payload, out float damage))
            {
                LogError("HealthBar: Invalid damage value: " + payload);
                return;
            }

            LogMessage("HealthBar: Player took " + damage + " damage");
            TakeDamage(damage);
        }

        private void OnPlayerHealed(string eventName, string payload)
        {
            if (isDead)
            {
                LogMessage("HealthBar: Ignoring heal - player is dead");
                return;
            }

            // Parse heal amount
            if (!float.TryParse(payload, out float healAmount))
            {
                LogError("HealthBar: Invalid heal value: " + payload);
                return;
            }

            LogMessage("HealthBar: Player healed " + healAmount + " HP");
            Heal(healAmount);
        }

        private void OnGameStart(string eventName, string payload)
        {
            LogMessage("HealthBar: Game started - resetting health");

            // Reset to full health
            currentHealth = maxHealth;
            isDead = false;

            // Update visual
            UpdateHealthBarVisual();
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("HealthBar: Game ended");
            // Could add logic here if needed (fade out, etc.)
        }

        // ===== Health Management =====

        private void TakeDamage(float damage)
        {
            if (isDead)
                return;

            // Reduce health
            currentHealth -= damage;

            // Clamp to zero
            if (currentHealth < 0.0f)
                currentHealth = 0.0f;

            // Update visual
            UpdateHealthBarVisual();

            // Log status
            float healthPercent = (currentHealth / maxHealth) * 100.0f;
            LogMessage("HealthBar: Took " + damage + " damage");
            LogMessage("  Current health: " + currentHealth + "/" + maxHealth + " (" + healthPercent + "%)");
        }

        private void Heal(float amount)
        {
            if (isDead)
                return;

            // Increase health
            currentHealth += amount;

            // Clamp to max
            if (currentHealth > maxHealth)
                currentHealth = maxHealth;

            // Update visual
            UpdateHealthBarVisual();

            // Log status
            float healthPercent = (currentHealth / maxHealth) * 100.0f;
            LogMessage("HealthBar: Healed " + amount + " HP");
            LogMessage("  Current health: " + currentHealth + "/" + maxHealth + " (" + healthPercent + "%)");
        }

        private void Die()
        {
            isDead = true;

            LogMessage("======================");
            LogMessage("PLAYER DIED!");
            LogMessage("======================");

            // Publish death event for UIStateManager and other systems
            Publish("PlayerHasDied", EntityID.ToString());
        }

        // ===== Visual Update =====

        private void UpdateHealthBarVisual()
        {
            // Calculate health percentage (0.0 to 1.0)
            float healthPercent = currentHealth / maxHealth;

            // Clamp to valid range
            if (healthPercent < 0.0f) healthPercent = 0.0f;
            if (healthPercent > 1.0f) healthPercent = 1.0f;

            // Calculate current width based on health
            float currentWidth = barMaxWidth * healthPercent;

            // Calculate bar center position
            // The bar shrinks from the right, keeping left edge at screenX
            float barCenterX = screenX + (currentWidth / 2.0f);
            float barCenterY = screenY + (barHeight / 2.0f);

            // Create position vector
            Vector3 position = new Vector3(barCenterX, barCenterY, 0.0f);

            // Create scale vector
            Vector3 scale = new Vector3(currentWidth, barHeight, 1.0f);

            // Apply to entity
            SetPosition((uint)EntityID, ref position);
            SetScale((uint)EntityID, ref scale);

            // Log for debugging
            LogMessage("HealthBar visual updated:");
            LogMessage("  Health: " + currentHealth + "/" + maxHealth + " (" + (healthPercent * 100.0f) + "%)");
            LogMessage("  Width: " + currentWidth + " (max: " + barMaxWidth + ")");
            LogMessage("  Position: (" + barCenterX + ", " + barCenterY + ")");
        }

        // ===== Public API =====

        /// <summary>
        /// Directly set health to a specific value
        /// </summary>
        public void SetHealth(float health)
        {
            currentHealth = health;

            // Clamp to valid range
            if (currentHealth < 0.0f) currentHealth = 0.0f;
            if (currentHealth > maxHealth) currentHealth = maxHealth;

            // Update visual
            UpdateHealthBarVisual();
        }

        /// <summary>
        /// Get current health value
        /// </summary>
        public float GetHealth()
        {
            return currentHealth;
        }

        /// <summary>
        /// Get maximum health value
        /// </summary>
        public float GetMaxHealth()
        {
            return maxHealth;
        }

        /// <summary>
        /// Check if player is dead
        /// </summary>
        public bool IsDead()
        {
            return isDead;
        }
    }
}
using System;

namespace Engine
{
    /// <summary>
    /// Health component.
    /// Stores current / max health and exposes simple damage / heal helpers.
    /// Purely managed (C#) – no native backing required.
    /// </summary>
    public class Health : Component
    {
        // ===== Serialized Fields (Inspector) =====

        [SerializeField("Max Health")]
        private float maxHealth = 100.0f;

        [SerializeField("Current Health")]
        private float currentHealth = 100.0f;

        [SerializeField("Can Die")]
        private bool canDie = true;

        // ===== Public Properties =====

        /// <summary>
        /// Maximum health value. Setting this clamps CurrentHealth.
        /// </summary>
        public float MaxHealth
        {
            get { return maxHealth; }
            set
            {
                if (value < 1.0f)
                    value = 1.0f;

                maxHealth = value;

                if (currentHealth > maxHealth)
                    currentHealth = maxHealth;
            }
        }

        /// <summary>
        /// Current health, clamped between 0 and MaxHealth.
        /// </summary>
        public float CurrentHealth
        {
            get { return currentHealth; }
            set
            {
                float clamped = Clamp(value, 0.0f, maxHealth);

                // Avoid tiny oscillations
                if (Math.Abs(clamped - currentHealth) > 0.0001f)
                {
                    currentHealth = clamped;
                    OnHealthChanged();

                    if (currentHealth <= 0.0f && canDie)
                    {
                        OnDeath();
                    }
                }
            }
        }

        /// <summary>
        /// Convenience alias to keep your original property name.
        /// </summary>
        public float health
        {
            get { return CurrentHealth; }
            set { CurrentHealth = value; }
        }

        /// <summary>
        /// True when health is zero or below.
        /// </summary>
        public bool IsDead
        {
            get { return currentHealth <= 0.0f; }
        }

        /// <summary>
        /// Whether this component is allowed to enter a "dead" state.
        /// </summary>
        public bool CanDie
        {
            get { return canDie; }
            set { canDie = value; }
        }

        // ===== Public API =====

        /// <summary>
        /// Deal damage to this entity.
        /// </summary>
        public void TakeDamage(float amount)
        {
            if (amount <= 0.0f || IsDead)
                return;

            CurrentHealth = CurrentHealth - amount;
        }

        /// <summary>
        /// Heal this entity.
        /// </summary>
        public void Heal(float amount)
        {
            if (amount <= 0.0f || IsDead)
                return;

            CurrentHealth = CurrentHealth + amount;
        }

        /// <summary>
        /// Instantly restore to full health.
        /// </summary>
        public void ResetHealth()
        {
            CurrentHealth = maxHealth;
        }

        // ===== Hooks for game logic / UI =====

        /// <summary>
        /// Called whenever CurrentHealth changes (after clamping).
        /// Override in a subclass or call this from other scripts if needed.
        /// </summary>
        protected virtual void OnHealthChanged()
        {
            // Example: hook your UI or event system from here in game code.
            // e.g. Engine.EventSystem.Publish("HealthChanged", "value=" + currentHealth);
        }

        /// <summary>
        /// Called once when health hits zero (if CanDie is true).
        /// </summary>
        protected virtual void OnDeath()
        {
            // Example: play death anim, disable controls, fire events, etc.
            // e.g. Engine.EventSystem.Publish("EntityDied", "health=0");
        }

        // ===== Utility =====

        private float Clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
    }
}

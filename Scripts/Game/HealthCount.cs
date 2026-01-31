using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// HealthCount - Displays player health as "100/100" text
    /// Attach this to a text entity
    /// Listens to the same "Health Change" event as HealthBar
    /// </summary>
    public class HealthCount : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_PLAYER_HEALTHCHANGE = "Health Change";
        private const float MaxHealth = 100.0f;

        // ===== State =====
        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== HealthCount OnStart ===");
            LogMessage("HealthCount EntityID: " + EntityID);

            // Subscribe to health change events
            Event.Subscribe(EVENT_PLAYER_HEALTHCHANGE, OnHealthChange);

            // Set initial text
            SetText((uint)EntityID, "100/100");

            initialized = true;
            LogMessage("[HealthCount] Initialized");
        }

        private void OnHealthChange(string eventName, string payload)
        {
            if (!float.TryParse(payload, out float currentHP))
            {
                LogError("[HealthCount] Failed to parse HP from payload: " + payload);
                return;
            }

            // Clamp
            if (currentHP < 0.0f) currentHP = 0.0f;
            if (currentHP > MaxHealth) currentHP = MaxHealth;

            // Format as "100/100"
            int current = (int)currentHP;
            int max = (int)MaxHealth;
            SetText((uint)EntityID, current + "/" + max);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_HEALTHCHANGE, OnHealthChange);
            LogMessage("=== HealthCount Destroyed ===");
        }
    }
}
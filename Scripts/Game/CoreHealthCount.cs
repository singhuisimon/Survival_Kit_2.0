using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// CoreHealthCount - Displays core health as "100/100" text
    /// Attach this to a text entity
    /// Listens to the same "Core Health Change" event as CoreHealthBar
    /// </summary>
    public class CoreHealthCount : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_CORE_HEALTHCHANGE = "Core Health Change";
        private const float MaxHealth = 100.0f;

        // ===== State =====
        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== CoreHealthCount OnStart ===");
            LogMessage("CoreHealthCount EntityID: " + EntityID);

            // Subscribe to core health change events
            Event.Subscribe(EVENT_CORE_HEALTHCHANGE, OnCoreHealthChange);

            // Set initial text
            SetText((uint)EntityID, "100/100");

            initialized = true;
            LogMessage("[CoreHealthCount] Initialized");
        }

        private void OnCoreHealthChange(string eventName, string payload)
        {
            if (!float.TryParse(payload, out float currentHP))
            {
                LogError("[CoreHealthCount] Failed to parse HP from payload: " + payload);
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
            Event.Unsubscribe(EVENT_CORE_HEALTHCHANGE, OnCoreHealthChange);
            LogMessage("=== CoreHealthCount Destroyed ===");
        }
    }
}
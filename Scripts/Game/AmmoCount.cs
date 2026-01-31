using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// AmmoCount - Displays player ammo as "100/100" text
    /// Attach this to a text entity
    /// Listens to the same "AmmoChange" event as AmmoBar
    /// </summary>
    public class AmmoCount : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_AMMO_CHANGE = "AmmoChange";
        private const float MaxAmmo = 100.0f;

        // ===== State =====
        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== AmmoCount OnStart ===");
            LogMessage("AmmoCount EntityID: " + EntityID);

            // Subscribe to ammo change events
            Event.Subscribe(EVENT_AMMO_CHANGE, OnAmmoChange);

            // Set initial text
            SetText((uint)EntityID, "100/100");

            initialized = true;
            LogMessage("[AmmoCount] Initialized");
        }

        private void OnAmmoChange(string eventName, string payload)
        {
            if (!float.TryParse(payload, out float currentAmmo))
            {
                LogError("[AmmoCount] Failed to parse ammo from payload: " + payload);
                return;
            }

            // Clamp
            if (currentAmmo < 0.0f) currentAmmo = 0.0f;
            if (currentAmmo > MaxAmmo) currentAmmo = MaxAmmo;

            // Format as "100/100"
            int current = (int)currentAmmo;
            int max = (int)MaxAmmo;
            SetText((uint)EntityID, current + "/" + max);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_AMMO_CHANGE, OnAmmoChange);
            LogMessage("=== AmmoCount Destroyed ===");
        }
    }
}
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
    /// Hides on win or lose
    /// </summary>
    public class AmmoCount : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_AMMO_CHANGE = "AmmoChange";
        private const float MaxAmmo = 100.0f;

        // ===== Game Over Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string EVENT_TIMER_FINISHED = "TimerFinished";

        // ===== State =====
        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== AmmoCount OnStart ===");
            LogMessage("AmmoCount EntityID: " + EntityID);

            // Subscribe to ammo change events
            Event.Subscribe(EVENT_AMMO_CHANGE, OnAmmoChange);

            // Subscribe to game over events
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameOver);

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

            if (currentAmmo < 0.0f) currentAmmo = 0.0f;
            if (currentAmmo > MaxAmmo) currentAmmo = MaxAmmo;

            int current = (int)currentAmmo;
            int max = (int)MaxAmmo;
            SetText((uint)EntityID, current + "/" + max);
        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[AmmoCount] Game over - hiding text");
            Text.SetIsVisible((uint)EntityID, false);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_AMMO_CHANGE, OnAmmoChange);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameOver);
            LogMessage("=== AmmoCount Destroyed ===");
        }
    }
}
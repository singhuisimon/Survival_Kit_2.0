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
    /// Hides on win or lose
    /// </summary>
    public class HealthCount : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_PLAYER_HEALTHCHANGE = "Health Change";
        private const float MaxHealth = 100.0f;

        // ===== Game Over Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string GAMEWIN = "GameWin";


        // ===== State =====
        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== HealthCount OnStart ===");
            LogMessage("HealthCount EntityID: " + EntityID);

            // Subscribe to health change events
            Event.Subscribe(EVENT_PLAYER_HEALTHCHANGE, OnHealthChange);

            // Subscribe to game over events
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameOver);

            Event.Subscribe(ENEMY_CORE_DEATH, OnGameOver);


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

            if (currentHP < 0.0f) currentHP = 0.0f;
            if (currentHP > MaxHealth) currentHP = MaxHealth;

            int current = (int)currentHP;
            int max = (int)MaxHealth;
            SetText((uint)EntityID, current + "/" + max);
        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[HealthCount] Game over - hiding text");
            Text.SetIsVisible((uint)EntityID, false);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_HEALTHCHANGE, OnHealthChange);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameOver);
            LogMessage("=== HealthCount Destroyed ===");
        }
    }
}
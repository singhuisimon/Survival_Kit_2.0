using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    public class EnemyCoreHPCount : ScriptBehaviour
    {
        private const string EVENT_ENEMYCORE_HEALTHCHANGE = "EnemyCore Health Change";
        private const float MaxHealth = 600.0f;

        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string GAMEWIN = "GameWin";

        public override void OnStart()
        {
            LogMessage("=== EnemyCoreHPCount OnStart ===");

            Event.Subscribe(EVENT_ENEMYCORE_HEALTHCHANGE, OnHealthChange);

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Subscribe(ENEMY_CORE_DEATH, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameOver);

            SetText((uint)EntityID, "600/600");
            LogMessage("[EnemyCoreHPCount] Initialized");
        }

        private void OnHealthChange(string eventName, string payload)
        {
            if (!float.TryParse(payload, out float currentHP))
            {
                LogError("[EnemyCoreHPCount] Failed to parse HP: " + payload);
                return;
            }

            if (currentHP < 0.0f) currentHP = 0.0f;
            if (currentHP > MaxHealth) currentHP = MaxHealth;

            SetText((uint)EntityID, (int)currentHP + "/" + (int)MaxHealth);
        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[EnemyCoreHPCount] Game over - hiding text");
            Text.SetIsVisible((uint)EntityID, false);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_ENEMYCORE_HEALTHCHANGE, OnHealthChange);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Unsubscribe(ENEMY_CORE_DEATH, OnGameOver);
            Event.Unsubscribe(GAMEWIN, OnGameOver);
        }
    }
}
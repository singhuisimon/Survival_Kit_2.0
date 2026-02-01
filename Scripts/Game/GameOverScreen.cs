using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.Audio;
using static Engine.AudioManager;

namespace Game
{
    /// <summary>
    /// GameOverScreen - Shows lose screen when player or core dies
    /// Attach this to the lose screen texture entity
    /// Listens to "PlayerDead" and "CoreMotherboardDestroyed" events
    /// </summary>
    public class GameOverScreen : ScriptBehaviour
    {
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== GameOverScreen OnStart ===");
            LogMessage("GameOverScreen EntityID: " + EntityID);

            // Subscribe to both lose conditions
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);

            // Start hidden
            SetIsVisible((uint)EntityID, false);

            initialized = true;
            LogMessage("[GameOverScreen] Initialized - waiting for lose condition");
        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[GameOverScreen] Game Over triggered by: " + eventName);
            StopAll();
            SetIsVisible((uint)EntityID, true);
            AudioPlay((uint)EntityID);
        }

        public override void OnDestroy()
        {
            AudioStop((uint)EntityID);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            LogMessage("=== GameOverScreen Destroyed ===");
        }
    }
}
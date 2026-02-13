using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.AudioManager;

namespace Game
{
    /// <summary>
    /// WinScreen - Shows win screen when timer runs out
    /// Attach this to the win screen texture entity
    /// Listens to "TimerFinished" event
    /// </summary>
    public class WinScreen : ScriptBehaviour
    {
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string GAMEWIN = "GameWin";

        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== WinScreen OnStart ===");
            LogMessage("WinScreen EntityID: " + EntityID);

            // Subscribe to timer finished
            Event.Subscribe(EVENT_TIMER_FINISHED, OnWin);
            Event.Subscribe(ENEMY_CORE_DEATH, OnWin);

            // Start hidden
            SetIsVisible((uint)EntityID, false);

            initialized = true;
            LogMessage("[WinScreen] Initialized - waiting for timer to finish");
        }

        private void OnWin(string eventName, string payload)
        {
            LogMessage("[WinScreen] Win! Timer finished!");
            StopGroup(AudioType.BGM);
            StopGroup(AudioType.SFX);
            Input.SetCursorVisible(true);
            SetIsVisible((uint)EntityID, true);
            Publish(GAMEWIN, "");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnWin);
            Event.Unsubscribe(ENEMY_CORE_DEATH, OnWin);
            LogMessage("=== WinScreen Destroyed ===");
        }
    }
}
using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

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

        private bool initialized = false;

        public override void OnStart()
        {
            LogMessage("=== WinScreen OnStart ===");
            LogMessage("WinScreen EntityID: " + EntityID);

            // Subscribe to timer finished
            Event.Subscribe(EVENT_TIMER_FINISHED, OnWin);

            // Start hidden
            SetIsVisible((uint)EntityID, false);

            initialized = true;
            LogMessage("[WinScreen] Initialized - waiting for timer to finish");
        }

        private void OnWin(string eventName, string payload)
        {
            LogMessage("[WinScreen] Win! Timer finished!");
            SetIsVisible((uint)EntityID, true);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnWin);
            LogMessage("=== WinScreen Destroyed ===");
        }
    }
}
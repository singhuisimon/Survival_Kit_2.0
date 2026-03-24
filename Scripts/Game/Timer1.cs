using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// TimerUI - Displays a count-up timer in 00 m 00 s format.
    /// Attach this script to a text entity to show the game timer.
    /// Counts up from 0. Stops on win or lose.
    /// </summary>
    public class Timer1 : ScriptBehaviour
    {
        // Events
        private const string EVENTPLAYERDEAD = "PlayerDead";
        private const string GAMEWIN = "GameWin";
        private const string EVENTDEBUGSETTIMER = "DebugSetTimer";

        // State
        private bool initialized = false;
        private bool gameOver = false;
        private float elapsedTime = 0.0f;

        public override void OnStart()
        {
            LogMessage("TimerUI1: OnStart");
            LogMessage("TimerUI1: EntityID " + EntityID);

            Event.Subscribe(EVENTPLAYERDEAD, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameOver);
            Event.Subscribe(EVENTDEBUGSETTIMER, OnDebugSetTimer);

            elapsedTime = 0.0f;
            gameOver = false;

            UpdateTimerDisplay();
            initialized = true;
            LogMessage("TimerUI1: Initialized - counting up from 0");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || gameOver) return;
            if (GameState.IsPaused) return;

            elapsedTime += deltaTime;
            UpdateTimerDisplay();
        }

        private void OnDebugSetTimer(string eventName, string payload)
        {
            if (float.TryParse(payload, out float newTime))
            {
                elapsedTime = newTime;
                LogMessage("TimerUI1: Debug timer set to " + newTime + " seconds");
                UpdateTimerDisplay();
            }
        }

        private void OnGameOver(string eventName, string payload)
        {
            if (gameOver) return;
            LogMessage("TimerUI1: Game over triggered by " + eventName + " - Timer stopped");
            gameOver = true;
            Text.SetIsVisible((uint)EntityID, false);

            // Publish elapsed time so EndScreenStats can display it
            Publish("ShowTimeSurvived", elapsedTime.ToString("F2"));
            LogMessage("TimerUI1: Time elapsed = " + elapsedTime);
        }

        private void UpdateTimerDisplay()
        {
            int minutes = (int)(elapsedTime / 60);
            int seconds = (int)(elapsedTime % 60);
            string timeText = string.Format("{0:00} m {1:00} s", minutes, seconds);
            SetText((uint)EntityID, timeText);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENTPLAYERDEAD, OnGameOver);
            Event.Unsubscribe(GAMEWIN, OnGameOver);
            Event.Unsubscribe(EVENTDEBUGSETTIMER, OnDebugSetTimer);
            LogMessage("TimerUI1: Destroyed");
        }
    }
}

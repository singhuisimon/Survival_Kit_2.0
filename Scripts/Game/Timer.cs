using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;
namespace Game
{
    /// <summary>
    /// TimerUI - Displays countdown timer in "00 m : 00 s" format
    /// Attach this script to a text entity to show game timer
    /// Counts down from 5 minutes to 0
    /// Stops on win (timer hits 0) or lose (player/core dies)
    /// </summary>
    public class Timer : ScriptBehaviour
    {
        // ===== Settings =====
        [SerializeField("Starting Time (seconds)")]
        private float startingTime = 300.0f;  // 5 minutes = 300 seconds

        // ===== Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        // ===== State =====
        private bool initialized = false;
        private bool gameOver = false;
        private float remainingTime = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== TimerUI OnStart ===");
            LogMessage("TimerUI EntityID: " + EntityID);

            // Subscribe to lose conditions
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);

            // Initialize with starting time
            remainingTime = startingTime;
            gameOver = false;

            // Display initial time
            UpdateTimerDisplay();

            initialized = true;
            LogMessage("[TimerUI] Initialized - Starting at " + startingTime + " seconds");
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!initialized || gameOver)
                return;

            // Don't count down when game is paused
            if (GameState.IsPaused)
                return;

            // Count down
            remainingTime -= (deltaTime / 2);

            // Clamp to 0 and publish win event once
            if (remainingTime <= 0.0f)
            {
                remainingTime = 0.0f;
                gameOver = true;
                Publish("TimerFinished", "");
                LogMessage("[TimerUI] Timer finished! Win!");
            }

            // Update display
            UpdateTimerDisplay();
        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[TimerUI] Game over triggered by: " + eventName + " - Timer stopped");
            gameOver = true;
        }

        private void UpdateTimerDisplay()
        {
            // Format time as "00 m : 00 s"
            int minutes = (int)(remainingTime / 60);
            int seconds = (int)(remainingTime % 60);
            string timeText = string.Format("{0:00} m : {1:00} s", minutes, seconds);

            // Update the text display
            SetText((uint)EntityID, timeText);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            LogMessage("=== TimerUI Destroyed ===");
        }
    }
}
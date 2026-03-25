using Engine;
using System;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Text;
namespace Game
{
    /// <summary>
    /// TimerUI - Displays countdown timer in "00 m : 00 s" format
    /// Attach this script to a text entity to show game timer
    /// Counts down from 5 minutes to 0
    /// Stops on win (timer hits 0) or lose (player/core dies)
    /// </summary>
    public class Timer2 : ScriptBehaviour
    {
        // ===== Settings =====
        [SerializeField("Starting Time2 (seconds)")]
        private float startingTime = 240.0f;  // 2 minutes = 120 seconds

        private float timePassed = 0.0f;
        private float time1_5min = 90.0f;
        private float time3min = 180.0f;

        // ===== Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string GAMEWIN = "GameWin";
        private const string EVENT_DEBUG_SET_TIMER = "DebugSetTimer";
        private const string EVENT_1_5MIN = "1_5MIN";
        private const string EVENT_3MIN = "3MIN";
        private const string EVENT_BOTSPAWN = "BOTSPAWN";
        private const string EVENT_WORMSPAWN = "WORMSPAWN";
        private const string EVENT_TUTORIALOVER = "TUTORIALOVER";

        // ===== State =====
        private bool initialized = false;
        private bool gameOver = false;
        private float remainingTime = 0.0f;

        private bool event1_5min = false;
        private bool event3min = false;

        public override void OnStart()
        {
            LogMessage("=== TimerUI2 OnStart ===");
            LogMessage("TimerUI2 EntityID: " + EntityID);

            // Subscribe to lose conditions
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);

            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameOver);
            Event.Subscribe(EVENT_DEBUG_SET_TIMER, OnDebugSetTimer);
            Event.Subscribe(EVENT_TUTORIALOVER, OnTutorialOver);

            // Subscribe to 

            // Initialize with starting time
            remainingTime = startingTime;
            gameOver = false;

            // Display initial time
            UpdateTimerDisplay();

            // Hide timer at the start before tutorial is done
            Text.SetIsVisible(EntityID, false);

            //initialized = true;
            //LogMessage("[TimerUI2] Initialized - Starting at " + startingTime + " seconds");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || gameOver)
                return;

            if (GameState.IsPaused)
                return;

            remainingTime -= deltaTime;
            timePassed += deltaTime;

            if (remainingTime <= 0.0f)
            {
                remainingTime = 0.0f;
                gameOver = true;
                Text.SetIsVisible((uint)EntityID, false);

                // Publish time survived (full duration since timer ran out)
                Publish("ShowTimeSurvived", startingTime.ToString("F2"));
                LogMessage("[TimerUI2] Timer finished - time survived: " + startingTime);

                Publish("TimerFinished", "");
                LogMessage("[TimerUI2] Timer finished! Win!");
                return;
            }

            UpdateTimerDisplay();
        }


        private void OnDebugSetTimer(string eventName, string payload)
        {
            float.TryParse(payload, out float newTime);
            remainingTime = newTime;
            LogMessage("[TimerUI2] Debug: timer set to " + newTime + " seconds");
            UpdateTimerDisplay();
        }

        private void OnGameOver(string eventName, string payload)
        {
            if (gameOver) return; // prevent double-firing
            LogMessage("[TimerUI2] Game over triggered by: " + eventName + " - Timer stopped");
            gameOver = true;
            Text.SetIsVisible((uint)EntityID, false);

            float timeSurvived = startingTime - remainingTime;
            if (timeSurvived < 0.0f) timeSurvived = 0.0f;
            Publish("ShowTimeSurvived", timeSurvived.ToString("F2"));
            LogMessage("[TimerUI2] Time survived: " + timeSurvived);
        }

        private void OnTutorialOver(string eventName, string payload)
        {
            initialized = true;
            // Show timer
            Text.SetIsVisible(EntityID, true);
            LogMessage("[TimerUI2] Level 2 tutorial is over, begin main game timer");
            LogMessage("[TimerUI2] Initialized - Starting at " + startingTime + " seconds");
        }

        private void UpdateTimerDisplay()
        {
            // Format time as "00 m : 00 s"
            int minutes = (int)(remainingTime / 60);
            int seconds = (int)(remainingTime % 60);
            string timeText = string.Format("{0:00} m : {1:00} s", minutes, seconds);

            // Update the text display
            SetText((uint)EntityID, timeText);

            if(!event1_5min || !event3min){
                CheckTimeForEvent();
            }

        }


        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Unsubscribe(GAMEWIN, OnGameOver);
            Event.Unsubscribe(EVENT_DEBUG_SET_TIMER, OnDebugSetTimer);
            Event.Unsubscribe(EVENT_TUTORIALOVER, OnTutorialOver);
            LogMessage("=== TimerUI2 Destroyed ===");
        }

        private void CheckTimeForEvent(){

            if(timePassed >= time1_5min && !event1_5min){
                Publish(EVENT_1_5MIN, "");
                event1_5min = true;
                return;
            }

            if (event1_5min)
            {
                if(timePassed >= time3min && !event3min)
                {
                    Publish(EVENT_3MIN, "");
                    event3min = true;
                    return;
                }
            }

        }
    }
}
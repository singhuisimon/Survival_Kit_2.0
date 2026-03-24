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
    public class Timer2 : ScriptBehaviour
    {
        // ===== Settings =====
        [SerializeField("Starting Time2 (seconds)")]
        private float startingTime = 240.0f;  // 2 minutes = 120 seconds

        private float timePassed = 0.0f;
        //private float time1min = 60.0f;
        private float time2min = 120.0f;

        // ===== Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string GAMEWIN = "GameWin";
        private const string EVENT_DEBUG_SET_TIMER = "DebugSetTimer";
        //private const string EVENT_1MIN = "1MIN";
        private const string EVENT_2MIN = "2MIN";
        private const string EVENT_BOTSPAWN = "BOTSPAWN";
        private const string EVENT_WORMSPAWN = "WORMSPAWN";
        private const string EVENT_TUTORIALOVER = "TUTORIALOVER";

        // ===== State =====
        private bool initialized = false;
        private bool gameOver = false;
        private float remainingTime = 0.0f;
        //private bool event1min = false;
        private bool event2min = false;

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

            //initialized = true;
            LogMessage("[TimerUI2] Initialized - Starting at " + startingTime + " seconds");
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
            LogMessage("[TimerUI2] Level 2 tutorial is over, begin main game timer");
        }



        private void UpdateTimerDisplay()
        {
            // Format time as "00 m : 00 s"
            int minutes = (int)(remainingTime / 60);
            int seconds = (int)(remainingTime % 60);
            string timeText = string.Format("{0:00} m : {1:00} s", minutes, seconds);

            // Update the text display
            SetText((uint)EntityID, timeText);

            //if(!event1min || !event2min){
            if(!event2min){
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
            LogMessage("=== TimerUI2 Destroyed ===");
        }

        private void CheckTimeForEvent(){
            // if(timePassed >= time1min && !event1min){
            //     Publish(EVENT_1MIN, "");
            //     event1min = true;
            // }

            //if(event1min && (timePassed >= time2min && !event2min)){
            if(timePassed >= time2min && !event2min){
                Publish(EVENT_2MIN, "");
                event2min = true;
            }
        }
    }
}
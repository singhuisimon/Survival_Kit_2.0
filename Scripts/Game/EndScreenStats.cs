using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;
using static Engine.Scene;

namespace Game
{
    /// <summary>
    /// EndScreenStats - Displays final score and time on both win and lose screens.
    /// Attach to any entity in the scene (can be invisible/empty).
    /// Listens to: "ShowFinalScore", "ShowTimeSurvived", "WinButtonsFaded", "LoseButtonsFaded"
    /// </summary>
    public class EndScreenStats : ScriptBehaviour
    {
        [SerializeField] private string scoreTextEntityName = "UI_EndScore";
        [SerializeField] private string timeTextEntityName = "UI_EndTimeSurvived";

        private const string EVENT_SHOW_SCORE = "ShowFinalScore";
        private const string EVENT_SHOW_TIME = "ShowTimeSurvived";
        private const string EVENT_WIN_BUTTONS = "WinButtonsFaded";
        private const string EVENT_LOSE_BUTTONS = "LoseButtonsFaded";
        private const string EVENT_GAMEOVER = "GameOver";

        private uint scoreTextID = 0;
        private uint timeTextID = 0;

        private string cachedScore = "";
        private string cachedTime = "";
        private bool buttonsReady = false;
        private bool statsShown = false;

        // Score animation
        private bool isAnimatingScore = false;
        private float displayedScore = 0.0f;
        private int targetScore = 0;
        private const float SCORE_ANIM_DURATION = 2.0f;
        private float scoreAnimElapsed = 0.0f;

        // Start delay
        private float scoreStartDelay = 1.0f;
        private float scoreStartDelayTimer = 0.0f;
        private bool scoreDelayDone = false;

        public override void OnStart()
        {
            LogMessage("=== EndScreenStats OnStart ===");

            scoreTextID = SceneFindEntityByName(scoreTextEntityName);
            timeTextID = SceneFindEntityByName(timeTextEntityName);

            if (scoreTextID == 0) LogMessage("[EndScreenStats] Could not find: " + scoreTextEntityName);
            if (timeTextID == 0) LogMessage("[EndScreenStats] Could not find: " + timeTextEntityName);

            if (scoreTextID != 0) Text.SetIsVisible(scoreTextID, false);
            if (timeTextID != 0) Text.SetIsVisible(timeTextID, false);

            cachedScore = "";
            cachedTime = "";
            buttonsReady = false;
            statsShown = false;
            isAnimatingScore = false;
            displayedScore = 0.0f;
            targetScore = 0;
            scoreAnimElapsed = 0.0f;
            scoreStartDelayTimer = 0.0f;
            scoreDelayDone = false;

            Event.Subscribe(EVENT_SHOW_SCORE, OnScoreReceived);
            Event.Subscribe(EVENT_SHOW_TIME, OnTimeReceived);
            Event.Subscribe(EVENT_WIN_BUTTONS, OnButtonsFaded);
            Event.Subscribe(EVENT_LOSE_BUTTONS, OnButtonsFaded);
            //Event.Subscribe(EVENT_GAMEOVER, OnGameReset);

            LogMessage("[EndScreenStats] Initialized");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!isAnimatingScore) return;

            // Wait 1 second before counting up
            if (!scoreDelayDone)
            {
                scoreStartDelayTimer += deltaTime;
                if (scoreStartDelayTimer >= scoreStartDelay)
                    scoreDelayDone = true;
                return;
            }

            scoreAnimElapsed += deltaTime;
            float t = scoreAnimElapsed / SCORE_ANIM_DURATION;

            if (t >= 1.0f)
            {
                t = 1.0f;
                isAnimatingScore = false;
            }

            // Ease out: fast at start, slows near the end
            float eased = 1.0f - (1.0f - t) * (1.0f - t);
            displayedScore = eased * targetScore;
            int current = (int)displayedScore;

            if (scoreTextID != 0)
                SetText(scoreTextID,current.ToString("D7"));
        }

        private void OnGameReset(string eventName, string payload)
        {
            cachedScore = "";
            cachedTime = "";
            buttonsReady = false;
            statsShown = false;
            isAnimatingScore = false;
            displayedScore = 0.0f;
            targetScore = 0;
            scoreAnimElapsed = 0.0f;
            scoreStartDelayTimer = 0.0f;
            scoreDelayDone = false;

            if (scoreTextID != 0) Text.SetIsVisible(scoreTextID, false);
            if (timeTextID != 0) Text.SetIsVisible(timeTextID, false);

            LogMessage("[EndScreenStats] State reset - triggered by: " + eventName);
        }

        private void OnScoreReceived(string eventName, string payload)
        {
            LogMessage("[EndScreenStats] Score received: " + payload);

            if (!int.TryParse(payload, out int scoreVal))
            {
                LogMessage("[EndScreenStats] Failed to parse score: " + payload);
                return;
            }

            cachedScore = scoreVal.ToString();
            LogMessage("[EndScreenStats] Cached score: " + cachedScore);
            TryShow();
        }

        private void OnTimeReceived(string eventName, string payload)
        {
            LogMessage("[EndScreenStats] Time received: " + payload);

            float timeSurvived = 0.0f;
            float.TryParse(payload, out timeSurvived);

            int minutes = (int)(timeSurvived / 60);
            int seconds = (int)(timeSurvived % 60);
            cachedTime = string.Format("{0:00} m : {1:00} s", minutes, seconds);
            LogMessage("[EndScreenStats] Cached time: " + cachedTime);
            TryShow();
        }

        private void OnButtonsFaded(string eventName, string payload)
        {
            LogMessage("[EndScreenStats] Buttons faded (" + eventName + ") - ready to show stats");
            buttonsReady = true;
            TryShow();
        }

        private void TryShow()
        {
            if (statsShown) return;
            if (!buttonsReady || cachedScore == "") return;

            statsShown = true;

            int.TryParse(cachedScore, out targetScore);
            displayedScore = 0.0f;
            scoreAnimElapsed = 0.0f;
            scoreStartDelayTimer = 0.0f;
            scoreDelayDone = false;
            isAnimatingScore = true;

            if (scoreTextID != 0)
            {
                SetText(scoreTextID, "0000000");
                Text.SetIsVisible(scoreTextID, true);
                LogMessage("[EndScreenStats] Score animation starting - target: " + targetScore);
            }

            if (timeTextID != 0 && cachedTime != "")
            {
                SetText(timeTextID, cachedTime);
                Text.SetIsVisible(timeTextID, true);
                LogMessage("[EndScreenStats] Time displayed: " + cachedTime);
            }
            else if (timeTextID != 0)
            {
                SetText(timeTextID, "N/A");
                Text.SetIsVisible(timeTextID, true);
                LogMessage("[EndScreenStats] No time data - showing N/A");
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_SHOW_SCORE, OnScoreReceived);
            Event.Unsubscribe(EVENT_SHOW_TIME, OnTimeReceived);
            Event.Unsubscribe(EVENT_WIN_BUTTONS, OnButtonsFaded);
            Event.Unsubscribe(EVENT_LOSE_BUTTONS, OnButtonsFaded);
            //Event.Unsubscribe(EVENT_GAMEOVER, OnGameReset);
            LogMessage("=== EndScreenStats Destroyed ===");
        }
    }
}

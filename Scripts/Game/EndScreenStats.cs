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

            Event.Subscribe(EVENT_SHOW_SCORE, OnScoreReceived);
            Event.Subscribe(EVENT_SHOW_TIME, OnTimeReceived);
            Event.Subscribe(EVENT_WIN_BUTTONS, OnButtonsFaded);
            Event.Subscribe(EVENT_LOSE_BUTTONS, OnButtonsFaded);
            //Event.Subscribe(EVENT_GAMEOVER, OnGameReset);

            LogMessage("[EndScreenStats] Initialized");
        }

        private void OnGameReset(string eventName, string payload)
        {
            cachedScore = "";
            cachedTime = "";
            buttonsReady = false;
            statsShown = false;

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

            cachedScore = "SCORE " + scoreVal.ToString("D7");
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

            if (scoreTextID != 0)
            {
                SetText(scoreTextID, cachedScore);
                Text.SetIsVisible(scoreTextID, true);
                LogMessage("[EndScreenStats] Score displayed: " + cachedScore);
            }

            if (timeTextID != 0 && cachedTime != "")
            {
                SetText(timeTextID, cachedTime);
                Text.SetIsVisible(timeTextID, true);
                LogMessage("[EndScreenStats] Time displayed: " + cachedTime);
            }
            else if (timeTextID != 0)
            {
            cachedTime = "N/A";
                SetText(timeTextID, cachedTime);
                Text.SetIsVisible(timeTextID, true);
                LogMessage("[EndScreenStats] No time data - time text stays hidden");
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

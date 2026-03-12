using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;
using static Engine.Scene;

namespace Game
{
    public class EndScreenStats : ScriptBehaviour
    {
        [SerializeField] private string scoreTextEntityName = "UI_EndScore";
        [SerializeField] private string timeTextEntityName = "UI_EndTimeSurvived";

        private const string EVENT_SHOW_SCORE = "ShowFinalScore";
        private const string EVENT_SHOW_TIME = "ShowTimeSurvived";
        private const string EVENT_BUTTONS_DONE = "WinButtonsFaded";
        private const string EVENT_GAMEOVER = "GameOver";
        private const string EVENT_GAMEWIN = "GameWin";

        private uint scoreTextID = 0;
        private uint timeTextID = 0;

        private string cachedScore = "";
        private string cachedTime = "";
        private bool buttonsReady = false;

        public override void OnStart()
        {
            LogMessage("=== EndScreenStats OnStart ===");

            scoreTextID = SceneFindEntityByName(scoreTextEntityName);
            timeTextID = SceneFindEntityByName(timeTextEntityName);

            if (scoreTextID == 0) LogMessage("[EndScreenStats] Could not find: " + scoreTextEntityName);
            if (timeTextID == 0) LogMessage("[EndScreenStats] Could not find: " + timeTextEntityName);

            if (scoreTextID != 0) SetIsVisible(scoreTextID, false);
            if (timeTextID != 0) SetIsVisible(timeTextID, false);

            // ===== Reset state on every start =====
            cachedScore = "";
            cachedTime = "";
            buttonsReady = false;

            Event.Subscribe(EVENT_SHOW_SCORE, OnScoreReceived);
            Event.Subscribe(EVENT_SHOW_TIME, OnTimeReceived);
            Event.Subscribe(EVENT_BUTTONS_DONE, OnButtonsFaded);
            Event.Subscribe(EVENT_GAMEOVER, OnGameReset);
            Event.Subscribe(EVENT_GAMEWIN, OnGameReset);

            LogMessage("[EndScreenStats] Initialized");
        }

        private void OnGameReset(string eventName, string payload)
        {
            // Reset every time a new game over/win starts
            // so stats don't show instantly on repeated loses
            cachedScore = "";
            cachedTime = "";
            buttonsReady = false;

            if (scoreTextID != 0) SetIsVisible(scoreTextID, false);
            if (timeTextID != 0) SetIsVisible(timeTextID, false);

            LogMessage("[EndScreenStats] State reset for new end screen");
        }

        private void OnScoreReceived(string eventName, string payload)
        {
            LogMessage("[EndScreenStats] Score received: " + payload);
            cachedScore = "SCORE " + int.Parse(payload).ToString("D7");
            TryShow();
        }

        private void OnTimeReceived(string eventName, string payload)
        {
            LogMessage("[EndScreenStats] Time received: " + payload);
            float timeSurvived = 0.0f;
            float.TryParse(payload, out timeSurvived);
            int minutes = (int)(timeSurvived / 60);
            int seconds = (int)(timeSurvived % 60);
            cachedTime = string.Format("TIME {0:00} m : {1:00} s", minutes, seconds);
            TryShow();
        }

        private void OnButtonsFaded(string eventName, string payload)
        {
            LogMessage("[EndScreenStats] Buttons faded - ready to show stats");
            buttonsReady = true;
            TryShow();
        }

        private void TryShow()
        {
            if (!buttonsReady || cachedScore == "") return;

            if (scoreTextID != 0)
            {
                SetText(scoreTextID, cachedScore);
                SetIsVisible(scoreTextID, true);
                LogMessage("[EndScreenStats] Score displayed: " + cachedScore);
            }

            if (timeTextID != 0 && cachedTime != "")
            {
                SetText(timeTextID, cachedTime);
                SetIsVisible(timeTextID, true);
                LogMessage("[EndScreenStats] Time displayed: " + cachedTime);
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_SHOW_SCORE, OnScoreReceived);
            Event.Unsubscribe(EVENT_SHOW_TIME, OnTimeReceived);
            Event.Unsubscribe(EVENT_BUTTONS_DONE, OnButtonsFaded);
            Event.Unsubscribe(EVENT_GAMEOVER, OnGameReset);
            Event.Unsubscribe(EVENT_GAMEWIN, OnGameReset);
            LogMessage("=== EndScreenStats Destroyed ===");
        }
    }
}

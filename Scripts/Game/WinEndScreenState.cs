// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;
using static Engine.Scene;

namespace Game
{
    /// <summary>
    /// Displays final score and time on the win popup inside the cutscene scene.
    /// Reads data from WinCutSceneContext (set by WinScreen2 before scene load).
    /// Separate from EndScreenStats which handles the loss screen in game scenes.
    ///
    /// Place this script in WinCutScene.json only.
    /// Listens for "WinScreenShow" published by WinCutScenePopUp.OnCutsceneComplete().
    ///
    /// Entity names expected in the cutscene scene:
    ///   UI_WinScore         – score text entity
    ///   UI_WinTimeSurvived  – time text entity
    /// </summary>
    public class WinEndScreenState : ScriptBehaviour
    {
        [SerializeField] private string scoreTextEntityName = "UI_WinScore";
        [SerializeField] private string timeTextEntityName  = "UI_WinTimeSurvived";

        // Published by WinCutScenePopUp.OnCutsceneComplete()
        private const string EVENT_WIN_SHOW = "WinScreenShow";

        private uint scoreTextID = 0;
        private uint timeTextID  = 0;

        private bool statsShown = false;

        // Score count-up animation
        private bool  isAnimatingScore     = false;
        private float displayedScore       = 0.0f;
        private int   targetScore          = 0;
        private const float SCORE_ANIM_DURATION = 2.0f;
        private float scoreAnimElapsed     = 0.0f;

        // Delay before count-up starts
        private const float SCORE_START_DELAY  = 1.0f;
        private float scoreStartDelayTimer = 0.0f;
        private bool  scoreDelayDone       = false;

        public override void OnStart()
        {
            LogMessage("=== WinEndScreenStats OnStart ===");

            scoreTextID = SceneFindEntityByName(scoreTextEntityName);
            timeTextID  = SceneFindEntityByName(timeTextEntityName);

            if (scoreTextID == 0) LogMessage("[WinEndScreenStats] Could not find: " + scoreTextEntityName);
            if (timeTextID  == 0) LogMessage("[WinEndScreenStats] Could not find: " + timeTextEntityName);

            // Hide both until the popup is visible
            if (scoreTextID != 0) Text.SetIsVisible(scoreTextID, false);
            if (timeTextID  != 0) Text.SetIsVisible(timeTextID,  false);

            statsShown           = false;
            isAnimatingScore     = false;
            displayedScore       = 0.0f;
            targetScore          = 0;
            scoreAnimElapsed     = 0.0f;
            scoreStartDelayTimer = 0.0f;
            scoreDelayDone       = false;

            Event.Subscribe(EVENT_WIN_SHOW, OnWinPopupShown);

            LogMessage("[WinEndScreenStats] Initialized – Score=" + WinCutSceneContext.FinalScore
                     + " Time=" + WinCutSceneContext.FinalTime);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!isAnimatingScore) return;

            // Wait before counting up
            if (!scoreDelayDone)
            {
                scoreStartDelayTimer += deltaTime;
                if (scoreStartDelayTimer >= SCORE_START_DELAY)
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

            // Ease out: fast at start, slows near end
            float eased = 1.0f - (1.0f - t) * (1.0f - t);
            displayedScore = eased * targetScore;

            if (scoreTextID != 0)
                SetText(scoreTextID, ((int)displayedScore).ToString("D7"));
        }

        // Called when WinCutScenePopUp shows the popup
        private void OnWinPopupShown(string eventName, string payload)
        {
            if (statsShown) return;
            statsShown = true;

            LogMessage("[WinEndScreenStats] Popup visible – showing stats");

            // Score
            if (!int.TryParse(WinCutSceneContext.FinalScore, out targetScore))
                targetScore = 0;

            displayedScore       = 0.0f;
            scoreAnimElapsed     = 0.0f;
            scoreStartDelayTimer = 0.0f;
            scoreDelayDone       = false;
            isAnimatingScore     = true;

            if (scoreTextID != 0)
            {
                SetText(scoreTextID, "0000000");
                Text.SetIsVisible(scoreTextID, true);
                LogMessage("[WinEndScreenStats] Score animation starting – target: " + targetScore);
            }

            // Time
            if (timeTextID != 0)
            {
                string timeDisplay = WinCutSceneContext.FinalTime != ""
                    ? WinCutSceneContext.FinalTime
                    : "N/A";
                SetText(timeTextID, timeDisplay);
                Text.SetIsVisible(timeTextID, true);
                LogMessage("[WinEndScreenStats] Time displayed: " + timeDisplay);
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_WIN_SHOW, OnWinPopupShown);
            LogMessage("=== WinEndScreenStats Destroyed ===");
        }
    }
}
// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using System.Collections.Generic;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Text;

namespace Game
{
    public class HighScorePopup : ScriptBehaviour
    {
        // Entity names to find
        private const string HIGHSCORE_BUTTON_NAME = "HighScore Button";
        private const string POPUP_NAME = "HighScore Popup";
        private const string CLOSE_BUTTON_NAME = "HighScore Close Button";
        
        // Text entity names for displaying scores
        private const string LEVEL1_TEXT_PREFIX = "Level1_Score_"; // Level1_Score_1, Level1_Score_2, etc.
        private const string LEVEL2_TEXT_PREFIX = "Level2_Score_"; // Level2_Score_1, Level2_Score_2, etc.

        // Positions
        private const float HIDDEN_Y = -500.0f;
        private const float VISIBLE_Y = 360.0f;
        private const float CENTER_X = 640.0f;

        // Absolute button position when visible
        private const float CLOSE_BUTTON_X = 1196.0f;
        private const float CLOSE_BUTTON_Y = 239.0f;

        // Score data files
        private const string SCORE_FILE_1 = "Resources/Sources/SaveData/ScoreData1.json";
        private const string SCORE_FILE_2 = "Resources/Sources/SaveData/ScoreData2.json";
        private const int MAX_SCORES_DISPLAY = 10;

        // Entity IDs
        private uint highscoreButtonId;
        private uint popupId;
        private uint closeButtonId;

        // Text entity IDs for displaying scores
        private uint[] level1TextIds = new uint[MAX_SCORES_DISPLAY];
        private uint[] level2TextIds = new uint[MAX_SCORES_DISPLAY];

        // Event names for popup coordination
        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID = "HighScore";

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("HighScorePopup: Initializing...");

            highscoreButtonId = SceneFindEntityByName(HIGHSCORE_BUTTON_NAME);
            popupId = SceneFindEntityByName(POPUP_NAME);
            closeButtonId = SceneFindEntityByName(CLOSE_BUTTON_NAME);

            if (highscoreButtonId == 0)
            {
                LogError("HighScorePopup: Could not find entity: " + HIGHSCORE_BUTTON_NAME);
                return;
            }
            if (popupId == 0)
            {
                LogError("HighScorePopup: Could not find entity: " + POPUP_NAME);
                return;
            }
            if (closeButtonId == 0)
            {
                LogError("HighScorePopup: Could not find entity: " + CLOSE_BUTTON_NAME);
                return;
            }

            // Find all score text entities
            FindScoreTextEntities();

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;

            // Subscribe to popup coordination events
            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);

            LogMessage("HighScorePopup: All entities found, ready!");
        }

        private void FindScoreTextEntities()
        {
            // Find Level 1 score text entities
            for (int i = 0; i < MAX_SCORES_DISPLAY; i++)
            {
                string entityName = LEVEL1_TEXT_PREFIX + (i + 1).ToString();
                level1TextIds[i] = SceneFindEntityByName(entityName);
                if (level1TextIds[i] == 0)
                    LogMessage("HighScorePopup: Optional text entity not found: " + entityName);
            }

            // Find Level 2 score text entities
            for (int i = 0; i < MAX_SCORES_DISPLAY; i++)
            {
                string entityName = LEVEL2_TEXT_PREFIX + (i + 1).ToString();
                level2TextIds[i] = SceneFindEntityByName(entityName);
                if (level2TextIds[i] == 0)
                    LogMessage("HighScorePopup: Optional text entity not found: " + entityName);
            }
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed)
            {
                HandleMouseClick();
            }
        }

        private void HandleMouseClick()
        {
            if (isPopupVisible)
            {
                bool closeHit = Collision2D.IsMouseCollidingWithEntity(closeButtonId);

                if (closeHit)
                {
                    LogMessage("HighScorePopup: Close button clicked - hiding popup");
                    HidePopup();
                }
            }
            else
            {
                bool highscoreHit = Collision2D.IsMouseCollidingWithEntity(highscoreButtonId);

                if (highscoreHit)
                {
                    LogMessage("HighScorePopup: HighScore button clicked - showing popup");
                    ShowPopup();
                }
            }
        }

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUP_ID && isPopupVisible)
            {
                LogMessage("HighScorePopup: Another popup opened (" + payload + ") - closing high scores");
                HidePopup();
            }
        }

        private void ShowPopup()
        {
            isPopupVisible = true;
            Event.Publish(EVENT_POPUP_OPENED, POPUP_ID);

            Vector3 popupPos = new Vector3(CENTER_X, VISIBLE_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            Vector3 closePos = new Vector3(CLOSE_BUTTON_X, CLOSE_BUTTON_Y, -0.6f);
            SetPosition(closeButtonId, ref closePos);

            // Load and display scores
            LoadAndDisplayScores();

            LogMessage("HighScorePopup: Popup shown");
        }

        private void HidePopup()
        {
            bool wasVisible = isPopupVisible;
            isPopupVisible = false;
            if (wasVisible)
                Event.Publish(EVENT_POPUP_CLOSED, POPUP_ID);

            Vector3 popupPos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            Vector3 closePos = new Vector3(CLOSE_BUTTON_X, HIDDEN_Y, -0.6f);
            SetPosition(closeButtonId, ref closePos);

            LogMessage("HighScorePopup: Popup hidden");
        }

        // ===== SCORE LOADING AND DISPLAY =====

        private void LoadAndDisplayScores()
        {
            // Load Level 1 scores
            List<int> level1Scores = LoadScoresFromFile(SCORE_FILE_1, "Level1");
            DisplayScores(level1Scores, level1TextIds, "Level 1");

            // Load Level 2 scores
            List<int> level2Scores = LoadScoresFromFile(SCORE_FILE_2, "Level2");
            DisplayScores(level2Scores, level2TextIds, "Level 2");
        }

        private List<int> LoadScoresFromFile(string filePath, string levelKey)
        {
            var scores = new List<int>();

            try
            {
                if (!FileIO.FileExists(filePath))
                {
                    LogMessage("HighScorePopup: No score file found at: " + filePath);
                    return scores;
                }

                string json = FileIO.ReadAllText(filePath);
                Dictionary<string, List<int>> allScores = ParseScoreJson(json);

                if (allScores.ContainsKey(levelKey))
                {
                    scores = allScores[levelKey];
                    LogMessage("HighScorePopup: Loaded " + scores.Count + " scores for " + levelKey);
                }
                else
                {
                    LogMessage("HighScorePopup: No scores found for " + levelKey + " in file");
                }
            }
            catch (Exception e)
            {
                LogError("HighScorePopup: Error loading scores from " + filePath + ": " + e.Message);
            }

            return scores;
        }

        private Dictionary<string, List<int>> ParseScoreJson(string json)
        {
            var result = new Dictionary<string, List<int>>();

            try
            {
                json = json.Trim().TrimStart('{').TrimEnd('}');

                // Split on "]," to separate level entries
                string[] entries = json.Split(new string[] { "]," }, StringSplitOptions.RemoveEmptyEntries);

                foreach (string entry in entries)
                {
                    string cleaned = entry.Trim().TrimEnd(']');
                    string[] parts = cleaned.Split(':');
                    if (parts.Length != 2) continue;

                    string key = parts[0].Trim().Trim('"');
                    string valuesRaw = parts[1].Trim().TrimStart('[');

                    var scores = new List<int>();
                    foreach (string v in valuesRaw.Split(','))
                    {
                        if (int.TryParse(v.Trim(), out int parsed))
                            scores.Add(parsed);
                    }

                    result[key] = scores;
                }
            }
            catch (Exception e)
            {
                LogError("HighScorePopup: JSON parse error: " + e.Message);
            }

            return result;
        }

        private void DisplayScores(List<int> scores, uint[] textEntityIds, string levelName)
        {
            for (int i = 0; i < MAX_SCORES_DISPLAY; i++)
            {
                if (textEntityIds[i] == 0)
                    continue;

                string displayText;
                if (i < scores.Count)
                {
                    // Format: "1. 0005000" (rank + 7-digit zero-padded score)
                    displayText = (i + 1).ToString() + ". " + scores[i].ToString("D7");
                }
                else
                {
                    // No score at this rank - show empty
                    displayText = (i + 1).ToString() + ". -------";
                }

                SetText(textEntityIds[i], displayText);
            }

            LogMessage("HighScorePopup: Displayed " + scores.Count + " scores for " + levelName);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("HighScorePopup: Destroyed");
        }
    }
}

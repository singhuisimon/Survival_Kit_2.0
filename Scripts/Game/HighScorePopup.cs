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
using static Engine.SpriteRenderer;

namespace Game
{
    public class HighScorePopup : ScriptBehaviour
    {
        // Entity names to find
        private const string HIGHSCORE_BUTTON_NAME = "Highscore Button";
        private const string POPUP_NAME = "HighScore Popup";
        private const string CLOSE_BUTTON_NAME = "HighScore Close Button";

        // Text entity names for displaying scores
        private const string LEVEL1_TEXT_PREFIX = "Level1_Score_"; // Level1_Score_1, Level1_Score_2, etc.
        private const string LEVEL2_TEXT_PREFIX = "Level2_Score_"; // Level2_Score_1, Level2_Score_2, etc.

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

            // Hide popup and close button initially
            SpriteRenderer.SetIsVisible(popupId, false);
            SpriteRenderer.SetIsVisible(closeButtonId, false);

            // Hide all score text entities initially
            HideAllScoreText();

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

            // Show popup and close button
            SpriteRenderer.SetIsVisible(popupId, true);
            SpriteRenderer.SetIsVisible(closeButtonId, true);

            // Show all score text entities
            ShowAllScoreText();

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

            // Hide popup and close button
            SpriteRenderer.SetIsVisible(popupId, false);
            SpriteRenderer.SetIsVisible(closeButtonId, false);

            // Hide all score text entities
            HideAllScoreText();

            LogMessage("HighScorePopup: Popup hidden");
        }

        // ===== VISIBILITY HELPERS =====

        private void ShowAllScoreText()
        {
            for (int i = 0; i < MAX_SCORES_DISPLAY; i++)
            {
                if (level1TextIds[i] != 0)
                    Text.SetIsVisible(level1TextIds[i], true);
                if (level2TextIds[i] != 0)
                    Text.SetIsVisible(level2TextIds[i], true);
            }
        }

        private void HideAllScoreText()
        {
            for (int i = 0; i < MAX_SCORES_DISPLAY; i++)
            {
                if (level1TextIds[i] != 0)
                    Text.SetIsVisible(level1TextIds[i], false);
                if (level2TextIds[i] != 0)
                    Text.SetIsVisible(level2TextIds[i], false);
            }
        }

        // ===== SCORE LOADING AND DISPLAY =====

        private void LoadAndDisplayScores()
        {
            LogMessage("=== LoadAndDisplayScores START ===");

            // Load Level 1 scores
            LogMessage("Loading Level 1 scores from: " + SCORE_FILE_1);
            List<ScoreEntry> level1Scores = LoadScoresFromFile(SCORE_FILE_1, "trench_run");
            LogMessage("Level 1 scores loaded: " + level1Scores.Count);
            DisplayScores(level1Scores, level1TextIds, "Level 1");

            // Load Level 2 scores
            LogMessage("Loading Level 2 scores from: " + SCORE_FILE_2);
            List<ScoreEntry> level2Scores = LoadScoresFromFile(SCORE_FILE_2, "level2");
            LogMessage("Level 2 scores loaded: " + level2Scores.Count);
            DisplayScores(level2Scores, level2TextIds, "Level 2");

            LogMessage("=== LoadAndDisplayScores END ===");
        }

        private List<ScoreEntry> LoadScoresFromFile(string filePath, string levelKey)
        {
            var scores = new List<ScoreEntry>();

            try
            {
                if (!FileIO.FileExists(filePath))
                {
                    LogMessage("HighScorePopup: No score file found at: " + filePath);
                    return scores;
                }

                string json = FileIO.ReadAllText(filePath);
                Dictionary<string, List<ScoreEntry>> allScores = ParseScoreJson(json);

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

        private Dictionary<string, List<ScoreEntry>> ParseScoreJson(string json)
        {
            var result = new Dictionary<string, List<ScoreEntry>>();

            try
            {
                LogMessage("=== ParseScoreJson START ===");

                // Remove all newlines and extra whitespace for easier parsing
                json = json.Replace("\n", "").Replace("\r", "").Replace("  ", " ");
                LogMessage("Cleaned JSON: " + json);

                json = json.Trim().TrimStart('{').TrimEnd('}').Trim();

                // Find the level key
                int levelKeyStart = json.IndexOf('"');
                int levelKeyEnd = json.IndexOf('"', levelKeyStart + 1);
                if (levelKeyStart == -1 || levelKeyEnd == -1)
                {
                    LogError("Could not find level key");
                    return result;
                }

                string levelKey = json.Substring(levelKeyStart + 1, levelKeyEnd - levelKeyStart - 1);
                LogMessage("Found level: " + levelKey);

                // Find the array start and end
                int arrayStart = json.IndexOf('[', levelKeyEnd);
                int arrayEnd = json.LastIndexOf(']');

                if (arrayStart == -1 || arrayEnd == -1)
                {
                    LogError("Could not find array boundaries");
                    return result;
                }

                string arrayContent = json.Substring(arrayStart + 1, arrayEnd - arrayStart - 1).Trim();
                LogMessage("Array content: " + arrayContent);

                var scores = new List<ScoreEntry>();

                // Parse objects one by one
                int pos = 0;
                int objectCount = 0;

                while (pos < arrayContent.Length)
                {
                    // Find next object start
                    int objStart = arrayContent.IndexOf('{', pos);
                    if (objStart == -1) break;

                    // Find matching closing brace
                    int objEnd = arrayContent.IndexOf('}', objStart);
                    if (objEnd == -1) break;

                    objectCount++;
                    string objContent = arrayContent.Substring(objStart + 1, objEnd - objStart - 1);
                    LogMessage("Object " + objectCount + ": " + objContent);

                    // Parse score
                    int scoreVal = 0;
                    int scoreKeyPos = objContent.IndexOf("\"score\"");
                    if (scoreKeyPos >= 0)
                    {
                        int colonPos = objContent.IndexOf(':', scoreKeyPos);
                        int commaPos = objContent.IndexOf(',', colonPos);
                        if (commaPos == -1) commaPos = objContent.Length;

                        string scoreStr = objContent.Substring(colonPos + 1, commaPos - colonPos - 1).Trim();
                        if (int.TryParse(scoreStr, out scoreVal))
                        {
                            LogMessage("  Score: " + scoreVal);
                        }
                    }

                    // Parse date
                    string dateVal = "";
                    int dateKeyPos = objContent.IndexOf("\"date\"");
                    if (dateKeyPos >= 0)
                    {
                        int colonPos = objContent.IndexOf(':', dateKeyPos);
                        int quoteStart = objContent.IndexOf('"', colonPos);
                        int quoteEnd = objContent.IndexOf('"', quoteStart + 1);

                        if (quoteStart >= 0 && quoteEnd >= 0)
                        {
                            dateVal = objContent.Substring(quoteStart + 1, quoteEnd - quoteStart - 1);
                            LogMessage("  Date: " + dateVal);
                        }
                    }

                    if (scoreVal > 0)
                    {
                        scores.Add(new ScoreEntry { score = scoreVal, date = dateVal });
                        LogMessage("  Added to list!");
                    }

                    // Move past this object
                    pos = objEnd + 1;
                }

                result[levelKey] = scores;
                LogMessage("Total parsed: " + scores.Count + " scores");
                LogMessage("=== ParseScoreJson END ===");
            }
            catch (Exception e)
            {
                LogError("HighScorePopup: JSON parse error: " + e.Message);
            }

            return result;
        }

        private void DisplayScores(List<ScoreEntry> scores, uint[] textEntityIds, string levelName)
        {
            LogMessage("=== DisplayScores for " + levelName + " ===");
            LogMessage("Scores count: " + scores.Count);

            // Safety: ensure we only display top 10
            int maxToDisplay = scores.Count > MAX_SCORES_DISPLAY ? MAX_SCORES_DISPLAY : scores.Count;

            for (int i = 0; i < MAX_SCORES_DISPLAY; i++)
            {
                if (textEntityIds[i] == 0)
                {
                    LogMessage("Text entity " + i + " is null, skipping");
                    continue;
                }

                string displayText;
                if (i < maxToDisplay)
                {
                    // Format: "1. 0005000    2025-02-23 14:30:45"
                    displayText = (i + 1).ToString() + ". " +
                                  scores[i].score.ToString("D7") +
                                  "    " +
                                  scores[i].date;
                    LogMessage("Setting text " + i + ": " + displayText);
                }
                else
                {
                    // No score at this rank - show empty
                    displayText = (i + 1).ToString() + ". -------    ----/--/-- --:--:--";
                }

                SetText(textEntityIds[i], displayText);
            }

            LogMessage("HighScorePopup: Displayed " + maxToDisplay + " scores for " + levelName);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("HighScorePopup: Destroyed");
        }

        // ===== HELPER CLASS =====
        private class ScoreEntry
        {
            public int score;
            public string date;
        }
    }
}
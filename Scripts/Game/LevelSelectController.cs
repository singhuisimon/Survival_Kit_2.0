using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// Controls level selection buttons and popup in the main menu.
    /// - Level 1 button always visible, Level 2 button visible after completing Level 1.
    /// - Popup defaults to Level 1. Clicking Level 2 button swaps popup to Level 2.
    /// - Clicking Level 1 button swaps popup back to Level 1.
    /// - Buttons do NOT load scenes directly; they only swap the popup.
    /// </summary>
    public class LevelSelectController : ScriptBehaviour
    {
        // Entity names
        private const string LEVEL1_BUTTON_NAME = "Level1_Button";
        private const string LEVEL2_BUTTON_NAME = "Level2_Button";
        private const string LEVEL1_BUTTON_SELECTED_NAME = "Level1_Button_Selected";
        private const string LEVEL2_BUTTON_SELECTED_NAME = "Level2_Button_Selected";
        private const string LEVEL_SELECTION_POPUP_1_NAME = "LevelSelectionPopup_Level1";
        private const string LEVEL_SELECTION_POPUP_2_NAME = "LevelSelectionPopup_Level2";

        // Entity IDs
        private uint level1ButtonId;
        private uint level2ButtonId;
        private uint level1ButtonSelectedId;
        private uint level2ButtonSelectedId;
        private uint levelSelectionPopup1Id;
        private uint levelSelectionPopup2Id;

        // State
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool level2Unlocked = false;
        public static bool IsLevel2Selected { get; private set; } = false;

        public override void OnStart()
        {
            LogMessage("LevelSelectController: Initializing...");

            // Load progress to check win state
            ProgressTracker.LoadProgress();
            level2Unlocked = ProgressTracker.HasWonTrenchRun;
            LogMessage("LevelSelectController: Level 2 unlocked = " + level2Unlocked);

            // Find button entities
            level1ButtonId = SceneFindEntityByName(LEVEL1_BUTTON_NAME);
            level2ButtonId = SceneFindEntityByName(LEVEL2_BUTTON_NAME);
            level1ButtonSelectedId = SceneFindEntityByName(LEVEL1_BUTTON_SELECTED_NAME);
            level2ButtonSelectedId = SceneFindEntityByName(LEVEL2_BUTTON_SELECTED_NAME);
            levelSelectionPopup1Id = SceneFindEntityByName(LEVEL_SELECTION_POPUP_1_NAME);
            levelSelectionPopup2Id = SceneFindEntityByName(LEVEL_SELECTION_POPUP_2_NAME);

            if (level1ButtonId == 0)
                LogMessage("LevelSelectController: Level 1 button not found");
            if (level2ButtonId == 0)
                LogMessage("LevelSelectController: Level 2 button not found");

            entitiesFound = true;
            wasMousePressed = false;
            IsLevel2Selected = false;

            // Ensure selected variants are visible so SetColor can control them
            if (level1ButtonSelectedId != 0)
            {
                SetIsVisible(level1ButtonSelectedId, true);
                SetColor(level1ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
            }

            // Hide Level 2 button and its selected variant if not unlocked
            if (!level2Unlocked)
            {
                if (level2ButtonId != 0)
                    SetIsVisible(level2ButtonId, false);
                if (level2ButtonSelectedId != 0)
                    SetIsVisible(level2ButtonSelectedId, false);
                LogMessage("LevelSelectController: Level 2 button hidden (not unlocked)");
            }
            else
            {
                if (level2ButtonId != 0)
                    SetIsVisible(level2ButtonId, true);
                if (level2ButtonSelectedId != 0)
                {
                    SetIsVisible(level2ButtonSelectedId, true);
                    SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                }
                LogMessage("LevelSelectController: Level 2 button visible (unlocked)");
            }

            // Default: show Level 1 popup, hide Level 2 popup
            ShowPopup(false);

            // Listen for progress reset to hide level 2 immediately
            Event.Subscribe("ProgressReset", OnProgressReset);

            LogMessage("LevelSelectController: Ready!");
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
            // Check Level 1 button click - swap popup to Level 1
            if (level1ButtonId != 0 && IsButtonClicked(level1ButtonId, level1ButtonSelectedId))
            {
                LogMessage("LevelSelectController: Level 1 button clicked - showing Level 1 popup");
                ShowPopup(false);
                return;
            }

            // Check Level 2 button click - swap popup to Level 2 (only if unlocked)
            if (level2Unlocked && level2ButtonId != 0 && IsButtonClicked(level2ButtonId, level2ButtonSelectedId))
            {
                LogMessage("LevelSelectController: Level 2 button clicked - showing Level 2 popup");
                ShowPopup(true);
                return;
            }
        }

        private void ShowPopup(bool showLevel2)
        {
            IsLevel2Selected = showLevel2;

            if (showLevel2)
            {
                // Show Level 2 popup, hide Level 1 popup
                if (levelSelectionPopup1Id != 0)
                    SetIsVisible(levelSelectionPopup1Id, false);
                if (levelSelectionPopup2Id != 0)
                    SetIsVisible(levelSelectionPopup2Id, true);
            }
            else
            {
                // Show Level 1 popup, hide Level 2 popup
                if (levelSelectionPopup1Id != 0)
                    SetIsVisible(levelSelectionPopup1Id, true);
                if (levelSelectionPopup2Id != 0)
                    SetIsVisible(levelSelectionPopup2Id, false);
            }

            // Update button visuals: show selected texture for the active level
            UpdateButtonSelection();
        }

        private void UpdateButtonSelection()
        {
            if (IsLevel2Selected)
            {
                // Level 2 selected: show Level 2 selected, Level 1 normal
                if (level1ButtonId != 0) SetColor(level1ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level1ButtonSelectedId != 0) SetColor(level1ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level2ButtonId != 0) SetColor(level2ButtonId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level2ButtonSelectedId != 0) SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f);
            }
            else
            {
                // Level 1 selected: show Level 1 selected, Level 2 normal
                if (level1ButtonId != 0) SetColor(level1ButtonId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level1ButtonSelectedId != 0) SetColor(level1ButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level2ButtonId != 0) SetColor(level2ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level2ButtonSelectedId != 0) SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
            }
        }

        private bool IsButtonClicked(uint normalId, uint hoveredId)
        {
            if (normalId != 0 && Collision2D.IsMouseCollidingWithEntity(normalId))
                return true;
            if (hoveredId != 0 && Collision2D.IsMouseCollidingWithEntity(hoveredId))
                return true;
            return false;
        }

        private void OnProgressReset(string eventName, string payload)
        {
            LogMessage("LevelSelectController: Progress reset - hiding Level 2");
            level2Unlocked = false;
            IsLevel2Selected = false;

            // Hide Level 2 button and selected variant
            if (level2ButtonId != 0)
                SetIsVisible(level2ButtonId, false);
            if (level2ButtonSelectedId != 0)
            {
                SetIsVisible(level2ButtonSelectedId, false);
                SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
            }

            // Swap popup back to Level 1
            ShowPopup(false);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe("ProgressReset", OnProgressReset);
            LogMessage("LevelSelectController: Destroyed");
        }
    }
}

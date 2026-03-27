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
    /// - Popup defaults to Level 1. Clicking buttons swaps popup accordingly.
    /// - Buttons do NOT load scenes directly; they only swap the popup.
    /// - SkipTutorial toggle available on Level 1 and Level 2 popups only.
    /// </summary>
    public class LevelSelectController : ScriptBehaviour
    {
        // Entity names - must match scene exactly
        private const string LEVEL1_BUTTON_NAME = "Level1_Button";
        private const string LEVEL2_BUTTON_NAME = "Level2_Button";
        private const string LEVEL3_BUTTON_NAME = "Level3_Button";
        private const string LEVEL1_BUTTON_SELECTED_NAME = "Level1_Button_Selected";
        private const string LEVEL2_BUTTON_SELECTED_NAME = "Level2_Button_Selected";
        private const string LEVEL3_BUTTON_SELECTED_NAME = "Level3_Button_Selected";
        private const string LEVEL_SELECTION_POPUP_1_NAME = "LevelSelectionPopup_Level1";
        private const string LEVEL_SELECTION_POPUP_2_NAME = "LevelSelectionPopup_Level2";
        private const string LEVEL_SELECTION_POPUP_3_NAME = "LevelSelectionPopup_Level3";

        // Skip tutorial entity names (Level 1 & 2 only)
        private const string SKIPTUTORIAL1_UNTICKED = "SkipTutorial1";
        private const string SKIPTUTORIAL1_TICKED = "SkipTutorial1Ticked";
        private const string SKIPTUTORIAL2_UNTICKED = "SkipTutorial2";
        private const string SKIPTUTORIAL2_TICKED = "SkipTutorial2Ticked";

        // Entity IDs - buttons
        private uint level1ButtonId;
        private uint level2ButtonId;
        private uint level3ButtonId;
        private uint level1ButtonSelectedId;
        private uint level2ButtonSelectedId;
        private uint level3ButtonSelectedId;

        // Entity IDs - popups
        private uint levelSelectionPopup1Id;
        private uint levelSelectionPopup2Id;
        private uint levelSelectionPopup3Id;

        // Entity IDs - skip tutorial toggles
        private uint skipTutorial1UntickedId;
        private uint skipTutorial1TickedId;
        private uint skipTutorial2UntickedId;
        private uint skipTutorial2TickedId;

        // State
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool level2Unlocked = false;
        private bool level3Unlocked = false;
        private bool skipLevel1 = false;
        private bool skipLevel2 = false;

        public static bool IsLevel2Selected { get; private set; } = false;
        public static bool IsLevel3Selected { get; private set; } = false;

        public override void OnStart()
        {
            LogMessage("LevelSelectController: Initializing...");

            ProgressTracker.LoadProgress();
            level2Unlocked = ProgressTracker.HasWonTrenchRun;
            level3Unlocked = ProgressTracker.HasWonLevel2;
            skipLevel1 = ProgressTracker.SkipTutorialLevel1;
            skipLevel2 = ProgressTracker.SkipTutorialLevel2;

            LogMessage("LevelSelectController: Level 2 unlocked = " + level2Unlocked);
            LogMessage("LevelSelectController: Level 3 unlocked = " + level3Unlocked);
            LogMessage("LevelSelectController: SkipLevel1=" + skipLevel1 + " SkipLevel2=" + skipLevel2);

            // Find button entities
            level1ButtonId = SceneFindEntityByName(LEVEL1_BUTTON_NAME);
            level2ButtonId = SceneFindEntityByName(LEVEL2_BUTTON_NAME);
            level3ButtonId = SceneFindEntityByName(LEVEL3_BUTTON_NAME);
            level1ButtonSelectedId = SceneFindEntityByName(LEVEL1_BUTTON_SELECTED_NAME);
            level2ButtonSelectedId = SceneFindEntityByName(LEVEL2_BUTTON_SELECTED_NAME);
            level3ButtonSelectedId = SceneFindEntityByName(LEVEL3_BUTTON_SELECTED_NAME);

            // Find popup entities
            levelSelectionPopup1Id = SceneFindEntityByName(LEVEL_SELECTION_POPUP_1_NAME);
            levelSelectionPopup2Id = SceneFindEntityByName(LEVEL_SELECTION_POPUP_2_NAME);
            levelSelectionPopup3Id = SceneFindEntityByName(LEVEL_SELECTION_POPUP_3_NAME);

            // Find skip tutorial entities
            skipTutorial1UntickedId = SceneFindEntityByName(SKIPTUTORIAL1_UNTICKED);
            skipTutorial1TickedId = SceneFindEntityByName(SKIPTUTORIAL1_TICKED);
            skipTutorial2UntickedId = SceneFindEntityByName(SKIPTUTORIAL2_UNTICKED);
            skipTutorial2TickedId = SceneFindEntityByName(SKIPTUTORIAL2_TICKED);

            if (level1ButtonId == 0) LogMessage("LevelSelectController: Level 1 button not found");
            if (level2ButtonId == 0) LogMessage("LevelSelectController: Level 2 button not found");
            if (level3ButtonId == 0) LogMessage("LevelSelectController: Level 3 button not found");
            if (skipTutorial1UntickedId == 0) LogMessage("LevelSelectController: " + SKIPTUTORIAL1_UNTICKED + " not found");
            if (skipTutorial1TickedId == 0) LogMessage("LevelSelectController: " + SKIPTUTORIAL1_TICKED + " not found");
            if (skipTutorial2UntickedId == 0) LogMessage("LevelSelectController: " + SKIPTUTORIAL2_UNTICKED + " not found");
            if (skipTutorial2TickedId == 0) LogMessage("LevelSelectController: " + SKIPTUTORIAL2_TICKED + " not found");

            entitiesFound = true;
            wasMousePressed = false;
            IsLevel2Selected = false;
            IsLevel3Selected = false;

            // Ensure selected variants are visible so SetColor can control them
            if (level1ButtonSelectedId != 0)
            {
                SetIsVisible(level1ButtonSelectedId, true);
                SetColor(level1ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
            }

            // Hide Level 2 button if not unlocked
            if (!level2Unlocked)
            {
                if (level2ButtonId != 0) SetIsVisible(level2ButtonId, false);
                if (level2ButtonSelectedId != 0) SetIsVisible(level2ButtonSelectedId, false);
                LogMessage("LevelSelectController: Level 2 button hidden (not unlocked)");
            }
            else
            {
                if (level2ButtonId != 0) SetIsVisible(level2ButtonId, true);
                if (level2ButtonSelectedId != 0)
                {
                    SetIsVisible(level2ButtonSelectedId, true);
                    SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                }
                LogMessage("LevelSelectController: Level 2 button visible (unlocked)");
            }

            // Hide Level 3 button if not unlocked
            if (!level3Unlocked)
            {
                if (level3ButtonId != 0) SetIsVisible(level3ButtonId, false);
                if (level3ButtonSelectedId != 0) SetIsVisible(level3ButtonSelectedId, false);
                LogMessage("LevelSelectController: Level 3 button hidden (not unlocked)");
            }
            else
            {
                if (level3ButtonId != 0) SetIsVisible(level3ButtonId, true);
                if (level3ButtonSelectedId != 0)
                {
                    SetIsVisible(level3ButtonSelectedId, true);
                    SetColor(level3ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                }
                LogMessage("LevelSelectController: Level 3 button visible (unlocked)");
            }

            // Pre-activate skip tutorial entities so SetIsVisible works
            if (skipTutorial1UntickedId != 0) SetIsVisible(skipTutorial1UntickedId, true);
            if (skipTutorial1TickedId != 0) SetIsVisible(skipTutorial1TickedId, true);
            if (skipTutorial2UntickedId != 0) SetIsVisible(skipTutorial2UntickedId, true);
            if (skipTutorial2TickedId != 0) SetIsVisible(skipTutorial2TickedId, true);

            Event.Subscribe("ProgressReset", OnProgressReset);

            ShowPopup(1);
            LogMessage("LevelSelectController: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound) return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed) HandleMouseClick();
        }

        private void HandleMouseClick()
        {
            // Level 1 button
            if (level1ButtonId != 0 && IsButtonClicked(level1ButtonId, level1ButtonSelectedId))
            {
                LogMessage("LevelSelectController: Level 1 button clicked - showing Level 1 popup");
                ShowPopup(1);
                return;
            }

            // Level 2 button (only if unlocked)
            if (level2Unlocked && level2ButtonId != 0 && IsButtonClicked(level2ButtonId, level2ButtonSelectedId))
            {
                LogMessage("LevelSelectController: Level 2 button clicked - showing Level 2 popup");
                ShowPopup(2);
                return;
            }

            // Level 3 button (only if unlocked)
            if (level3Unlocked && level3ButtonId != 0 && IsButtonClicked(level3ButtonId, level3ButtonSelectedId))
            {
                LogMessage("LevelSelectController: Level 3 button clicked - showing Level 3 popup");
                ShowPopup(3);
                return;
            }

            // Skip Tutorial toggle - Level 1 popup only
            if (!IsLevel2Selected && !IsLevel3Selected)
            {
                if ((skipTutorial1UntickedId != 0 && Collision2D.IsMouseCollidingWithEntity(skipTutorial1UntickedId)) ||
                    (skipTutorial1TickedId != 0 && Collision2D.IsMouseCollidingWithEntity(skipTutorial1TickedId)))
                {
                    skipLevel1 = !skipLevel1;
                    ProgressTracker.LoadProgress();
                    ProgressTracker.SkipTutorialLevel1 = skipLevel1;
                    ProgressTracker.SaveProgress();
                    UpdateSkipTutorialVisual1();
                    LogMessage("LevelSelectController: SkipTutorialLevel1 toggled = " + skipLevel1);
                    return;
                }
            }

            // Skip Tutorial toggle - Level 2 popup only
            if (IsLevel2Selected)
            {
                if ((skipTutorial2UntickedId != 0 && Collision2D.IsMouseCollidingWithEntity(skipTutorial2UntickedId)) ||
                    (skipTutorial2TickedId != 0 && Collision2D.IsMouseCollidingWithEntity(skipTutorial2TickedId)))
                {
                    skipLevel2 = !skipLevel2;
                    ProgressTracker.LoadProgress();
                    ProgressTracker.SkipTutorialLevel2 = skipLevel2;
                    ProgressTracker.SaveProgress();
                    UpdateSkipTutorialVisual2();
                    LogMessage("LevelSelectController: SkipTutorialLevel2 toggled = " + skipLevel2);
                    return;
                }
            }
        }

        private void ShowPopup(int level)
        {
            IsLevel2Selected = (level == 2);
            IsLevel3Selected = (level == 3);

            if (levelSelectionPopup1Id != 0) SetIsVisible(levelSelectionPopup1Id, level == 1);
            if (levelSelectionPopup2Id != 0) SetIsVisible(levelSelectionPopup2Id, level == 2);
            if (levelSelectionPopup3Id != 0) SetIsVisible(levelSelectionPopup3Id, level == 3);

            // Show the correct skip tutorial for the active popup, hide others
            if (level == 1)
            {
                UpdateSkipTutorialVisual1();
                if (skipTutorial2UntickedId != 0) SetIsVisible(skipTutorial2UntickedId, false);
                if (skipTutorial2TickedId != 0) SetIsVisible(skipTutorial2TickedId, false);
            }
            else if (level == 2)
            {
                UpdateSkipTutorialVisual2();
                if (skipTutorial1UntickedId != 0) SetIsVisible(skipTutorial1UntickedId, false);
                if (skipTutorial1TickedId != 0) SetIsVisible(skipTutorial1TickedId, false);
            }
            else
            {
                if (skipTutorial1UntickedId != 0) SetIsVisible(skipTutorial1UntickedId, false);
                if (skipTutorial1TickedId != 0) SetIsVisible(skipTutorial1TickedId, false);
                if (skipTutorial2UntickedId != 0) SetIsVisible(skipTutorial2UntickedId, false);
                if (skipTutorial2TickedId != 0) SetIsVisible(skipTutorial2TickedId, false);
            }

            UpdateButtonSelection();
        }

        private void UpdateSkipTutorialVisual1()
        {
            if (skipTutorial1UntickedId != 0) SetIsVisible(skipTutorial1UntickedId, !skipLevel1);
            if (skipTutorial1TickedId != 0) SetIsVisible(skipTutorial1TickedId, skipLevel1);
        }

        private void UpdateSkipTutorialVisual2()
        {
            if (skipTutorial2UntickedId != 0) SetIsVisible(skipTutorial2UntickedId, !skipLevel2);
            if (skipTutorial2TickedId != 0) SetIsVisible(skipTutorial2TickedId, skipLevel2);
        }

        private void UpdateButtonSelection()
        {
            if (IsLevel3Selected)
            {
                if (level1ButtonId != 0) SetColor(level1ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level1ButtonSelectedId != 0) SetColor(level1ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level2ButtonId != 0) SetColor(level2ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level2ButtonSelectedId != 0) SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level3ButtonId != 0) SetColor(level3ButtonId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level3ButtonSelectedId != 0) SetColor(level3ButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f);
            }
            else if (IsLevel2Selected)
            {
                if (level1ButtonId != 0) SetColor(level1ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level1ButtonSelectedId != 0) SetColor(level1ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level2ButtonId != 0) SetColor(level2ButtonId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level2ButtonSelectedId != 0) SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level3ButtonId != 0) SetColor(level3ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level3ButtonSelectedId != 0) SetColor(level3ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
            }
            else
            {
                // Level 1 selected
                if (level1ButtonId != 0) SetColor(level1ButtonId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level1ButtonSelectedId != 0) SetColor(level1ButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level2ButtonId != 0) SetColor(level2ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level2ButtonSelectedId != 0) SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (level3ButtonId != 0) SetColor(level3ButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (level3ButtonSelectedId != 0) SetColor(level3ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
            }
        }

        private bool IsButtonClicked(uint normalId, uint selectedId)
        {
            if (normalId != 0 && Collision2D.IsMouseCollidingWithEntity(normalId)) return true;
            if (selectedId != 0 && Collision2D.IsMouseCollidingWithEntity(selectedId)) return true;
            return false;
        }

        private void OnProgressReset(string eventName, string payload)
        {
            LogMessage("LevelSelectController: Progress reset - hiding Level 2 and Level 3");
            level2Unlocked = false;
            level3Unlocked = false;
            skipLevel1 = false;
            skipLevel2 = false;
            IsLevel2Selected = false;
            IsLevel3Selected = false;

            if (level2ButtonId != 0) SetIsVisible(level2ButtonId, false);
            if (level2ButtonSelectedId != 0) { SetIsVisible(level2ButtonSelectedId, false); SetColor(level2ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f); }
            if (level3ButtonId != 0) SetIsVisible(level3ButtonId, false);
            if (level3ButtonSelectedId != 0) { SetIsVisible(level3ButtonSelectedId, false); SetColor(level3ButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f); }

            ShowPopup(1);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe("ProgressReset", OnProgressReset);
            LogMessage("LevelSelectController: Destroyed");
        }
    }
}

using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;

namespace Game
{
    /// <summary>
    /// Controls level selection buttons in the main menu.
    /// Hides Level 2 button until Level 2 has been won.
    /// Handles click on Level 1 and Level 2 buttons to load scenes.
    /// </summary>
    public class LevelSelectController : ScriptBehaviour
    {
        // Entity names
        private const string LEVEL1_BUTTON_NAME = "Level1_Button";
        private const string LEVEL2_BUTTON_NAME = "Level2_Button";
        private const string LEVEL_SELECTION_POPUP_1_NAME = "LevelSelectionPopUp_Level1";
        private const string LEVEL_SELECTION_POPUP_2_NAME = "LevelSelectionPopUp_Level2";

        // Scene paths
        private const string TRENCH_RUN_SCENE_PATH = "Resources/Sources/Scenes/trench_run.json";
        private const string LEVEL2_SCENE_PATH = "Resources/Sources/Scenes/level2.json";

        // Entity IDs
        private uint level1ButtonId;
        private uint level2ButtonId;
        private uint levelSelectionPopup1Id;
        private uint levelSelectionPopup2Id;

        // State
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool level2Unlocked = false;

        public override void OnStart()
        {
            LogMessage("LevelSelectController: Initializing...");

            // Load progress to check win state
            ProgressTracker.LoadProgress();
            level2Unlocked = ProgressTracker.HasWonLevel2;
            LogMessage("LevelSelectController: Level 2 unlocked = " + level2Unlocked);

            // Find button entities
            level1ButtonId = SceneFindEntityByName(LEVEL1_BUTTON_NAME);
            level2ButtonId = SceneFindEntityByName(LEVEL2_BUTTON_NAME);
            levelSelectionPopup1Id = SceneFindEntityByName(LEVEL_SELECTION_POPUP_1_NAME);
            levelSelectionPopup2Id = SceneFindEntityByName(LEVEL_SELECTION_POPUP_2_NAME);

            if (level1ButtonId == 0)
                LogMessage("LevelSelectController: Level 1 button not found (optional)");
            if (level2ButtonId == 0)
                LogMessage("LevelSelectController: Level 2 button not found (optional)");

            entitiesFound = true;
            wasMousePressed = false;

            // Hide Level 2 button if not unlocked
            if (!level2Unlocked)
            {
                if (level2ButtonId != 0)
                    SetIsVisible(level2ButtonId, false);
                if (levelSelectionPopup2Id != 0)
                    SetIsVisible(levelSelectionPopup2Id, false);
                LogMessage("LevelSelectController: Level 2 button hidden (not unlocked)");
            }
            else
            {
                if (level2ButtonId != 0)
                    SetIsVisible(level2ButtonId, true);
                LogMessage("LevelSelectController: Level 2 button visible (unlocked)");
            }

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
            // Check Level 1 button click
            if (level1ButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(level1ButtonId))
            {
                LogMessage("LevelSelectController: Level 1 button clicked - loading trench_run");
                LoadScene(TRENCH_RUN_SCENE_PATH);
                return;
            }

            // Check Level 2 button click (only if unlocked)
            if (level2Unlocked && level2ButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(level2ButtonId))
            {
                LogMessage("LevelSelectController: Level 2 button clicked - loading level2");
                LoadScene(LEVEL2_SCENE_PATH);
                return;
            }
        }

        private void LoadScene(string scenePath)
        {
            // Stop all audio
            AudioManager.StopGroup(AudioType.BGM);
            AudioManager.StopGroup(AudioType.SFX);

            // Hide cursor for gameplay
            Input.SetCursorVisible(false);

            bool success = Scene.SceneLoadFromFile(scenePath);
            if (success)
            {
                LogMessage("LevelSelectController: Scene loaded successfully: " + scenePath);
            }
            else
            {
                LogError("LevelSelectController: Failed to load scene: " + scenePath);
                Input.SetCursorVisible(true);
            }
        }

        public override void OnDestroy()
        {
            LogMessage("LevelSelectController: Destroyed");
        }
    }
}

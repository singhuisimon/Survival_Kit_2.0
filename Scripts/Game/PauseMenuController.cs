using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.AudioManager;

namespace Game
{
    /// <summary>
    /// Attach this to an entity in your GAME SCENES (trench_run.json, level2.json)
    /// Detects P key press and loads the separate pause menu scene.
    /// </summary>
    public class PauseMenuController : ScriptBehaviour
    {
        // Scene paths
        private const string PAUSE_MENU_PATH = "Resources/Sources/Scenes/PauseMenuScene.json";
        private const string GAME_SCENE_PATH = "Resources/Sources/Scenes/trench_run.json";
        private const string LEVEL2_SCENE_PATH = "Resources/Sources/Scenes/level2.json";

        // Pause events
        private const string EVENT_GAME_PAUSED = "GamePaused";
        private const string EVENT_GAME_RESUMED = "GameResumed";

        // State
        private bool wasPauseKeyPressed = false;
        private bool gameEnded = false;
        private string currentGameScenePath = GAME_SCENE_PATH;

        public override void OnStart()
        {
            LogMessage("PauseMenuController: Initializing...");

            gameEnded = false;

            // Detect which level we're in
            uint[] turrets = SceneFindEntitiesByTag("EnemyTurret");
            if (turrets != null && turrets.Length > 0)
            {
                currentGameScenePath = LEVEL2_SCENE_PATH;
                LogMessage("PauseMenuController: Detected Level 2");
            }
            else
            {
                currentGameScenePath = GAME_SCENE_PATH;
                LogMessage("PauseMenuController: Detected Level 1");
            }

            // Subscribe to win/lose events to block pause menu
            Event.Subscribe("GameOver", OnGameEnded);
            Event.Subscribe("GameWin", OnGameEnded);
            Event.Subscribe("GameRestart", OnGameRestart);

            LogMessage("PauseMenuController: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Block pause menu when win/lose screen is active
            if (gameEnded)
            {
                wasPauseKeyPressed = Input.IsKeyPressed(KeyCode.P);
                return;
            }

            // Handle P key to toggle pause
            bool pauseKeyPressed = Input.IsKeyPressed(KeyCode.P);
            bool pauseKeyJustPressed = pauseKeyPressed && !wasPauseKeyPressed;
            wasPauseKeyPressed = pauseKeyPressed;

            if (pauseKeyJustPressed)
            {
                LogMessage("PauseMenuController: P key pressed - loading pause menu");
                ShowPauseMenu();
            }
        }

        private void ShowPauseMenu()
        {
            LogMessage("PauseMenuController: Loading pause menu scene");

            // Set global pause state BEFORE loading scene
            GameState.IsPaused = true;

            // Show cursor for menu
            Input.SetCursorVisible(true);

            // Publish event with current scene path so pause menu knows where to return
            Event.Publish(EVENT_GAME_PAUSED, currentGameScenePath);

            // Load pause menu scene
            bool success = Scene.SceneLoadFromFile(PAUSE_MENU_PATH);
            if (success)
            {
                LogMessage("PauseMenuController: Pause menu loaded successfully");
            }
            else
            {
                LogError("PauseMenuController: Failed to load pause menu scene");
                // Reset state if loading failed
                GameState.IsPaused = false;
                Input.SetCursorVisible(false);
            }
        }

        private void OnGameEnded(string eventName, string payload)
        {
            LogMessage("PauseMenuController: Game ended (" + eventName + ") - disabling pause menu");
            gameEnded = true;
        }

        private void OnGameRestart(string eventName, string payload)
        {
            LogMessage("PauseMenuController: Game restarted - re-enabling pause menu");
            gameEnded = false;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe("GameOver", OnGameEnded);
            Event.Unsubscribe("GameWin", OnGameEnded);
            Event.Unsubscribe("GameRestart", OnGameRestart);
            LogMessage("PauseMenuController: Destroyed");
        }
    }
}
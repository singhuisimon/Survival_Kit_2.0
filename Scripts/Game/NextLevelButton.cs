using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// NextLevelButton_Win - Next level button on win screen
    /// Uses exact same collision logic as pause menu
    /// </summary>
    public class NextLevelButton : ScriptBehaviour
    {
        // Scene paths
        private const string LEVEL_2_SCENE_PATH = "Sources/Scenes/level2_player.json";
        private const string FALLBACK_SCENE_PATH = "Sources/Scenes/MainMenu.json";

        // Win event
        private const string EVENT_TIMER_FINISHED = "TimerFinished";

        // State
        private bool isButtonActive = false;
        private bool wasMousePressed = false;

        // Current level tracking
        [SerializeField("Current Level")]
        private int currentLevel = 1;

        public override void OnStart()
        {
            LogMessage("=== NextLevelButton_Win OnStart ===");
            LogMessage("NextLevelButton_Win EntityID: " + EntityID);
            LogMessage("Current Level: " + currentLevel);

            // Subscribe to win event
            Event.Subscribe(EVENT_TIMER_FINISHED, OnWinCondition);

            // Start invisible and inactive
            SetIsVisible((uint)EntityID, false);
            isButtonActive = false;

            LogMessage("[NextLevelButton_Win] Initialized");
        }

        private void OnWinCondition(string eventName, string payload)
        {
            LogMessage("[NextLevelButton_Win] Win condition triggered - showing button");

            // Make button visible and active
            SetIsVisible((uint)EntityID, true);
            isButtonActive = true;
        }

        public override void OnUpdate(float deltaTime)
        {
            // Only process when button is active
            if (!isButtonActive)
                return;

            // Handle mouse click (exact same as pause menu)
            HandleMouseClick();
        }

        private void HandleMouseClick()
        {
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (!mouseJustPressed) return;

            // Check if button clicked (exact same as pause menu)
            if (IsButtonClicked((uint)EntityID))
            {
                LogMessage("[NextLevelButton_Win] Button clicked - loading next level");

                // Stop all audio
                AudioManager.StopGroup(AudioType.BGM);
                AudioManager.StopGroup(AudioType.SFX);

                // Hide cursor for gameplay
                Input.SetCursorVisible(false);

                // Determine which level to load
                string nextScenePath = GetNextLevelPath();
                LogMessage("Loading scene: " + nextScenePath);

                // Load next level
                //Event.Publish("LoadScene", nextScenePath);
                bool success = Scene.SceneLoadFromFile(nextScenePath);
            }
        }

        private bool IsButtonClicked(uint buttonId)
        {
            return (buttonId != 0 && Collision2D.IsMouseCollidingWithEntity(buttonId));
        }

        private string GetNextLevelPath()
        {
            switch (currentLevel)
            {
                case 1:
                    return LEVEL_2_SCENE_PATH;

                case 2:
                    LogMessage("No more levels - returning to main menu");
                    Input.SetCursorVisible(true);
                    return FALLBACK_SCENE_PATH;

                default:
                    LogMessage("Unknown level - returning to main menu");
                    Input.SetCursorVisible(true);
                    return FALLBACK_SCENE_PATH;
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnWinCondition);
            LogMessage("=== NextLevelButton_Win Destroyed ===");
        }
    }
}
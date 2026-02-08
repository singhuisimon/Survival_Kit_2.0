using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// MainMenuButton_WinLose - Main menu button for win/lose screens
    /// Uses exact same collision logic as pause menu
    /// Set "Is Win Button" to true for win screen, false for lose screen
    /// </summary>
    public class MainMenuButton : ScriptBehaviour
    {
        // Scene path
        private const string MAIN_MENU_SCENE_PATH = "Sources/Scenes/MainMenu.json";

        // Win/Lose events
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string GAMEWIN = "GameWin";

        // Configuration
        [SerializeField("Is Win Button")]
        private bool isWinButton = false;

        // State
        private bool isButtonActive = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("=== MainMenuButton_WinLose OnStart ===");
            LogMessage("MainMenuButton_WinLose EntityID: " + EntityID);
            LogMessage("Is Win Button: " + isWinButton);


                Event.Subscribe(EVENT_PLAYER_DEAD, OnShowCondition);
                Event.Subscribe(EVENT_CORE_DESTROYED, OnShowCondition);
            

            // Start invisible and inactive
            SetIsVisible((uint)EntityID, false);
            isButtonActive = false;

            LogMessage("[MainMenuButton_WinLose] Initialized");
        }

        private void OnShowCondition(string eventName, string payload)
        {
            LogMessage("[MainMenuButton_WinLose] Show condition triggered - showing button (isWin=" + isWinButton + ")");

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
                LogMessage("[MainMenuButton_WinLose] Button clicked - returning to main menu");

                // Stop all audio
                AudioManager.StopGroup(AudioType.BGM);
                AudioManager.StopGroup(AudioType.SFX);

                // Show cursor for main menu
                Input.SetCursorVisible(true);

                // Clear pause state
                GameState.IsPaused = false;

                // Load main menu scene
                //Event.Publish("LoadScene", MAIN_MENU_SCENE_PATH);
                bool success = Scene.SceneLoadFromFile(MAIN_MENU_SCENE_PATH);
                if (success)
                {
                    LogMessage("Main Menu Scene loaded successfully!");
                }
            }
        }

        private bool IsButtonClicked(uint buttonId)
        {
            return (buttonId != 0 && Collision2D.IsMouseCollidingWithEntity(buttonId));
        }

        public override void OnDestroy()
        {
            if (isWinButton)
            {
                Event.Unsubscribe(EVENT_TIMER_FINISHED, OnShowCondition);
                Event.Unsubscribe(GAMEWIN, OnShowCondition);
            }
            else
            {
                Event.Unsubscribe(EVENT_PLAYER_DEAD, OnShowCondition);
                Event.Unsubscribe(EVENT_CORE_DESTROYED, OnShowCondition);
            }
            LogMessage("=== MainMenuButton_WinLose Destroyed ===");
        }
    }
}
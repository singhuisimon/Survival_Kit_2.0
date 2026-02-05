using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// RestartButton_Lose - Restart button on lose screen
    /// Uses exact same collision logic as pause menu
    /// </summary>
    public class RestartButton : ScriptBehaviour
    {
        // Scene path
        private const string GAME_SCENE_PATH = "Resources/Sources/Scenes/Level1_NewPlayer.json";

        // Lose events
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        // State
        private bool isButtonActive = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("=== RestartButton_Lose OnStart ===");
            LogMessage("RestartButton_Lose EntityID: " + EntityID);

            // Subscribe to lose events
            Event.Subscribe(EVENT_PLAYER_DEAD, OnLoseCondition);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnLoseCondition);

            // Start invisible and inactive
            SetIsVisible((uint)EntityID, false);
            isButtonActive = false;

            LogMessage("[RestartButton_Lose] Initialized");
        }

        private void OnLoseCondition(string eventName, string payload)
        {
            LogMessage("[RestartButton_Lose] Lose condition triggered - showing button");

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
                LogMessage("[RestartButton_Lose] Button clicked - reloading game scene");

                // Stop all audio
                AudioManager.StopGroup(AudioType.BGM);
                AudioManager.StopGroup(AudioType.SFX);

                // Hide cursor for gameplay
                Input.SetCursorVisible(false);

                // Load game scene
                Event.Publish("LoadScene", GAME_SCENE_PATH);
            }
        }

        private bool IsButtonClicked(uint buttonId)
        {
            return (buttonId != 0 && Collision2D.IsMouseCollidingWithEntity(buttonId));
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnLoseCondition);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnLoseCondition);
            LogMessage("=== RestartButton_Lose Destroyed ===");
        }
    }
}
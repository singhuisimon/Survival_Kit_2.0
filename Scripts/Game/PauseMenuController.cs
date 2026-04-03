using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.AudioManager;

namespace Game
{
    /// <summary>
    /// Attach this to an entity in your GAME SCENES (trench_run.json, level2.json, level3.json).
    /// Detects P key press and forwards it to the inline PauseMenuPopup / PauseMenuPopup3
    /// via the "TogglePauseMenu" event.  Does NOT load a separate pause scene.
    /// </summary>
    public class PauseMenuController : ScriptBehaviour
    {
        // Pause events
        private const string EVENT_TOGGLE_PAUSE_MENU = "TogglePauseMenu";

        // State
        private bool wasPauseKeyPressed = false;
        private bool gameEnded = false;

        public override void OnStart()
        {
            LogMessage("PauseMenuController: Initializing...");

            gameEnded = false;

            // Subscribe to win/lose events to block the P-key toggle
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

            // Handle P key to toggle pause — delegate to the inline popup
            bool pauseKeyPressed = Input.IsKeyPressed(KeyCode.P);
            bool pauseKeyJustPressed = pauseKeyPressed && !wasPauseKeyPressed;
            wasPauseKeyPressed = pauseKeyPressed;

            if (pauseKeyJustPressed)
            {
                LogMessage("PauseMenuController: P key pressed - forwarding TogglePauseMenu");
                Event.Publish(EVENT_TOGGLE_PAUSE_MENU, "");
            }
        }

        private void OnGameEnded(string eventName, string payload)
        {
            LogMessage("PauseMenuController: Game ended (" + eventName + ") - disabling P-key pause");
            gameEnded = true;
        }

        private void OnGameRestart(string eventName, string payload)
        {
            LogMessage("PauseMenuController: Game restarted - re-enabling P-key pause");
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

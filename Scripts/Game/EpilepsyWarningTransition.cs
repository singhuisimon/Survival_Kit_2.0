// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;

namespace Game
{
    /// <summary>
    /// Handles transition from epilepsy warning screen to main menu
    /// Detects ANY keyboard or mouse button press to trigger transition
    /// </summary>
    public class EpilepsyWarningTransition : ScriptBehaviour
    {
        // Configurable fields
        [SerializeField]
        private string mainMenuScenePath = "Sources/Scenes/MainMenu.json";

        [SerializeField]
        private float minimumDisplayTime = 1.0f;  // Minimum time to display warning (seconds)

        // Internal state
        private float elapsedTime = 0.0f;
        private bool canTransition = false;
        private bool hasTransitioned = false;

        public override void OnStart()
        {
            LogMessage("EpilepsyWarningTransition: Started");
            LogMessage("Will transition to: " + mainMenuScenePath);
            LogMessage("Minimum display time: " + minimumDisplayTime + " seconds");
            
            elapsedTime = 0.0f;
            canTransition = false;
            hasTransitioned = false;
        }

        public override void OnUpdate(float deltaTime)
        {
            // Don't do anything if already transitioned
            if (hasTransitioned)
                return;

            // Track elapsed time
            elapsedTime += deltaTime;

            // Allow transition only after minimum display time
            if (!canTransition && elapsedTime >= minimumDisplayTime)
            {
                canTransition = true;
                LogMessage("EpilepsyWarningTransition: Minimum display time reached - ready for input");
            }

            // Check for any input if we can transition
            if (canTransition && CheckAnyInput())
            {
                LogMessage("EpilepsyWarningTransition: Input detected - transitioning to main menu");
                TransitionToMainMenu();
            }
        }

        /// <summary>
        /// Checks for ANY keyboard key or mouse button press
        /// </summary>
        private bool CheckAnyInput()
        {
            // Check common keyboard keys
            if (CheckKeyboardInput())
                return true;

            // Check mouse buttons
            if (CheckMouseInput())
                return true;

            return false;
        }

        /// <summary>
        /// Check for keyboard input across common keys
        /// </summary>
        private bool CheckKeyboardInput()
        {
            // Check letter keys (A-Z)
            for (int key = (int)KeyCode.A; key <= (int)KeyCode.Z; key++)
            {
                if (Input.IsKeyPressed((KeyCode)key))
                {
                    LogMessage("Key pressed: " + ((KeyCode)key).ToString());
                    return true;
                }
            }

            // Check number keys (0-9)
            for (int key = (int)KeyCode.D0; key <= (int)KeyCode.D9; key++)
            {
                if (Input.IsKeyPressed((KeyCode)key))
                {
                    LogMessage("Number key pressed: " + ((KeyCode)key).ToString());
                    return true;
                }
            }

            // Check special keys
            if (Input.IsKeyPressed(KeyCode.Space))
            {
                LogMessage("Space pressed");
                return true;
            }

            if (Input.IsKeyPressed(KeyCode.Enter))
            {
                LogMessage("Enter pressed");
                return true;
            }

            if (Input.IsKeyPressed(KeyCode.Escape))
            {
                LogMessage("Escape pressed");
                return true;
            }

            // Check arrow keys
            if (Input.IsKeyPressed(KeyCode.Up) ||
                Input.IsKeyPressed(KeyCode.Down) ||
                Input.IsKeyPressed(KeyCode.Left) ||
                Input.IsKeyPressed(KeyCode.Right))
            {
                LogMessage("Arrow key pressed");
                return true;
            }

            return false;
        }

        /// <summary>
        /// Check for mouse button input
        /// </summary>
        private bool CheckMouseInput()
        {
            if (Input.IsMouseButtonPressed(MouseButton.Left))
            {
                LogMessage("Left mouse button pressed");
                return true;
            }

            if (Input.IsMouseButtonPressed(MouseButton.Right))
            {
                LogMessage("Right mouse button pressed");
                return true;
            }

            if (Input.IsMouseButtonPressed(MouseButton.Middle))
            {
                LogMessage("Middle mouse button pressed");
                return true;
            }

            return false;
        }

        /// <summary>
        /// Transitions to the main menu scene by publishing an event
        /// </summary>
        private void TransitionToMainMenu()
        {
            if (hasTransitioned)
                return;

            hasTransitioned = true;

            LogMessage("===========================================");
            LogMessage("TRANSITIONING TO MAIN MENU");
            LogMessage("Scene path: " + mainMenuScenePath);
            LogMessage("===========================================");
            bool success = Scene.SceneLoadFromFile(mainMenuScenePath);

            if (success)
            {
                LogMessage("Scene loaded successfully!");
            }
            else
            {
                LogMessage("ERROR: Failed to load scene!");
                // Optionally, you could retry or show an error message
            }
            // Publish event to trigger scene transition
            // The Game.cpp or a scene manager should listen to this event
            //Publish("LoadScene", mainMenuScenePath);
        }

        public override void OnDestroy()
        {
            LogMessage("EpilepsyWarningTransition: Destroyed");
        }
    }
}

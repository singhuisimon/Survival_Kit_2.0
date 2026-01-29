// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;

namespace Game
{
    public class InstallButtonHandler : ScriptBehaviour
    {
        private const string INSTALL_BUTTON_NAME = "Install Button";
        private const string GLITCH_OVERLAY_NAME = "Glitch Overlay";
        private const string START_GAME_POPUP_NAME = "Start Game Popup";
        private const string START_GAME_YES_BUTTON_NAME = "Start Game Yes Button";
        private const int NUM_ERROR_POPUPS = 8;

        private const float HIDDEN_Y = -500.0f;

        // Visible positions for each error popup (index 0 = popup 1, index 7 = popup 8)
        private static readonly float[] POPUP_VISIBLE_X = { 750.0f, 850.0f, 600.0f, 450.0f, 400.0f, 350.0f, 300.0f, 250.0f };
        private static readonly float[] POPUP_VISIBLE_Y = { 450.0f, 250.0f, 500.0f, 400.0f, 300.0f, 250.0f, 200.0f, 150.0f };

        // Timestamps for showing popups (in seconds)
        // First batch: popup 8, 7, 6, 5 at 0.15s, 0.25s, 0.32s, 0.36s
        // Glitch overlay at 0.39s
        // Second batch: popup 4, 3, 2, 1 at 0.42s, 0.46s, 0.50s, 0.55s
        // Start game popup + button at 1.03s
        private static readonly float[] POPUP_TIMESTAMPS = {
            0.55f,  // Popup 1
            0.50f,  // Popup 2
            0.46f,  // Popup 3
            0.42f,  // Popup 4
            0.36f,  // Popup 5
            0.32f,  // Popup 6
            0.25f,  // Popup 7
            0.15f   // Popup 8
        };
        private const float GLITCH_TIMESTAMP = 0.39f;
        private const float START_GAME_TIMESTAMP = 1.03f;

        // Entity IDs
        private uint installButtonId;
        private uint[] errorPopupIds = new uint[NUM_ERROR_POPUPS];
        private uint glitchOverlayId;
        private uint startGamePopupId;
        private uint startGameYesButtonId;

        // State
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool sequenceStarted = false;
        private bool sequenceComplete = false;
        private float elapsedTime = 0.0f;

        // Track which elements have been shown
        private bool[] popupShown = new bool[NUM_ERROR_POPUPS];
        private bool glitchShown = false;
        private bool startGameShown = false;

        // Glitch overlay position
        private const float GLITCH_X = 640.0f;
        private const float GLITCH_Y = 360.0f;
        private const float GLITCH_Z = -0.9f;

        // Start game popup position
        private const float START_POPUP_X = 640.0f;
        private const float START_POPUP_Y = 360.0f;
        private const float START_POPUP_Z = -1.0f;

        // Start game yes button position
        private const float START_BUTTON_X = 568.0f;
        private const float START_BUTTON_Y = 437.0f;
        private const float START_BUTTON_Z = -0.2f;

        public override void OnStart()
        {
            LogMessage("InstallButtonHandler: Initializing...");

            // Find install button
            installButtonId = SceneFindEntityByName(INSTALL_BUTTON_NAME);
            if (installButtonId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + INSTALL_BUTTON_NAME);
                return;
            }

            // Find error popups
            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                string name = "Error Popup " + (i + 1);
                errorPopupIds[i] = SceneFindEntityByName(name);

                if (errorPopupIds[i] == 0)
                {
                    LogError("InstallButtonHandler: Could not find entity: " + name);
                    return;
                }
                popupShown[i] = false;
            }

            // Find glitch overlay
            glitchOverlayId = SceneFindEntityByName(GLITCH_OVERLAY_NAME);
            if (glitchOverlayId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + GLITCH_OVERLAY_NAME);
                return;
            }

            // Find start game popup
            startGamePopupId = SceneFindEntityByName(START_GAME_POPUP_NAME);
            if (startGamePopupId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + START_GAME_POPUP_NAME);
                return;
            }

            // Find start game yes button
            startGameYesButtonId = SceneFindEntityByName(START_GAME_YES_BUTTON_NAME);
            if (startGameYesButtonId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + START_GAME_YES_BUTTON_NAME);
                return;
            }

            entitiesFound = true;
            wasMousePressed = false;

            // Hide all elements initially (move off-screen)
            HideAllElements();

            LogMessage("InstallButtonHandler: Ready! All entities found.");
        }

        private void HideAllElements()
        {
            // Hide error popups
            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                Vector3 hidePos = new Vector3(POPUP_VISIBLE_X[i], HIDDEN_Y, -0.7f);
                SetPosition(errorPopupIds[i], ref hidePos);
            }

            // Hide glitch overlay
            Vector3 glitchHidePos = new Vector3(GLITCH_X, HIDDEN_Y, GLITCH_Z);
            SetPosition(glitchOverlayId, ref glitchHidePos);

            // Hide start game popup
            Vector3 startPopupHidePos = new Vector3(START_POPUP_X, HIDDEN_Y, START_POPUP_Z);
            SetPosition(startGamePopupId, ref startPopupHidePos);

            // Hide start game yes button
            Vector3 startButtonHidePos = new Vector3(START_BUTTON_X, HIDDEN_Y, START_BUTTON_Z);
            SetPosition(startGameYesButtonId, ref startButtonHidePos);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            // Handle mouse click detection
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            // If sequence not started, check for install button click
            if (!sequenceStarted && mouseJustPressed)
            {
                if (Collision2D.IsMouseCollidingWithEntity(installButtonId))
                {
                    LogMessage("InstallButtonHandler: Install button clicked - starting sequence");
                    sequenceStarted = true;
                    elapsedTime = 0.0f;
                }
            }

            // Update sequence if started
            if (sequenceStarted && !sequenceComplete)
            {
                elapsedTime += deltaTime;
                UpdateSequence();
            }

            // Handle start game yes button click after sequence is complete
            if (sequenceComplete && mouseJustPressed)
            {
                if (Collision2D.IsMouseCollidingWithEntity(startGameYesButtonId))
                {
                    LogMessage("InstallButtonHandler: Start Game Yes button clicked - loading gameplay");
                    // TODO: Load gameplay scene
                    Publish("StartGame", "");
                }
            }
        }

        private void UpdateSequence()
        {
            // Show error popups at their timestamps
            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                if (!popupShown[i] && elapsedTime >= POPUP_TIMESTAMPS[i])
                {
                    ShowErrorPopup(i);
                    popupShown[i] = true;
                    LogMessage("InstallButtonHandler: Error Popup " + (i + 1) + " shown at " + elapsedTime + "s");
                }
            }

            // Show glitch overlay
            if (!glitchShown && elapsedTime >= GLITCH_TIMESTAMP)
            {
                ShowGlitchOverlay();
                glitchShown = true;
                LogMessage("InstallButtonHandler: Glitch overlay shown at " + elapsedTime + "s");
            }

            // Show start game popup and button
            if (!startGameShown && elapsedTime >= START_GAME_TIMESTAMP)
            {
                ShowStartGamePopup();
                startGameShown = true;
                sequenceComplete = true;
                LogMessage("InstallButtonHandler: Start game popup shown at " + elapsedTime + "s - Sequence complete");
            }
        }

        private void ShowErrorPopup(int index)
        {
            Vector3 pos = new Vector3(POPUP_VISIBLE_X[index], POPUP_VISIBLE_Y[index], -0.7f - index * 0.01f);
            SetPosition(errorPopupIds[index], ref pos);
        }

        private void ShowGlitchOverlay()
        {
            Vector3 pos = new Vector3(GLITCH_X, GLITCH_Y, GLITCH_Z);
            SetPosition(glitchOverlayId, ref pos);
        }

        private void ShowStartGamePopup()
        {
            // Show popup
            Vector3 popupPos = new Vector3(START_POPUP_X, START_POPUP_Y, START_POPUP_Z);
            SetPosition(startGamePopupId, ref popupPos);

            // Show yes button
            Vector3 buttonPos = new Vector3(START_BUTTON_X, START_BUTTON_Y, START_BUTTON_Z);
            SetPosition(startGameYesButtonId, ref buttonPos);
        }

        public override void OnDestroy()
        {
            LogMessage("InstallButtonHandler: Destroyed");
        }
    }
}

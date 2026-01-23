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
    /// <summary>
    /// Handles the quit confirmation popup in the main menu.
    /// Shows popup when shutdown button is clicked, handles Yes/No responses.
    /// </summary>
    public class QuitConfirmationPopup : ScriptBehaviour
    {
        // Entity names to find
        private const string SHUTDOWN_BUTTON_NAME = "Shutdown Button";
        private const string POPUP_NAME = "Quit Confirmation Popup";
        private const string YES_BUTTON_NAME = "Quit Yes Button";
        private const string NO_BUTTON_NAME = "Quit No Button";

        // Positions
        private const float HIDDEN_Y = -500.0f;
        private const float VISIBLE_Y = 360.0f;
        private const float CENTER_X = 640.0f;

        // Button offsets from popup center when visible
        private const float YES_BUTTON_OFFSET_X = -60.0f;
        private const float NO_BUTTON_OFFSET_X = 60.0f;
        private const float BUTTON_OFFSET_Y = -60.0f;

        // Entity IDs
        private uint shutdownButtonId;
        private uint popupId;
        private uint yesButtonId;
        private uint noButtonId;

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;  // For edge detection

        public override void OnStart()
        {
            LogMessage("QuitConfirmationPopup: Initializing...");

            // Find all required entities
            shutdownButtonId = SceneFindEntityByName(SHUTDOWN_BUTTON_NAME);
            popupId = SceneFindEntityByName(POPUP_NAME);
            yesButtonId = SceneFindEntityByName(YES_BUTTON_NAME);
            noButtonId = SceneFindEntityByName(NO_BUTTON_NAME);

            // Verify all entities were found
            if (shutdownButtonId == 0)
            {
                LogError("QuitConfirmationPopup: Could not find entity: " + SHUTDOWN_BUTTON_NAME);
                return;
            }
            if (popupId == 0)
            {
                LogError("QuitConfirmationPopup: Could not find entity: " + POPUP_NAME);
                return;
            }
            if (yesButtonId == 0)
            {
                LogError("QuitConfirmationPopup: Could not find entity: " + YES_BUTTON_NAME);
                return;
            }
            if (noButtonId == 0)
            {
                LogError("QuitConfirmationPopup: Could not find entity: " + NO_BUTTON_NAME);
                return;
            }

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;

            LogMessage("QuitConfirmationPopup: All entities found, ready!");
            LogMessage("QuitConfirmationPopup: Shutdown Button ID: " + shutdownButtonId);
            LogMessage("QuitConfirmationPopup: Popup ID: " + popupId);
            LogMessage("QuitConfirmationPopup: Yes Button ID: " + yesButtonId);
            LogMessage("QuitConfirmationPopup: No Button ID: " + noButtonId);
            LogMessage("QuitConfirmationPopup: entitiesFound = " + entitiesFound);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            // Edge detection for mouse click (just pressed)
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            // Check for left mouse button click
            if (mouseJustPressed)
            {
                LogMessage("QuitConfirmationPopup: Mouse button just pressed detected!");
                HandleMouseClick();
            }
        }

        private void HandleMouseClick()
        {
            Vector2 mousePos = Input.GetMousePosition();
            LogMessage("QuitConfirmationPopup: Mouse clicked at (" + mousePos.X + ", " + mousePos.Y + ")");
            LogMessage("QuitConfirmationPopup: isPopupVisible = " + isPopupVisible);

            if (isPopupVisible)
            {
                // Popup is visible - check Yes/No buttons
                bool yesHit = Collision2D.IsMouseCollidingWithEntity(yesButtonId);
                bool noHit = Collision2D.IsMouseCollidingWithEntity(noButtonId);
                LogMessage("QuitConfirmationPopup: Yes button collision = " + yesHit);
                LogMessage("QuitConfirmationPopup: No button collision = " + noHit);

                if (yesHit)
                {
                    LogMessage("QuitConfirmationPopup: Yes clicked - quitting game");
                    OnYesClicked();
                }
                else if (noHit)
                {
                    LogMessage("QuitConfirmationPopup: No clicked - hiding popup");
                    OnNoClicked();
                }
            }
            else
            {
                // Popup is hidden - check shutdown button
                bool shutdownHit = Collision2D.IsMouseCollidingWithEntity(shutdownButtonId);
                LogMessage("QuitConfirmationPopup: Shutdown button ID = " + shutdownButtonId);
                LogMessage("QuitConfirmationPopup: Shutdown collision = " + shutdownHit);

                if (shutdownHit)
                {
                    LogMessage("QuitConfirmationPopup: Shutdown button clicked - showing popup");
                    ShowPopup();
                }
            }
        }

        private void ShowPopup()
        {
            isPopupVisible = true;

            // Move popup to center of screen
            Vector3 popupPos = new Vector3(CENTER_X, VISIBLE_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            // Move Yes button (left of center)
            Vector3 yesPos = new Vector3(CENTER_X + YES_BUTTON_OFFSET_X, VISIBLE_Y + BUTTON_OFFSET_Y, -0.6f);
            SetPosition(yesButtonId, ref yesPos);

            // Move No button (right of center)
            Vector3 noPos = new Vector3(CENTER_X + NO_BUTTON_OFFSET_X, VISIBLE_Y + BUTTON_OFFSET_Y, -0.6f);
            SetPosition(noButtonId, ref noPos);

            LogMessage("QuitConfirmationPopup: Popup shown at center");
        }

        private void HidePopup()
        {
            isPopupVisible = false;

            // Move popup off screen
            Vector3 popupPos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            // Move Yes button off screen
            Vector3 yesPos = new Vector3(CENTER_X + YES_BUTTON_OFFSET_X, HIDDEN_Y + BUTTON_OFFSET_Y, -0.6f);
            SetPosition(yesButtonId, ref yesPos);

            // Move No button off screen
            Vector3 noPos = new Vector3(CENTER_X + NO_BUTTON_OFFSET_X, HIDDEN_Y + BUTTON_OFFSET_Y, -0.6f);
            SetPosition(noButtonId, ref noPos);

            LogMessage("QuitConfirmationPopup: Popup hidden");
        }

        private void OnYesClicked()
        {
            // Publish quit event - Game.cpp will handle closing the window
            Publish("QuitGame", "");
        }

        private void OnNoClicked()
        {
            HidePopup();
        }

        public override void OnDestroy()
        {
            LogMessage("QuitConfirmationPopup: Destroyed");
        }
    }
}

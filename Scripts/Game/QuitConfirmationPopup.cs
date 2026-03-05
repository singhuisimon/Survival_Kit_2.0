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
        private const string BLACK_OVERLAY = "Black Overlay";

        // Positions
        private const float HIDDEN_Y = -500.0f;
        private const float VISIBLE_Y = 360.0f;
        private const float CENTER_X = 640.0f;

        // Absolute button positions when visible
        private const float YES_BUTTON_X = 568.0f;
        private const float YES_BUTTON_Y = 437.0f;
        private const float NO_BUTTON_X = 712.0f;
        private const float NO_BUTTON_Y = 437.0f;

        [SerializeField]
        private string noButtonEntityName = "QuitConfirmation NoButton";

        // Button dimensions for click detection (matches scale in scene)
        [SerializeField]
        private float shutdownButtonWidth = 60.0f;

        [SerializeField]
        private float shutdownButtonHeight = 20.0f;

        [SerializeField]
        private float yesNoButtonWidth = 50.0f;

        [SerializeField]
        private float yesNoButtonHeight = 20.0f;

        [SerializeField]
        private float popupWidth = 150.0f;

        [SerializeField]
        private float popupHeight = 100.0f;

        // Animation settings
        [SerializeField]
        private float fadeInDuration = 0.1f;

        // Positions - initialized in OnStart
        private Vector3 hiddenPosition;
        private Vector3 visiblePopupPosition;
        private Vector3 visibleYesPosition;
        private Vector3 visibleNoPosition;

        // Entity IDs
        private uint shutdownButtonId;
        private uint popupId;
        private uint yesButtonId;
        private uint noButtonId;
        private uint overlayId;

        // Event names for popup coordination
        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID_QUIT = "Quit";

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool isAnimating = false;
        private float animationTimer = 0.0f;

        // Input state for edge detection
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("QuitConfirmationPopup: Initializing...");

            // Find all required entities
            shutdownButtonId = SceneFindEntityByName(SHUTDOWN_BUTTON_NAME);
            popupId = SceneFindEntityByName(POPUP_NAME);
            yesButtonId = SceneFindEntityByName(YES_BUTTON_NAME);
            noButtonId = SceneFindEntityByName(NO_BUTTON_NAME);
            overlayId = SceneFindEntityByName(BLACK_OVERLAY);

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
            if (overlayId == 0)
            {
                LogError("QuitConfirmationPopup: Could not find entity: " + NO_BUTTON_NAME);
                return;
            }

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;

            // Subscribe to popup coordination events
            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);

            LogMessage("QuitConfirmationPopup: All entities found, ready!");
            LogMessage("QuitConfirmationPopup: Shutdown Button ID: " + shutdownButtonId);
            LogMessage("QuitConfirmationPopup: Popup ID: " + popupId);
            LogMessage("QuitConfirmationPopup: Yes Button ID: " + yesButtonId);
            LogMessage("QuitConfirmationPopup: No Button ID: " + noButtonId);
            LogMessage("QuitConfirmationPopup: Black Overlay ID: " + overlayId);
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
                    return;
                }
                else if (noHit)
                {
                    LogMessage("QuitConfirmationPopup: No clicked - hiding popup");
                    OnNoClicked();
                    return;
                }

                // Click outside popup closes it
                if (!IsMouseOverButton(mousePos, visiblePopupPosition, popupWidth, popupHeight))
                {
                    LogMessage("QuitConfirmationPopup: Clicked outside popup - hiding");
                    HidePopup();
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

        private bool IsMouseOverButton(Vector2 mousePos, Vector3 buttonPos, float width, float height)
        {
            float halfW = width * 0.5f;
            float halfH = height * 0.5f;

            float minX = buttonPos.X - halfW;
            float maxX = buttonPos.X + halfW;
            float minY = buttonPos.Y - halfH;
            float maxY = buttonPos.Y + halfH;

            return mousePos.X >= minX && mousePos.X <= maxX &&
                   mousePos.Y >= minY && mousePos.Y <= maxY;
        }

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUP_ID_QUIT && isPopupVisible)
            {
                LogMessage("QuitConfirmationPopup: Another popup opened (" + payload + ") - closing quit popup");
                HidePopup();
            }
        }

        private void ShowPopup()
        {
            if (isPopupVisible)
                return;

            isPopupVisible = true;
            isAnimating = true;
            animationTimer = 0.0f;
            Event.Publish(EVENT_POPUP_OPENED, POPUP_ID_QUIT);

            // Move overlay to center of screen
            Vector3 overlayPos = new Vector3(CENTER_X, VISIBLE_Y, -0.4f);
            SetPosition(overlayId, ref overlayPos);

            // Move popup to center of screen
            Vector3 popupPos = new Vector3(CENTER_X, VISIBLE_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            // Move Yes button to visible position
            Vector3 yesPos = new Vector3(YES_BUTTON_X, YES_BUTTON_Y, -0.6f);
            SetPosition(yesButtonId, ref yesPos);

            // Move No button to visible position
            Vector3 noPos = new Vector3(NO_BUTTON_X, NO_BUTTON_Y, -0.6f);
            SetPosition(noButtonId, ref noPos);

            LogMessage("QuitConfirmationPopup: Popup shown at center");
        }

        private void HidePopup()
        {
            bool wasVisible = isPopupVisible;
            isPopupVisible = false;
            isAnimating = false;
            if (wasVisible)
                Event.Publish(EVENT_POPUP_CLOSED, POPUP_ID_QUIT);

            // Move entities off-screen
            Vector3 hidePos = hiddenPosition;
            Vector3 yesHidePos = new Vector3(580.0f, -500.0f, -0.3f);
            Vector3 noHidePos = new Vector3(700.0f, -500.0f, -0.3f);

            // Move overlay off screen
            Vector3 overlayPos = new Vector3(CENTER_X, HIDDEN_Y, -0.4f);
            SetPosition(overlayId, ref overlayPos);

            // Move popup off screen
            Vector3 popupPos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            // Move Yes button off screen
            Vector3 yesPos = new Vector3(YES_BUTTON_X, HIDDEN_Y, -0.6f);
            SetPosition(yesButtonId, ref yesPos);

            // Move No button off screen
            Vector3 noPos = new Vector3(NO_BUTTON_X, HIDDEN_Y, -0.6f);
            SetPosition(noButtonId, ref noPos);

            LogMessage("QuitConfirmationPopup: Popup hidden");
        }

        private void OnYesClicked()
        {
            // Publish quit event - Game.cpp will handle closing the window
            Publish("QuitGame", "");
            LogMessage("QuitConfirmationPopup: QuitGame event published");
        }

        private void OnNoClicked()
        {
            HidePopup();
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("QuitConfirmationPopup: Destroyed");
        }
    }
}

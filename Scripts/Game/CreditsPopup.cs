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
    public class CreditsPopup : ScriptBehaviour
    {
        // Entity names to find
        private const string CREDITS_BUTTON_NAME = "Credits Button";
        private const string POPUP_NAME = "Credits Popup";
        private const string CLOSE_BUTTON_NAME = "Credits Close Button";
        private const string CLOSE_BUTTON_2_NAME = "Credits Close Button 2";

        // Positions
        private const float HIDDEN_Y = -500.0f;
        private const float VISIBLE_Y = 360.0f;
        private const float CENTER_X = 640.0f;

        // Absolute button positions when visible
        private const float CLOSE_BUTTON_X = 725.0f;
        private const float CLOSE_BUTTON_Y = 50.0f;
        private const float CLOSE_BUTTON_2_X = 1196.0f;
        private const float CLOSE_BUTTON_2_Y = 239.0f;

        // Entity IDs
        private uint creditsButtonId;
        private uint popupId;
        private uint closeButtonId;
        private uint closeButton2Id;

        // Event names for popup coordination
        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID = "Credits";

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("CreditsPopup: Initializing...");

            creditsButtonId = SceneFindEntityByName(CREDITS_BUTTON_NAME);
            popupId = SceneFindEntityByName(POPUP_NAME);
            closeButtonId = SceneFindEntityByName(CLOSE_BUTTON_NAME);
            closeButton2Id = SceneFindEntityByName(CLOSE_BUTTON_2_NAME);

            if (creditsButtonId == 0)
            {
                LogError("CreditsPopup: Could not find entity: " + CREDITS_BUTTON_NAME);
                return;
            }
            if (popupId == 0)
            {
                LogError("CreditsPopup: Could not find entity: " + POPUP_NAME);
                return;
            }
            if (closeButtonId == 0)
            {
                LogError("CreditsPopup: Could not find entity: " + CLOSE_BUTTON_NAME);
                return;
            }
            if (closeButton2Id == 0)
            {
                LogError("CreditsPopup: Could not find entity: " + CLOSE_BUTTON_2_NAME);
                return;
            }

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;

            // Subscribe to popup coordination events
            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);

            LogMessage("CreditsPopup: All entities found, ready!");
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
            if (isPopupVisible)
            {
                bool closeHit = Collision2D.IsMouseCollidingWithEntity(closeButtonId);
                bool close2Hit = Collision2D.IsMouseCollidingWithEntity(closeButton2Id);

                if (closeHit || close2Hit)
                {
                    LogMessage("CreditsPopup: Close button clicked - hiding popup");
                    HidePopup();
                }
            }
            else
            {
                bool creditsHit = Collision2D.IsMouseCollidingWithEntity(creditsButtonId);

                if (creditsHit)
                {
                    LogMessage("CreditsPopup: Credits button clicked - showing popup");
                    ShowPopup();
                }
            }
        }

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUP_ID && isPopupVisible)
            {
                LogMessage("CreditsPopup: Another popup opened (" + payload + ") - closing credits");
                HidePopup();
            }
        }

        private void ShowPopup()
        {
            isPopupVisible = true;
            Event.Publish(EVENT_POPUP_OPENED, POPUP_ID);

            Vector3 popupPos = new Vector3(CENTER_X, VISIBLE_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            Vector3 closePos = new Vector3(CLOSE_BUTTON_X, CLOSE_BUTTON_Y, -0.6f);
            SetPosition(closeButtonId, ref closePos);

            Vector3 close2Pos = new Vector3(CLOSE_BUTTON_2_X, CLOSE_BUTTON_2_Y, -0.6f);
            SetPosition(closeButton2Id, ref close2Pos);

            LogMessage("CreditsPopup: Popup shown");
        }

        private void HidePopup()
        {
            bool wasVisible = isPopupVisible;
            isPopupVisible = false;
            if (wasVisible)
                Event.Publish(EVENT_POPUP_CLOSED, POPUP_ID);

            Vector3 popupPos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            SetPosition(popupId, ref popupPos);

            Vector3 closePos = new Vector3(CLOSE_BUTTON_X, HIDDEN_Y, -0.6f);
            SetPosition(closeButtonId, ref closePos);

            Vector3 close2Pos = new Vector3(CLOSE_BUTTON_2_X, HIDDEN_Y, -0.6f);
            SetPosition(closeButton2Id, ref close2Pos);

            LogMessage("CreditsPopup: Popup hidden");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("CreditsPopup: Destroyed");
        }
    }
}

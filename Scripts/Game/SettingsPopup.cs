// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.Transform;

namespace Game
{
    /// <summary>
    /// Handles the settings popup in the main menu.
    /// Shows popup when settings button is clicked, handles close button.
    /// </summary>
    public class SettingsPopup : ScriptBehaviour
    {
        // Entity names
        private const string SETTINGS_BUTTON_NAME = "Settings Button";
        private const string SETTINGS_POPUP_NAME = "Settings Popup";
        private const string CLOSE_BUTTON_NAME = "Settings Close Button";
        private const string PLUS_BUTTON_1_NAME = "Settings Plus Button 1";
        private const string PLUS_BUTTON_2_NAME = "Settings Plus Button 2";
        private const string PLUS_BUTTON_3_NAME = "Settings Plus Button 3";
        private const string MINUS_BUTTON_1_NAME = "Settings Minus Button 1";
        private const string MINUS_BUTTON_2_NAME = "Settings Minus Button 2";
        private const string MINUS_BUTTON_3_NAME = "Settings Minus Button 3";
        private const string VOLUME_FILL_1_NAME = "Settings Volume Fill 1";
        private const string VOLUME_FILL_2_NAME = "Settings Volume Fill 2";
        private const string VOLUME_FILL_3_NAME = "Settings Volume Fill 3";

        // Positions
        private const float HIDDEN_Y = -500.0f;
        private const float VISIBLE_Y = 360.0f;
        private const float CENTER_X = 640.0f;

        // Entity IDs
        private uint settingsButtonId;
        private uint settingsPopupId;
        private uint closeButtonId;
        private uint plusButton1Id;
        private uint plusButton2Id;
        private uint plusButton3Id;
        private uint minusButton1Id;
        private uint minusButton2Id;
        private uint minusButton3Id;
        private uint volumeFill1Id;
        private uint volumeFill2Id;
        private uint volumeFill3Id;

        // Visible positions
        private Vector3 popupVisiblePos = new Vector3(640.0f, 350.0f, -0.5f);
        private Vector3 closeButtonVisiblePos = new Vector3(707.0f, 140.0f, -0.6f);
        private Vector3 plusButton1VisiblePos = new Vector3(591.0f, 235.0f, -0.6f);
        private Vector3 plusButton2VisiblePos = new Vector3(591.0f, 307.0f, -0.6f);
        private Vector3 plusButton3VisiblePos = new Vector3(591.0f, 378.0f, -0.6f);
        private Vector3 minusButton1VisiblePos = new Vector3(559.0f, 235.0f, -0.6f);
        private Vector3 minusButton2VisiblePos = new Vector3(559.0f, 307.0f, -0.6f);
        private Vector3 minusButton3VisiblePos = new Vector3(559.0f, 378.0f, -0.6f);
        private Vector3 volumeFill1VisiblePos = new Vector3(353.0f, 234.0f, -0.6f);
        private Vector3 volumeFill2VisiblePos = new Vector3(353.0f, 307.0f, -0.6f);
        private Vector3 volumeFill3VisiblePos = new Vector3(353.0f, 377.0f, -0.6f);

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("SettingsPopup: Initializing...");

            // Find settings button
            settingsButtonId = SceneFindEntityByName(SETTINGS_BUTTON_NAME);
            if (settingsButtonId == 0)
            {
                LogError("SettingsPopup: Could not find entity: " + SETTINGS_BUTTON_NAME);
                return;
            }

            // Find popup elements
            settingsPopupId = SceneFindEntityByName(SETTINGS_POPUP_NAME);
            closeButtonId = SceneFindEntityByName(CLOSE_BUTTON_NAME);
            plusButton1Id = SceneFindEntityByName(PLUS_BUTTON_1_NAME);
            plusButton2Id = SceneFindEntityByName(PLUS_BUTTON_2_NAME);
            plusButton3Id = SceneFindEntityByName(PLUS_BUTTON_3_NAME);
            minusButton1Id = SceneFindEntityByName(MINUS_BUTTON_1_NAME);
            minusButton2Id = SceneFindEntityByName(MINUS_BUTTON_2_NAME);
            minusButton3Id = SceneFindEntityByName(MINUS_BUTTON_3_NAME);
            volumeFill1Id = SceneFindEntityByName(VOLUME_FILL_1_NAME);
            volumeFill2Id = SceneFindEntityByName(VOLUME_FILL_2_NAME);
            volumeFill3Id = SceneFindEntityByName(VOLUME_FILL_3_NAME);

            // Log which entities were found
            if (settingsPopupId == 0) LogError("SettingsPopup: Could not find: " + SETTINGS_POPUP_NAME);
            if (closeButtonId == 0) LogError("SettingsPopup: Could not find: " + CLOSE_BUTTON_NAME);

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;

            // Hide all popup elements initially
            HidePopup();

            LogMessage("SettingsPopup: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            // Edge detection for mouse click
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
                // Check close button
                if (closeButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(closeButtonId))
                {
                    LogMessage("SettingsPopup: Close button clicked");
                    HidePopup();
                    return;
                }

                // Check if clicked outside popup to close
                if (settingsPopupId != 0 && !Collision2D.IsMouseCollidingWithEntity(settingsPopupId))
                {
                    LogMessage("SettingsPopup: Clicked outside - closing");
                    HidePopup();
                }
            }
            else
            {
                // Check settings button
                if (Collision2D.IsMouseCollidingWithEntity(settingsButtonId))
                {
                    LogMessage("SettingsPopup: Settings button clicked - showing popup");
                    ShowPopup();
                }
            }
        }

        private void ShowPopup()
        {
            if (isPopupVisible)
                return;

            isPopupVisible = true;

            // Show all popup elements at their visible positions
            if (settingsPopupId != 0) SetPosition(settingsPopupId, ref popupVisiblePos);
            if (closeButtonId != 0) SetPosition(closeButtonId, ref closeButtonVisiblePos);
            if (plusButton1Id != 0) SetPosition(plusButton1Id, ref plusButton1VisiblePos);
            if (plusButton2Id != 0) SetPosition(plusButton2Id, ref plusButton2VisiblePos);
            if (plusButton3Id != 0) SetPosition(plusButton3Id, ref plusButton3VisiblePos);
            if (minusButton1Id != 0) SetPosition(minusButton1Id, ref minusButton1VisiblePos);
            if (minusButton2Id != 0) SetPosition(minusButton2Id, ref minusButton2VisiblePos);
            if (minusButton3Id != 0) SetPosition(minusButton3Id, ref minusButton3VisiblePos);
            if (volumeFill1Id != 0) SetPosition(volumeFill1Id, ref volumeFill1VisiblePos);
            if (volumeFill2Id != 0) SetPosition(volumeFill2Id, ref volumeFill2VisiblePos);
            if (volumeFill3Id != 0) SetPosition(volumeFill3Id, ref volumeFill3VisiblePos);

            LogMessage("SettingsPopup: Popup shown");
        }

        private void HidePopup()
        {
            isPopupVisible = false;

            // Move all popup elements off-screen
            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, -0.6f);

            if (settingsPopupId != 0) SetPosition(settingsPopupId, ref hidePos);
            if (closeButtonId != 0) SetPosition(closeButtonId, ref hidePos2);
            if (plusButton1Id != 0) SetPosition(plusButton1Id, ref hidePos2);
            if (plusButton2Id != 0) SetPosition(plusButton2Id, ref hidePos2);
            if (plusButton3Id != 0) SetPosition(plusButton3Id, ref hidePos2);
            if (minusButton1Id != 0) SetPosition(minusButton1Id, ref hidePos2);
            if (minusButton2Id != 0) SetPosition(minusButton2Id, ref hidePos2);
            if (minusButton3Id != 0) SetPosition(minusButton3Id, ref hidePos2);
            if (volumeFill1Id != 0) SetPosition(volumeFill1Id, ref hidePos2);
            if (volumeFill2Id != 0) SetPosition(volumeFill2Id, ref hidePos2);
            if (volumeFill3Id != 0) SetPosition(volumeFill3Id, ref hidePos2);

            LogMessage("SettingsPopup: Popup hidden");
        }

        public override void OnDestroy()
        {
            LogMessage("SettingsPopup: Destroyed");
        }
    }
}

// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;
using static Game.AudioSettings;  // ADD THIS LINE


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
        private Vector3 popupVisiblePos = new Vector3(640.0f, 375.0f, -0.5f);
        private Vector3 closeButtonVisiblePos = new Vector3(712.4f, 61.8f, -0.6f);
        private Vector3 plusButton1VisiblePos = new Vector3(598.0f, 423.9f, -0.6f);
        private Vector3 plusButton2VisiblePos = new Vector3(598.0f, 495.0f, -0.6f);
        private Vector3 plusButton3VisiblePos = new Vector3(598.0f, 567.3f, -0.6f);
        private Vector3 minusButton1VisiblePos = new Vector3(565.0f, 424.2f, -0.6f);
        private Vector3 minusButton2VisiblePos = new Vector3(565.0f, 495.2f, -0.6f);
        private Vector3 minusButton3VisiblePos = new Vector3(565.0f, 566.3f, -0.6f);
        private Vector3 volumeFill1VisiblePos = new Vector3(359.4f, 423.0f, -0.6f);
        private Vector3 volumeFill2VisiblePos = new Vector3(359.4f, 494.0f, -0.6f);
        private Vector3 volumeFill3VisiblePos = new Vector3(359.4f, 565.0f, -0.6f);

        private Vector3 GammaPlus = new Vector3(359.4f, 565.0f, -0.6f);
        private Vector3 GammaMinus = new Vector3(359.4f, 565.0f, -0.6f);
        private Vector3 GammaVolume = new Vector3(359.4f, 565.0f, -0.6f);
        private Vector3 GammaDefault = new Vector3(359.4f, 565.0f, -0.6f);


        private Vector3 MousePlus = new Vector3(359.4f, 565.0f, -0.6f);
        private Vector3 MouseMinus = new Vector3(359.4f, 565.0f, -0.6f);
        private Vector3 MouseVolume = new Vector3(359.4f, 565.0f, -0.6f);
        private Vector3 MouseDefault = new Vector3(359.4f, 565.0f, -0.6f);




        // Event names for popup coordination
        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID = "Settings";

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;

        private float volumeFill1InitialWidth;
        private float volumeFill2InitialWidth;
        private float volumeFill3InitialWidth;
        private Vector3 volumeFill1InitialPosition;
        private Vector3 volumeFill2InitialPosition;
        private Vector3 volumeFill3InitialPosition;

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

            // Store initial widths and positions for volume fills (horizontal bars)
            if (volumeFill1Id != 0)
            {
                Vector3 scale1 = GetScale(volumeFill1Id);
                volumeFill1InitialWidth = scale1.X;
                volumeFill1InitialPosition = volumeFill1VisiblePos;  // USE THE VISIBLE POSITION, NOT GetPosition!
                LogMessage("SettingsPopup: Volume Fill 1 initial width = " + volumeFill1InitialWidth);
            }

            if (volumeFill2Id != 0)
            {
                Vector3 scale2 = GetScale(volumeFill2Id);
                volumeFill2InitialWidth = scale2.X;
                volumeFill2InitialPosition = volumeFill2VisiblePos;  // USE THE VISIBLE POSITION, NOT GetPosition!
                LogMessage("SettingsPopup: Volume Fill 2 initial width = " + volumeFill2InitialWidth);
            }

            if (volumeFill3Id != 0)
            {
                Vector3 scale3 = GetScale(volumeFill3Id);
                volumeFill3InitialWidth = scale3.X;
                volumeFill3InitialPosition = volumeFill3VisiblePos;  // USE THE VISIBLE POSITION, NOT GetPosition!
                LogMessage("SettingsPopup: Volume Fill 3 initial width = " + volumeFill3InitialWidth);
            }

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;

            // Subscribe to popup coordination events
            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);

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

                // Check volume buttons
                if (AudioSettings.Instance != null)
                {
                    // Volume Fill 1 - Master Volume
                    if (plusButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(plusButton1Id))
                    {
                        float currentVolume = AudioSettings.Instance.GetMasterVolume();
                        AudioSettings.Instance.SetMasterVolume(currentVolume + 0.1f);
                        LogMessage("SettingsPopup: Master Volume + (Now: " + AudioSettings.Instance.GetMasterVolume().ToString("F2") + ")");
                        UpdateVolumeFillVisual(volumeFill1Id, AudioSettings.Instance.GetMasterVolume(),
                                             volumeFill1InitialWidth, volumeFill1InitialPosition);
                        return;
                    }

                    if (minusButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(minusButton1Id))
                    {
                        float currentVolume = AudioSettings.Instance.GetMasterVolume();
                        AudioSettings.Instance.SetMasterVolume(currentVolume - 0.1f);
                        LogMessage("SettingsPopup: Master Volume - (Now: " + AudioSettings.Instance.GetMasterVolume().ToString("F2") + ")");
                        UpdateVolumeFillVisual(volumeFill1Id, AudioSettings.Instance.GetMasterVolume(),
                                             volumeFill1InitialWidth, volumeFill1InitialPosition);
                        return;
                    }

                    // Volume Fill 2 - BGM Volume
                    if (plusButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(plusButton2Id))
                    {
                        float currentVolume = AudioSettings.Instance.GetBGMVolume();
                        AudioSettings.Instance.SetBGMVolume(currentVolume + 0.1f);
                        LogMessage("SettingsPopup: BGM Volume + (Now: " + AudioSettings.Instance.GetBGMVolume().ToString("F2") + ")");
                        UpdateVolumeFillVisual(volumeFill2Id, AudioSettings.Instance.GetBGMVolume(),
                                             volumeFill2InitialWidth, volumeFill2InitialPosition);
                        return;
                    }

                    if (minusButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(minusButton2Id))
                    {
                        float currentVolume = AudioSettings.Instance.GetBGMVolume();
                        AudioSettings.Instance.SetBGMVolume(currentVolume - 0.1f);
                        LogMessage("SettingsPopup: BGM Volume - (Now: " + AudioSettings.Instance.GetBGMVolume().ToString("F2") + ")");
                        UpdateVolumeFillVisual(volumeFill2Id, AudioSettings.Instance.GetBGMVolume(),
                                             volumeFill2InitialWidth, volumeFill2InitialPosition);
                        return;
                    }

                    // Volume Fill 3 - SFX Volume
                    if (plusButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(plusButton3Id))
                    {
                        float currentVolume = AudioSettings.Instance.GetSFXVolume();
                        AudioSettings.Instance.SetSFXVolume(currentVolume + 0.1f);
                        LogMessage("SettingsPopup: SFX Volume + (Now: " + AudioSettings.Instance.GetSFXVolume().ToString("F2") + ")");
                        UpdateVolumeFillVisual(volumeFill3Id, AudioSettings.Instance.GetSFXVolume(),
                                             volumeFill3InitialWidth, volumeFill3InitialPosition);
                        return;
                    }

                    if (minusButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(minusButton3Id))
                    {
                        float currentVolume = AudioSettings.Instance.GetSFXVolume();
                        AudioSettings.Instance.SetSFXVolume(currentVolume - 0.1f);
                        LogMessage("SettingsPopup: SFX Volume - (Now: " + AudioSettings.Instance.GetSFXVolume().ToString("F2") + ")");
                        UpdateVolumeFillVisual(volumeFill3Id, AudioSettings.Instance.GetSFXVolume(),
                                             volumeFill3InitialWidth, volumeFill3InitialPosition);
                        return;
                    }
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

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUP_ID && isPopupVisible)
            {
                LogMessage("SettingsPopup: Another popup opened (" + payload + ") - closing settings");
                HidePopup();
            }
        }

        private void ShowPopup()
        {
            if (isPopupVisible)
                return;

            isPopupVisible = true;
            Event.Publish(EVENT_POPUP_OPENED, POPUP_ID);

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

            
                UpdateVolumeFillVisual(volumeFill1Id, AudioSettings.Instance.GetMasterVolume(),
                                     volumeFill1InitialWidth, volumeFill1InitialPosition);
                UpdateVolumeFillVisual(volumeFill2Id, AudioSettings.Instance.GetBGMVolume(),
                                     volumeFill2InitialWidth, volumeFill2InitialPosition);
                UpdateVolumeFillVisual(volumeFill3Id, AudioSettings.Instance.GetSFXVolume(),
                                     volumeFill3InitialWidth, volumeFill3InitialPosition);
            
            LogMessage("SettingsPopup: Popup shown");
        }

        private void HidePopup()
        {
            bool wasVisible = isPopupVisible;
            isPopupVisible = false;
            if (wasVisible)
                Event.Publish(EVENT_POPUP_CLOSED, POPUP_ID);

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

        // ADD THIS METHOD:
        /// <summary>
        /// Updates the volume fill bar visual width based on volume (0.0 to 1.0)
        /// Horizontal bar - adjusts width and position to keep left edge fixed
        /// </summary>
        private void UpdateVolumeFillVisual(uint volumeFillId, float volume, float initialWidth, Vector3 initialPosition)
        {
            LogMessage("UpdateVolumeFillVisual called: volumeFillId=" + volumeFillId + " volume=" + volume + " initialWidth=" + initialWidth);

            if (volumeFillId == 0)
            {
                LogMessage("UpdateVolumeFillVisual: volumeFillId is 0, returning");
                return;
            }

            // Clamp volume to 0-1 range
            if (volume < 0.0f) volume = 0.0f;
            if (volume > 1.0f) volume = 1.0f;

            // Calculate new width based on volume
            float newWidth = initialWidth * volume;
            LogMessage("UpdateVolumeFillVisual: newWidth calculated = " + newWidth);

            // Get current scale
            Vector3 currentScale = GetScale(volumeFillId);
            LogMessage("UpdateVolumeFillVisual: current scale = " + currentScale.X + ", " + currentScale.Y);

            // Update scale with new width
            Vector3 newScale = new Vector3(
                newWidth,           // Width based on volume
                currentScale.Y,     // Keep height
                currentScale.Z      // Keep depth
            );
            SetScale(volumeFillId, ref newScale);
            LogMessage("UpdateVolumeFillVisual: new scale set = " + newWidth + ", " + currentScale.Y);

            // Adjust position to keep LEFT edge fixed
            float widthDifference = initialWidth - newWidth;
            Vector3 newPosition = new Vector3(
                initialPosition.X - widthDifference,
                initialPosition.Y,
                initialPosition.Z
            );
            SetPosition(volumeFillId, ref newPosition);
            LogMessage("UpdateVolumeFillVisual: new position set = " + newPosition.X + ", " + newPosition.Y);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("SettingsPopup: Destroyed");
        }
    }
}

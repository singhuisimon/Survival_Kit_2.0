// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.SpriteRenderer;
using static Game.AudioSettings;

namespace Game
{
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

        // Mute button entity names
        private const string MUTE_MASTER_UNTICKED_NAME = "MuteMasterButton";
        private const string MUTE_BGM_UNTICKED_NAME = "MuteBGMButton";
        private const string MUTE_SFX_UNTICKED_NAME = "MuteSFXButton";
        private const string MUTE_MASTER_TICKED_NAME = "MuteMasterButton_Ticked";
        private const string MUTE_BGM_TICKED_NAME = "MuteBGMButton_Ticked";
        private const string MUTE_SFX_TICKED_NAME = "MuteSFXButton_Ticked";

        private const float HIDDEN_Y = -500.0f;
        private const float VISIBLE_Y = 360.0f;
        private const float CENTER_X = 640.0f;

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
        private uint gammaPlusId;
        private uint gammaMinusId;
        private uint gammaVolumeId;
        private uint mousePlusId;
        private uint mouseMinusId;
        private uint mouseVolumeId;
        private uint gammaDefaultId;
        private uint mouseDefaultId;

        // Mute button IDs
        private uint muteMasterId;
        private uint muteBGMId;
        private uint muteSFXId;
        private uint muteMasterTickedId;
        private uint muteBGMTickedId;
        private uint muteSFXTickedId;

        // Visible positions
        private Vector3 popupVisiblePos = new Vector3(640.0f, 375.0f, -0.5f);
        private Vector3 closeButtonVisiblePos = new Vector3(801.4f, 63.6f, -0.6f);
        private Vector3 plusButton1VisiblePos = new Vector3(598.0f, 423.9f, -0.6f);
        private Vector3 plusButton2VisiblePos = new Vector3(598.0f, 495.0f, -0.6f);
        private Vector3 plusButton3VisiblePos = new Vector3(598.0f, 567.3f, -0.6f);
        private Vector3 minusButton1VisiblePos = new Vector3(565.0f, 424.2f, -0.6f);
        private Vector3 minusButton2VisiblePos = new Vector3(565.0f, 495.2f, -0.6f);
        private Vector3 minusButton3VisiblePos = new Vector3(565.0f, 566.3f, -0.6f);
        private Vector3 volumeFill1VisiblePos = new Vector3(359.4f, 423.0f, -0.6f);
        private Vector3 volumeFill2VisiblePos = new Vector3(359.4f, 494.0f, -0.6f);
        private Vector3 volumeFill3VisiblePos = new Vector3(359.4f, 565.0f, -0.6f);
        private Vector3 GammaMinus = new Vector3(565.3f, 161.3f, -0.6f);
        private Vector3 GammaPlus = new Vector3(598.7f, 161.3f, -0.6f);
        private Vector3 GammaVolume = new Vector3(359.4f, 160.5f, -0.6f);
        private Vector3 GammaDefault = new Vector3(225.9f, 199.5f, -0.6f);
        private Vector3 MouseMinus = new Vector3(564.8f, 269.8f, -0.6f);
        private Vector3 MousePlus = new Vector3(598.2f, 269.3f, -0.6f);
        private Vector3 MouseVolume = new Vector3(359.4f, 269.3f, -0.6f);
        private Vector3 MouseDefault = new Vector3(226.2f, 306.8f, -0.6f);

        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID = "Settings";

        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;

        private float volumeFill1InitialWidth;
        private float volumeFill2InitialWidth;
        private float volumeFill3InitialWidth;
        private Vector3 volumeFill1InitialPosition;
        private Vector3 volumeFill2InitialPosition;
        private Vector3 volumeFill3InitialPosition;
        private float gammaVolumeInitialWidth;
        private Vector3 gammaVolumeInitialPosition;
        private float mouseVolumeInitialWidth;
        private Vector3 mouseVolumeInitialPosition;

        public override void OnStart()
        {
            LogMessage("SettingsPopup: Initializing...");

            settingsButtonId = SceneFindEntityByName(SETTINGS_BUTTON_NAME);
            if (settingsButtonId == 0) { LogError("SettingsPopup: Could not find entity: " + SETTINGS_BUTTON_NAME); return; }

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
            gammaPlusId = SceneFindEntityByName("GammaPlus");
            gammaMinusId = SceneFindEntityByName("GammaMinus");
            gammaVolumeId = SceneFindEntityByName("GammaVolume");
            mousePlusId = SceneFindEntityByName("MousePlus");
            mouseMinusId = SceneFindEntityByName("MouseMinus");
            mouseVolumeId = SceneFindEntityByName("MouseVolume");
            gammaDefaultId = SceneFindEntityByName("GammaDefault");
            mouseDefaultId = SceneFindEntityByName("MouseDefault");

            // Mute buttons
            muteMasterId = SceneFindEntityByName(MUTE_MASTER_UNTICKED_NAME);
            muteBGMId = SceneFindEntityByName(MUTE_BGM_UNTICKED_NAME);
            muteSFXId = SceneFindEntityByName(MUTE_SFX_UNTICKED_NAME);
            muteMasterTickedId = SceneFindEntityByName(MUTE_MASTER_TICKED_NAME);
            muteBGMTickedId = SceneFindEntityByName(MUTE_BGM_TICKED_NAME);
            muteSFXTickedId = SceneFindEntityByName(MUTE_SFX_TICKED_NAME);

            if (settingsPopupId == 0) LogError("SettingsPopup: Could not find: " + SETTINGS_POPUP_NAME);
            if (closeButtonId == 0) LogError("SettingsPopup: Could not find: " + CLOSE_BUTTON_NAME);

            if (volumeFill1Id != 0) { volumeFill1InitialWidth = GetScale(volumeFill1Id).X; volumeFill1InitialPosition = volumeFill1VisiblePos; LogMessage("SettingsPopup: Volume Fill 1 initial width = " + volumeFill1InitialWidth); }
            if (volumeFill2Id != 0) { volumeFill2InitialWidth = GetScale(volumeFill2Id).X; volumeFill2InitialPosition = volumeFill2VisiblePos; LogMessage("SettingsPopup: Volume Fill 2 initial width = " + volumeFill2InitialWidth); }
            if (volumeFill3Id != 0) { volumeFill3InitialWidth = GetScale(volumeFill3Id).X; volumeFill3InitialPosition = volumeFill3VisiblePos; LogMessage("SettingsPopup: Volume Fill 3 initial width = " + volumeFill3InitialWidth); }
            if (gammaVolumeId != 0) { gammaVolumeInitialWidth = GetScale(gammaVolumeId).X; gammaVolumeInitialPosition = GammaVolume; }
            if (mouseVolumeId != 0) { mouseVolumeInitialWidth = GetScale(mouseVolumeId).X; mouseVolumeInitialPosition = MouseVolume; }

            // Hide all mute buttons on start
            if (muteMasterId != 0) SetIsVisible(muteMasterId, false);
            if (muteBGMId != 0) SetIsVisible(muteBGMId, false);
            if (muteSFXId != 0) SetIsVisible(muteSFXId, false);
            if (muteMasterTickedId != 0) SetIsVisible(muteMasterTickedId, false);
            if (muteBGMTickedId != 0) SetIsVisible(muteBGMTickedId, false);
            if (muteSFXTickedId != 0) SetIsVisible(muteSFXTickedId, false);

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;

            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            HidePopup();
            LogMessage("SettingsPopup: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound) return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed) HandleMouseClick();
        }

        private void HandleMouseClick()
        {
            if (isPopupVisible)
            {
                if (closeButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(closeButtonId))
                {
                    LogMessage("SettingsPopup: Close button clicked");
                    HidePopup();
                    return;
                }

                if (AudioSettings.Instance == null)
                {
                    LogMessage("SettingsPopup: AudioSettings not ready yet");
                    if (settingsPopupId != 0 && !Collision2D.IsMouseCollidingWithEntity(settingsPopupId))
                        HidePopup();
                    return;
                }

                // ===== Master Volume =====
                if (plusButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(plusButton1Id))
                {
                    AudioSettings.Instance.SetMasterVolume(AudioSettings.Instance.GetMasterVolume() + 0.1f);
                    UpdateVolumeFillVisual(volumeFill1Id, AudioSettings.Instance.GetMasterVolume(), volumeFill1InitialWidth, volumeFill1InitialPosition);
                    LogMessage("SettingsPopup: Master Volume + (Now: " + AudioSettings.Instance.GetMasterVolume().ToString("F2") + ")");
                    return;
                }
                if (minusButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(minusButton1Id))
                {
                    AudioSettings.Instance.SetMasterVolume(AudioSettings.Instance.GetMasterVolume() - 0.1f);
                    UpdateVolumeFillVisual(volumeFill1Id, AudioSettings.Instance.GetMasterVolume(), volumeFill1InitialWidth, volumeFill1InitialPosition);
                    LogMessage("SettingsPopup: Master Volume - (Now: " + AudioSettings.Instance.GetMasterVolume().ToString("F2") + ")");
                    return;
                }

                // ===== BGM Volume =====
                if (plusButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(plusButton2Id))
                {
                    AudioSettings.Instance.SetBGMVolume(AudioSettings.Instance.GetBGMVolume() + 0.1f);
                    UpdateVolumeFillVisual(volumeFill2Id, AudioSettings.Instance.GetBGMVolume(), volumeFill2InitialWidth, volumeFill2InitialPosition);
                    LogMessage("SettingsPopup: BGM Volume + (Now: " + AudioSettings.Instance.GetBGMVolume().ToString("F2") + ")");
                    return;
                }
                if (minusButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(minusButton2Id))
                {
                    AudioSettings.Instance.SetBGMVolume(AudioSettings.Instance.GetBGMVolume() - 0.1f);
                    UpdateVolumeFillVisual(volumeFill2Id, AudioSettings.Instance.GetBGMVolume(), volumeFill2InitialWidth, volumeFill2InitialPosition);
                    LogMessage("SettingsPopup: BGM Volume - (Now: " + AudioSettings.Instance.GetBGMVolume().ToString("F2") + ")");
                    return;
                }

                // ===== SFX Volume =====
                if (plusButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(plusButton3Id))
                {
                    AudioSettings.Instance.SetSFXVolume(AudioSettings.Instance.GetSFXVolume() + 0.1f);
                    UpdateVolumeFillVisual(volumeFill3Id, AudioSettings.Instance.GetSFXVolume(), volumeFill3InitialWidth, volumeFill3InitialPosition);
                    LogMessage("SettingsPopup: SFX Volume + (Now: " + AudioSettings.Instance.GetSFXVolume().ToString("F2") + ")");
                    return;
                }
                if (minusButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(minusButton3Id))
                {
                    AudioSettings.Instance.SetSFXVolume(AudioSettings.Instance.GetSFXVolume() - 0.1f);
                    UpdateVolumeFillVisual(volumeFill3Id, AudioSettings.Instance.GetSFXVolume(), volumeFill3InitialWidth, volumeFill3InitialPosition);
                    LogMessage("SettingsPopup: SFX Volume - (Now: " + AudioSettings.Instance.GetSFXVolume().ToString("F2") + ")");
                    return;
                }

                // ===== Mute Buttons =====
                if ((muteMasterId != 0 && Collision2D.IsMouseCollidingWithEntity(muteMasterId)) ||
                    (muteMasterTickedId != 0 && Collision2D.IsMouseCollidingWithEntity(muteMasterTickedId)))
                {
                    AudioSettings.Instance.ToggleMasterMute();
                    bool muted = AudioSettings.Instance.IsMasterMuted();
                    if (muteMasterId != 0) SetIsVisible(muteMasterId, !muted);
                    if (muteMasterTickedId != 0) SetIsVisible(muteMasterTickedId, muted);
                    LogMessage("SettingsPopup: Master Mute toggled (Now: " + muted + ")");
                    return;
                }
                if ((muteBGMId != 0 && Collision2D.IsMouseCollidingWithEntity(muteBGMId)) ||
                    (muteBGMTickedId != 0 && Collision2D.IsMouseCollidingWithEntity(muteBGMTickedId)))
                {
                    AudioSettings.Instance.ToggleBGMMute();
                    bool muted = AudioSettings.Instance.IsBGMMuted();
                    if (muteBGMId != 0) SetIsVisible(muteBGMId, !muted);
                    if (muteBGMTickedId != 0) SetIsVisible(muteBGMTickedId, muted);
                    LogMessage("SettingsPopup: BGM Mute toggled (Now: " + muted + ")");
                    return;
                }
                if ((muteSFXId != 0 && Collision2D.IsMouseCollidingWithEntity(muteSFXId)) ||
                    (muteSFXTickedId != 0 && Collision2D.IsMouseCollidingWithEntity(muteSFXTickedId)))
                {
                    AudioSettings.Instance.ToggleSFXMute();
                    bool muted = AudioSettings.Instance.IsSFXMuted();
                    if (muteSFXId != 0) SetIsVisible(muteSFXId, !muted);
                    if (muteSFXTickedId != 0) SetIsVisible(muteSFXTickedId, muted);
                    LogMessage("SettingsPopup: SFX Mute toggled (Now: " + muted + ")");
                    return;
                }

                // ===== Gamma =====
                if (gammaPlusId != 0 && Collision2D.IsMouseCollidingWithEntity(gammaPlusId))
                {
                    AudioSettings.Instance.IncrementGamma();
                    UpdateVolumeFillVisual(gammaVolumeId, AudioSettings.Instance.GetGammaNormalized(), gammaVolumeInitialWidth, gammaVolumeInitialPosition);
                    LogMessage("SettingsPopup: Gamma + (Now: " + AudioSettings.Instance.GetGamma().ToString("F1") + ")");
                    return;
                }
                if (gammaMinusId != 0 && Collision2D.IsMouseCollidingWithEntity(gammaMinusId))
                {
                    AudioSettings.Instance.DecrementGamma();
                    UpdateVolumeFillVisual(gammaVolumeId, AudioSettings.Instance.GetGammaNormalized(), gammaVolumeInitialWidth, gammaVolumeInitialPosition);
                    LogMessage("SettingsPopup: Gamma - (Now: " + AudioSettings.Instance.GetGamma().ToString("F1") + ")");
                    return;
                }
                if (gammaDefaultId != 0 && Collision2D.IsMouseCollidingWithEntity(gammaDefaultId))
                {
                    AudioSettings.Instance.ResetGamma();
                    UpdateVolumeFillVisual(gammaVolumeId, AudioSettings.Instance.GetGammaNormalized(), gammaVolumeInitialWidth, gammaVolumeInitialPosition);
                    LogMessage("SettingsPopup: Gamma reset to default");
                    return;
                }

                // ===== Mouse Sensitivity =====
                if (mousePlusId != 0 && Collision2D.IsMouseCollidingWithEntity(mousePlusId))
                {
                    AudioSettings.Instance.SetMouseSensitivityUp();
                    UpdateVolumeFillVisual(mouseVolumeId, AudioSettings.Instance.GetMouseSensitivityNormalized(), mouseVolumeInitialWidth, mouseVolumeInitialPosition);
                    LogMessage("SettingsPopup: Mouse Sensitivity + (Now: " + AudioSettings.Instance.GetMouseSensitivity().ToString("F2") + ")");
                    return;
                }
                if (mouseMinusId != 0 && Collision2D.IsMouseCollidingWithEntity(mouseMinusId))
                {
                    AudioSettings.Instance.SetMouseSensitivityDown();
                    UpdateVolumeFillVisual(mouseVolumeId, AudioSettings.Instance.GetMouseSensitivityNormalized(), mouseVolumeInitialWidth, mouseVolumeInitialPosition);
                    LogMessage("SettingsPopup: Mouse Sensitivity - (Now: " + AudioSettings.Instance.GetMouseSensitivity().ToString("F2") + ")");
                    return;
                }
                if (mouseDefaultId != 0 && Collision2D.IsMouseCollidingWithEntity(mouseDefaultId))
                {
                    AudioSettings.Instance.ResetMouseSensitivity();
                    UpdateVolumeFillVisual(mouseVolumeId, AudioSettings.Instance.GetMouseSensitivityNormalized(), mouseVolumeInitialWidth, mouseVolumeInitialPosition);
                    LogMessage("SettingsPopup: Mouse Sensitivity reset to default");
                    return;
                }

                // Clicked outside popup - close
                if (settingsPopupId != 0 && !Collision2D.IsMouseCollidingWithEntity(settingsPopupId))
                {
                    LogMessage("SettingsPopup: Clicked outside - closing");
                    HidePopup();
                }
            }
            else
            {
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
            if (isPopupVisible) return;

            isPopupVisible = true;
            Event.Publish(EVENT_POPUP_OPENED, POPUP_ID);

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
            if (gammaPlusId != 0) SetPosition(gammaPlusId, ref GammaPlus);
            if (gammaMinusId != 0) SetPosition(gammaMinusId, ref GammaMinus);
            if (gammaVolumeId != 0) SetPosition(gammaVolumeId, ref GammaVolume);
            if (mousePlusId != 0) SetPosition(mousePlusId, ref MousePlus);
            if (mouseMinusId != 0) SetPosition(mouseMinusId, ref MouseMinus);
            if (mouseVolumeId != 0) SetPosition(mouseVolumeId, ref MouseVolume);
            if (gammaDefaultId != 0) SetPosition(gammaDefaultId, ref GammaDefault);
            if (mouseDefaultId != 0) SetPosition(mouseDefaultId, ref MouseDefault);

            if (AudioSettings.Instance != null)
            {
                UpdateVolumeFillVisual(volumeFill1Id, AudioSettings.Instance.GetMasterVolume(), volumeFill1InitialWidth, volumeFill1InitialPosition);
                UpdateVolumeFillVisual(volumeFill2Id, AudioSettings.Instance.GetBGMVolume(), volumeFill2InitialWidth, volumeFill2InitialPosition);
                UpdateVolumeFillVisual(volumeFill3Id, AudioSettings.Instance.GetSFXVolume(), volumeFill3InitialWidth, volumeFill3InitialPosition);
                UpdateVolumeFillVisual(gammaVolumeId, AudioSettings.Instance.GetGammaNormalized(), gammaVolumeInitialWidth, gammaVolumeInitialPosition);
                UpdateVolumeFillVisual(mouseVolumeId, AudioSettings.Instance.GetMouseSensitivityNormalized(), mouseVolumeInitialWidth, mouseVolumeInitialPosition);

                // Show correct mute sprite based on current mute state
                bool masterMuted = AudioSettings.Instance.IsMasterMuted();
                bool bgmMuted = AudioSettings.Instance.IsBGMMuted();
                bool sfxMuted = AudioSettings.Instance.IsSFXMuted();

                if (muteMasterId != 0) SetIsVisible(muteMasterId, !masterMuted);
                if (muteMasterTickedId != 0) SetIsVisible(muteMasterTickedId, masterMuted);
                if (muteBGMId != 0) SetIsVisible(muteBGMId, !bgmMuted);
                if (muteBGMTickedId != 0) SetIsVisible(muteBGMTickedId, bgmMuted);
                if (muteSFXId != 0) SetIsVisible(muteSFXId, !sfxMuted);
                if (muteSFXTickedId != 0) SetIsVisible(muteSFXTickedId, sfxMuted);
            }

            LogMessage("SettingsPopup: Popup shown");
        }

        private void HidePopup()
        {
            bool wasVisible = isPopupVisible;
            isPopupVisible = false;
            if (wasVisible) Event.Publish(EVENT_POPUP_CLOSED, POPUP_ID);

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
            if (gammaPlusId != 0) SetPosition(gammaPlusId, ref hidePos2);
            if (gammaMinusId != 0) SetPosition(gammaMinusId, ref hidePos2);
            if (gammaVolumeId != 0) SetPosition(gammaVolumeId, ref hidePos2);
            if (mousePlusId != 0) SetPosition(mousePlusId, ref hidePos2);
            if (mouseMinusId != 0) SetPosition(mouseMinusId, ref hidePos2);
            if (mouseVolumeId != 0) SetPosition(mouseVolumeId, ref hidePos2);
            if (gammaDefaultId != 0) SetPosition(gammaDefaultId, ref hidePos2);
            if (mouseDefaultId != 0) SetPosition(mouseDefaultId, ref hidePos2);

            // Hide all mute buttons
            if (muteMasterId != 0) SetIsVisible(muteMasterId, false);
            if (muteBGMId != 0) SetIsVisible(muteBGMId, false);
            if (muteSFXId != 0) SetIsVisible(muteSFXId, false);
            if (muteMasterTickedId != 0) SetIsVisible(muteMasterTickedId, false);
            if (muteBGMTickedId != 0) SetIsVisible(muteBGMTickedId, false);
            if (muteSFXTickedId != 0) SetIsVisible(muteSFXTickedId, false);

            LogMessage("SettingsPopup: Popup hidden");
        }

        private void UpdateVolumeFillVisual(uint volumeFillId, float volume, float initialWidth, Vector3 initialPosition)
        {
            LogMessage("UpdateVolumeFillVisual called: volumeFillId=" + volumeFillId + " volume=" + volume + " initialWidth=" + initialWidth);

            if (volumeFillId == 0) { LogMessage("UpdateVolumeFillVisual: volumeFillId is 0, returning"); return; }

            if (volume < 0.0f) volume = 0.0f;
            if (volume > 1.0f) volume = 1.0f;

            float newWidth = initialWidth * volume;
            Vector3 currentScale = GetScale(volumeFillId);
            Vector3 newScale = new Vector3(newWidth, currentScale.Y, currentScale.Z);
            SetScale(volumeFillId, ref newScale);

            float widthDiff = initialWidth - newWidth;
            Vector3 newPosition = new Vector3(initialPosition.X - widthDiff, initialPosition.Y, initialPosition.Z);
            SetPosition(volumeFillId, ref newPosition);

            LogMessage("UpdateVolumeFillVisual: width=" + newWidth + " posX=" + newPosition.X);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("SettingsPopup: Destroyed");
        }
    }
}

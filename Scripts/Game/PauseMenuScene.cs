using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.AudioManager;
using static Engine.SpriteRenderer;
using static Game.AudioSettings;

namespace Game
{
    /// <summary>
    /// Handles the pause menu scene buttons and navigation.
    /// This script lives in the PauseMenu.json scene.
    /// </summary>
    public class PauseMenuScene : ScriptBehaviour
    {
        // Button entity names
        private const string RESUME_BUTTON_NAME = "Paused_ResumeButton";
        private const string RESUME_BUTTON_HOVERED_NAME = "Paused_ResumeButton_Hovered";
        private const string RESTART_BUTTON_NAME = "Paused_RestartButton";
        private const string RESTART_BUTTON_HOVERED_NAME = "Paused_RestartButton_Hovered";
        private const string MAINMENU_BUTTON_NAME = "Paused_MainMenuButton";
        private const string MAINMENU_BUTTON_HOVERED_NAME = "Paused_MainMenuButton_Hovered";
        private const string HOWTOPLAY_BUTTON_NAME = "Paused_HowToPlayButton";
        private const string HOWTOPLAY_BUTTON_HOVERED_NAME = "Paused_HowToPlayButton_Hovered";

        // Volume controls
        private const string PLUS_BUTTON_NAME = "Paused_PlusButton";
        private const string PLUS_BUTTON_HOVERED_NAME = "Paused_PlusButton_Hovered";
        private const string MIXER_FILL_NAME = "Paused_MixerFill";
        private const string MINUS_BUTTON_NAME = "Paused_MinusButton";
        private const string MINUS_BUTTON_HOVERED_NAME = "Paused_MinusButton_Hovered";

        // Mixer Set 2 (BGM)
        private const string MIXER_FILL_2_NAME = "Paused_MixerFill_2";
        private const string PLUS_BUTTON_2_NAME = "Paused_PlusButton_2";
        private const string PLUS_BUTTON_2_HOVERED_NAME = "Paused_PlusButton_2_Hovered";
        private const string MINUS_BUTTON_2_NAME = "Paused_MinusButton_2";
        private const string MINUS_BUTTON_2_HOVERED_NAME = "Paused_MinusButton_2_Hovered";

        // Mixer Set 3 (SFX)
        private const string MIXER_FILL_3_NAME = "Paused_MixerFill_3";
        private const string PLUS_BUTTON_3_NAME = "Paused_PlusButton_3";
        private const string PLUS_BUTTON_3_HOVERED_NAME = "Paused_PlusButton_3_Hovered";
        private const string MINUS_BUTTON_3_NAME = "Paused_MinusButton_3";
        private const string MINUS_BUTTON_3_HOVERED_NAME = "Paused_MinusButton_3_Hovered";

        // Checkboxes
        private const string CHECKBOX_MASTER_UNTICKED_NAME = "Paused_Checkbox_Master_Unticked";
        private const string CHECKBOX_MASTER_TICKED_NAME = "Paused_Checkbox_Master_Ticked";
        private const string CHECKBOX_BGM_UNTICKED_NAME = "Paused_Checkbox_BGM_Unticked";
        private const string CHECKBOX_BGM_TICKED_NAME = "Paused_Checkbox_BGM_Ticked";
        private const string CHECKBOX_SFX_UNTICKED_NAME = "Paused_Checkbox_SFX_Unticked";
        private const string CHECKBOX_SFX_TICKED_NAME = "Paused_Checkbox_SFX_Ticked";

        // Scene paths
        private const string MAIN_MENU_SCENE_PATH = "Resources/Sources/Scenes/MainMenu.json";

        // Screen positions
        private const float CENTER_X = 640.0f;
        private const float HIDDEN_Y = -500.0f;

        // Checkbox positions
        private Vector3 checkboxMasterVisiblePos = new Vector3(833.8f, 620.0f, 0.0f);  
        private Vector3 checkboxBGMVisiblePos = new Vector3(987.1f, 620.0f, 0.0f);     
        private Vector3 checkboxSFXVisiblePos = new Vector3(1143.5f, 620.0f, 0.0f);

        // Entity IDs - Buttons
        private uint resumeButtonId;
        private uint resumeButtonHoveredId;
        private uint restartButtonId;
        private uint restartButtonHoveredId;
        private uint mainMenuButtonId;
        private uint mainMenuButtonHoveredId;
        private uint howToPlayButtonId;
        private uint howToPlayButtonHoveredId;

        // Entity IDs - Volume Controls Set 1 (Master)
        private uint plusButtonId;
        private uint plusButtonHoveredId;
        private uint mixerFillId;
        private uint minusButtonId;
        private uint minusButtonHoveredId;

        // Entity IDs - Volume Controls Set 2 (BGM)
        private uint mixerFillId2;
        private uint plusButtonId2;
        private uint plusButtonHoveredId2;
        private uint minusButtonId2;
        private uint minusButtonHoveredId2;

        // Entity IDs - Volume Controls Set 3 (SFX)
        private uint mixerFillId3;
        private uint plusButtonId3;
        private uint plusButtonHoveredId3;
        private uint minusButtonId3;
        private uint minusButtonHoveredId3;

        // Entity IDs - Checkboxes
        private uint checkboxMasterUntickedId;
        private uint checkboxMasterTickedId;
        private uint checkboxBGMUntickedId;
        private uint checkboxBGMTickedId;
        private uint checkboxSFXUntickedId;
        private uint checkboxSFXTickedId;

        // Mixer visual state
        private float mixerFill1InitialWidth;
        private float mixerFill2InitialWidth;
        private float mixerFill3InitialWidth;
        private Vector3 mixerFill1InitialPosition;
        private Vector3 mixerFill2InitialPosition;
        private Vector3 mixerFill3InitialPosition;

        // State
        private bool wasMousePressed = false;
        private string returnGameScenePath = "";

        public override void OnStart()
        {
            LogMessage("PauseMenuScene: Initializing...");

            // Find button entities
            resumeButtonId = SceneFindEntityByName(RESUME_BUTTON_NAME);
            resumeButtonHoveredId = SceneFindEntityByName(RESUME_BUTTON_HOVERED_NAME);
            restartButtonId = SceneFindEntityByName(RESTART_BUTTON_NAME);
            restartButtonHoveredId = SceneFindEntityByName(RESTART_BUTTON_HOVERED_NAME);
            mainMenuButtonId = SceneFindEntityByName(MAINMENU_BUTTON_NAME);
            mainMenuButtonHoveredId = SceneFindEntityByName(MAINMENU_BUTTON_HOVERED_NAME);
            howToPlayButtonId = SceneFindEntityByName(HOWTOPLAY_BUTTON_NAME);
            howToPlayButtonHoveredId = SceneFindEntityByName(HOWTOPLAY_BUTTON_HOVERED_NAME);

            // Find volume control entities - Master
            plusButtonId = SceneFindEntityByName(PLUS_BUTTON_NAME);
            plusButtonHoveredId = SceneFindEntityByName(PLUS_BUTTON_HOVERED_NAME);
            mixerFillId = SceneFindEntityByName(MIXER_FILL_NAME);
            minusButtonId = SceneFindEntityByName(MINUS_BUTTON_NAME);
            minusButtonHoveredId = SceneFindEntityByName(MINUS_BUTTON_HOVERED_NAME);

            // Find volume control entities - BGM (Set 2)
            mixerFillId2 = SceneFindEntityByName(MIXER_FILL_2_NAME);
            plusButtonId2 = SceneFindEntityByName(PLUS_BUTTON_2_NAME);
            plusButtonHoveredId2 = SceneFindEntityByName(PLUS_BUTTON_2_HOVERED_NAME);
            minusButtonId2 = SceneFindEntityByName(MINUS_BUTTON_2_NAME);
            minusButtonHoveredId2 = SceneFindEntityByName(MINUS_BUTTON_2_HOVERED_NAME);

            // Find volume control entities - SFX (Set 3)
            mixerFillId3 = SceneFindEntityByName(MIXER_FILL_3_NAME);
            plusButtonId3 = SceneFindEntityByName(PLUS_BUTTON_3_NAME);
            plusButtonHoveredId3 = SceneFindEntityByName(PLUS_BUTTON_3_HOVERED_NAME);
            minusButtonId3 = SceneFindEntityByName(MINUS_BUTTON_3_NAME);
            minusButtonHoveredId3 = SceneFindEntityByName(MINUS_BUTTON_3_HOVERED_NAME);

            // Find checkbox entities
            checkboxMasterUntickedId = SceneFindEntityByName(CHECKBOX_MASTER_UNTICKED_NAME);
            checkboxMasterTickedId = SceneFindEntityByName(CHECKBOX_MASTER_TICKED_NAME);
            checkboxBGMUntickedId = SceneFindEntityByName(CHECKBOX_BGM_UNTICKED_NAME);
            checkboxBGMTickedId = SceneFindEntityByName(CHECKBOX_BGM_TICKED_NAME);
            checkboxSFXUntickedId = SceneFindEntityByName(CHECKBOX_SFX_UNTICKED_NAME);
            checkboxSFXTickedId = SceneFindEntityByName(CHECKBOX_SFX_TICKED_NAME);

            // Store initial mixer fill states
            if (mixerFillId != 0)
            {
                Vector3 scale1 = GetScale(mixerFillId);
                mixerFill1InitialWidth = scale1.Y;
                mixerFill1InitialPosition = GetPosition(mixerFillId);
                LogMessage("PauseMenuScene: Mixer 1 initial height = " + mixerFill1InitialWidth);
            }

            if (mixerFillId2 != 0)
            {
                Vector3 scale2 = GetScale(mixerFillId2);
                mixerFill2InitialWidth = scale2.Y;
                mixerFill2InitialPosition = GetPosition(mixerFillId2);
                LogMessage("PauseMenuScene: Mixer 2 initial height = " + mixerFill2InitialWidth);
            }

            if (mixerFillId3 != 0)
            {
                Vector3 scale3 = GetScale(mixerFillId3);
                mixerFill3InitialWidth = scale3.Y;
                mixerFill3InitialPosition = GetPosition(mixerFillId3);
                LogMessage("PauseMenuScene: Mixer 3 initial height = " + mixerFill3InitialWidth);
            }

            // Subscribe to pause event to get the game scene path
            Event.Subscribe("GamePaused", OnGamePaused);

            // Ensure cursor is visible
            Input.SetCursorVisible(true);

            // Update all visuals based on current audio settings
            if (Instance != null)
            {
                UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                                     mixerFill1InitialWidth, mixerFill1InitialPosition);
                UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                     mixerFill2InitialWidth, mixerFill2InitialPosition);
                UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                                     mixerFill3InitialWidth, mixerFill3InitialPosition);
                UpdateCheckboxVisuals();
            }

            LogMessage("PauseMenuScene: Ready!");
        }

        private void OnGamePaused(string eventName, string payload)
        {
            // Store which game scene we came from
            returnGameScenePath = payload;
            LogMessage("PauseMenuScene: Received game scene path: " + returnGameScenePath);
        }

        public override void OnUpdate(float deltaTime)
        {
            HandleHoverStates();
            HandleMouseClick();
        }

        private void HandleHoverStates()
        {
            // Button hovers
            UpdateButtonHover(resumeButtonId, resumeButtonHoveredId);
            UpdateButtonHover(restartButtonId, restartButtonHoveredId);
            UpdateButtonHover(mainMenuButtonId, mainMenuButtonHoveredId);
            UpdateButtonHover(howToPlayButtonId, howToPlayButtonHoveredId);

            // Volume control hovers - Master
            UpdateButtonHover(plusButtonId, plusButtonHoveredId);
            UpdateButtonHover(minusButtonId, minusButtonHoveredId);

            // Volume control hovers - BGM
            UpdateButtonHover(plusButtonId2, plusButtonHoveredId2);
            UpdateButtonHover(minusButtonId2, minusButtonHoveredId2);

            // Volume control hovers - SFX
            UpdateButtonHover(plusButtonId3, plusButtonHoveredId3);
            UpdateButtonHover(minusButtonId3, minusButtonHoveredId3);
        }

        private void UpdateButtonHover(uint normalId, uint hoveredId)
        {
            if (normalId == 0 || hoveredId == 0) return;

            bool isHovered = Collision2D.IsMouseCollidingWithEntity(normalId) ||
                            Collision2D.IsMouseCollidingWithEntity(hoveredId);

            if (isHovered)
            {
                SetColor(normalId, 1.0f, 1.0f, 1.0f, 0.0f); // Hide normal
                SetColor(hoveredId, 1.0f, 1.0f, 1.0f, 1.0f); // Show hovered
            }
            else
            {
                SetColor(normalId, 1.0f, 1.0f, 1.0f, 1.0f); // Show normal
                SetColor(hoveredId, 1.0f, 1.0f, 1.0f, 0.0f); // Hide hovered
            }
        }

        private void HandleMouseClick()
        {
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (!mouseJustPressed) return;

            // Resume button - return to game
            if (IsButtonClicked(resumeButtonId, resumeButtonHoveredId))
            {
                LogMessage("PauseMenuScene: Resume clicked");
                ResumeGame();
                return;
            }

            // Restart button - reload game scene
            if (IsButtonClicked(restartButtonId, restartButtonHoveredId))
            {
                LogMessage("PauseMenuScene: Restart clicked");
                RestartGame();
                return;
            }

            // Main Menu button
            if (IsButtonClicked(mainMenuButtonId, mainMenuButtonHoveredId))
            {
                LogMessage("PauseMenuScene: Main Menu clicked");
                GoToMainMenu();
                return;
            }

            // How To Play button
            if (IsButtonClicked(howToPlayButtonId, howToPlayButtonHoveredId))
            {
                LogMessage("PauseMenuScene: How To Play clicked");
                // TODO: Implement how to play
                return;
            }

            // Master Volume Controls
            if (IsButtonClicked(plusButtonId, plusButtonHoveredId))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetMasterVolume();
                    Instance.SetMasterVolume(currentVolume + 0.111f);
                    LogMessage("PauseMenuScene: Master Volume + (Now: " + Instance.GetMasterVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                                         mixerFill1InitialWidth, mixerFill1InitialPosition);
                }
                return;
            }

            if (IsButtonClicked(minusButtonId, minusButtonHoveredId))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetMasterVolume();
                    Instance.SetMasterVolume(currentVolume - 0.111f);
                    LogMessage("PauseMenuScene: Master Volume - (Now: " + Instance.GetMasterVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                                         mixerFill1InitialWidth, mixerFill1InitialPosition);
                }
                return;
            }

            // BGM Volume Controls
            if (IsButtonClicked(plusButtonId2, plusButtonHoveredId2))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetBGMVolume();
                    Instance.SetBGMVolume(currentVolume + 0.111f);
                    LogMessage("PauseMenuScene: BGM Volume + (Now: " + Instance.GetBGMVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                         mixerFill2InitialWidth, mixerFill2InitialPosition);
                }
                return;
            }

            if (IsButtonClicked(minusButtonId2, minusButtonHoveredId2))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetBGMVolume();
                    Instance.SetBGMVolume(currentVolume - 0.111f);
                    LogMessage("PauseMenuScene: BGM Volume - (Now: " + Instance.GetBGMVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                         mixerFill2InitialWidth, mixerFill2InitialPosition);
                }
                return;
            }

            // SFX Volume Controls
            if (IsButtonClicked(plusButtonId3, plusButtonHoveredId3))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetSFXVolume();
                    Instance.SetSFXVolume(currentVolume + 0.111f);
                    LogMessage("PauseMenuScene: SFX Volume + (Now: " + Instance.GetSFXVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                                         mixerFill3InitialWidth, mixerFill3InitialPosition);
                }
                return;
            }

            if (IsButtonClicked(minusButtonId3, minusButtonHoveredId3))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetSFXVolume();
                    Instance.SetSFXVolume(currentVolume - 0.111f);
                    LogMessage("PauseMenuScene: SFX Volume - (Now: " + Instance.GetSFXVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                                         mixerFill3InitialWidth, mixerFill3InitialPosition);
                }
                return;
            }

            // Checkbox Handling
            // Master checkbox
            if (IsCheckboxClicked(checkboxMasterUntickedId, checkboxMasterTickedId))
            {
                if (Instance != null)
                {
                    Instance.ToggleMasterMute();
                    bool isMuted = Instance.IsMasterMuted();
                    LogMessage("PauseMenuScene: Master Mute toggled to: " + isMuted);
                    UpdateCheckboxVisuals();
                }
                return;
            }

            // BGM checkbox
            if (IsCheckboxClicked(checkboxBGMUntickedId, checkboxBGMTickedId))
            {
                if (Instance != null)
                {
                    Instance.ToggleBGMMute();
                    bool isMuted = Instance.IsBGMMuted();
                    LogMessage("PauseMenuScene: BGM Mute toggled to: " + isMuted);
                    UpdateCheckboxVisuals();
                }
                return;
            }

            // SFX checkbox
            if (IsCheckboxClicked(checkboxSFXUntickedId, checkboxSFXTickedId))
            {
                if (Instance != null)
                {
                    Instance.ToggleSFXMute();
                    bool isMuted = Instance.IsSFXMuted();
                    LogMessage("PauseMenuScene: SFX Mute toggled to: " + isMuted);
                    UpdateCheckboxVisuals();
                }
                return;
            }
        }

        private bool IsButtonClicked(uint normalId, uint hoveredId)
        {
            return (normalId != 0 && Collision2D.IsMouseCollidingWithEntity(normalId)) ||
                   (hoveredId != 0 && Collision2D.IsMouseCollidingWithEntity(hoveredId));
        }

        private bool IsCheckboxClicked(uint untickedId, uint tickedId)
        {
            return (untickedId != 0 && Collision2D.IsMouseCollidingWithEntity(untickedId)) ||
                   (tickedId != 0 && Collision2D.IsMouseCollidingWithEntity(tickedId));
        }

        /// <summary>
        /// Updates the mixer fill bar visual height based on volume (0.0 to 1.0)
        /// Adjusts height and position to keep bottom edge fixed
        /// </summary>
        private void UpdateMixerFillVisual(uint mixerFillId, float volume, float initialWidth, Vector3 initialPosition)
        {
            if (mixerFillId == 0) return;

            // Clamp volume to 0-1 range
            if (volume < 0.0f) volume = 0.0f;
            if (volume > 1.0f) volume = 1.0f;

            // Calculate new height based on volume
            float newHeight = initialWidth * volume;

            // Get current scale
            Vector3 currentScale = GetScale(mixerFillId);

            // Update scale with new height
            Vector3 newScale = new Vector3(
                currentScale.X,     // Keep width
                newHeight,          // Height based on volume
                currentScale.Z      // Keep depth
            );
            SetScale(mixerFillId, ref newScale);

            // Adjust position to keep BOTTOM edge fixed
            float heightDifference = initialWidth - newHeight;
            Vector3 newPosition = new Vector3(
                initialPosition.X,
                initialPosition.Y + heightDifference,
                initialPosition.Z
            );
            SetPosition(mixerFillId, ref newPosition);
        }

        private void UpdateCheckboxVisuals()
        {
            if (Instance == null) return;

            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, 0.0f);

            // Master checkbox
            bool masterMuted = Instance.IsMasterMuted();
            if (masterMuted)
            {
                SetPosition(checkboxMasterTickedId, ref checkboxMasterVisiblePos);
                SetPosition(checkboxMasterUntickedId, ref hidePos);
            }
            else
            {
                SetPosition(checkboxMasterUntickedId, ref checkboxMasterVisiblePos);
                SetPosition(checkboxMasterTickedId, ref hidePos);
            }

            // BGM checkbox
            bool bgmMuted = Instance.IsBGMMuted();
            if (bgmMuted)
            {
                SetPosition(checkboxBGMTickedId, ref checkboxBGMVisiblePos);
                SetPosition(checkboxBGMUntickedId, ref hidePos);
            }
            else
            {
                SetPosition(checkboxBGMUntickedId, ref checkboxBGMVisiblePos);
                SetPosition(checkboxBGMTickedId, ref hidePos);
            }

            // SFX checkbox
            bool sfxMuted = Instance.IsSFXMuted();
            if (sfxMuted)
            {
                SetPosition(checkboxSFXTickedId, ref checkboxSFXVisiblePos);
                SetPosition(checkboxSFXUntickedId, ref hidePos);
            }
            else
            {
                SetPosition(checkboxSFXUntickedId, ref checkboxSFXVisiblePos);
                SetPosition(checkboxSFXTickedId, ref hidePos);
            }
        }

        private void ResumeGame()
        {
            // Clear pause state
            GameState.IsPaused = false;

            // Hide cursor for gameplay
            Input.SetCursorVisible(false);

            // Publish event to resume game
            Event.Publish("ResumeGame", returnGameScenePath);

            // Load game scene
            if (!string.IsNullOrEmpty(returnGameScenePath))
            {
                bool success = Scene.SceneLoadFromFile(returnGameScenePath);
                if (success)
                {
                    LogMessage("PauseMenuScene: Returned to game successfully");
                }
            }
        }

        private void RestartGame()
        {
            // Clear pause state
            GameState.IsPaused = false;

            // Stop all audio
            StopGroup(AudioType.BGM);
            StopGroup(AudioType.SFX);

            // Hide cursor
            Input.SetCursorVisible(false);

            // Reload game scene
            if (!string.IsNullOrEmpty(returnGameScenePath))
            {
                bool success = Scene.SceneLoadFromFile(returnGameScenePath);
                if (success)
                {
                    LogMessage("PauseMenuScene: Game restarted successfully");
                }
            }
        }

        private void GoToMainMenu()
        {
            // Clear pause state
            GameState.IsPaused = false;

            // Stop all audio
            StopGroup(AudioType.BGM);
            StopGroup(AudioType.SFX);

            // Show cursor for menu
            Input.SetCursorVisible(true);

            // Load main menu
            bool success = Scene.SceneLoadFromFile(MAIN_MENU_SCENE_PATH);
            if (success)
            {
                LogMessage("PauseMenuScene: Returned to main menu successfully");
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe("GamePaused", OnGamePaused);
            LogMessage("PauseMenuScene: Destroyed");
        }
    }
}
using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.AudioManager;
using static Engine.SpriteRenderer;
using static Engine.Text;
using static Game.AudioSettings;

namespace Game
{
    /// <summary>
    /// Handles the pause menu popup during gameplay.
    /// Uses screen-space coordinates like other UI popups.
    /// Press P to toggle pause menu.
    /// </summary>
    public class PauseMenuPopup : ScriptBehaviour
    {
        // Entity names - must match tags in Level1_Player.json
        private const string BG_NAME = "PauseMenu_EngineBG";
        private const string RESUME_BUTTON_NAME = "Paused_ResumeButton";
        private const string RESUME_BUTTON_HOVERED_NAME = "Paused_ResumeButton_Hovered";
        private const string RESTART_BUTTON_NAME = "Paused_RestartButton";
        private const string RESTART_BUTTON_HOVERED_NAME = "Paused_RestartButton_Hovered";
        private const string PLUS_BUTTON_NAME = "Paused_PlusButton";
        private const string PLUS_BUTTON_HOVERED_NAME = "Paused_PlusButton_Hovered";
        private const string MIXER_FILL_NAME = "Paused_MixerFill";
        private const string MINUS_BUTTON_NAME = "Paused_MinusButton";
        private const string MINUS_BUTTON_HOVERED_NAME = "Paused_MinusButton_Hovered";
        private const string MAINMENU_BUTTON_NAME = "Paused_MainMenuButton";
        private const string MAINMENU_BUTTON_HOVERED_NAME = "Paused_MainMenuButton_Hovered";
        private const string HOWTOPLAY_BUTTON_NAME = "Paused_HowToPlayButton";
        private const string HOWTOPLAY_BUTTON_HOVERED_NAME = "Paused_HowToPlayButton_Hovered";
        // Mixer Set 2
        private const string MIXER_FILL_2_NAME = "Paused_MixerFill_2";
        private const string PLUS_BUTTON_2_NAME = "Paused_PlusButton_2";
        private const string PLUS_BUTTON_2_HOVERED_NAME = "Paused_PlusButton_2_Hovered";
        private const string MINUS_BUTTON_2_NAME = "Paused_MinusButton_2";
        private const string MINUS_BUTTON_2_HOVERED_NAME = "Paused_MinusButton_2_Hovered";
        // Mixer Set 3
        private const string MIXER_FILL_3_NAME = "Paused_MixerFill_3";
        private const string PLUS_BUTTON_3_NAME = "Paused_PlusButton_3";
        private const string PLUS_BUTTON_3_HOVERED_NAME = "Paused_PlusButton_3_Hovered";
        private const string MINUS_BUTTON_3_NAME = "Paused_MinusButton_3";
        private const string MINUS_BUTTON_3_HOVERED_NAME = "Paused_MinusButton_3_Hovered";

        // Checkbox entity names - Code Added by Rio
        // Master Checkbox
        private const string CHECKBOX_MASTER_UNTICKED_NAME = "Paused_Checkbox_Master_Unticked";
        private const string CHECKBOX_MASTER_TICKED_NAME = "Paused_Checkbox_Master_Ticked";
        // BGM Checkbox
        private const string CHECKBOX_BGM_UNTICKED_NAME = "Paused_Checkbox_BGM_Unticked";
        private const string CHECKBOX_BGM_TICKED_NAME = "Paused_Checkbox_BGM_Ticked";
        // SFX Checkbox 
        private const string CHECKBOX_SFX_UNTICKED_NAME = "Paused_Checkbox_SFX_Unticked";
        private const string CHECKBOX_SFX_TICKED_NAME = "Paused_Checkbox_SFX_Ticked";

        // HUD elements to hide when paused
        private static readonly string[] HUD_ELEMENT_NAMES = {
            "TImer",
            "StageCount",
            "scoreboard",
            "WeaponAmmoCount",
            "HealthCount",
            "CoreHealthCount",
            "HealthBarFill",
            "HealthBarWindow",
            "PlayerHealth",
            "PrimaryWeaponFrame",
            "PrimaryWeaponWindow",
            "SecondayWeaponUI",
            "SentryWindow",
            "AltFireIcon",
            "WeaponBarReload",
            "CoreHealthBarRefill",
            "CoreHealthBarFrame",
            "CoreHealthBarWindow",
            "Crosshair",
            "Crosshair2"
        };

        // Scene paths for navigation
        private const string MAIN_MENU_SCENE_PATH = "Resources/Sources/Scenes/MainMenu.json";
        private const string LEVEL1_SCENE_PATH = "Resources/Sources/Scenes/Level1_NewPlayer.json";
        private const string LEVEL2_SCENE_PATH = "Resources/Sources/Scenes/level2_player.json";
        private string currentGameScenePath = LEVEL1_SCENE_PATH;

        // Pause events - other scripts subscribe to these
        private const string EVENT_GAME_PAUSED = "GamePaused";
        private const string EVENT_GAME_RESUMED = "GameResumed";

        // Screen-space positions (like SettingsPopup uses)
        // Center of screen is approximately (640, 360) for 1280x720
        private const float CENTER_X = 640.0f;
        private const float CENTER_Y = 360.0f;
        private const float HIDDEN_Y = -500.0f;

        // Visible positions for each element (screen-space coordinates)
        // Positions match Level1_Player.json
        private Vector3 bgVisiblePos = new Vector3(640.0f, 360.0f, 0.0f);
        private Vector3 resumeVisiblePos = new Vector3(640.0f, 235.0f, 0.0f);
        private Vector3 restartVisiblePos = new Vector3(640.0f, 310.0f, 0.0f);
        private Vector3 plusVisiblePos = new Vector3(890.0f, 525.0f, 0.0f);
        private Vector3 mixerVisiblePos = new Vector3(844.0f, 415.0f, 0.0f);
        private Vector3 minusVisiblePos = new Vector3(890.0f, 559.0f, 0.0f);
        private Vector3 mainMenuVisiblePos = new Vector3(640.0f, 455.0f, 0.0f);
        private Vector3 howToPlayVisiblePos = new Vector3(640.0f, 380.0f, 0.0f);
        // Mixer Set 2 positions
        private Vector3 mixerVisiblePos2 = new Vector3(998.0f, 415.0f, 0.0f);
        private Vector3 plusVisiblePos2 = new Vector3(1044.0f, 525.0f, 0.0f);
        private Vector3 minusVisiblePos2 = new Vector3(1044.0f, 559.0f, 0.0f);
        // Mixer Set 3 positions
        private Vector3 mixerVisiblePos3 = new Vector3(1150.0f, 415.0f, 0.0f);
        private Vector3 plusVisiblePos3 = new Vector3(1200.0f, 526.0f, 0.0f);
        private Vector3 minusVisiblePos3 = new Vector3(1200.0f, 559.0f, 0.0f);
        // Checkbox Set positions - added by Rio
        private Vector3 checkboxMasterVisiblePos = new Vector3(833.8f, 620.0f, 0.0f);  
        private Vector3 checkboxBGMVisiblePos = new Vector3(987.1f, 620.0f, 0.0f);     
        private Vector3 checkboxSFXVisiblePos = new Vector3(1143.5f, 620.0f, 0.0f);     


        // Entity IDs
        private uint bgId;
        private uint resumeButtonId;
        private uint resumeButtonHoveredId;
        private uint restartButtonId;
        private uint restartButtonHoveredId;
        private uint plusButtonId;
        private uint plusButtonHoveredId;
        private uint mixerFillId;
        private uint minusButtonId;
        private uint minusButtonHoveredId;
        private uint mainMenuButtonId;
        private uint mainMenuButtonHoveredId;
        private uint howToPlayButtonId;
        private uint howToPlayButtonHoveredId;
        // Mixer Set 2 IDs
        private uint mixerFillId2;
        private uint plusButtonId2;
        private uint plusButtonHoveredId2;
        private uint minusButtonId2;
        private uint minusButtonHoveredId2;
        // Mixer Set 3 IDs
        private uint mixerFillId3;
        private uint plusButtonId3;
        private uint plusButtonHoveredId3;
        private uint minusButtonId3;
        private uint minusButtonHoveredId3;
        // Checkbox IDs - added by Rio
        private uint checkboxMasterUntickedId;
        private uint checkboxMasterTickedId;
        private uint checkboxBGMUntickedId;
        private uint checkboxBGMTickedId;
        private uint checkboxSFXUntickedId;
        private uint checkboxSFXTickedId;

        // HUD element IDs (to hide when paused)
        private uint[] hudElementIds;
        // Original positions of HUD elements (to restore when unpaused)
        private Vector3[] hudElementOriginalPositions;

        // State
        private bool isPaused = false;
        private bool entitiesFound = false;
        private bool wasPauseKeyPressed = false;
        private bool wasMousePressed = false;
        private bool wasF1Pressed = false;
        private bool gameEnded = false;

        //visual bar
        // Add these fields to store initial widths at the top of your class
        private float mixerFill1InitialWidth;
        private float mixerFill2InitialWidth;
        private float mixerFill3InitialWidth;
        private Vector3 mixerFill1InitialPosition;
        private Vector3 mixerFill2InitialPosition;
        private Vector3 mixerFill3InitialPosition;

        public override void OnStart()
        {
            LogMessage("PauseMenuPopup: Initializing...");

            // Find all pause menu entities
            bgId = SceneFindEntityByName(BG_NAME);
            resumeButtonId = SceneFindEntityByName(RESUME_BUTTON_NAME);
            resumeButtonHoveredId = SceneFindEntityByName(RESUME_BUTTON_HOVERED_NAME);
            restartButtonId = SceneFindEntityByName(RESTART_BUTTON_NAME);
            restartButtonHoveredId = SceneFindEntityByName(RESTART_BUTTON_HOVERED_NAME);
            plusButtonId = SceneFindEntityByName(PLUS_BUTTON_NAME);
            plusButtonHoveredId = SceneFindEntityByName(PLUS_BUTTON_HOVERED_NAME);
            mixerFillId = SceneFindEntityByName(MIXER_FILL_NAME);
            minusButtonId = SceneFindEntityByName(MINUS_BUTTON_NAME);
            minusButtonHoveredId = SceneFindEntityByName(MINUS_BUTTON_HOVERED_NAME);
            mainMenuButtonId = SceneFindEntityByName(MAINMENU_BUTTON_NAME);
            mainMenuButtonHoveredId = SceneFindEntityByName(MAINMENU_BUTTON_HOVERED_NAME);
            howToPlayButtonId = SceneFindEntityByName(HOWTOPLAY_BUTTON_NAME);
            howToPlayButtonHoveredId = SceneFindEntityByName(HOWTOPLAY_BUTTON_HOVERED_NAME);
            // Mixer Set 2
            mixerFillId2 = SceneFindEntityByName(MIXER_FILL_2_NAME);
            plusButtonId2 = SceneFindEntityByName(PLUS_BUTTON_2_NAME);
            plusButtonHoveredId2 = SceneFindEntityByName(PLUS_BUTTON_2_HOVERED_NAME);
            minusButtonId2 = SceneFindEntityByName(MINUS_BUTTON_2_NAME);
            minusButtonHoveredId2 = SceneFindEntityByName(MINUS_BUTTON_2_HOVERED_NAME);
            // Mixer Set 3
            mixerFillId3 = SceneFindEntityByName(MIXER_FILL_3_NAME);
            plusButtonId3 = SceneFindEntityByName(PLUS_BUTTON_3_NAME);
            plusButtonHoveredId3 = SceneFindEntityByName(PLUS_BUTTON_3_HOVERED_NAME);
            minusButtonId3 = SceneFindEntityByName(MINUS_BUTTON_3_NAME);
            minusButtonHoveredId3 = SceneFindEntityByName(MINUS_BUTTON_3_HOVERED_NAME);
            // Find checkboxes - Added by Rio
            checkboxMasterUntickedId = SceneFindEntityByName(CHECKBOX_MASTER_UNTICKED_NAME);
            checkboxMasterTickedId = SceneFindEntityByName(CHECKBOX_MASTER_TICKED_NAME);
            checkboxBGMUntickedId = SceneFindEntityByName(CHECKBOX_BGM_UNTICKED_NAME);
            checkboxBGMTickedId = SceneFindEntityByName(CHECKBOX_BGM_TICKED_NAME);
            checkboxSFXUntickedId = SceneFindEntityByName(CHECKBOX_SFX_UNTICKED_NAME);
            checkboxSFXTickedId = SceneFindEntityByName(CHECKBOX_SFX_TICKED_NAME);
             

            // Find HUD elements to hide when paused and store their original positions
            hudElementIds = new uint[HUD_ELEMENT_NAMES.Length];
            hudElementOriginalPositions = new Vector3[HUD_ELEMENT_NAMES.Length];
            for (int i = 0; i < HUD_ELEMENT_NAMES.Length; i++)
            {
                hudElementIds[i] = SceneFindEntityByName(HUD_ELEMENT_NAMES[i]);
                if (hudElementIds[i] == 0)
                {
                    LogMessage("PauseMenuPopup: HUD element not found: " + HUD_ELEMENT_NAMES[i]);
                }
                else
                {
                    // Store original position so we can restore it later
                    hudElementOriginalPositions[i] = GetPosition(hudElementIds[i]);
                }
            }
            if (mixerFillId != 0)
            {
                Vector3 scale1 = GetScale(mixerFillId);
                mixerFill1InitialWidth = scale1.Y;  // Changed from scale1.X to scale1.Y
                mixerFill1InitialPosition = GetPosition(mixerFillId);
                LogMessage("PauseMenuPopup: Mixer 1 initial height = " + mixerFill1InitialWidth);
            }

            if (mixerFillId2 != 0)
            {
                Vector3 scale2 = GetScale(mixerFillId2);
                mixerFill2InitialWidth = scale2.Y;  // Changed from scale2.X to scale2.Y
                mixerFill2InitialPosition = GetPosition(mixerFillId2);
                LogMessage("PauseMenuPopup: Mixer 2 initial height = " + mixerFill2InitialWidth);
            }

            if (mixerFillId3 != 0)
            {
                Vector3 scale3 = GetScale(mixerFillId3);
                mixerFill3InitialWidth = scale3.Y;  // Changed from scale3.X to scale3.Y
                mixerFill3InitialPosition = GetPosition(mixerFillId3);
                LogMessage("PauseMenuPopup: Mixer 3 initial height = " + mixerFill3InitialWidth);
            }

            // Log found entities
            LogMessage("PauseMenuPopup: bgId=" + bgId);
            LogMessage("PauseMenuPopup: resumeButtonId=" + resumeButtonId);

            if (bgId == 0)
            {
                LogError("PauseMenuPopup: Could not find: " + BG_NAME);
            }

            entitiesFound = (bgId != 0);
            isPaused = false;
            gameEnded = false;

            // Detect which level we are in by looking for level-specific entities
            uint[] turrets = SceneFindEntitiesByTag("EnemyTurret");
            if (turrets != null && turrets.Length > 0)
            {
                currentGameScenePath = LEVEL2_SCENE_PATH;
                LogMessage("PauseMenuPopup: Detected Level 2");
            }
            else
            {
                currentGameScenePath = LEVEL1_SCENE_PATH;
                LogMessage("PauseMenuPopup: Detected Level 1");
            }

            // Subscribe to win/lose events to block pause menu
            Event.Subscribe("GameOver", OnGameEnded);
            Event.Subscribe("GameWin", OnGameEnded);
            Event.Subscribe("GameRestart", OnGameRestart);

            // Hide all initially
            HidePauseMenu();

            LogMessage("PauseMenuPopup: Ready! Restart path: " + currentGameScenePath);
        }

        /// <summary>
        /// Updates the mixer fill bar visual height based on volume (0.0 to 1.0)
        /// Similar to HealthBar - adjusts height and position to keep bottom edge fixed
        /// </summary>
        private void UpdateMixerFillVisual(uint mixerFillId, float volume, float initialWidth, Vector3 initialPosition)
        {
            if (mixerFillId == 0) return;

            // Clamp volume to 0-1 range
            if (volume < 0.0f) volume = 0.0f;
            if (volume > 1.0f) volume = 1.0f;

            // Calculate new height based on volume (volume 1.0 = full height, 0.0 = zero height)
            float newHeight = initialWidth * volume;  // initialWidth is actually initialHeight now

            // Get current scale
            Vector3 currentScale = GetScale(mixerFillId);

            // Update scale with new height
            Vector3 newScale = new Vector3(
                currentScale.X,     // Keep width
                newHeight,          // Height based on volume
                currentScale.Z      // Keep depth
            );
            SetScale(mixerFillId, ref newScale);

            // Adjust position to keep BOTTOM edge fixed (move UP as height decreases)
            float heightDifference = initialWidth - newHeight;
            Vector3 newPosition = new Vector3(
                initialPosition.X,
                initialPosition.Y + heightDifference,  // Changed from - to + (move UP instead of DOWN)
                initialPosition.Z
            );
            SetPosition(mixerFillId, ref newPosition);

            LogMessage("UpdateMixerFill: Volume=" + volume.ToString("F2") +
                       " Height=" + newHeight.ToString("F1") +
                       " Offset=" + heightDifference.ToString("F1"));
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            // Block pause menu when win/lose screen is active
            if (gameEnded)
            {
                wasPauseKeyPressed = Input.IsKeyPressed(KeyCode.P);
                return;
            }

            // Handle P key to toggle pause
            bool pauseKeyPressed = Input.IsKeyPressed(KeyCode.P);
            bool pauseKeyJustPressed = pauseKeyPressed && !wasPauseKeyPressed;
            wasPauseKeyPressed = pauseKeyPressed;

            if (pauseKeyJustPressed)
            {
                LogMessage("PauseMenuPopup: P key pressed! isPaused=" + isPaused);

                if (isPaused)
                    HidePauseMenu();
                else
                    ShowPauseMenu();
            }

            // Handle hover and clicks when paused
            if (isPaused)
            {
                HandleHoverStates();
                HandleMouseClick();
            }
        }

        private void HandleHoverStates()
        {
            // Resume button hover
            UpdateButtonHover(resumeButtonId, resumeButtonHoveredId, resumeVisiblePos);
            // Restart button hover
            UpdateButtonHover(restartButtonId, restartButtonHoveredId, restartVisiblePos);
            // Plus button hover
            UpdateButtonHover(plusButtonId, plusButtonHoveredId, plusVisiblePos);
            // Minus button hover
            UpdateButtonHover(minusButtonId, minusButtonHoveredId, minusVisiblePos);
            // Main Menu button hover
            UpdateButtonHover(mainMenuButtonId, mainMenuButtonHoveredId, mainMenuVisiblePos);
            // How To Play button hover
            UpdateButtonHover(howToPlayButtonId, howToPlayButtonHoveredId, howToPlayVisiblePos);
            // Mixer Set 2 buttons hover
            UpdateButtonHover(plusButtonId2, plusButtonHoveredId2, plusVisiblePos2);
            UpdateButtonHover(minusButtonId2, minusButtonHoveredId2, minusVisiblePos2);
            // Mixer Set 3 buttons hover
            UpdateButtonHover(plusButtonId3, plusButtonHoveredId3, plusVisiblePos3);
            UpdateButtonHover(minusButtonId3, minusButtonHoveredId3, minusVisiblePos3);
        }

        private void UpdateButtonHover(uint normalId, uint hoveredId, Vector3 visiblePos)
        {
            if (normalId == 0 || hoveredId == 0) return;

            bool isHovered = Collision2D.IsMouseCollidingWithEntity(normalId) ||
                            Collision2D.IsMouseCollidingWithEntity(hoveredId);

            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, 0.0f);

            if (isHovered)
            {
                // Show hovered, hide normal
                SetPosition(hoveredId, ref visiblePos);
                SetPosition(normalId, ref hidePos);
            }
            else
            {
                // Show normal, hide hovered
                SetPosition(normalId, ref visiblePos);
                SetPosition(hoveredId, ref hidePos);
            }
        }

        private void HandleMouseClick()
        {
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            bool isF1Pressed = Input.IsKeyPressed(KeyCode.F1);
            bool f1JustPressed = isF1Pressed && !wasF1Pressed;
            wasF1Pressed = isF1Pressed;

            if (!mouseJustPressed && !f1JustPressed) return;

            // Check Resume button
            if (IsButtonClicked(resumeButtonId, resumeButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Resume clicked");
                HidePauseMenu();
                return;
            }

            // Check Restart button - reload current scene (same logic as win screen)
            if (IsButtonClicked(restartButtonId, restartButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Restart clicked - reloading scene");
                LogMessage("Scene path: " + currentGameScenePath);

                // Stop all audio before reloading
                StopGroup(AudioType.BGM);
                StopGroup(AudioType.SFX);

                // Hide cursor for gameplay
                Input.SetCursorVisible(false);

                // Reset pause state before scene change
                isPaused = false;
                GameState.IsPaused = false;

                // Set current scene path and reload
                GameState.CurrentScenePath = currentGameScenePath;
                Event.Publish("LoadScene", currentGameScenePath);
                return;
            }

            // Check Main Menu button - go to main menu
            if (IsButtonClicked(mainMenuButtonId, mainMenuButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Main Menu clicked - returning to main menu");
                LogMessage("Scene path: " + MAIN_MENU_SCENE_PATH);

                // Stop all game audio before transitioning
                StopGroup(AudioType.BGM);
                StopGroup(AudioType.SFX);

                // Reset all pause state BEFORE scene change
                isPaused = false;
                GameState.IsPaused = false;

                // Show cursor for main menu
                Input.SetCursorVisible(true);

                // Notify scripts game is resumed before scene change
                Event.Publish(EVENT_GAME_RESUMED, "");

                // Load main menu scene
                Event.Publish("LoadScene", MAIN_MENU_SCENE_PATH);
                return;
            }

            // Check How To Play button
            if (IsButtonClicked(howToPlayButtonId, howToPlayButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: How To Play clicked");
                return;
            }

            // Check Plus button
            if (IsButtonClicked(plusButtonId, plusButtonHoveredId))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetMasterVolume();
                    Instance.SetMasterVolume(currentVolume + 0.1f);
                    LogMessage("PauseMenuPopup: Master Volume + (Now: " + Instance.GetMasterVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                               mixerFill1InitialWidth, mixerFill1InitialPosition);
                }
                return;
            }

            // Check Minus button
            if (IsButtonClicked(minusButtonId, minusButtonHoveredId))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetMasterVolume();
                    Instance.SetMasterVolume(currentVolume - 0.1f);
                    LogMessage("PauseMenuPopup: Master Volume - (Now: " + Instance.GetMasterVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                               mixerFill1InitialWidth, mixerFill1InitialPosition);
                }
                return;
            }

            // Mixer Set 2 buttons
            if (IsButtonClicked(plusButtonId2, plusButtonHoveredId2))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetBGMVolume();
                    Instance.SetBGMVolume(currentVolume + 0.1f);
                    LogMessage("PauseMenuPopup: BGM Volume + (Now: " + Instance.GetBGMVolume().ToString("F2") + ")");
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
                    Instance.SetBGMVolume(currentVolume - 0.1f);
                    LogMessage("PauseMenuPopup: BGM Volume - (Now: " + Instance.GetBGMVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                mixerFill2InitialWidth, mixerFill2InitialPosition);
                }
                return;
            }

            // Mixer Set 3 buttons
            if (IsButtonClicked(plusButtonId3, plusButtonHoveredId3))
            {
                if (Instance != null)
                {
                    float currentVolume = Instance.GetSFXVolume();
                    Instance.SetSFXVolume(currentVolume + 0.1f);
                    LogMessage("PauseMenuPopup: SFX Volume + (Now: " + Instance.GetSFXVolume().ToString("F2") + ")");
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
                    Instance.SetSFXVolume(currentVolume - 0.1f);
                    LogMessage("PauseMenuPopup: SFX Volume - (Now: " + Instance.GetSFXVolume().ToString("F2") + ")");
                    UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                              mixerFill3InitialWidth, mixerFill3InitialPosition);
                }
                return;
            }

            // Checkbox Handling - Added by Rio
            // Master checkbox
            if (IsCheckboxClicked(checkboxMasterUntickedId, checkboxMasterTickedId))
            {
                if (Instance != null)
                {
                    Instance.ToggleMasterMute();
                    bool isMuted = Instance.IsMasterMuted();
                    LogMessage("PauseMenuPopup: Master Mute toggled to: " + isMuted);
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
                    LogMessage("PauseMenuPopup: BGM Mute toggled to: " + isMuted);
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
                    LogMessage("PauseMenuPopup: SFX Mute toggled to: " + isMuted);
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

        // Checking for it Checkbox is clicked - Added by Rio
        private bool IsCheckboxClicked(uint untickedId, uint tickedId)
        {
            return (untickedId != 0 && Collision2D.IsMouseCollidingWithEntity(untickedId)) ||
                   (tickedId != 0 && Collision2D.IsMouseCollidingWithEntity(tickedId));
        }

        // Updating checkbox visual based on AudioSettings mute state - Added by Rio
        private void UpdateCheckboxVisuals()
        {
            if (Instance == null) return;

            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, 0.0f);

            // Master checkbox
            bool masterMuted = Instance.IsMasterMuted();
            if (masterMuted)
            {
                // Show ticked, hide unticked
                SetPosition(checkboxMasterTickedId, ref checkboxMasterVisiblePos);
                SetPosition(checkboxMasterUntickedId, ref hidePos);
            }
            else
            {
                // Show unticked, hide ticked
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

        private void ShowPauseMenu()
        {
            if (isPaused) return;

            isPaused = true;
            LogMessage("PauseMenuPopup: Showing pause menu");

            // Set global pause state FIRST so all scripts stop immediately
            GameState.IsPaused = true;

            // Show cursor for menu interaction
            Input.SetCursorVisible(true);

            // Notify other scripts that game is paused (for scripts that use events)
            Event.Publish(EVENT_GAME_PAUSED, "");

            // Position all elements at their visible screen positions
            SetPosition(bgId, ref bgVisiblePos);
            SetPosition(resumeButtonId, ref resumeVisiblePos);
            SetPosition(restartButtonId, ref restartVisiblePos);
            SetPosition(plusButtonId, ref plusVisiblePos);
            SetPosition(mixerFillId, ref mixerVisiblePos);
            SetPosition(minusButtonId, ref minusVisiblePos);
            SetPosition(mainMenuButtonId, ref mainMenuVisiblePos);
            SetPosition(howToPlayButtonId, ref howToPlayVisiblePos);
            // Mixer Set 2
            SetPosition(mixerFillId2, ref mixerVisiblePos2);
            SetPosition(plusButtonId2, ref plusVisiblePos2);
            SetPosition(minusButtonId2, ref minusVisiblePos2);
            // Mixer Set 3
            SetPosition(mixerFillId3, ref mixerVisiblePos3);
            SetPosition(plusButtonId3, ref plusVisiblePos3);
            SetPosition(minusButtonId3, ref minusVisiblePos3);

            // Update mixer fill visuals if AudioSettings is available
            if (Instance != null)
            {
                UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                                     mixerFill1InitialWidth, mixerFill1InitialPosition);
                UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                     mixerFill2InitialWidth, mixerFill2InitialPosition);
                UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                                     mixerFill3InitialWidth, mixerFill3InitialPosition);
            }

            // Hide all hovered versions initially
            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, 0.0f);
            SetPosition(resumeButtonHoveredId, ref hidePos);
            SetPosition(restartButtonHoveredId, ref hidePos);
            SetPosition(plusButtonHoveredId, ref hidePos);
            SetPosition(minusButtonHoveredId, ref hidePos);
            SetPosition(mainMenuButtonHoveredId, ref hidePos);
            SetPosition(howToPlayButtonHoveredId, ref hidePos);
            // Mixer Set 2 hovered
            SetPosition(plusButtonHoveredId2, ref hidePos);
            SetPosition(minusButtonHoveredId2, ref hidePos);
            // Mixer Set 3 hovered
            SetPosition(plusButtonHoveredId3, ref hidePos);
            SetPosition(minusButtonHoveredId3, ref hidePos);

            // Hide HUD elements by moving them off-screen
            Vector3 hudHidePos = new Vector3(CENTER_X, HIDDEN_Y, 0.0f);
            for (int i = 0; i < hudElementIds.Length; i++)
            {
                if (hudElementIds[i] != 0)
                {
                    SetPosition(hudElementIds[i], ref hudHidePos);
                }
            }

            UpdateCheckboxVisuals(); // added by Rio

            LogMessage("PauseMenuPopup: Menu shown at screen center");
        }

        private void HidePauseMenu()
        {
            isPaused = false;
            LogMessage("PauseMenuPopup: Hiding pause menu");

            // Hide all elements by moving off-screen
            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, 0.0f);
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, 0.0f);

            SetPosition(bgId, ref hidePos);
            SetPosition(resumeButtonId, ref hidePos2);
            SetPosition(resumeButtonHoveredId, ref hidePos2);
            SetPosition(restartButtonId, ref hidePos2);
            SetPosition(restartButtonHoveredId, ref hidePos2);
            SetPosition(plusButtonId, ref hidePos2);
            SetPosition(plusButtonHoveredId, ref hidePos2);
            SetPosition(mixerFillId, ref hidePos2);
            SetPosition(minusButtonId, ref hidePos2);
            SetPosition(minusButtonHoveredId, ref hidePos2);
            SetPosition(mainMenuButtonId, ref hidePos2);
            SetPosition(mainMenuButtonHoveredId, ref hidePos2);
            SetPosition(howToPlayButtonId, ref hidePos2);
            SetPosition(howToPlayButtonHoveredId, ref hidePos2);
            // Mixer Set 2
            SetPosition(mixerFillId2, ref hidePos2);
            SetPosition(plusButtonId2, ref hidePos2);
            SetPosition(plusButtonHoveredId2, ref hidePos2);
            SetPosition(minusButtonId2, ref hidePos2);
            SetPosition(minusButtonHoveredId2, ref hidePos2);
            // Mixer Set 3
            SetPosition(mixerFillId3, ref hidePos2);
            SetPosition(plusButtonId3, ref hidePos2);
            SetPosition(plusButtonHoveredId3, ref hidePos2);
            SetPosition(minusButtonId3, ref hidePos2);
            SetPosition(minusButtonHoveredId3, ref hidePos2);

            // Hide checkboxes - added by Rio
            SetPosition(checkboxMasterUntickedId, ref hidePos2);
            SetPosition(checkboxMasterTickedId, ref hidePos2);
            SetPosition(checkboxBGMUntickedId, ref hidePos2);
            SetPosition(checkboxBGMTickedId, ref hidePos2);
            SetPosition(checkboxSFXUntickedId, ref hidePos2);
            SetPosition(checkboxSFXTickedId, ref hidePos2);

            // Show HUD elements again by restoring their original positions
            for (int i = 0; i < hudElementIds.Length; i++)
            {
                if (hudElementIds[i] != 0)
                {
                    SetPosition(hudElementIds[i], ref hudElementOriginalPositions[i]);
                }
            }

            // Clear global pause state
            GameState.IsPaused = false;

            // Notify other scripts that game is resumed (for scripts that use events)
            Event.Publish(EVENT_GAME_RESUMED, "");

            // Hide cursor for gameplay
            Input.SetCursorVisible(false);
        }

        private void OnGameEnded(string eventName, string payload)
        {
            LogMessage("PauseMenuPopup: Game ended (" + eventName + ") - disabling pause menu");
            gameEnded = true;

            // If pause menu is open when game ends, close it
            if (isPaused)
                HidePauseMenu();
        }

        private void OnGameRestart(string eventName, string payload)
        {
            LogMessage("PauseMenuPopup: Game restarted - re-enabling pause menu");
            gameEnded = false;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe("GameOver", OnGameEnded);
            Event.Unsubscribe("GameWin", OnGameEnded);
            Event.Unsubscribe("GameRestart", OnGameRestart);
            LogMessage("PauseMenuPopup: Destroyed");
        }
    }
}

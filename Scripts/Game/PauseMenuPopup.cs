using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.AudioManager;
using static Engine.SpriteRenderer;
using static Engine.Text;

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
        private const string GAME_SCENE_PATH = "Resources/Sources/Scenes/Level1_Player.json";

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

        // HUD element IDs (to hide when paused)
        private uint[] hudElementIds;
        // Original positions of HUD elements (to restore when unpaused)
        private Vector3[] hudElementOriginalPositions;

        // State
        private bool isPaused = false;
        private bool entitiesFound = false;
        private bool wasPauseKeyPressed = false;
        private bool wasMousePressed = false;

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

            // Log found entities
            LogMessage("PauseMenuPopup: bgId=" + bgId);
            LogMessage("PauseMenuPopup: resumeButtonId=" + resumeButtonId);

            if (bgId == 0)
            {
                LogError("PauseMenuPopup: Could not find: " + BG_NAME);
            }

            entitiesFound = (bgId != 0);
            isPaused = false;

            // Hide all initially
            HidePauseMenu();

            LogMessage("PauseMenuPopup: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

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

            if (!mouseJustPressed) return;

            // Check Resume button
            if (IsButtonClicked(resumeButtonId, resumeButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Resume clicked");
                HidePauseMenu();
                return;
            }

            // Check Restart button - reload current scene
            if (IsButtonClicked(restartButtonId, restartButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Restart clicked - reloading scene");
                LogMessage("Scene path: " + GAME_SCENE_PATH);
                HidePauseMenu();
                Event.Publish("LoadScene", GAME_SCENE_PATH);
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
                LogMessage("PauseMenuPopup: Plus clicked");
                return;
            }

            // Check Minus button
            if (IsButtonClicked(minusButtonId, minusButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Minus clicked");
                return;
            }

            // Mixer Set 2 buttons
            if (IsButtonClicked(plusButtonId2, plusButtonHoveredId2))
            {
                LogMessage("PauseMenuPopup: Plus 2 clicked");
                return;
            }
            if (IsButtonClicked(minusButtonId2, minusButtonHoveredId2))
            {
                LogMessage("PauseMenuPopup: Minus 2 clicked");
                return;
            }

            // Mixer Set 3 buttons
            if (IsButtonClicked(plusButtonId3, plusButtonHoveredId3))
            {
                LogMessage("PauseMenuPopup: Plus 3 clicked");
                return;
            }
            if (IsButtonClicked(minusButtonId3, minusButtonHoveredId3))
            {
                LogMessage("PauseMenuPopup: Minus 3 clicked");
                return;
            }
        }

        private bool IsButtonClicked(uint normalId, uint hoveredId)
        {
            return (normalId != 0 && Collision2D.IsMouseCollidingWithEntity(normalId)) ||
                   (hoveredId != 0 && Collision2D.IsMouseCollidingWithEntity(hoveredId));
        }

        private void ShowPauseMenu()
        {
            if (isPaused) return;

            isPaused = true;
            LogMessage("PauseMenuPopup: Showing pause menu");

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

            // Show cursor for menu interaction
            Input.SetCursorVisible(true);

            // Set global pause state
            GameState.IsPaused = true;

            // Notify other scripts that game is paused (for scripts that use events)
            Event.Publish(EVENT_GAME_PAUSED, "");
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

        public override void OnDestroy()
        {
            LogMessage("PauseMenuPopup: Destroyed");
        }
    }
}

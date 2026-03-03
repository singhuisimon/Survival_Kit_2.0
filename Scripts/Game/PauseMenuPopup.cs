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
    /// Uses SetIsVisible to show/hide positions are defined in the scene JSON and never change.
    /// Press P to toggle pause menu.
    /// </summary>
    public class PauseMenuPopup : ScriptBehaviour
    {
        // Entity names - must match names in scene JSON
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
        private const string CHECKBOX_MASTER_UNTICKED_NAME = "Paused_Checkbox_Master_Unticked";
        private const string CHECKBOX_MASTER_TICKED_NAME = "Paused_Checkbox_Master_Ticked";
        private const string CHECKBOX_BGM_UNTICKED_NAME = "Paused_Checkbox_BGM_Unticked";
        private const string CHECKBOX_BGM_TICKED_NAME = "Paused_Checkbox_BGM_Ticked";
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
        private const string GAME_SCENE_PATH = "Resources/Sources/Scenes/trench_run.json";
        private const string LEVEL2_SCENE_PATH = "Resources/Sources/Scenes/level2.json";

        // Pause events
        private const string EVENT_GAME_PAUSED = "GamePaused";
        private const string EVENT_GAME_RESUMED = "GameResumed";

        private const float CENTER_X = 640.0f;

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

        // HUD element IDs
        private uint[] hudElementIds;

        // State
        private bool isPaused = false;
        private bool entitiesFound = false;
        private bool wasPauseKeyPressed = false;
        private bool wasMousePressed = false;
        private bool gameEnded = false;
        private string currentGameScenePath = GAME_SCENE_PATH;

        // Mixer bar initial values (read from scene at startup, before any hiding)
        private float mixerFill1InitialWidth;
        private float mixerFill2InitialWidth;
        private float mixerFill3InitialWidth;
        private Vector3 mixerFill1InitialPosition;
        private Vector3 mixerFill2InitialPosition;
        private Vector3 mixerFill3InitialPosition;

        // =====================================================================
        // HELPER: Safe visibility setter � skips if ID is 0
        // =====================================================================
        private static void SafeSetVisible(uint id, bool visible)
        {
            if (id == 0) return;
            SpriteRenderer.SetIsVisible(id, visible);
            Text.SetIsVisible(id, visible);
        }

        // =====================================================================
        // ON START
        // =====================================================================
        public override void OnStart()
        {
            LogMessage("PauseMenuPopup: Initializing...");

            // --- 1. Find all pause menu entities ---
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
            // Checkboxes
            checkboxMasterUntickedId = SceneFindEntityByName(CHECKBOX_MASTER_UNTICKED_NAME);
            checkboxMasterTickedId = SceneFindEntityByName(CHECKBOX_MASTER_TICKED_NAME);
            checkboxBGMUntickedId = SceneFindEntityByName(CHECKBOX_BGM_UNTICKED_NAME);
            checkboxBGMTickedId = SceneFindEntityByName(CHECKBOX_BGM_TICKED_NAME);
            checkboxSFXUntickedId = SceneFindEntityByName(CHECKBOX_SFX_UNTICKED_NAME);
            checkboxSFXTickedId = SceneFindEntityByName(CHECKBOX_SFX_TICKED_NAME);

            // --- 2. IMMEDIATELY hide all pause menu entities to prevent first-frame flash ---
            //        Safe because SafeSetVisible skips IDs that are 0.
            HidePauseMenu();

            // --- 3. Read mixer initial values BEFORE any position changes ---
            //        Positions are still at their scene JSON values at this point.
            if (mixerFillId != 0)
            {
                mixerFill1InitialWidth = GetScale(mixerFillId).Y;
                mixerFill1InitialPosition = GetPosition(mixerFillId);
                LogMessage("PauseMenuPopup: Mixer 1 initial height = " + mixerFill1InitialWidth);
            }
            if (mixerFillId2 != 0)
            {
                mixerFill2InitialWidth = GetScale(mixerFillId2).Y;
                mixerFill2InitialPosition = GetPosition(mixerFillId2);
                LogMessage("PauseMenuPopup: Mixer 2 initial height = " + mixerFill2InitialWidth);
            }
            if (mixerFillId3 != 0)
            {
                mixerFill3InitialWidth = GetScale(mixerFillId3).Y;
                mixerFill3InitialPosition = GetPosition(mixerFillId3);
                LogMessage("PauseMenuPopup: Mixer 3 initial height = " + mixerFill3InitialWidth);
            }

            // --- 4. Find HUD elements ---
            hudElementIds = new uint[HUD_ELEMENT_NAMES.Length];
            for (int i = 0; i < HUD_ELEMENT_NAMES.Length; i++)
            {
                hudElementIds[i] = SceneFindEntityByName(HUD_ELEMENT_NAMES[i]);
                if (hudElementIds[i] == 0)
                    LogMessage("PauseMenuPopup: HUD element not found: " + HUD_ELEMENT_NAMES[i]);
            }

            // --- 5. Final state setup ---
            if (bgId == 0)
                LogError("PauseMenuPopup: Could not find: " + BG_NAME);

            entitiesFound = (bgId != 0);
            isPaused = false;
            gameEnded = false;

            // Detect which level we're in
            uint[] turrets = SceneFindEntitiesByTag("EnemyTurret");
            if (turrets != null && turrets.Length > 0)
            {
                currentGameScenePath = LEVEL2_SCENE_PATH;
                LogMessage("PauseMenuPopup: Detected Level 2");
            }
            else
            {
                currentGameScenePath = GAME_SCENE_PATH;
                LogMessage("PauseMenuPopup: Detected Level 1");
            }

            Event.Subscribe("GameOver", OnGameEnded);
            Event.Subscribe("GameWin", OnGameEnded);
            Event.Subscribe("GameRestart", OnGameRestart);

            LogMessage("PauseMenuPopup: Ready!");
        }

        // =====================================================================
        // ON UPDATE
        // =====================================================================
        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound) return;

            if (gameEnded)
            {
                wasPauseKeyPressed = Input.IsKeyPressed(KeyCode.P);
                return;
            }

            bool pauseKeyPressed = Input.IsKeyPressed(KeyCode.P);
            bool pauseKeyJustPressed = pauseKeyPressed && !wasPauseKeyPressed;
            wasPauseKeyPressed = pauseKeyPressed;

            if (pauseKeyJustPressed)
            {
                LogMessage("PauseMenuPopup: P key pressed! isPaused=" + isPaused);
                if (isPaused) HidePauseMenu();
                else ShowPauseMenu();
            }

            if (isPaused)
            {
                HandleHoverStates();
                HandleMouseClick();
            }
        }

        // =====================================================================
        // SHOW PAUSE MENU
        // Positions are already correct in the scene JSON � just make visible.
        // =====================================================================
        private void ShowPauseMenu()
        {
            if (isPaused) return;

            isPaused = true;
            LogMessage("PauseMenuPopup: Showing pause menu");

            GameState.IsPaused = true;
            Input.SetCursorVisible(true);
            Event.Publish(EVENT_GAME_PAUSED, "");

            // Show all main buttons (not hovered � HandleHoverStates manages those)
            SafeSetVisible(bgId, true);
            SafeSetVisible(resumeButtonId, true);
            SafeSetVisible(restartButtonId, true);
            SafeSetVisible(plusButtonId, true);
            SafeSetVisible(mixerFillId, true);
            SafeSetVisible(minusButtonId, true);
            SafeSetVisible(mainMenuButtonId, true);
            SafeSetVisible(howToPlayButtonId, true);
            // Mixer Set 2
            SafeSetVisible(mixerFillId2, true);
            SafeSetVisible(plusButtonId2, true);
            SafeSetVisible(minusButtonId2, true);
            // Mixer Set 3
            SafeSetVisible(mixerFillId3, true);
            SafeSetVisible(plusButtonId3, true);
            SafeSetVisible(minusButtonId3, true);

            // Hovered versions start hidden � HandleHoverStates will show them on mouse-over
            SafeSetVisible(resumeButtonHoveredId, false);
            SafeSetVisible(restartButtonHoveredId, false);
            SafeSetVisible(plusButtonHoveredId, false);
            SafeSetVisible(minusButtonHoveredId, false);
            SafeSetVisible(mainMenuButtonHoveredId, false);
            SafeSetVisible(howToPlayButtonHoveredId, false);
            SafeSetVisible(plusButtonHoveredId2, false);
            SafeSetVisible(minusButtonHoveredId2, false);
            SafeSetVisible(plusButtonHoveredId3, false);
            SafeSetVisible(minusButtonHoveredId3, false);

            // Update mixer bar heights to reflect current volume
            if (Instance != null)
            {
                UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                                      mixerFill1InitialWidth, mixerFill1InitialPosition);
                UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                      mixerFill2InitialWidth, mixerFill2InitialPosition);
                UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                                      mixerFill3InitialWidth, mixerFill3InitialPosition);
            }

            // Hide HUD
            if (hudElementIds != null)
            {
                for (int i = 0; i < hudElementIds.Length; i++)
                    SafeSetVisible(hudElementIds[i], false);
            }

            UpdateCheckboxVisuals();

            LogMessage("PauseMenuPopup: Menu shown");
        }

        // =====================================================================
        // HIDE PAUSE MENU
        // Guards ensure this is safe to call during OnStart before full init.
        // =====================================================================
        private void HidePauseMenu()
        {
            isPaused = false;

            // Hide all pause menu entities
            SafeSetVisible(bgId, false);
            SafeSetVisible(resumeButtonId, false);
            SafeSetVisible(resumeButtonHoveredId, false);
            SafeSetVisible(restartButtonId, false);
            SafeSetVisible(restartButtonHoveredId, false);
            SafeSetVisible(plusButtonId, false);
            SafeSetVisible(plusButtonHoveredId, false);
            SafeSetVisible(mixerFillId, false);
            SafeSetVisible(minusButtonId, false);
            SafeSetVisible(minusButtonHoveredId, false);
            SafeSetVisible(mainMenuButtonId, false);
            SafeSetVisible(mainMenuButtonHoveredId, false);
            SafeSetVisible(howToPlayButtonId, false);
            SafeSetVisible(howToPlayButtonHoveredId, false);
            // Mixer Set 2
            SafeSetVisible(mixerFillId2, false);
            SafeSetVisible(plusButtonId2, false);
            SafeSetVisible(plusButtonHoveredId2, false);
            SafeSetVisible(minusButtonId2, false);
            SafeSetVisible(minusButtonHoveredId2, false);
            // Mixer Set 3
            SafeSetVisible(mixerFillId3, false);
            SafeSetVisible(plusButtonId3, false);
            SafeSetVisible(plusButtonHoveredId3, false);
            SafeSetVisible(minusButtonId3, false);
            SafeSetVisible(minusButtonHoveredId3, false);
            // Checkboxes
            SafeSetVisible(checkboxMasterUntickedId, false);
            SafeSetVisible(checkboxMasterTickedId, false);
            SafeSetVisible(checkboxBGMUntickedId, false);
            SafeSetVisible(checkboxBGMTickedId, false);
            SafeSetVisible(checkboxSFXUntickedId, false);
            SafeSetVisible(checkboxSFXTickedId, false);

            // Restore HUD � only if array is initialized (safe during OnStart early call)
            if (hudElementIds != null)
            {
                for (int i = 0; i < hudElementIds.Length; i++)
                    SafeSetVisible(hudElementIds[i], true);
            }

            // Only fire events/cursor change if fully initialized
            // (prevents side effects when called as a first-frame hide during OnStart)
            if (entitiesFound)
            {
                GameState.IsPaused = false;
                Event.Publish(EVENT_GAME_RESUMED, "");
                Input.SetCursorVisible(false);
            }
        }

        // =====================================================================
        // HOVER STATES
        // Uses SetIsVisible instead of SetPosition � no position changes needed.
        // =====================================================================
        private void HandleHoverStates()
        {
            UpdateButtonHover(resumeButtonId, resumeButtonHoveredId);
            UpdateButtonHover(restartButtonId, restartButtonHoveredId);
            UpdateButtonHover(plusButtonId, plusButtonHoveredId);
            UpdateButtonHover(minusButtonId, minusButtonHoveredId);
            UpdateButtonHover(mainMenuButtonId, mainMenuButtonHoveredId);
            UpdateButtonHover(howToPlayButtonId, howToPlayButtonHoveredId);
            UpdateButtonHover(plusButtonId2, plusButtonHoveredId2);
            UpdateButtonHover(minusButtonId2, minusButtonHoveredId2);
            UpdateButtonHover(plusButtonId3, plusButtonHoveredId3);
            UpdateButtonHover(minusButtonId3, minusButtonHoveredId3);
        }

        private void UpdateButtonHover(uint normalId, uint hoveredId)
        {
            if (normalId == 0 || hoveredId == 0) return;

            bool isHovered = Collision2D.IsMouseCollidingWithEntity(normalId) ||
                             Collision2D.IsMouseCollidingWithEntity(hoveredId);

            // Show whichever state matches, hide the other
            SpriteRenderer.SetIsVisible(normalId, !isHovered);
            SpriteRenderer.SetIsVisible(hoveredId, isHovered);
        }

        // =====================================================================
        // MOUSE CLICK
        // =====================================================================
        private void HandleMouseClick()
        {
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (!mouseJustPressed) return;

            if (IsButtonClicked(resumeButtonId, resumeButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Resume clicked");
                HidePauseMenu();
                return;
            }

            if (IsButtonClicked(restartButtonId, restartButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Restart clicked - reloading scene: " + currentGameScenePath);
                StopGroup(AudioType.BGM);
                StopGroup(AudioType.SFX);
                Input.SetCursorVisible(false);
                isPaused = false;
                GameState.IsPaused = false;
                bool success = Scene.SceneLoadFromFile(currentGameScenePath);
                if (success) LogMessage("Restart scene load success.");
                return;
            }

            if (IsButtonClicked(mainMenuButtonId, mainMenuButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: Main Menu clicked");
                StopGroup(AudioType.BGM);
                StopGroup(AudioType.SFX);
                isPaused = false;
                GameState.IsPaused = false;
                Input.SetCursorVisible(true);
                Event.Publish(EVENT_GAME_RESUMED, "");
                bool success = Scene.SceneLoadFromFile(MAIN_MENU_SCENE_PATH);
                if (success) LogMessage("Main menu load success.");
                return;
            }

            if (IsButtonClicked(howToPlayButtonId, howToPlayButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: How To Play clicked");
                return;
            }

            // Master volume +
            if (IsButtonClicked(plusButtonId, plusButtonHoveredId))
            {
                if (Instance != null)
                {
                    Instance.SetMasterVolume(Instance.GetMasterVolume() + 0.111f);
                    UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                                         mixerFill1InitialWidth, mixerFill1InitialPosition);
                    LogMessage("Master Volume: " + Instance.GetMasterVolume().ToString("F2"));
                }
                return;
            }

            // Master volume -
            if (IsButtonClicked(minusButtonId, minusButtonHoveredId))
            {
                if (Instance != null)
                {
                    Instance.SetMasterVolume(Instance.GetMasterVolume() - 0.111f);
                    UpdateMixerFillVisual(mixerFillId, Instance.GetMasterVolume(),
                                         mixerFill1InitialWidth, mixerFill1InitialPosition);
                    LogMessage("Master Volume: " + Instance.GetMasterVolume().ToString("F2"));
                }
                return;
            }

            // BGM volume +
            if (IsButtonClicked(plusButtonId2, plusButtonHoveredId2))
            {
                if (Instance != null)
                {
                    Instance.SetBGMVolume(Instance.GetBGMVolume() + 0.111f);
                    UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                         mixerFill2InitialWidth, mixerFill2InitialPosition);
                    LogMessage("BGM Volume: " + Instance.GetBGMVolume().ToString("F2"));
                }
                return;
            }

            // BGM volume -
            if (IsButtonClicked(minusButtonId2, minusButtonHoveredId2))
            {
                if (Instance != null)
                {
                    Instance.SetBGMVolume(Instance.GetBGMVolume() - 0.111f);
                    UpdateMixerFillVisual(mixerFillId2, Instance.GetBGMVolume(),
                                         mixerFill2InitialWidth, mixerFill2InitialPosition);
                    LogMessage("BGM Volume: " + Instance.GetBGMVolume().ToString("F2"));
                }
                return;
            }

            // SFX volume +
            if (IsButtonClicked(plusButtonId3, plusButtonHoveredId3))
            {
                if (Instance != null)
                {
                    Instance.SetSFXVolume(Instance.GetSFXVolume() + 0.111f);
                    UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                                         mixerFill3InitialWidth, mixerFill3InitialPosition);
                    LogMessage("SFX Volume: " + Instance.GetSFXVolume().ToString("F2"));
                }
                return;
            }

            // SFX volume -
            if (IsButtonClicked(minusButtonId3, minusButtonHoveredId3))
            {
                if (Instance != null)
                {
                    Instance.SetSFXVolume(Instance.GetSFXVolume() - 0.111f);
                    UpdateMixerFillVisual(mixerFillId3, Instance.GetSFXVolume(),
                                         mixerFill3InitialWidth, mixerFill3InitialPosition);
                    LogMessage("SFX Volume: " + Instance.GetSFXVolume().ToString("F2"));
                }
                return;
            }

            // Master mute checkbox
            if (IsCheckboxClicked(checkboxMasterUntickedId, checkboxMasterTickedId))
            {
                if (Instance != null) { Instance.ToggleMasterMute(); UpdateCheckboxVisuals(); }
                return;
            }

            // BGM mute checkbox
            if (IsCheckboxClicked(checkboxBGMUntickedId, checkboxBGMTickedId))
            {
                if (Instance != null) { Instance.ToggleBGMMute(); UpdateCheckboxVisuals(); }
                return;
            }

            // SFX mute checkbox
            if (IsCheckboxClicked(checkboxSFXUntickedId, checkboxSFXTickedId))
            {
                if (Instance != null) { Instance.ToggleSFXMute(); UpdateCheckboxVisuals(); }
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

        // =====================================================================
        // CHECKBOX VISUALS � show ticked or unticked based on mute state
        // =====================================================================
        private void UpdateCheckboxVisuals()
        {
            if (Instance == null) return;

            bool masterMuted = Instance.IsMasterMuted();
            SpriteRenderer.SetIsVisible(checkboxMasterTickedId, masterMuted);
            SpriteRenderer.SetIsVisible(checkboxMasterUntickedId, !masterMuted);

            bool bgmMuted = Instance.IsBGMMuted();
            SpriteRenderer.SetIsVisible(checkboxBGMTickedId, bgmMuted);
            SpriteRenderer.SetIsVisible(checkboxBGMUntickedId, !bgmMuted);

            bool sfxMuted = Instance.IsSFXMuted();
            SpriteRenderer.SetIsVisible(checkboxSFXTickedId, sfxMuted);
            SpriteRenderer.SetIsVisible(checkboxSFXUntickedId, !sfxMuted);
        }

        // =====================================================================
        // MIXER BAR VISUAL � adjusts height based on volume
        // =====================================================================
        private void UpdateMixerFillVisual(uint fillId, float volume, float initialHeight, Vector3 initialPosition)
        {
            if (fillId == 0) return;

            volume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);

            float newHeight = initialHeight * volume;
            Vector3 currentScale = GetScale(fillId);
            Vector3 newScale = new Vector3(currentScale.X, newHeight, currentScale.Z);
            SetScale(fillId, ref newScale);

            float heightDiff = initialHeight - newHeight;
            Vector3 newPos = new Vector3(initialPosition.X, initialPosition.Y + heightDiff, initialPosition.Z);
            SetPosition(fillId, ref newPos);

            LogMessage("UpdateMixerFill: Volume=" + volume.ToString("F2") +
                       " Height=" + newHeight.ToString("F1"));
        }

        // =====================================================================
        // EVENTS
        // =====================================================================
        private void OnGameEnded(string eventName, string payload)
        {
            LogMessage("PauseMenuPopup: Game ended (" + eventName + ")");
            gameEnded = true;
            if (isPaused) HidePauseMenu();
        }

        private void OnGameRestart(string eventName, string payload)
        {
            LogMessage("PauseMenuPopup: Game restarted");
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
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
    public class PauseMenuPopup : ScriptBehaviour
    {
        // Entity names
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
        // Mixer Set 2 - BGM
        private const string MIXER_FILL_2_NAME = "Paused_MixerFill_2";
        private const string PLUS_BUTTON_2_NAME = "Paused_PlusButton_2";
        private const string PLUS_BUTTON_2_HOVERED_NAME = "Paused_PlusButton_2_Hovered";
        private const string MINUS_BUTTON_2_NAME = "Paused_MinusButton_2";
        private const string MINUS_BUTTON_2_HOVERED_NAME = "Paused_MinusButton_2_Hovered";
        // Mixer Set 3 - SFX
        private const string MIXER_FILL_3_NAME = "Paused_MixerFill_3";
        private const string PLUS_BUTTON_3_NAME = "Paused_PlusButton_3";
        private const string PLUS_BUTTON_3_HOVERED_NAME = "Paused_PlusButton_3_Hovered";
        private const string MINUS_BUTTON_3_NAME = "Paused_MinusButton_3";
        private const string MINUS_BUTTON_3_HOVERED_NAME = "Paused_MinusButton_3_Hovered";
        // Mixer Set 4 - Gamma
        private const string MIXER_FILL_4_NAME = "Paused_MixerFill_4";
        private const string PLUS_BUTTON_4_NAME = "Paused_PlusButton_4";
        private const string PLUS_BUTTON_4_HOVERED_NAME = "Paused_PlusButton_4_Hovered";
        private const string MINUS_BUTTON_4_NAME = "Paused_MinusButton_4";
        private const string MINUS_BUTTON_4_HOVERED_NAME = "Paused_MinusButton_4_Hovered";
        private const string DEFAULT_BUTTON_4_NAME = "Paused_DefaultButton_4";
        private const string DEFAULT_BUTTON_4_HOVERED_NAME = "Paused_DefaultButton_4_Hovered";
        // Mixer Set 5 - Mouse Sensitivity
        private const string MIXER_FILL_5_NAME = "Paused_MixerFill_5";
        private const string PLUS_BUTTON_5_NAME = "Paused_PlusButton_5";
        private const string PLUS_BUTTON_5_HOVERED_NAME = "Paused_PlusButton_5_Hovered";
        private const string MINUS_BUTTON_5_NAME = "Paused_MinusButton_5";
        private const string MINUS_BUTTON_5_HOVERED_NAME = "Paused_MinusButton_5_Hovered";
        private const string DEFAULT_BUTTON_5_NAME = "Paused_DefaultButton_5";
        private const string DEFAULT_BUTTON_5_HOVERED_NAME = "Paused_DefaultButton_5_Hovered";

        // Checkboxes
        private const string CHECKBOX_MASTER_UNTICKED_NAME = "Paused_Checkbox_Master_Unticked";
        private const string CHECKBOX_MASTER_TICKED_NAME = "Paused_Checkbox_Master_Ticked";
        private const string CHECKBOX_BGM_UNTICKED_NAME = "Paused_Checkbox_BGM_Unticked";
        private const string CHECKBOX_BGM_TICKED_NAME = "Paused_Checkbox_BGM_Ticked";
        private const string CHECKBOX_SFX_UNTICKED_NAME = "Paused_Checkbox_SFX_Unticked";
        private const string CHECKBOX_SFX_TICKED_NAME = "Paused_Checkbox_SFX_Ticked";

        // HUD elements to hide when paused
        private static readonly string[] HUD_ELEMENT_NAMES = {
            "TImer", "StageCount", "scoreboard", "WeaponAmmoCount",
            "HealthCount", "CoreHealthCount", "HealthBarFill", "HealthBarWindow",
            "PlayerHealth", "PrimaryWeaponFrame", "PrimaryWeaponWindow",
            "SecondayWeaponUI", "AltFireIcon", "WeaponBarReload",
            "CoreHealthBarRefill", "CoreHealthBarFrame", "CoreHealthBarWindow",
            "Crosshair", "Crosshair2"
        };

        // Scene paths
        private const string MAIN_MENU_SCENE_PATH = "Resources/Sources/Scenes/MainMenu.json";
        private const string GAME_SCENE_PATH = "Resources/Sources/Scenes/trench_run.json";
        private const string LEVEL2_SCENE_PATH = "Resources/Sources/Scenes/level2.json";

        private const string EVENT_GAME_PAUSED = "GamePaused";
        private const string EVENT_GAME_RESUMED = "GameResumed";

        // Entity IDs
        private uint bgId;
        private uint resumeButtonId, resumeButtonHoveredId;
        private uint restartButtonId, restartButtonHoveredId;
        private uint plusButtonId, plusButtonHoveredId;
        private uint mixerFillId;
        private uint minusButtonId, minusButtonHoveredId;
        private uint mainMenuButtonId, mainMenuButtonHoveredId;
        private uint howToPlayButtonId, howToPlayButtonHoveredId;
        // Mixer 2 - BGM
        private uint mixerFillId2;
        private uint plusButtonId2, plusButtonHoveredId2;
        private uint minusButtonId2, minusButtonHoveredId2;
        // Mixer 3 - SFX
        private uint mixerFillId3;
        private uint plusButtonId3, plusButtonHoveredId3;
        private uint minusButtonId3, minusButtonHoveredId3;
        // Mixer 4 - Gamma
        private uint mixerFillId4;
        private uint plusButtonId4, plusButtonHoveredId4;
        private uint minusButtonId4, minusButtonHoveredId4;
        private uint defaultButtonId4, defaultButtonHoveredId4;
        // Mixer 5 - Mouse Sensitivity
        private uint mixerFillId5;
        private uint plusButtonId5, plusButtonHoveredId5;
        private uint minusButtonId5, minusButtonHoveredId5;
        private uint defaultButtonId5, defaultButtonHoveredId5;
        // Checkboxes
        private uint checkboxMasterUntickedId, checkboxMasterTickedId;
        private uint checkboxBGMUntickedId, checkboxBGMTickedId;
        private uint checkboxSFXUntickedId, checkboxSFXTickedId;

        private uint[] hudElementIds;

        // State
        private bool isPaused = false;
        private bool entitiesFound = false;
        private bool wasPauseKeyPressed = false;
        private bool wasMousePressed = false;
        private bool gameEnded = false;
        private string currentGameScenePath = GAME_SCENE_PATH;

        // Mixer initial X scales and positions (horizontal fill, anchored right)
        private float mixerFill1InitialWidth;
        private float mixerFill2InitialWidth;
        private float mixerFill3InitialWidth;
        private float mixerFill4InitialWidth;
        private float mixerFill5InitialWidth;
        private Vector3 mixerFill1InitialPosition;
        private Vector3 mixerFill2InitialPosition;
        private Vector3 mixerFill3InitialPosition;
        private Vector3 mixerFill4InitialPosition;
        private Vector3 mixerFill5InitialPosition;

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
            // Mixer 2
            mixerFillId2 = SceneFindEntityByName(MIXER_FILL_2_NAME);
            plusButtonId2 = SceneFindEntityByName(PLUS_BUTTON_2_NAME);
            plusButtonHoveredId2 = SceneFindEntityByName(PLUS_BUTTON_2_HOVERED_NAME);
            minusButtonId2 = SceneFindEntityByName(MINUS_BUTTON_2_NAME);
            minusButtonHoveredId2 = SceneFindEntityByName(MINUS_BUTTON_2_HOVERED_NAME);
            // Mixer 3
            mixerFillId3 = SceneFindEntityByName(MIXER_FILL_3_NAME);
            plusButtonId3 = SceneFindEntityByName(PLUS_BUTTON_3_NAME);
            plusButtonHoveredId3 = SceneFindEntityByName(PLUS_BUTTON_3_HOVERED_NAME);
            minusButtonId3 = SceneFindEntityByName(MINUS_BUTTON_3_NAME);
            minusButtonHoveredId3 = SceneFindEntityByName(MINUS_BUTTON_3_HOVERED_NAME);
            // Mixer 4 - Gamma
            mixerFillId4 = SceneFindEntityByName(MIXER_FILL_4_NAME);
            plusButtonId4 = SceneFindEntityByName(PLUS_BUTTON_4_NAME);
            plusButtonHoveredId4 = SceneFindEntityByName(PLUS_BUTTON_4_HOVERED_NAME);
            minusButtonId4 = SceneFindEntityByName(MINUS_BUTTON_4_NAME);
            minusButtonHoveredId4 = SceneFindEntityByName(MINUS_BUTTON_4_HOVERED_NAME);
            defaultButtonId4 = SceneFindEntityByName(DEFAULT_BUTTON_4_NAME);
            defaultButtonHoveredId4 = SceneFindEntityByName(DEFAULT_BUTTON_4_HOVERED_NAME);
            // Mixer 5 - Mouse Sensitivity
            mixerFillId5 = SceneFindEntityByName(MIXER_FILL_5_NAME);
            plusButtonId5 = SceneFindEntityByName(PLUS_BUTTON_5_NAME);
            plusButtonHoveredId5 = SceneFindEntityByName(PLUS_BUTTON_5_HOVERED_NAME);
            minusButtonId5 = SceneFindEntityByName(MINUS_BUTTON_5_NAME);
            minusButtonHoveredId5 = SceneFindEntityByName(MINUS_BUTTON_5_HOVERED_NAME);
            defaultButtonId5 = SceneFindEntityByName(DEFAULT_BUTTON_5_NAME);
            defaultButtonHoveredId5 = SceneFindEntityByName(DEFAULT_BUTTON_5_HOVERED_NAME);
            // Checkboxes
            checkboxMasterUntickedId = SceneFindEntityByName(CHECKBOX_MASTER_UNTICKED_NAME);
            checkboxMasterTickedId = SceneFindEntityByName(CHECKBOX_MASTER_TICKED_NAME);
            checkboxBGMUntickedId = SceneFindEntityByName(CHECKBOX_BGM_UNTICKED_NAME);
            checkboxBGMTickedId = SceneFindEntityByName(CHECKBOX_BGM_TICKED_NAME);
            checkboxSFXUntickedId = SceneFindEntityByName(CHECKBOX_SFX_UNTICKED_NAME);
            checkboxSFXTickedId = SceneFindEntityByName(CHECKBOX_SFX_TICKED_NAME);

            HidePauseMenu();

            // Read initial X scales and positions for mixer fills
            if (mixerFillId != 0)
            {
                mixerFill1InitialWidth = GetScale(mixerFillId).X;
                mixerFill1InitialPosition = GetPosition(mixerFillId);
                LogMessage("PauseMenuPopup: Mixer 1 initial width = " + mixerFill1InitialWidth);
            }
            if (mixerFillId2 != 0)
            {
                mixerFill2InitialWidth = GetScale(mixerFillId2).X;
                mixerFill2InitialPosition = GetPosition(mixerFillId2);
                LogMessage("PauseMenuPopup: Mixer 2 initial width = " + mixerFill2InitialWidth);
            }
            if (mixerFillId3 != 0)
            {
                mixerFill3InitialWidth = GetScale(mixerFillId3).X;
                mixerFill3InitialPosition = GetPosition(mixerFillId3);
                LogMessage("PauseMenuPopup: Mixer 3 initial width = " + mixerFill3InitialWidth);
            }
            if (mixerFillId4 != 0)
            {
                mixerFill4InitialWidth = GetScale(mixerFillId4).X;
                mixerFill4InitialPosition = GetPosition(mixerFillId4);
                LogMessage("PauseMenuPopup: Mixer 4 (Gamma) initial width = " + mixerFill4InitialWidth);
            }
            if (mixerFillId5 != 0)
            {
                mixerFill5InitialWidth = GetScale(mixerFillId5).X;
                mixerFill5InitialPosition = GetPosition(mixerFillId5);
                LogMessage("PauseMenuPopup: Mixer 5 (MouseSens) initial width = " + mixerFill5InitialWidth);
            }

            hudElementIds = new uint[HUD_ELEMENT_NAMES.Length];
            for (int i = 0; i < HUD_ELEMENT_NAMES.Length; i++)
            {
                hudElementIds[i] = SceneFindEntityByName(HUD_ELEMENT_NAMES[i]);
                if (hudElementIds[i] == 0)
                    LogMessage("PauseMenuPopup: HUD element not found: " + HUD_ELEMENT_NAMES[i]);
            }

            if (bgId == 0) LogError("PauseMenuPopup: Could not find: " + BG_NAME);

            entitiesFound = (bgId != 0);
            isPaused = false;
            gameEnded = false;

            uint[] turrets = SceneFindEntitiesByTag("EnemyTurret");
            if (turrets != null && turrets.Length > 0)
            {
                currentGameScenePath = GAME_SCENE_PATH;
                LogMessage("PauseMenuPopup: Detected Level 1");
            }
            else
            {
                currentGameScenePath = LEVEL2_SCENE_PATH;
                LogMessage("PauseMenuPopup: Detected Level 2");
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
                wasPauseKeyPressed = Input.IsKeyPressed(KeyCode.Escape);
                return;
            }

            bool pauseKeyPressed = Input.IsKeyPressed(KeyCode.Escape);
            bool pauseKeyJustPressed = pauseKeyPressed && !wasPauseKeyPressed;
            wasPauseKeyPressed = pauseKeyPressed;

            if (pauseKeyJustPressed)
            {
                LogMessage("PauseMenuPopup: Escape pressed! isPaused=" + isPaused);
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
        // =====================================================================
        private void ShowPauseMenu()
        {
            if (isPaused) return;

            isPaused = true;
            GameState.IsPaused = true;
            Input.SetCursorVisible(true);
            Event.Publish(EVENT_GAME_PAUSED, "");

            SafeSetVisible(bgId, true);
            SafeSetVisible(resumeButtonId, true);
            SafeSetVisible(restartButtonId, true);
            SafeSetVisible(plusButtonId, true);
            SafeSetVisible(mixerFillId, true);
            SafeSetVisible(minusButtonId, true);
            SafeSetVisible(mainMenuButtonId, true);
            SafeSetVisible(howToPlayButtonId, true);
            // Mixer 2
            SafeSetVisible(mixerFillId2, true);
            SafeSetVisible(plusButtonId2, true);
            SafeSetVisible(minusButtonId2, true);
            // Mixer 3
            SafeSetVisible(mixerFillId3, true);
            SafeSetVisible(plusButtonId3, true);
            SafeSetVisible(minusButtonId3, true);
            // Mixer 4 - Gamma
            SafeSetVisible(mixerFillId4, true);
            SafeSetVisible(plusButtonId4, true);
            SafeSetVisible(minusButtonId4, true);
            SafeSetVisible(defaultButtonId4, true);
            // Mixer 5 - Mouse Sensitivity
            SafeSetVisible(mixerFillId5, true);
            SafeSetVisible(plusButtonId5, true);
            SafeSetVisible(minusButtonId5, true);
            SafeSetVisible(defaultButtonId5, true);

            // All hovered versions start hidden
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
            SafeSetVisible(plusButtonHoveredId4, false);
            SafeSetVisible(minusButtonHoveredId4, false);
            SafeSetVisible(defaultButtonHoveredId4, false);
            SafeSetVisible(plusButtonHoveredId5, false);
            SafeSetVisible(minusButtonHoveredId5, false);
            SafeSetVisible(defaultButtonHoveredId5, false);

            // Update all mixer fills
            if (Instance != null)
            {
                UpdateMixerFill(mixerFillId, Instance.GetMasterVolume(),
                                mixerFill1InitialWidth, mixerFill1InitialPosition);
                UpdateMixerFill(mixerFillId2, Instance.GetBGMVolume(),
                                mixerFill2InitialWidth, mixerFill2InitialPosition);
                UpdateMixerFill(mixerFillId3, Instance.GetSFXVolume(),
                                mixerFill3InitialWidth, mixerFill3InitialPosition);
                UpdateMixerFill(mixerFillId4, Instance.GetGammaNormalized(),
                                mixerFill4InitialWidth, mixerFill4InitialPosition);
                UpdateMixerFill(mixerFillId5, Instance.GetMouseSensitivityNormalized(),
                                mixerFill5InitialWidth, mixerFill5InitialPosition);
            }

            if (hudElementIds != null)
                for (int i = 0; i < hudElementIds.Length; i++)
                    SafeSetVisible(hudElementIds[i], false);

            UpdateCheckboxVisuals();
            LogMessage("PauseMenuPopup: Menu shown");
        }

        // =====================================================================
        // HIDE PAUSE MENU
        // =====================================================================
        private void HidePauseMenu()
        {
            isPaused = false;

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
            // Mixer 2
            SafeSetVisible(mixerFillId2, false);
            SafeSetVisible(plusButtonId2, false);
            SafeSetVisible(plusButtonHoveredId2, false);
            SafeSetVisible(minusButtonId2, false);
            SafeSetVisible(minusButtonHoveredId2, false);
            // Mixer 3
            SafeSetVisible(mixerFillId3, false);
            SafeSetVisible(plusButtonId3, false);
            SafeSetVisible(plusButtonHoveredId3, false);
            SafeSetVisible(minusButtonId3, false);
            SafeSetVisible(minusButtonHoveredId3, false);
            // Mixer 4 - Gamma
            SafeSetVisible(mixerFillId4, false);
            SafeSetVisible(plusButtonId4, false);
            SafeSetVisible(plusButtonHoveredId4, false);
            SafeSetVisible(minusButtonId4, false);
            SafeSetVisible(minusButtonHoveredId4, false);
            SafeSetVisible(defaultButtonId4, false);
            SafeSetVisible(defaultButtonHoveredId4, false);
            // Mixer 5 - Mouse Sensitivity
            SafeSetVisible(mixerFillId5, false);
            SafeSetVisible(plusButtonId5, false);
            SafeSetVisible(plusButtonHoveredId5, false);
            SafeSetVisible(minusButtonId5, false);
            SafeSetVisible(minusButtonHoveredId5, false);
            SafeSetVisible(defaultButtonId5, false);
            SafeSetVisible(defaultButtonHoveredId5, false);
            // Checkboxes
            SafeSetVisible(checkboxMasterUntickedId, false);
            SafeSetVisible(checkboxMasterTickedId, false);
            SafeSetVisible(checkboxBGMUntickedId, false);
            SafeSetVisible(checkboxBGMTickedId, false);
            SafeSetVisible(checkboxSFXUntickedId, false);
            SafeSetVisible(checkboxSFXTickedId, false);

            if (hudElementIds != null)
                for (int i = 0; i < hudElementIds.Length; i++)
                    SafeSetVisible(hudElementIds[i], true);

            if (entitiesFound)
            {
                GameState.IsPaused = false;
                Event.Publish(EVENT_GAME_RESUMED, "");
                Input.SetCursorVisible(false);
            }
        }

        // =====================================================================
        // HOVER STATES
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
            UpdateButtonHover(plusButtonId4, plusButtonHoveredId4);
            UpdateButtonHover(minusButtonId4, minusButtonHoveredId4);
            UpdateButtonHover(defaultButtonId4, defaultButtonHoveredId4);
            UpdateButtonHover(plusButtonId5, plusButtonHoveredId5);
            UpdateButtonHover(minusButtonId5, minusButtonHoveredId5);
            UpdateButtonHover(defaultButtonId5, defaultButtonHoveredId5);
        }

        private void UpdateButtonHover(uint normalId, uint hoveredId)
        {
            if (normalId == 0 || hoveredId == 0) return;
            bool isHovered = Collision2D.IsMouseCollidingWithEntity(normalId) ||
                             Collision2D.IsMouseCollidingWithEntity(hoveredId);
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
                LogMessage("PauseMenuPopup: Restart clicked");
                StopGroup(AudioType.BGM);
                StopGroup(AudioType.SFX);
                Input.SetCursorVisible(false);
                isPaused = false;
                GameState.IsPaused = false;
                Scene.SceneLoadFromFile(currentGameScenePath);
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
                Scene.SceneLoadFromFile(MAIN_MENU_SCENE_PATH);
                return;
            }

            if (IsButtonClicked(howToPlayButtonId, howToPlayButtonHoveredId))
            {
                LogMessage("PauseMenuPopup: How To Play clicked");
                return;
            }

            // Master volume +/-
            if (IsButtonClicked(plusButtonId, plusButtonHoveredId))
            {
                if (Instance != null)
                {
                    Instance.SetMasterVolume(Instance.GetMasterVolume() + 0.1f);
                    UpdateMixerFill(mixerFillId, Instance.GetMasterVolume(),
                                    mixerFill1InitialWidth, mixerFill1InitialPosition);
                }
                return;
            }
            if (IsButtonClicked(minusButtonId, minusButtonHoveredId))
            {
                if (Instance != null)
                {
                    Instance.SetMasterVolume(Instance.GetMasterVolume() - 0.1f);
                    UpdateMixerFill(mixerFillId, Instance.GetMasterVolume(),
                                    mixerFill1InitialWidth, mixerFill1InitialPosition);
                }
                return;
            }

            // BGM volume +/-
            if (IsButtonClicked(plusButtonId2, plusButtonHoveredId2))
            {
                if (Instance != null)
                {
                    Instance.SetBGMVolume(Instance.GetBGMVolume() + 0.1f);
                    UpdateMixerFill(mixerFillId2, Instance.GetBGMVolume(),
                                    mixerFill2InitialWidth, mixerFill2InitialPosition);
                }
                return;
            }
            if (IsButtonClicked(minusButtonId2, minusButtonHoveredId2))
            {
                if (Instance != null)
                {
                    Instance.SetBGMVolume(Instance.GetBGMVolume() - 0.1f);
                    UpdateMixerFill(mixerFillId2, Instance.GetBGMVolume(),
                                    mixerFill2InitialWidth, mixerFill2InitialPosition);
                }
                return;
            }

            // SFX volume +/-
            if (IsButtonClicked(plusButtonId3, plusButtonHoveredId3))
            {
                if (Instance != null)
                {
                    Instance.SetSFXVolume(Instance.GetSFXVolume() + 0.1f);
                    UpdateMixerFill(mixerFillId3, Instance.GetSFXVolume(),
                                    mixerFill3InitialWidth, mixerFill3InitialPosition);
                }
                return;
            }
            if (IsButtonClicked(minusButtonId3, minusButtonHoveredId3))
            {
                if (Instance != null)
                {
                    Instance.SetSFXVolume(Instance.GetSFXVolume() - 0.1f);
                    UpdateMixerFill(mixerFillId3, Instance.GetSFXVolume(),
                                    mixerFill3InitialWidth, mixerFill3InitialPosition);
                }
                return;
            }

            // Gamma +/-/default
            if (IsButtonClicked(plusButtonId4, plusButtonHoveredId4))
            {
                if (Instance != null)
                {
                    Instance.IncrementGamma();
                    UpdateMixerFill(mixerFillId4, Instance.GetGammaNormalized(),
                                    mixerFill4InitialWidth, mixerFill4InitialPosition);
                }
                return;
            }
            if (IsButtonClicked(minusButtonId4, minusButtonHoveredId4))
            {
                if (Instance != null)
                {
                    Instance.DecrementGamma();
                    UpdateMixerFill(mixerFillId4, Instance.GetGammaNormalized(),
                                    mixerFill4InitialWidth, mixerFill4InitialPosition);
                }
                return;
            }
            if (IsButtonClicked(defaultButtonId4, defaultButtonHoveredId4))
            {
                if (Instance != null)
                {
                    Instance.ResetGamma();
                    UpdateMixerFill(mixerFillId4, Instance.GetGammaNormalized(),
                                    mixerFill4InitialWidth, mixerFill4InitialPosition);
                    LogMessage("PauseMenuPopup: Gamma reset to default");
                }
                return;
            }

            // Mouse Sensitivity +/-/default
            if (IsButtonClicked(plusButtonId5, plusButtonHoveredId5))
            {
                if (Instance != null)
                {
                    Instance.SetMouseSensitivityUp();
                    UpdateMixerFill(mixerFillId5, Instance.GetMouseSensitivityNormalized(),
                                    mixerFill5InitialWidth, mixerFill5InitialPosition);
                }
                return;
            }
            if (IsButtonClicked(minusButtonId5, minusButtonHoveredId5))
            {
                if (Instance != null)
                {
                    Instance.SetMouseSensitivityDown();
                    UpdateMixerFill(mixerFillId5, Instance.GetMouseSensitivityNormalized(),
                                    mixerFill5InitialWidth, mixerFill5InitialPosition);
                }
                return;
            }
            if (IsButtonClicked(defaultButtonId5, defaultButtonHoveredId5))
            {
                if (Instance != null)
                {
                    Instance.ResetMouseSensitivity();
                    UpdateMixerFill(mixerFillId5, Instance.GetMouseSensitivityNormalized(),
                                    mixerFill5InitialWidth, mixerFill5InitialPosition);
                    LogMessage("PauseMenuPopup: Mouse sensitivity reset to default");
                }
                return;
            }

            // Checkboxes
            if (IsCheckboxClicked(checkboxMasterUntickedId, checkboxMasterTickedId))
            {
                if (Instance != null) { Instance.ToggleMasterMute(); UpdateCheckboxVisuals(); }
                return;
            }
            if (IsCheckboxClicked(checkboxBGMUntickedId, checkboxBGMTickedId))
            {
                if (Instance != null) { Instance.ToggleBGMMute(); UpdateCheckboxVisuals(); }
                return;
            }
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
        // CHECKBOX VISUALS
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
        // MIXER FILL - matches SettingsPopup logic exactly
        // Shrinks X scale, shifts X by full widthDiff to anchor right edge
        // =====================================================================
        private void UpdateMixerFill(uint fillId, float volume, float initialWidth, Vector3 initialPosition)
        {
            if (fillId == 0) return;

            if (volume < 0.0f) volume = 0.0f;
            if (volume > 1.0f) volume = 1.0f;

            float newWidth = initialWidth * volume;
            Vector3 currentScale = GetScale(fillId);
            Vector3 newScale = new Vector3(newWidth, currentScale.Y, currentScale.Z);
            SetScale(fillId, ref newScale);

            float widthDiff = initialWidth - newWidth;
            Vector3 newPos = new Vector3(initialPosition.X - widthDiff, initialPosition.Y, initialPosition.Z);
            SetPosition(fillId, ref newPos);

            LogMessage("PauseMenuPopup MixerFill: volume=" + volume.ToString("F2") +
                       " width=" + newWidth.ToString("F1") + " posX=" + newPos.X.ToString("F1"));
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

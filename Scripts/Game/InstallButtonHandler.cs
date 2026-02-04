// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Audio;

namespace Game
{
    public class InstallButtonHandler : ScriptBehaviour
    {
        // Entity names
        private const string INSTALL_BUTTON_NAME = "Install Button";
        private const string GLITCH_OVERLAY_NAME = "Glitch Overlay";
        private const string START_GAME_POPUP_NAME = "Start Game Popup";
        private const string START_GAME_YES_BUTTON_NAME = "Start Game Yes Button";
        private const string BSOD_SCREEN_1_NAME = "BSOD Screen 1";
        private const string BSOD_SCREEN_2_NAME = "BSOD Screen 2";
        private const string BSOD_SCREEN_3_NAME = "BSOD Screen 3";
        private const string BLACK_SCREEN_NAME = "Black Screen";

        // Audio entity names
        private const string BGM_OFFICE_AMBIENCE_NAME = "BGM Office Ambience";
        private const string BGM_ROOM_AMBIENCE_NAME = "BGM Room Ambience";
        private const string BSOD_ERROR_SOUND_NAME = "UI BSOD Error Sound";

        private const string MAIN_GAME_SCENE_PATH = "Resources/Sources/Scenes/Level1_NewPlayer.json";
        private const int NUM_ERROR_POPUPS = 8;

        private const float HIDDEN_Y = -500.0f;
        private const float SCREEN_CENTER_X = 640.0f;
        private const float SCREEN_CENTER_Y = 360.0f;

        // Visible positions for each error popup (index 0 = popup 1, index 7 = popup 8)
        private static readonly float[] POPUP_VISIBLE_X = { 750.0f, 850.0f, 600.0f, 450.0f, 400.0f, 350.0f, 300.0f, 250.0f };
        private static readonly float[] POPUP_VISIBLE_Y = { 450.0f, 250.0f, 500.0f, 400.0f, 300.0f, 250.0f, 200.0f, 150.0f };

        // Timestamps for showing popups (in seconds)
        // First batch: popup 8, 7, 6, 5 at 0.15s, 0.25s, 0.32s, 0.36s
        // Glitch overlay at 0.39s
        // Second batch: popup 4, 3, 2, 1 at 0.42s, 0.46s, 0.50s, 0.55s
        // Start game popup + button at 1.03s
        private static readonly float[] POPUP_TIMESTAMPS = {
            0.55f,  // Popup 1
            0.50f,  // Popup 2
            0.46f,  // Popup 3
            0.42f,  // Popup 4
            0.36f,  // Popup 5
            0.32f,  // Popup 6
            0.25f,  // Popup 7
            0.15f   // Popup 8
        };
        private const float GLITCH_TIMESTAMP = 0.39f;
        private const float START_GAME_TIMESTAMP = 1.03f;

        // BSOD sequence timestamps
        private const float BSOD_1_TIMESTAMP = 0.0f;    // Immediately
        private const float BSOD_2_TIMESTAMP = 0.54f;
        private const float BSOD_3_TIMESTAMP = 1.17f;
        private const float BLACK_SCREEN_TIMESTAMP = 2.05f;
        private const float LOAD_SCENE_TIMESTAMP = 2.56f;

        // Entity IDs
        private uint installButtonId;
        private uint[] errorPopupIds = new uint[NUM_ERROR_POPUPS];
        private uint glitchOverlayId;
        private uint startGamePopupId;
        private uint startGameYesButtonId;
        private uint bsodScreen1Id;
        private uint bsodScreen2Id;
        private uint bsodScreen3Id;
        private uint blackScreenId;

        // Audio entity IDs
        private uint bgmOfficeAmbienceId;
        private uint bgmRoomAmbienceId;
        private uint bsodErrorSoundId;
        private uint[] errorPopupSoundIds = new uint[NUM_ERROR_POPUPS];

        // State
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool sequenceStarted = false;
        private bool sequenceComplete = false;
        private bool bsodSequenceStarted = false;
        private bool sceneLoadTriggered = false;
        private float elapsedTime = 0.0f;
        private float bsodElapsedTime = 0.0f;

        // Track which elements have been shown
        private bool[] popupShown = new bool[NUM_ERROR_POPUPS];
        private bool glitchShown = false;
        private bool startGameShown = false;
        private bool bsod1Shown = false;
        private bool bsod2Shown = false;
        private bool bsod3Shown = false;
        private bool blackScreenShown = false;

        // Glitch overlay position
        private const float GLITCH_X = 640.0f;
        private const float GLITCH_Y = 360.0f;
        private const float GLITCH_Z = -0.9f;

        // Start game popup position
        private const float START_POPUP_X = 640.0f;
        private const float START_POPUP_Y = 360.0f;
        private const float START_POPUP_Z = -1.0f;

        // Start game yes button position
        private const float START_BUTTON_X = 568.0f;
        private const float START_BUTTON_Y = 437.0f;
        private const float START_BUTTON_Z = -0.2f;

        // BSOD Z positions (layered on top of each other, using working range)
        private const float BSOD_1_Z = -0.2f;
        private const float BSOD_2_Z = -0.25f;
        private const float BSOD_3_Z = -0.3f;
        private const float BLACK_SCREEN_Z = -0.35f;

        public override void OnStart()
        {
            LogMessage("InstallButtonHandler: Initializing...");

            // Find install button
            installButtonId = SceneFindEntityByName(INSTALL_BUTTON_NAME);
            if (installButtonId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + INSTALL_BUTTON_NAME);
                return;
            }

            // Find error popups
            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                string name = "Error Popup " + (i + 1);
                errorPopupIds[i] = SceneFindEntityByName(name);

                if (errorPopupIds[i] == 0)
                {
                    LogError("InstallButtonHandler: Could not find entity: " + name);
                    return;
                }
                popupShown[i] = false;
            }

            // Find glitch overlay
            glitchOverlayId = SceneFindEntityByName(GLITCH_OVERLAY_NAME);
            if (glitchOverlayId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + GLITCH_OVERLAY_NAME);
                return;
            }

            // Find start game popup
            startGamePopupId = SceneFindEntityByName(START_GAME_POPUP_NAME);
            if (startGamePopupId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + START_GAME_POPUP_NAME);
                return;
            }

            // Find start game yes button
            startGameYesButtonId = SceneFindEntityByName(START_GAME_YES_BUTTON_NAME);
            if (startGameYesButtonId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + START_GAME_YES_BUTTON_NAME);
                return;
            }

            // Find BSOD screens
            bsodScreen1Id = SceneFindEntityByName(BSOD_SCREEN_1_NAME);
            if (bsodScreen1Id == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + BSOD_SCREEN_1_NAME);
                return;
            }

            bsodScreen2Id = SceneFindEntityByName(BSOD_SCREEN_2_NAME);
            if (bsodScreen2Id == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + BSOD_SCREEN_2_NAME);
                return;
            }

            bsodScreen3Id = SceneFindEntityByName(BSOD_SCREEN_3_NAME);
            if (bsodScreen3Id == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + BSOD_SCREEN_3_NAME);
                return;
            }

            blackScreenId = SceneFindEntityByName(BLACK_SCREEN_NAME);
            if (blackScreenId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + BLACK_SCREEN_NAME);
                return;
            }

            // Find audio entities (optional - don't fail if not found)
            bgmOfficeAmbienceId = SceneFindEntityByName(BGM_OFFICE_AMBIENCE_NAME);
            if (bgmOfficeAmbienceId == 0)
            {
                LogMessage("InstallButtonHandler: BGM Office Ambience not found (optional)");
            }

            bgmRoomAmbienceId = SceneFindEntityByName(BGM_ROOM_AMBIENCE_NAME);
            if (bgmRoomAmbienceId == 0)
            {
                LogMessage("InstallButtonHandler: BGM Room Ambience not found (optional)");
            }

            bsodErrorSoundId = SceneFindEntityByName(BSOD_ERROR_SOUND_NAME);
            if (bsodErrorSoundId == 0)
            {
                LogMessage("InstallButtonHandler: BSOD Error Sound not found (optional)");
            }

            // Find error popup sound entities (one per popup for overlapping sounds)
            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                string soundName = "UI Error Popup Sound " + (i + 1);
                errorPopupSoundIds[i] = SceneFindEntityByName(soundName);
                if (errorPopupSoundIds[i] == 0)
                {
                    LogError("InstallButtonHandler: " + soundName + " NOT FOUND!");
                }
                else
                {
                    LogMessage("InstallButtonHandler: Found " + soundName + " with ID: " + errorPopupSoundIds[i]);
                }
            }

            entitiesFound = true;
            wasMousePressed = false;

            // Hide all elements initially (move off-screen)
            HideAllElements();

            LogMessage("InstallButtonHandler: Ready! All entities found.");
        }

        private void HideAllElements()
        {
            // Hide error popups
            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                Vector3 hidePos = new Vector3(POPUP_VISIBLE_X[i], HIDDEN_Y, -0.7f);
                SetPosition(errorPopupIds[i], ref hidePos);
            }

            // Hide glitch overlay
            Vector3 glitchHidePos = new Vector3(GLITCH_X, HIDDEN_Y, GLITCH_Z);
            SetPosition(glitchOverlayId, ref glitchHidePos);

            // Hide start game popup
            Vector3 startPopupHidePos = new Vector3(START_POPUP_X, HIDDEN_Y, START_POPUP_Z);
            SetPosition(startGamePopupId, ref startPopupHidePos);

            // Hide start game yes button
            Vector3 startButtonHidePos = new Vector3(START_BUTTON_X, HIDDEN_Y, START_BUTTON_Z);
            SetPosition(startGameYesButtonId, ref startButtonHidePos);

            // Hide BSOD screens
            Vector3 bsod1HidePos = new Vector3(SCREEN_CENTER_X, HIDDEN_Y, BSOD_1_Z);
            SetPosition(bsodScreen1Id, ref bsod1HidePos);

            Vector3 bsod2HidePos = new Vector3(SCREEN_CENTER_X, HIDDEN_Y, BSOD_2_Z);
            SetPosition(bsodScreen2Id, ref bsod2HidePos);

            Vector3 bsod3HidePos = new Vector3(SCREEN_CENTER_X, HIDDEN_Y, BSOD_3_Z);
            SetPosition(bsodScreen3Id, ref bsod3HidePos);

            Vector3 blackScreenHidePos = new Vector3(SCREEN_CENTER_X, HIDDEN_Y, BLACK_SCREEN_Z);
            SetPosition(blackScreenId, ref blackScreenHidePos);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            // Handle mouse click detection
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            // If sequence not started, check for install button click
            if (!sequenceStarted && mouseJustPressed)
            {
                if (Collision2D.IsMouseCollidingWithEntity(installButtonId))
                {
                    LogMessage("InstallButtonHandler: Install button clicked - starting sequence");
                    sequenceStarted = true;
                    elapsedTime = 0.0f;
                }
            }

            // Update error popup sequence if started
            if (sequenceStarted && !sequenceComplete)
            {
                elapsedTime += deltaTime;
                UpdateSequence();
            }

            // Handle start game yes button click after sequence is complete (before BSOD)
            if (sequenceComplete && !bsodSequenceStarted && mouseJustPressed)
            {
                if (Collision2D.IsMouseCollidingWithEntity(startGameYesButtonId))
                {
                    LogMessage("InstallButtonHandler: Start Game Yes button clicked - starting BSOD sequence");
                    bsodSequenceStarted = true;
                    bsodElapsedTime = 0.0f;
                }
            }

            // Update BSOD sequence if started
            if (bsodSequenceStarted && !sceneLoadTriggered)
            {
                bsodElapsedTime += deltaTime;
                UpdateBSODSequence();
            }
        }

        private void UpdateSequence()
        {
            // Show error popups at their timestamps
            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                if (!popupShown[i] && elapsedTime >= POPUP_TIMESTAMPS[i])
                {
                    ShowErrorPopup(i);
                    popupShown[i] = true;
                    LogMessage("InstallButtonHandler: Error Popup " + (i + 1) + " shown at " + elapsedTime + "s");
                }
            }

            // Show glitch overlay
            if (!glitchShown && elapsedTime >= GLITCH_TIMESTAMP)
            {
                ShowGlitchOverlay();
                glitchShown = true;
                LogMessage("InstallButtonHandler: Glitch overlay shown at " + elapsedTime + "s");
            }

            // Show start game popup and button
            if (!startGameShown && elapsedTime >= START_GAME_TIMESTAMP)
            {
                ShowStartGamePopup();
                startGameShown = true;
                sequenceComplete = true;
                LogMessage("InstallButtonHandler: Start game popup shown at " + elapsedTime + "s - Sequence complete");
            }
        }

        private void UpdateBSODSequence()
        {
            // Show BSOD Screen 1 immediately
            if (!bsod1Shown && bsodElapsedTime >= BSOD_1_TIMESTAMP)
            {
                ShowBSODScreen1();
                bsod1Shown = true;
                LogMessage("InstallButtonHandler: BSOD Screen 1 shown at " + bsodElapsedTime + "s");
            }

            // Show BSOD Screen 2 at 0.54s
            if (!bsod2Shown && bsodElapsedTime >= BSOD_2_TIMESTAMP)
            {
                ShowBSODScreen2();
                bsod2Shown = true;
                LogMessage("InstallButtonHandler: BSOD Screen 2 shown at " + bsodElapsedTime + "s");
            }

            // Show BSOD Screen 3 at 1.17s
            if (!bsod3Shown && bsodElapsedTime >= BSOD_3_TIMESTAMP)
            {
                ShowBSODScreen3();
                bsod3Shown = true;
                LogMessage("InstallButtonHandler: BSOD Screen 3 shown at " + bsodElapsedTime + "s");
            }

            // Show Black Screen at 2.05s
            if (!blackScreenShown && bsodElapsedTime >= BLACK_SCREEN_TIMESTAMP)
            {
                ShowBlackScreen();
                blackScreenShown = true;
                LogMessage("InstallButtonHandler: Black Screen shown at " + bsodElapsedTime + "s");
            }

            // Load scene at 2.56s
            if (!sceneLoadTriggered && bsodElapsedTime >= LOAD_SCENE_TIMESTAMP)
            {
                sceneLoadTriggered = true;
                LogMessage("InstallButtonHandler: Loading main game scene at " + bsodElapsedTime + "s");
                LogMessage("Scene path: " + MAIN_GAME_SCENE_PATH);
                Publish("LoadScene", MAIN_GAME_SCENE_PATH);
            }
        }

        private void ShowErrorPopup(int index)
        {
            Vector3 pos = new Vector3(POPUP_VISIBLE_X[index], POPUP_VISIBLE_Y[index], -0.7f - index * 0.01f);
            SetPosition(errorPopupIds[index], ref pos);

            // Play error popup sound using dedicated audio entity for this popup
            if (errorPopupSoundIds[index] != 0)
            {
                AudioPlay(errorPopupSoundIds[index]);
                LogMessage("InstallButtonHandler: Playing error popup sound " + (index + 1) + " (entity ID: " + errorPopupSoundIds[index] + ")");
            }
            else
            {
                LogError("InstallButtonHandler: Cannot play sound - entity for popup " + (index + 1) + " is null!");
            }
        }

        private void ShowGlitchOverlay()
        {
            Vector3 pos = new Vector3(GLITCH_X, GLITCH_Y, GLITCH_Z);
            SetPosition(glitchOverlayId, ref pos);
        }

        private void ShowStartGamePopup()
        {
            // Show popup
            Vector3 popupPos = new Vector3(START_POPUP_X, START_POPUP_Y, START_POPUP_Z);
            SetPosition(startGamePopupId, ref popupPos);

            // Show yes button
            Vector3 buttonPos = new Vector3(START_BUTTON_X, START_BUTTON_Y, START_BUTTON_Z);
            SetPosition(startGameYesButtonId, ref buttonPos);
        }

        private void ShowBSODScreen1()
        {
            Vector3 pos = new Vector3(SCREEN_CENTER_X, SCREEN_CENTER_Y, BSOD_1_Z);
            SetPosition(bsodScreen1Id, ref pos);

            // Stop BGM when BSOD appears
            if (bgmOfficeAmbienceId != 0)
            {
                AudioStop(bgmOfficeAmbienceId);
                LogMessage("InstallButtonHandler: Stopped BGM Office Ambience");
            }

            if (bgmRoomAmbienceId != 0)
            {
                AudioStop(bgmRoomAmbienceId);
                LogMessage("InstallButtonHandler: Stopped BGM Room Ambience");
            }

            // Play BSOD error sound
            if (bsodErrorSoundId != 0)
            {
                AudioPlay(bsodErrorSoundId);
                LogMessage("InstallButtonHandler: Playing BSOD Error Sound");
            }
        }

        private void ShowBSODScreen2()
        {
            Vector3 pos = new Vector3(SCREEN_CENTER_X, SCREEN_CENTER_Y, BSOD_2_Z);
            SetPosition(bsodScreen2Id, ref pos);
        }

        private void ShowBSODScreen3()
        {
            Vector3 pos = new Vector3(SCREEN_CENTER_X, SCREEN_CENTER_Y, BSOD_3_Z);
            SetPosition(bsodScreen3Id, ref pos);
        }

        private void ShowBlackScreen()
        {
            Vector3 pos = new Vector3(SCREEN_CENTER_X, SCREEN_CENTER_Y, BLACK_SCREEN_Z);
            SetPosition(blackScreenId, ref pos);
        }

        public override void OnDestroy()
        {
            LogMessage("InstallButtonHandler: Destroyed");
        }
    }
}

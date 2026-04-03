using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Camera;
using static Engine.Input;
using static Engine.ProgressTracker;

namespace Game
{
    public class TutorialUIManagerLevel3 : ScriptBehaviour
    {
        private enum TutorialState
        {
            Instruction,    // Show level 3 objective
            Done
        }

        private TutorialState currentState = TutorialState.Instruction;

        // Entity IDs - UI
        private uint instructionID;     // UI_InstructionLevel3
        private uint crosshairID;       // Crosshair
        private uint crosshair2ID;      // Crosshair2

        // Entity Names
        [SerializeField] private string instructionsName = "UI_InstructionLevel3";
        [SerializeField] private string crossHairName = "Crosshair";
        [SerializeField] private string crossHair2Name = "Crosshair2";

        // Events
        private const string EVENT_GAMELOSE = "LoseScreenShow";
        private const string EVENT_GAMEWIN = "GameWin";

        // Fading
        private bool EPressed = false;
        private float fadeOutElapsed = 0.0f;
        [SerializeField] private float fadeOutTime = 0.2f;
        private float fadeUpElapsed = 0.0f;
        [SerializeField] private float fadeUpTime = 1.0f;
        [SerializeField] private float uiStartFadePos = 260.0f;

        // -----------------------------------------------------------------------
        // Pause ownership model
        //   pauseForTutorial    : tutorial is actively driving the game pause
        //   playerPauseOnTutorial: user opened their pause menu OVER a tutorial pause
        //
        // While playerPauseOnTutorial == true, we publish TutorialPauseMenu=false
        // so the popup knows the tutorial is NOT the current pause owner, and can
        // unconditionally clear GameState.IsPaused when the user resumes.
        // -----------------------------------------------------------------------
        private bool gameEnd = false;
        private bool pauseForTutorial = false;       // tutorialForcedPause
        private bool playerPauseOnTutorial = false;  // userPauseMenuOpen (over tutorial)
        private bool prevEscapePressed = false;

        public override void OnStart()
        {
            instructionID = SceneFindEntityByName(instructionsName);
            crosshairID = SceneFindEntityByName(crossHairName);
            crosshair2ID = SceneFindEntityByName(crossHair2Name);

            SpriteRenderer.SetIsVisible(instructionID, false);

            Subscribe(EVENT_GAMELOSE, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);

            // Subscribe to GameResumed so we detect when the user closed the popup
            // via the Resume button (not just Escape). Clears playerPauseOnTutorial
            // so the tutorial reclaims pause ownership next frame.
            Subscribe("GameResumed", OnGameResumed);

            // Start the game paused for tutorial
            pauseForTutorial = true;
            LogMessage("[TutorialUIManagerLevel3] PAUSE-OWNER: tutorialForcedPause=true at start");
        }

        public override void OnUpdate(float deltaTime)
        {
            bool escapeJustPressed = IsEscapeJustPressed();

            // ---------------------------------------------------------------
            // BRANCH: user's pause menu is open on top of a tutorial pause.
            //
            // FIX: Publish TutorialPauseMenu=false while in this branch.
            //      The popup sees tutorial is NOT the pause owner and can
            //      cleanly clear GameState.IsPaused when Resume is pressed,
            //      without soft-locking.
            //
            //      On Escape, user is closing their menu → tutorial resumes
            //      control next frame.
            // ---------------------------------------------------------------
            if (pauseForTutorial && playerPauseOnTutorial)
            {
                // Tutorial yields ownership; user menu is active
                Publish("TutorialPauseAudio", false.ToString());
                Publish("TutorialPauseMenu", false.ToString());

                if (escapeJustPressed)
                {
                    playerPauseOnTutorial = false;
                    LogMessage("[TutorialUIManagerLevel3] PAUSE-OWNER: user closed menu via Escape; tutorial resumes control next frame");
                }
                return;
            }

            // ---------------------------------------------------------------
            // BRANCH: tutorial is actively forcing a pause.
            //
            // If Escape is pressed, user wants to open the pause menu.
            // Set playerPauseOnTutorial=true and return; the popup detects
            // Escape independently and opens the menu.
            // ---------------------------------------------------------------
            if (pauseForTutorial)
            {
                GameState.IsPaused = true;

                if (escapeJustPressed)
                {
                    playerPauseOnTutorial = true;
                    Publish("TutorialPauseAudio", false.ToString());
                    LogMessage("[TutorialUIManagerLevel3] PAUSE-OWNER: user pressed Escape during tutorial pause; yielding to user menu");
                    return;
                }
                else
                {
                    Publish("TutorialPauseAudio", pauseForTutorial.ToString());
                    Publish("TutorialPauseMenu", pauseForTutorial.ToString());
                    CrosshairVisibility(false);
                }

                switch (currentState)
                {
                    case TutorialState.Instruction:
                        HandleInstructionState(deltaTime);
                        break;
                    case TutorialState.Done:
                        break;
                }

                return;
            }

            // ---------------------------------------------------------------
            // BRANCH: tutorial is not forcing a pause (normal gameplay).
            // ---------------------------------------------------------------
            if (GameState.IsPaused)
                return;

            if (gameEnd)
                return;

            // Signal subscribers that Level 3 tutorial pause is not active
            if (currentState != TutorialState.Done)
            {
                Publish("TutorialPauseAudio", pauseForTutorial.ToString());
                Publish("TutorialPauseMenu", pauseForTutorial.ToString());
                CrosshairVisibility(true);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_GAMELOSE, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
            Unsubscribe("GameResumed", OnGameResumed);
        }

        private void HandleInstructionState(float dt)
        {
            ShowUI(instructionID, true, dt);

            if (fadeUpElapsed > fadeUpTime)
            {
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                if (EPressed)
                    ShowUI(instructionID, false, dt);

                if (fadeOutElapsed > fadeOutTime)
                {
                    EPressed = false;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.Done;
                    pauseForTutorial = false;
                    GameState.IsPaused = false;
                    Publish("BGMVOStart", "");
                    Publish("TUTORIALOVER", "");
                    Publish("TutorialPauseAudio", pauseForTutorial.ToString());
                    Publish("TutorialPauseMenu", pauseForTutorial.ToString());
                    CrosshairVisibility(true);
                    LogMessage("[TutorialUIManagerLevel3] PAUSE-OWNER: tutorialForcedPause=false, tutorial complete");
                }
            }
        }

        private void ShowUI(uint entityID, bool value, float dt)
        {
            if (value == true)
            {
                if (fadeUpElapsed < fadeUpTime) fadeUpElapsed += dt;
                SpriteRenderer.FadeIn(entityID, fadeUpElapsed, fadeUpTime);

                Vector3 newPos = Transform.GetPosition(entityID);
                newPos.Y = uiStartFadePos - (10.0f * fadeUpElapsed / fadeUpTime);
                Transform.SetPosition(entityID, ref newPos);

                if (SpriteRenderer.GetIsVisible(entityID) != value) SpriteRenderer.SetIsVisible(entityID, value);
            }
            else
            {
                fadeOutElapsed += dt;
                SpriteRenderer.FadeOut(entityID, fadeOutElapsed, fadeOutTime);

                if (fadeOutElapsed > fadeOutTime) SpriteRenderer.SetIsVisible(entityID, value);
            }
        }

        private bool IsEscapeJustPressed()
        {
            bool pressed = IsKeyPressed(KeyCode.Escape);
            bool justPressed = pressed && !prevEscapePressed;
            prevEscapePressed = pressed;
            return justPressed;
        }

        private void CrosshairVisibility(bool value)
        {
            SpriteRenderer.SetIsVisible(crosshairID, value);
            SpriteRenderer.SetIsVisible(crosshair2ID, value);
        }

        // -----------------------------------------------------------------------
        // Called when the popup closes (Resume button or Escape).
        // Clears playerPauseOnTutorial so the tutorial reclaims pause ownership
        // next frame. Handles the Resume-button case where no second Escape fires.
        // -----------------------------------------------------------------------
        private void OnGameResumed(string eventName, string payload)
        {
            if (playerPauseOnTutorial)
            {
                playerPauseOnTutorial = false;
                LogMessage("[TutorialUIManagerLevel3] PAUSE-OWNER: GameResumed received; tutorial reclaims pause control next frame");
            }
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("[TutorialUIManagerLevel3] Detect game end, hiding all tooltip");
            SpriteRenderer.SetIsVisible(instructionID, false);
            gameEnd = true;
        }
    }
}

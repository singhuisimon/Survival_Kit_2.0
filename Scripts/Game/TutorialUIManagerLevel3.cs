using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Camera;
using static Engine.Input;

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
        private uint crosshair2ID;       // Crosshair2

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

        // Flying Cam
        private Vector3 lastCamPos = Vector3.Zero;

        // Toggles/States
        private bool gameEnd = false;
        private bool pauseForTutorial = false;
        private bool playerPauseOnTutorial = false;
        private bool prevEscapePressed = false;

        public override void OnStart()
        {
            // Find all entities' id
            instructionID = SceneFindEntityByName(instructionsName);
            crosshairID = SceneFindEntityByName(crossHairName);
            crosshair2ID = SceneFindEntityByName(crossHair2Name);

            // Hide all tutorial UI
            SpriteRenderer.SetIsVisible(instructionID, false); 

            // Subscribe to the events
            Subscribe(EVENT_GAMELOSE, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);

            // Start the game paused for tutorial
            pauseForTutorial = true;
     
        }

        public override void OnUpdate(float deltaTime)
        {
            // Check for escape press state
            bool escapeJustPressed = IsEscapeJustPressed();

            // State when pausing during tutorial pause
            if(pauseForTutorial && playerPauseOnTutorial) {
                // Return to tutorial from pause menu
                if (escapeJustPressed) {
                    playerPauseOnTutorial = false;
                    Publish("TutorialPauseAudio", pauseForTutorial.ToString()); // Audio
                    Publish("TutorialPauseMenu", pauseForTutorial.ToString());  // Pause menu
                }
                return;
            }

            // Main tutorial logic
            if (pauseForTutorial) {

                GameState.IsPaused = true;

                // Ensure game is pausable in-game
                if (escapeJustPressed) {
                    // Inform subscribers pause is user-induced
                    playerPauseOnTutorial = true;
                    Publish("TutorialPauseAudio", false.ToString());
                    return;
                } else {
                    // Inform subscribers pause is tutorial-induced
                    Publish("TutorialPauseAudio", pauseForTutorial.ToString()); // Audio
                    Publish("TutorialPauseMenu", pauseForTutorial.ToString());  // Pause menu
                    CrosshairVisibility(false);
                }

                // Most updated player position
                switch (currentState) {
                    case TutorialState.Instruction:
                        HandleInstructionState(deltaTime);
                        break;
                    case TutorialState.Done:
                        // Do nothing, tutorial is done
                        break;
                }
                
                return;
            }

            // Non-tutorial pause from user
            if (GameState.IsPaused)
                return;

            if (gameEnd){
                return;
            }

            // Ensure subscribers know level 3 pause is not tutorial-induced
            if(currentState != TutorialState.Done)
            {
                Publish("TutorialPauseAudio", pauseForTutorial.ToString()); // Audio
                Publish("TutorialPauseMenu", pauseForTutorial.ToString());  // Pause menu
                CrosshairVisibility(true);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_GAMELOSE, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
        }

        private void HandleInstructionState(float dt)
        {
            // Show UI when state begins 
            ShowUI(instructionID, true, dt);

            // Ensure UI fades in fully before pausing state and waiting for interaction
            if (fadeUpElapsed > fadeUpTime) {

                // Check for 'E' input
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                // Begin fade out
                if (EPressed) {
                    ShowUI(instructionID, false, dt);
                    //ShowProceedText(false);  // Remove assistance message
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.Done;
                    pauseForTutorial = false;
                    GameState.IsPaused = false;
                    Publish("BGMVOStart", ""); // Signal BGM_VO to begin playing
                    Publish("TUTORIALOVER", ""); // Signal timer and other systems to start
                    Publish("TutorialPauseAudio", pauseForTutorial.ToString()); // Audio
                    Publish("TutorialPauseMenu", pauseForTutorial.ToString());  // Pause menu
                    CrosshairVisibility(true);
                }
            }
        }

        // UI Functions
        private void ShowUI(uint entityID, bool value, float dt)
        {
            // Fade Up if true
            if (value == true) {

                // Fade up
                if (fadeUpElapsed < fadeUpTime) fadeUpElapsed += dt;
                SpriteRenderer.FadeIn(entityID, fadeUpElapsed, fadeUpTime);

                Vector3 newPos = Transform.GetPosition(entityID);
                newPos.Y = uiStartFadePos - (10.0f * fadeUpElapsed / fadeUpTime);
                Transform.SetPosition(entityID, ref newPos);

                // Set sprite to be true if not visible
                if (SpriteRenderer.GetIsVisible(entityID) != value) SpriteRenderer.SetIsVisible(entityID, value);

            } else { // Fade out if false

                // Fade out
                fadeOutElapsed += dt;
                SpriteRenderer.FadeOut(entityID, fadeOutElapsed, fadeOutTime);

                // Ensure sprite is not visible once it faded out
                if (fadeOutElapsed > fadeOutTime) SpriteRenderer.SetIsVisible(entityID, value);
            }
        }

        private bool IsEscapeJustPressed() {
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

        private void OnGameEnd(string eventName, string payload){
            LogMessage("[TutorialUIManagerLevel3] Detect game end, hiding all tooltip");

            //Hide all the Tooltip UI
            SpriteRenderer.SetIsVisible(instructionID, false);
            gameEnd = true;
            return;
        }
    }
}
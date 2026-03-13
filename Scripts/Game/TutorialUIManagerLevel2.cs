using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Camera;
using static Engine.Input;

namespace Game
{
    public class TutorialUIManagerLevel2 : ScriptBehaviour
    {
        private enum TutorialState
        {
            Instruction,    // Show level objective
            BotnetInfo,     // Show info on botnet
            WormInfo,       // Show info on worm
            LoveletterInfo, // Show info on loveletter
            Done
        }

        private TutorialState currentState = TutorialState.Instruction;

        // Entity IDs - UI
        private uint instructionID;     // UI_InstructionLevel2
        private uint botnetInfoID;      // UI_BotnetInfo
        private uint wormInfoID;        // UI_WormInfo
        private uint loveletterInfoID;  // UI_LoveletterInfo
        private uint proceedID;         // UI_E_ToProceed

        // Entity IDs - Player
        private uint playerID;

        // Entity Names
        [SerializeField] private string playerName = "Player";
        [SerializeField] private string instructionsName = "UI_InstructionLevel2";
        [SerializeField] private string botnetInfoName = "UI_BotnetInfo";
        [SerializeField] private string wormInfoName = "UI_WormInfo";
        [SerializeField] private string loveletterInfoName = "UI_LoveletterInfo";
        [SerializeField] private string proceedName = "UI_E_ToProceed";

        // Events
        //private string EVENT_ONE_TURRET_DESTROYED = "OneTurretDestroyed";
        //private string EVENT_FIVE_TURRETS_DESTROYED = "FiveTurretsDestroyed";
        //private const string EVENT_ULT_CHARGED = "UltCharged";
        //private const string EVENT_ALT_FIRED = "AltFired";
        private const string EVENT_GAMELOSE = "LoseScreenShow";
        private const string EVENT_GAMEWIN = "GameWin";

        // Fading
        private bool EPressed = false;
        private float tooltipElapsed = 0.0f;
        [SerializeField] private float tooltipMinTime = 3.0f;
        private float fadeOutElapsed = 0.0f;
        [SerializeField] private float fadeOutTime = 0.2f;
        private float fadeUpElapsed = 0.0f;
        [SerializeField] private float fadeUpTime = 1.0f;
        [SerializeField] private float uiStartFadePos = 360.0f;
        [SerializeField] private float switchTime = 0.5f;

        private bool instructionsRead = false;
        private bool gameEnd = false;
        //private bool botnetSeen = false;
        //private bool wormSeen = false;
        //private bool loveletterSeen = false;

        public override void OnStart()
        {
            // Find all entities' id
            playerID = SceneFindEntityByName(playerName);
            instructionID = SceneFindEntityByName(instructionsName);
            botnetInfoID = SceneFindEntityByName(botnetInfoName);
            wormInfoID = SceneFindEntityByName(wormInfoName);
            loveletterInfoID = SceneFindEntityByName(loveletterInfoName);
            proceedID = SceneFindEntityByName(proceedName);

            // Hide all UI
            SpriteRenderer.SetIsVisible(instructionID, false); // Delay for a bit, fade it in upon entering level 2
            SpriteRenderer.SetIsVisible(botnetInfoID, false);
            SpriteRenderer.SetIsVisible(wormInfoID, false);
            SpriteRenderer.SetIsVisible(loveletterInfoID, false);
            SpriteRenderer.SetIsVisible(proceedID, false);

            // Subscribe to the events
            Subscribe(EVENT_GAMELOSE, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);

        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused)
                return;

            if (gameEnd){
                return;
            }

            // Most updated player position
            Vector3 currentPos = Transform.GetPosition(playerID);

            if (!instructionsRead) {

                // Ensure at least 3 seconds delay before first tool tip fades in
                if(tooltipElapsed < 3.0f) {
                    tooltipElapsed += deltaTime;
                } else {
                    ShowUI(instructionID, true, deltaTime);
                }

                if(fadeUpElapsed > fadeUpTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    instructionsRead = true;
                }
                return;
            }

            switch (currentState)
            {
                case TutorialState.Instruction:
                    HandleInstructionState(currentPos, deltaTime);
                    break;

                case TutorialState.BotnetInfo:
                    HandleBotnetInfoState(currentPos, deltaTime);
                    break;

                case TutorialState.WormInfo:
                    HandleWormInfoState(currentPos, deltaTime);
                    break;

                case TutorialState.LoveletterInfo:
                    HandleLoveletterInfoState(currentPos, deltaTime);
                    break;

                case TutorialState.Done:
                    // Do nothing, tutorial is done
                    break;
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_GAMELOSE, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
        }

        private void HandleInstructionState(Vector3 currentPos, float dt)
        {
            // Update tooltip elapsed time
            tooltipElapsed += dt;

            // Fade out upon pressing E only AFTER tooltip exists beyond min duration
            if (tooltipElapsed > tooltipMinTime) { 

                // Display assistance message: "Press "E" to proceed"
                ShowProceedText(true);

                // Check for 'E' input
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                // Begin fade out
                if (EPressed) {
                    ShowUI(instructionID, false, dt);
                    ShowProceedText(false);  // Remove assistance message
                }

                // Begin fade up upon encountering botnet (TO DO)
                if (/* LOGIC HERE && */ EPressed && (fadeOutElapsed > switchTime))
                {
                    ShowUI(botnetInfoID, true, dt);
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.BotnetInfo;
                }
            }
        }

        private void HandleBotnetInfoState(Vector3 currentPos, float dt)
        {
            // Update tooltip elapsed time
            tooltipElapsed += dt;

            // Fade out upon pressing E only AFTER tooltip exists beyond min duration
            if (tooltipElapsed > tooltipMinTime) {

                // Display assistance message: "Press "E" to proceed"
                SetProceedTextPosition(1122, 290);
                ShowProceedText(true);

                // Check for 'E' input
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                // Begin fade out
                if (EPressed) {
                    ShowUI(botnetInfoID, false, dt);
                    ShowProceedText(false);  // Remove assistance message
                }

                // Begin fade up upon encountering worm (TO DO)
                if (/* LOGIC HERE && */ EPressed && (fadeOutElapsed > switchTime)) {
                    ShowUI(wormInfoID, true, dt);
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.WormInfo;
                }
            }
        }

        private void HandleWormInfoState(Vector3 currentPos, float dt)
        {
            // Update tooltip elapsed time
            tooltipElapsed += dt;

            // Fade out upon pressing E only AFTER tooltip exists beyond min duration
            if (tooltipElapsed > tooltipMinTime) {

                // Display assistance message: "Press "E" to proceed"
                SetProceedTextPosition(1122, 285);
                ShowProceedText(true);

                // Check for 'E' input
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                // Begin fade out
                if (EPressed) {
                    ShowUI(wormInfoID, false, dt);
                    ShowProceedText(false);  // Remove assistance message
                }

                // Begin fade up upon encountering loveletter (TO DO)
                if (/* LOGIC HERE && */ EPressed && (fadeOutElapsed > switchTime)) {
                    ShowUI(loveletterInfoID, true, dt);
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.LoveletterInfo;
                }
            }
        }

        private void HandleLoveletterInfoState(Vector3 currentPos, float dt)
        {
            // Update tooltip elapsed time
            tooltipElapsed += dt;

            // Fade out upon pressing E only AFTER tooltip exists beyond min duration
            if (tooltipElapsed > tooltipMinTime) {

                // Display assistance message: "Press "E" to proceed"
                SetProceedTextPosition(1122, 275);
                ShowProceedText(true);

                // Check for 'E' input
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                // Begin fade out
                if (EPressed) {
                    ShowUI(loveletterInfoID, false, dt);
                    ShowProceedText(false);  // Remove assistance message
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime) {

                    // Reset all elapsed time, 'E' pressed state, and end tutorial
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.Done;
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

        private void ShowProceedText(bool value)
        {
            Engine.Text.SetIsVisible(proceedID, value);
        }

        private void SetProceedTextPosition(float x, float y)
        {
            Vector3 newPos = new Vector3(x, y, 0.0f);
            Transform.SetPosition(proceedID, ref newPos);
        }
        
        private void OnGameEnd(string eventName, string payload){
            LogMessage("[TutorialUIManagerLevel2] Detect game end, hiding all tooltip");

            //Hide all the Tooltip UI
            ShowProceedText(false);

            SpriteRenderer.SetIsVisible(instructionID, false);
            SpriteRenderer.SetIsVisible(botnetInfoID, false);
            SpriteRenderer.SetIsVisible(wormInfoID, false);
            SpriteRenderer.SetIsVisible(loveletterInfoID, false);
            SpriteRenderer.SetIsVisible(proceedID, false);
            gameEnd = true;
            return;
        }
    }
}
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

        // Entity IDs - Player, camera, and enemies
        private uint playerID;
        private uint cameraID;
        private uint tutorialBotnetID;
        private uint tutorialWormID;
        private uint tutorialLoveletterID;

        // Entity Names
        [SerializeField] private string playerName = "Player";
        [SerializeField] private string cameraName = "PlayerCam";
        [SerializeField] private string instructionsName = "UI_InstructionLevel2";
        [SerializeField] private string botnetInfoName = "UI_BotnetInfo";
        [SerializeField] private string wormInfoName = "UI_WormInfo";
        [SerializeField] private string loveletterInfoName = "UI_LoveletterInfo";
        [SerializeField] private string proceedName = "UI_E_ToProceed";

        // Events
        private string EVENT_BOTNET_TUTORIAL_SPAWN = "BotnetTutorialSpawn";
        private string EVENT_WORM_TUTORIAL_SPAWN = "WormTutorialSpawn";
        private string EVENT_LOVELETTER_TUTORIAL_SPAWN = "LoveletterTutorialSpawn";
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
        private float camFlySpeed = 30000.0f;
        //private float camRotateSpeed = 1.0f;
        private float distToBotnet = 100.0f;
        private float distToWorm = 150.0f;
        private float distToLoveletter = 1000.0f;

        // Flying Cam
        private Vector3 lastCamPos = Vector3.Zero;

        // Toggles/States
        private bool instructionsRead = false;
        private bool gameEnd = false;
        private bool botnetSeen = false;
        private bool wormSeen = false;
        private bool loveletterSeen = false;
        private bool pauseForTutorial = false;
        private bool playerPauseOnTutorial = false;
        private bool tutorialEnd = false;
        private bool prevEscapePressed = false;

        public override void OnStart()
        {
            // Find all entities' id
            playerID = SceneFindEntityByName(playerName);
            cameraID = SceneFindEntityByName(cameraName);
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
            Subscribe(EVENT_BOTNET_TUTORIAL_SPAWN, OnBotnetSpawn); 
            Subscribe(EVENT_WORM_TUTORIAL_SPAWN, OnWormSpawn); 
            Subscribe(EVENT_LOVELETTER_TUTORIAL_SPAWN, OnLoveletterSpawn);

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
                    Publish("Level2TutorialPause", pauseForTutorial.ToString());        // Audio
                    Publish("Level2TutorialPauseMenu", pauseForTutorial.ToString());    // Pause menu
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
                    Publish("Level2TutorialPause", false.ToString());
                    return;
                } else {
                    // Inform subscribers pause is tutorial-induced
                    Publish("Level2TutorialPause", pauseForTutorial.ToString());        // Audio
                    Publish("Level2TutorialPauseMenu", pauseForTutorial.ToString());    // Pause menu
                }

                // Most updated player position
                Vector3 currentPos = Transform.GetPosition(playerID);
                switch (currentState) {
                    case TutorialState.Instruction:
                        HandleInstructionState(currentPos, deltaTime);
                        break;
                    case TutorialState.BotnetInfo:
                        if(!tutorialEnd) {
                            // Begin flying cam to botnet
                            if(FlyCamToTarget(deltaTime, tutorialBotnetID, distToBotnet)) {
                                // Display botnet tooltip
                                HandleBotnetInfoState(currentPos, deltaTime);
                            }
                        } else {
                            // End with cam flying back to player
                            if(FlyCamBackToPlayer(deltaTime)) {
                                // Set up next state
                                currentState = TutorialState.WormInfo;
                                botnetSeen = false;
                                pauseForTutorial = false;
                                GameState.IsPaused = false;
                                tutorialEnd = false;
                            }
                        }
                        break;
                    case TutorialState.WormInfo:
                        if (!tutorialEnd) {
                            // Begin flying cam to worm
                            if (FlyCamToTarget(deltaTime, tutorialWormID, distToWorm)) {
                                // Display worm tooltip
                                HandleWormInfoState(currentPos, deltaTime);
                            }
                        } else {
                            // End with cam flying back to player
                            if (FlyCamBackToPlayer(deltaTime)) {
                                // Set up next state
                                currentState = TutorialState.LoveletterInfo;
                                wormSeen = false;
                                pauseForTutorial = false;
                                GameState.IsPaused = false;
                                tutorialEnd = false;
                            }
                        }
                        break;
                    case TutorialState.LoveletterInfo:
                        if (!tutorialEnd) {
                            // Begin flying cam to loveletter
                            if (FlyCamToTarget(deltaTime, tutorialLoveletterID, distToLoveletter)) {
                                // Display loveletter tooltip
                                HandleLoveletterInfoState(currentPos, deltaTime);
                            }
                        } else {
                            // End with cam flying back to player
                            if (FlyCamBackToPlayer(deltaTime)) {
                                // Set up next state
                                currentState = TutorialState.Done;
                                loveletterSeen = false;
                                pauseForTutorial = false;
                                GameState.IsPaused = false;
                                tutorialEnd = false;
                            }
                        }
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

            // Ensure subscribers know level 2 pause is not tutorial-induced
            Publish("Level2TutorialPause", pauseForTutorial.ToString());        // Audio
            Publish("Level2TutorialPauseMenu", pauseForTutorial.ToString());    // Pause menu


            // To begin the entire tutorial
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
                    pauseForTutorial = true;
                    GameState.IsPaused = true;
                    playerPauseOnTutorial = false;
                }
                return;
            }

            // Check for enemy spawning events if tutorial is ongoing
            if(currentState != TutorialState.Done)
            {
                // Check if either enemy is seen
                if(botnetSeen || wormSeen || loveletterSeen)
                {
                    pauseForTutorial = true;
                    GameState.IsPaused = true;
                    playerPauseOnTutorial = false;

                    // Save camera last position
                    lastCamPos = Transform.GetPosition(cameraID);
                }
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_GAMELOSE, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
            Unsubscribe(EVENT_BOTNET_TUTORIAL_SPAWN, OnBotnetSpawn);
            Unsubscribe(EVENT_WORM_TUTORIAL_SPAWN, OnWormSpawn); 
            Unsubscribe(EVENT_LOVELETTER_TUTORIAL_SPAWN, OnLoveletterSpawn);
        }

        private bool FlyCamToTarget(float dt, uint targetID, float distToTarget) 
        {
            // Get camera and target world positions
            Vector3 camPos = Transform.GetPosition(cameraID);
            Vector3 targetPos = Transform.GetPosition(targetID);

            // Set camera target to the target enemy
            SetTarget(cameraID, ref targetPos);

            // Current distance from camera to target
            Vector3 toTarget = targetPos - camPos;
            float currentDist = toTarget.Magnitude;

            //string debugMessage = "[TutorialUIManagerLevel2] Camera is distance from enemy: ";
            //debugMessage += currentDist.ToString();
            //LogMessage(debugMessage);

            // Already close enough / desired distance already satisfied
            if (currentDist <= distToTarget) return true;

            Vector3 dirToTarget = toTarget / currentDist;

            // How much closer we are allowed to move this frame
            float moveStep = camFlySpeed * dt;

            // Never move past the desired stopping distance
            float allowedMove = currentDist - distToTarget;
            float actualMove = SimpleMath.Min(moveStep, allowedMove);

            Vector3 newCamPos = camPos + dirToTarget * actualMove;
            Transform.SetPosition(cameraID, ref newCamPos);

            return false;
        }

        private bool FlyCamBackToPlayer(float dt)
        {
            // Get camera current world positions
            Vector3 currCamPos = Transform.GetPosition(cameraID);

            // Current distance from current camera to pre-tutorial camera position
            Vector3 toTarget = lastCamPos - currCamPos;
            float currentDist = toTarget.Magnitude;

            //LogMessage("[TutorialUIManagerLevel2] Camera distance from pre-tutorial position: " + currentDist.ToString());

            // Already close enough / desired distance already satisfied
            float allowedMove = 1000.0f;
            if (currentDist <= 1000.0f) {
                // Cap movement step when close
                allowedMove = 100.0f;
                if (currentDist <= 50.0f)
                {
                    Transform.SetPosition(cameraID, ref lastCamPos);
                    return true;
                }
            }

            Vector3 dirToTarget = toTarget / currentDist;

            // How much closer we are allowed to move this frame
            float moveStep = camFlySpeed * dt;
            float actualMove = SimpleMath.Min(moveStep, allowedMove);

            Vector3 newCamPos = currCamPos + dirToTarget * actualMove;
            Transform.SetPosition(cameraID, ref newCamPos);

            return false;
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

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.BotnetInfo;
                    pauseForTutorial = false;
                    GameState.IsPaused = false;
                }
            }
        }

        private void HandleBotnetInfoState(Vector3 currentPos, float dt)
        {

            // Show UI when state begins (Botnet spawns)
            ShowUI(botnetInfoID, true, dt);

            // Ensure UI fades in fully before pausing state and waiting for interaction
            if (fadeUpElapsed > fadeUpTime) {

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
                        ShowUI(botnetInfoID, false, dt);
                        ShowProceedText(false);  // Remove assistance message
                    }

                    // Ensure all fading effects are completed before moving on
                    if (fadeOutElapsed > fadeOutTime) {

                        // Reset all elapsed time, 'E' pressed state, and set up for next state
                        EPressed = false;
                        tooltipElapsed = 0.0f;
                        fadeOutElapsed = 0.0f;
                        fadeUpElapsed = 0.0f;
                        tutorialEnd = true;
                    }
                }
            }
        }

        private void HandleWormInfoState(Vector3 currentPos, float dt)
        {
            // Show UI when state begins (Worm spawns)
            ShowUI(wormInfoID, true, dt);

            if (fadeUpElapsed > fadeUpTime) {

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
                        ShowUI(wormInfoID, false, dt);
                        ShowProceedText(false);  // Remove assistance message
                    }

                    // Ensure all fading effects are completed before moving on
                    if (fadeOutElapsed > fadeOutTime) {

                        // Reset all elapsed time, 'E' pressed state, and set up for next state
                        EPressed = false;
                        tooltipElapsed = 0.0f;
                        fadeOutElapsed = 0.0f;
                        fadeUpElapsed = 0.0f;
                        tutorialEnd = true;

                    }
                }
            }
        }

        private void HandleLoveletterInfoState(Vector3 currentPos, float dt)
        {
            // Show UI when state begins (Loveletter spawns)
            ShowUI(loveletterInfoID, true, dt);

            if (fadeUpElapsed > fadeUpTime) {

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
                        ShowUI(loveletterInfoID, false, dt);
                        ShowProceedText(false);  // Remove assistance message
                    }

                    // Ensure all fading effects are completed before moving on
                    if (fadeOutElapsed > fadeOutTime) {

                        // Reset all elapsed time, 'E' pressed state, and set up for next state
                        EPressed = false;
                        tooltipElapsed = 0.0f;
                        fadeOutElapsed = 0.0f;
                        fadeUpElapsed = 0.0f;
                        tutorialEnd = true;

                    }
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
            Engine.SpriteRenderer.SetIsVisible(proceedID, value);
        }

        private void SetProceedTextPosition(float x, float y)
        {
            Vector3 newPos = new Vector3(x, y, 0.0f);
            Transform.SetPosition(proceedID, ref newPos);
        }

        private bool IsEscapeJustPressed() {
            bool pressed = IsKeyPressed(KeyCode.Escape);
            bool justPressed = pressed && !prevEscapePressed;
            prevEscapePressed = pressed;
            return justPressed;
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

        private void OnBotnetSpawn(string eventName, string payload) {
            // Ensure tutorial know which enemy is seen
            botnetSeen = true;
            LogMessage("[TutorialUIManagerLevel2] Botnet spawned for tutorial, pause to show tooltip");

            // Extract enemy id from payload
            if (uint.TryParse(payload, out uint botnetID)) {
                // Save enemy ID to local script
                tutorialBotnetID = botnetID;

                string message = "[TutorialUIManagerLevel2] Botnet ID for tutorial retrieved: " + tutorialBotnetID.ToString();
                LogMessage(message);
            }
        }

        private void OnWormSpawn(string eventName, string payload) {
            // Ensure tutorial know which enemy is seen
            wormSeen = true;
            LogMessage("[TutorialUIManagerLevel2] Worm spawned for tutorial, pause to show tooltip");

            // Extract enemy id from payload
            if (uint.TryParse(payload, out uint wormID)) {
                // Save enemy ID to local script
                tutorialWormID = wormID;

                string message = "[TutorialUIManagerLevel2] Worm ID for tutorial retrieved: " + tutorialWormID.ToString();
                LogMessage(message);
            }
        }

        private void OnLoveletterSpawn(string eventName, string payload)
        {
            // Ensure tutorial know which enemy is seen
            loveletterSeen = true;
            LogMessage("[TutorialUIManagerLevel2] Loveletter spawned for tutorial, pause to show tooltip");

            // Extract enemy id from payload
            if (uint.TryParse(payload, out uint loveletterID)) {
                // Save enemy ID to local script
                tutorialLoveletterID = loveletterID;

                string message = "[TutorialUIManagerLevel2] Loveletter ID for tutorial retrieved: " + tutorialLoveletterID.ToString();
                LogMessage(message);
            }
        }
    }
}
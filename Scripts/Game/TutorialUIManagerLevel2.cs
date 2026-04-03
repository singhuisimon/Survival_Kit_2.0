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
        private uint crosshairID;       // Crosshair
        private uint crosshair2ID;      // Crosshair2

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
        [SerializeField] private string crossHairName = "Crosshair";
        [SerializeField] private string crossHair2Name = "Crosshair2";

        // Events
        private string EVENT_BOTNET_TUTORIAL_SPAWN = "BotnetTutorialSpawn";
        private string EVENT_WORM_TUTORIAL_SPAWN = "WormTutorialSpawn";
        private string EVENT_LOVELETTER_TUTORIAL_SPAWN = "LoveletterTutorialSpawn";
        private const string EVENT_GAMELOSE = "LoseScreenShow";
        private const string EVENT_GAMEWIN = "GameWin";
        private const string EVENT_SKIPTUTORIAL = "SkipTutorial";

        // Fading
        private bool EPressed = false;
        private float tooltipElapsed = 0.0f;
        [SerializeField] private float tooltipMinTime = 3.0f;
        private float fadeOutElapsed = 0.0f;
        [SerializeField] private float fadeOutTime = 0.2f;
        private float fadeUpElapsed = 0.0f;
        [SerializeField] private float fadeUpTime = 1.0f;
        [SerializeField] private float uiStartFadePos = 260.0f;
        [SerializeField] private float switchTime = 0.5f;
        private float camFlySpeed = 30000.0f;
        private float distToBotnet = 100.0f;
        private float distToWorm = 150.0f;
        private float distToLoveletter = 1000.0f;

        // Flying Cam
        private Vector3 lastCamPos = Vector3.Zero;

        // -----------------------------------------------------------------------
        // Pause ownership model
        //   pauseForTutorial    : tutorial is actively driving the game pause
        //   playerPauseOnTutorial: user opened their pause menu OVER a tutorial pause
        //
        // While playerPauseOnTutorial == true, we publish TutorialPauseMenu=false
        // so the popup knows the tutorial is NOT the current pause owner, and can
        // unconditionally clear GameState.IsPaused when the user resumes.
        // -----------------------------------------------------------------------
        private bool instructionsRead = false;
        private bool gameEnd = false;
        private bool botnetSeen = false;
        private bool wormSeen = false;
        private bool loveletterSeen = false;
        private bool pauseForTutorial = false;       // tutorialForcedPause
        private bool playerPauseOnTutorial = false;  // userPauseMenuOpen (over tutorial)
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
            crosshairID = SceneFindEntityByName(crossHairName);
            crosshair2ID = SceneFindEntityByName(crossHair2Name);

            // Hide all tutorial UI
            SpriteRenderer.SetIsVisible(instructionID, false);
            SpriteRenderer.SetIsVisible(botnetInfoID, false);
            SpriteRenderer.SetIsVisible(wormInfoID, false);
            SpriteRenderer.SetIsVisible(loveletterInfoID, false);

            // Subscribe to events
            Subscribe(EVENT_GAMELOSE, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);
            Subscribe(EVENT_BOTNET_TUTORIAL_SPAWN, OnBotnetSpawn);
            Subscribe(EVENT_WORM_TUTORIAL_SPAWN, OnWormSpawn);
            Subscribe(EVENT_LOVELETTER_TUTORIAL_SPAWN, OnLoveletterSpawn);

            // Subscribe to GameResumed so we detect when the user closed the popup
            // via the Resume button (not just Escape). Clears playerPauseOnTutorial
            // so the tutorial reclaims pause ownership next frame.
            Subscribe("GameResumed", OnGameResumed);

            // Start the game paused for tutorial
            pauseForTutorial = true;
            LogMessage("[TutorialUIManagerLevel2] PAUSE-OWNER: tutorialForcedPause=true at start");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Check if skip tutorial
            if (ProgressTracker.SkipTutorialLevel2)
            {
                Publish(EVENT_SKIPTUTORIAL, true.ToString());
                if (currentState == TutorialState.Done) return;
            }
            else
            {
                Publish(EVENT_SKIPTUTORIAL, false.ToString());
            }

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
                    LogMessage("[TutorialUIManagerLevel2] PAUSE-OWNER: user closed menu via Escape; tutorial resumes control next frame");
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
                    LogMessage("[TutorialUIManagerLevel2] PAUSE-OWNER: user pressed Escape during tutorial pause; yielding to user menu");
                    return;
                }
                else
                {
                    Publish("TutorialPauseAudio", pauseForTutorial.ToString());
                    Publish("TutorialPauseMenu", pauseForTutorial.ToString());
                    CrosshairVisibility(false);
                }

                // Most updated player position
                Vector3 currentPos = Transform.GetPosition(playerID);
                switch (currentState)
                {
                    case TutorialState.Instruction:
                        HandleInstructionState(currentPos, deltaTime);
                        break;
                    case TutorialState.BotnetInfo:
                        if (!tutorialEnd)
                        {
                            if (FlyCamToTarget(deltaTime, tutorialBotnetID, distToBotnet))
                                HandleBotnetInfoState(currentPos, deltaTime);
                        }
                        else
                        {
                            currentState = TutorialState.WormInfo;
                            botnetSeen = false;
                            tutorialEnd = false;
                        }
                        break;
                    case TutorialState.WormInfo:
                        if (!tutorialEnd)
                        {
                            if (FlyCamToTarget(deltaTime, tutorialWormID, distToWorm))
                                HandleWormInfoState(currentPos, deltaTime);
                        }
                        else
                        {
                            currentState = TutorialState.LoveletterInfo;
                            wormSeen = false;
                            tutorialEnd = false;
                        }
                        break;
                    case TutorialState.LoveletterInfo:
                        if (!tutorialEnd)
                        {
                            if (FlyCamToTarget(deltaTime, tutorialLoveletterID, distToLoveletter))
                                HandleLoveletterInfoState(currentPos, deltaTime);
                        }
                        else
                        {
                            if (FlyCamBackToPlayer(deltaTime))
                            {
                                currentState = TutorialState.Done;
                                loveletterSeen = false;
                                pauseForTutorial = false;
                                GameState.IsPaused = false;
                                tutorialEnd = false;
                                LogMessage("[TutorialUIManagerLevel2] PAUSE-OWNER: tutorialForcedPause=false, tutorial complete");
                            }
                        }
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

            // Signal that tutorial is not pausing right now
            Publish("TutorialPauseAudio", pauseForTutorial.ToString());
            Publish("TutorialPauseMenu", pauseForTutorial.ToString());
            CrosshairVisibility(true);

            // Check for enemy spawning events if tutorial is ongoing
            if (currentState != TutorialState.Done)
            {
                if (botnetSeen || wormSeen || loveletterSeen)
                {
                    pauseForTutorial = true;
                    GameState.IsPaused = true;
                    playerPauseOnTutorial = false;
                    lastCamPos = Transform.GetPosition(cameraID);
                    LogMessage("[TutorialUIManagerLevel2] PAUSE-OWNER: enemy seen, tutorialForcedPause=true");
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
            Unsubscribe("GameResumed", OnGameResumed);
        }

        private bool FlyCamToTarget(float dt, uint targetID, float distToTarget)
        {
            Vector3 camPos = Transform.GetPosition(cameraID);
            Vector3 targetPos = Transform.GetPosition(targetID);

            SetTarget(cameraID, ref targetPos);

            Vector3 toTarget = targetPos - camPos;
            float currentDist = toTarget.Magnitude;

            if (currentDist <= distToTarget) return true;

            Vector3 dirToTarget = toTarget / currentDist;

            float moveStep = camFlySpeed * dt;
            float allowedMove = currentDist - distToTarget;
            float actualMove = SimpleMath.Min(moveStep, allowedMove);

            Vector3 newCamPos = camPos + dirToTarget * actualMove;
            Transform.SetPosition(cameraID, ref newCamPos);

            return false;
        }

        private bool FlyCamBackToPlayer(float dt)
        {
            Vector3 currCamPos = Transform.GetPosition(cameraID);

            Vector3 toTarget = lastCamPos - currCamPos;
            float currentDist = toTarget.Magnitude;

            float allowedMove = 1000.0f;
            if (currentDist <= 1000.0f)
            {
                allowedMove = 100.0f;
                if (currentDist <= 50.0f)
                {
                    Transform.SetPosition(cameraID, ref lastCamPos);
                    return true;
                }
            }

            Vector3 dirToTarget = toTarget / currentDist;

            float moveStep = camFlySpeed * dt;
            float actualMove = SimpleMath.Min(moveStep, allowedMove);

            Vector3 newCamPos = currCamPos + dirToTarget * actualMove;
            Transform.SetPosition(cameraID, ref newCamPos);

            return false;
        }

        private void HandleInstructionState(Vector3 currentPos, float dt)
        {
            ShowUI(instructionID, true, dt);

            if (fadeUpElapsed > fadeUpTime)
            {
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                if (EPressed)
                    ShowUI(instructionID, false, dt);

                if (fadeOutElapsed > fadeOutTime)
                {
                    if (ProgressTracker.SkipTutorialLevel2)
                    {
                        currentState = TutorialState.Done;
                        tutorialEnd = false;
                    }
                    else
                    {
                        currentState = TutorialState.BotnetInfo;
                    }

                    EPressed = false;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    pauseForTutorial = false;
                    GameState.IsPaused = false;
                    Publish("BGMVOStart", "");
                    LogMessage("[TutorialUIManagerLevel2] PAUSE-OWNER: instruction done, tutorialForcedPause=false");
                }
            }
        }

        private void HandleBotnetInfoState(Vector3 currentPos, float dt)
        {
            ShowUI(botnetInfoID, true, dt);

            if (fadeUpElapsed > fadeUpTime)
            {
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                if (EPressed)
                    ShowUI(botnetInfoID, false, dt);

                if (fadeOutElapsed > fadeOutTime)
                {
                    EPressed = false;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    tutorialEnd = true;
                }
            }
        }

        private void HandleWormInfoState(Vector3 currentPos, float dt)
        {
            ShowUI(wormInfoID, true, dt);

            if (fadeUpElapsed > fadeUpTime)
            {
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                if (EPressed)
                    ShowUI(wormInfoID, false, dt);

                if (fadeOutElapsed > fadeOutTime)
                {
                    EPressed = false;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    tutorialEnd = true;
                }
            }
        }

        private void HandleLoveletterInfoState(Vector3 currentPos, float dt)
        {
            ShowUI(loveletterInfoID, true, dt);

            if (fadeUpElapsed > fadeUpTime)
            {
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                if (EPressed)
                    ShowUI(loveletterInfoID, false, dt);

                if (fadeOutElapsed > fadeOutTime)
                {
                    EPressed = false;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    tutorialEnd = true;
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
                LogMessage("[TutorialUIManagerLevel2] PAUSE-OWNER: GameResumed received; tutorial reclaims pause control next frame");
            }
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("[TutorialUIManagerLevel2] Detect game end, hiding all tooltip");

            SpriteRenderer.SetIsVisible(instructionID, false);
            SpriteRenderer.SetIsVisible(botnetInfoID, false);
            SpriteRenderer.SetIsVisible(wormInfoID, false);
            SpriteRenderer.SetIsVisible(loveletterInfoID, false);
            gameEnd = true;
        }

        private void OnBotnetSpawn(string eventName, string payload)
        {
            botnetSeen = true;
            LogMessage("[TutorialUIManagerLevel2] Botnet spawned for tutorial, pause to show tooltip");

            if (uint.TryParse(payload, out uint botnetID))
            {
                tutorialBotnetID = botnetID;
                LogMessage("[TutorialUIManagerLevel2] Botnet ID for tutorial retrieved: " + tutorialBotnetID.ToString());
            }
        }

        private void OnWormSpawn(string eventName, string payload)
        {
            wormSeen = true;
            LogMessage("[TutorialUIManagerLevel2] Worm spawned for tutorial, pause to show tooltip");

            if (uint.TryParse(payload, out uint wormID))
            {
                tutorialWormID = wormID;
                LogMessage("[TutorialUIManagerLevel2] Worm ID for tutorial retrieved: " + tutorialWormID.ToString());
            }
        }

        private void OnLoveletterSpawn(string eventName, string payload)
        {
            loveletterSeen = true;
            LogMessage("[TutorialUIManagerLevel2] Loveletter spawned for tutorial, pause to show tooltip");

            if (uint.TryParse(payload, out uint loveletterID))
            {
                tutorialLoveletterID = loveletterID;
                LogMessage("[TutorialUIManagerLevel2] Loveletter ID for tutorial retrieved: " + tutorialLoveletterID.ToString());
            }
        }
    }
}

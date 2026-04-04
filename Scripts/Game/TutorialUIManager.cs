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
    public class TutorialUIManager : ScriptBehaviour
    {
        private enum TutorialState
        {
            Move,
            FlyThrough,
            ShootWall,
            DestroyTurret,
            DestroyEnemies,
            Wait,
            AltFire,
            CollectUpgradeModule,
            SummonSentry
        }

        private enum FlyThroughBlackoutState
        {
            None,
            TeleportFadeIn,
            TeleportFadeOut,
            FinalFadeIn,
            FinalFadeOut
        }

        private TutorialState currentState = TutorialState.Move;

        // Entity IDs - UI
        private uint pressWASDID;
        private uint pressFlyTunnelID;
        private uint pressShootID;
        private uint destroyTurretID;
        private uint destroyEnemiesID;
        private uint altFireID;
        private uint proceedID;
        private uint collectUpgradeModuleID;
        private uint sentrySummonID;

        // Entity IDs - Player, Wall, Fade
        private uint playerID;
        private uint wallID;
        private uint fadeBlackID;
        private uint cameraID;

        // Entity Names
        [SerializeField] private string pressWASDName = "UI_PressWASD";
        [SerializeField] private string pressFlyTunnelName = "UI_FlyTunnel";
        [SerializeField] private string playerName = "Player";
        [SerializeField] private string wallName = "DestructableWall1";
        [SerializeField] private string pressShootName = "UI_Shoot";
        [SerializeField] private string destroyTurretName = "UI_DestroyTurret";
        [SerializeField] private string destroyEnemiesName = "UI_DestroyEnemies";
        [SerializeField] private string altFireName = "UI_AltFire";
        [SerializeField] private string proceedName = "UI_E_ToProceed";
        [SerializeField] private string collectUpgradeModuleName = "UI_CollectUpgradeModule";
        [SerializeField] private string sentrySummonName = "UI_CombatAbilitySentry";
        [SerializeField] private string cameraName = "PlayerCam";
        [SerializeField] private string fadeBlackName = "FadeBlack";

        // Events
        private string EVENT_ONE_TURRET_DESTROYED = "OneTurretDestroyed";
        private string EVENT_FIVE_TURRETS_DESTROYED = "FiveTurretsDestroyed";
        private const string EVENT_ULT_CHARGED = "UltCharged";
        private const string EVENT_ALT_FIRED = "AltFired";
        private const string EVENT_CORE_DEAD = "CoreDeadTriggerPostTrenchRun";
        private const string EVENT_COLLECT_PAYLOAD = "CollectPayload";
        private const string EVENT_SENTRY_SPAWNED = "SentrySpawnedTrench";
        private const string EVENT_CINEMATIC_PLAY = "PlayCinematic";
        private const string EVENT_CINEMATIC_STOP = "StopCinematic";

        private const string EVENT_FADE_BLACK_IN = "FadeBlackFadeIn";
        private const string EVENT_FADE_BLACK_OUT = "FadeBlackFadeOut";
        private const string EVENT_FADE_BLACK_IN_DONE = "FadeBlackFadeInDone";
        private const string EVENT_FADE_BLACK_OUT_DONE = "FadeBlackFadeOutDone";

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
        [SerializeField] private float sentrySummonFadeOutTime = 1.0f;

        private bool movedWASD = false;
        private bool movedSpacebar = false;
        private bool movedShift = false;
        private bool oneturretDestroyed = false;
        private bool turretsDestroyed = false;
        private bool ultCharged = false;
        private bool altUsed = false;
        private bool altFireShown = false;
        private bool hasCollectedUpgrade = false;
        private bool hasSummonedSentry = false;

        private Vector3 wallPos = new Vector3(0.0f, 0.0f, 0.0f);

        private bool wallDestroyedPublished = false;
        private bool skipTutorial = false;
        private string EVENT_TUTORIAL_SKIP = "SkipTutorial";

        private bool pauseForTutorial = false;
        private bool playerPauseOnTutorial = false;
        private bool prevEscapePressed = false;
        private bool UIShown = false;

        [SerializeField] private float camFlySpeed = 100.0f;

        private bool tutorialCamMoveActive = false;
        private bool tutorialCamMoveCompleted = false;
        private bool tutorialCamPoseSaved = false;
        private Vector3 tutorialCamSavedPos = Vector3.Zero;
        private Vector3 tutorialCamSavedTarget = Vector3.Zero;
        private Vector3 tutorialCamMoveStartPos = Vector3.Zero;
        private Vector3 tutorialCamMoveStartTarget = Vector3.Zero;
        private Vector3 tutorialCamMoveEndPos = Vector3.Zero;
        private Vector3 tutorialCamMoveEndTarget = Vector3.Zero;
        private float tutorialCamMoveElapsed = 0.0f;
        private float tutorialCamMoveDuration = 0.0f;

        private bool shootWallStateEntered = false;
        private bool shootWallReturnStarted = false;
        private bool destroyTurretStateEntered = false;
        private bool destroyTurretReturnStarted = false;
        private bool collectUpgradeStateEntered = false;
        private bool collectUpgradeReturnStarted = false;

        private readonly Vector3 shootWallCamPos = new Vector3(-325.0f, 25.0f, -25.0f);
        private readonly Vector3 shootWallCamTarget = new Vector3(-400.0f, -25.0f, 45.0f);
        private readonly Vector3 destroyTurretCamPos = new Vector3(-640.0f, -35.0f, 45.0f);
        private readonly Vector3 destroyTurretCamTarget = new Vector3(-750.0f, -20.0f, -30.0f);
        private readonly Vector3 collectUpgradeCamPos = new Vector3(-5570.0f, -400.0f, 630.0f);
        private readonly Vector3 collectUpgradeCamTarget = new Vector3(-5750.0f, -440.0f, 670.0f);

        // ===== Fly-through Cinematic =====
        [SerializeField] private float flyThroughLookAheadStartT = 0.8f;
        [SerializeField] private float flyThroughFinalLookDistance = 250.0f;

        [SerializeField] private int flyThroughTeleportWaypointIndex = 5;
        [SerializeField] private float flyThroughTeleportFadeLeadTime = 2.0f;
        [SerializeField] private float flyThroughFinalFadeLeadTime = 1.0f;

        private bool flyThroughStarted = false;
        private bool flyThroughFinished = false;
        private int flyThroughSegmentIndex = 0;
        private float flyThroughSegmentElapsed = 0.0f;
        private bool flyThroughImmediateReturnRequested = false;

        private Vector3 savedFlyThroughCamPos = Vector3.Zero;
        private Vector3 savedFlyThroughCamTarget = Vector3.Zero;

        private Vector3 savedFlyThroughPlayerPos = Vector3.Zero;
        private Quat savedFlyThroughPlayerRot;

        private FlyThroughBlackoutState flyThroughBlackoutState = FlyThroughBlackoutState.None;
        private bool blackFadeInDone = false;
        private bool blackFadeOutDone = false;
        private bool flyThroughTeleportFadeTriggered = false;
        private bool flyThroughFinalFadeTriggered = false;
        private Vector3 flyThroughHoldCamPos = Vector3.Zero;
        private Vector3 flyThroughHoldCamTarget = Vector3.Zero;

        private readonly Vector3[] flyThroughWaypoints = new Vector3[]
        {
            new Vector3(-275.0f,    3.0f,    0.0f),
            new Vector3(-450.0f,    0.0f,   30.0f),
            new Vector3(-780.0f,    0.0f,  -90.0f),
            new Vector3(-850.0f,   10.0f,  -82.5f),
            new Vector3(-1080.0f,  60.0f,  -82.5f),
            new Vector3(-5415.0f, -440.0f, 645.0f),
            new Vector3(-5450.0f, -435.0f, 645.0f)
        };

        private readonly float[] flyThroughWaypointTimes = new float[]
        {
            1.0f, 1.0f, 1.0f, 0.6f, 1.0f, 0.0001f, 5.0f
        };

        public override void OnStart()
        {
            pressWASDID = SceneFindEntityByName(pressWASDName);
            pressFlyTunnelID = SceneFindEntityByName(pressFlyTunnelName);
            playerID = SceneFindEntityByName(playerName);
            wallID = SceneFindEntityByName(wallName);
            pressShootID = SceneFindEntityByName(pressShootName);
            destroyTurretID = SceneFindEntityByName(destroyTurretName);
            destroyEnemiesID = SceneFindEntityByName(destroyEnemiesName);
            altFireID = SceneFindEntityByName(altFireName);
            proceedID = SceneFindEntityByName(proceedName);
            collectUpgradeModuleID = SceneFindEntityByName(collectUpgradeModuleName);
            sentrySummonID = SceneFindEntityByName(sentrySummonName);
            cameraID = SceneFindEntityByName(cameraName);
            fadeBlackID = SceneFindEntityByName(fadeBlackName);

            ResetFlyThroughCinematicState();
            ResetTutorialFocusState();

            if (flyThroughWaypoints.Length != flyThroughWaypointTimes.Length)
            {
                LogError("[TutorialUIManager] Fly-through waypoint count does not match timing count.");
            }

            if (ProgressTracker.SkipTutorialLevel1)
            {
                skipTutorial = true;
                SpriteRenderer.SetIsVisible(pressWASDID, false);
                Publish(EVENT_TUTORIAL_SKIP, "");
            }
            else
            {
                skipTutorial = false;
                SpriteRenderer.SetIsVisible(pressWASDID, true);
            }

            SpriteRenderer.SetIsVisible(pressFlyTunnelID, false);
            SpriteRenderer.SetIsVisible(pressShootID, false);
            SpriteRenderer.SetIsVisible(destroyTurretID, false);
            SpriteRenderer.SetIsVisible(destroyEnemiesID, false);
            SpriteRenderer.SetIsVisible(altFireID, false);
            SpriteRenderer.SetIsVisible(proceedID, false);
            SpriteRenderer.SetIsVisible(collectUpgradeModuleID, false);
            SpriteRenderer.SetIsVisible(sentrySummonID, false);

            wallPos = Transform.GetPosition(wallID);

            Subscribe(EVENT_ONE_TURRET_DESTROYED, OnTurretDestroyed);
            Subscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
            Subscribe(EVENT_ULT_CHARGED, OnUltCharged);
            Subscribe(EVENT_ALT_FIRED, OnAltFired);
            Subscribe(EVENT_CORE_DEAD, OnCoreDeath);
            Subscribe(EVENT_COLLECT_PAYLOAD, OnCollectPayload);
            Subscribe(EVENT_SENTRY_SPAWNED, OnSentrySpawned);
            Subscribe(EVENT_FADE_BLACK_IN_DONE, OnFadeBlackInDone);
            Subscribe(EVENT_FADE_BLACK_OUT_DONE, OnFadeBlackOutDone);
            Subscribe("GameResumed", OnGameResumed);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (skipTutorial)
            {
                currentState = TutorialState.Move;
                return;
            }

            bool escapeJustPressed = IsEscapeJustPressed();

            if (!pauseForTutorial && playerPauseOnTutorial)
                playerPauseOnTutorial = false;

            if (pauseForTutorial)
            {
                GameState.IsPaused = true;

                if (escapeJustPressed)
                {
                    playerPauseOnTutorial = !playerPauseOnTutorial;
                }

                if (playerPauseOnTutorial)
                {
                    Publish("TutorialPauseAudio", false.ToString());
                    Publish("TutorialPauseMenu", false.ToString());
                }
                else
                {
                    Publish("TutorialPauseAudio", pauseForTutorial.ToString());
                    Publish("TutorialPauseMenu", pauseForTutorial.ToString());
                }
            }

            Vector3 currentPos = Transform.GetPosition(playerID);

            switch (currentState)
            {
                case TutorialState.Move:
                    HandleMoveState(currentPos, deltaTime);
                    break;

                case TutorialState.FlyThrough:
                    HandleFlyThroughState(currentPos, wallPos, deltaTime);
                    break;

                case TutorialState.ShootWall:
                    HandleShootWallState(currentPos, wallPos, deltaTime);
                    break;

                case TutorialState.DestroyTurret:
                    HandleDestroyTurretState(deltaTime);
                    break;

                case TutorialState.DestroyEnemies:
                    HandleDestroyEnemiesState(deltaTime);
                    break;

                case TutorialState.Wait:
                    HandleWaitState(deltaTime);
                    break;

                case TutorialState.AltFire:
                    HandleAltFire(deltaTime);
                    break;

                case TutorialState.CollectUpgradeModule:
                    HandleCollectUpgradeModuleState(deltaTime);
                    break;

                case TutorialState.SummonSentry:
                    HandleSummonSentryState(deltaTime);
                    break;

                default:
                    break;
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_ONE_TURRET_DESTROYED, OnTurretDestroyed);
            Unsubscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
            Unsubscribe(EVENT_ULT_CHARGED, OnUltCharged);
            Unsubscribe(EVENT_ALT_FIRED, OnAltFired);
            Unsubscribe(EVENT_CORE_DEAD, OnCoreDeath);
            Unsubscribe(EVENT_COLLECT_PAYLOAD, OnCollectPayload);
            Unsubscribe(EVENT_SENTRY_SPAWNED, OnSentrySpawned);
            Unsubscribe(EVENT_FADE_BLACK_IN_DONE, OnFadeBlackInDone);
            Unsubscribe(EVENT_FADE_BLACK_OUT_DONE, OnFadeBlackOutDone);
            Unsubscribe("GameResumed", OnGameResumed);
        }

        private void HandleMoveState(Vector3 currentPos, float dt)
        {
            if (!movedWASD && (IsKeyPressed(KeyCode.W) || IsKeyPressed(KeyCode.S) || IsKeyPressed(KeyCode.D) || IsKeyPressed(KeyCode.A) || IsKeyPressed(KeyCode.E)))
                movedWASD = true;

            if (!movedSpacebar && IsKeyPressed(KeyCode.Space))
                movedSpacebar = true;

            if (!movedShift && IsKeyPressed(KeyCode.LeftShift))
                movedShift = true;

            if (movedWASD)
            {
                ShowUI(pressWASDID, false, dt);

                if (fadeOutElapsed > switchTime)
                {
                    if (!UIShown)
                    {
                        fadeUpElapsed = 0.0f;
                        UIShown = true;
                    }

                    ShowUI(pressFlyTunnelID, true, dt);
                }

                if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime)
                {
                    EPressed = false;
                    UIShown = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.FlyThrough;
                }
            }
        }

        private void HandleFlyThroughState(Vector3 currentPos, Vector3 currentWallPos, float dt)
        {
            tooltipElapsed += dt;

            if (!flyThroughStarted)
                StartFlyThroughCinematic();

            if (!flyThroughFinished)
            {
                if (IsKeyPressed(KeyCode.E))
                    BeginFlyThroughImmediateReturnToPlayer();

                if (flyThroughImmediateReturnRequested)
                    ShowUI(pressFlyTunnelID, false, dt);
                else if (!EPressed)
                    ShowUI(pressFlyTunnelID, true, dt);

                UpdateFlyThroughCinematic(dt);
                pauseForTutorial = true;
                GameState.IsPaused = true;
                return;
            }

            bool canExitFlyThroughState = flyThroughFinished;

            if (canExitFlyThroughState)
            {
                ShowUI(pressFlyTunnelID, false, dt);
                pauseForTutorial = false;
                GameState.IsPaused = false;
            }
            else
            {
                pauseForTutorial = true;
                GameState.IsPaused = true;
            }

            if ((currentPos.X - currentWallPos.X) < 70.0f && canExitFlyThroughState && (fadeOutElapsed > switchTime))
            {
                EPressed = false;
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                currentState = TutorialState.ShootWall;
            }
        }

        private void HandleShootWallState(Vector3 currentPos, Vector3 currentWallPos, float dt)
        {
            altUsed = false;

            bool wallAlreadyDestroyed = IsWallAlreadyDestroyed(currentPos, currentWallPos);

            if (wallAlreadyDestroyed && !wallDestroyedPublished)
            {
                wallDestroyedPublished = true;
                Publish("DestructableWallDestroyed", true.ToString());
            }

            if (wallAlreadyDestroyed && !shootWallStateEntered)
            {
                SpriteRenderer.SetIsVisible(pressShootID, false);
                pauseForTutorial = false;
                GameState.IsPaused = false;

                shootWallStateEntered = false;
                shootWallReturnStarted = false;
                tutorialCamPoseSaved = false;
                tutorialCamMoveActive = false;
                tutorialCamMoveCompleted = false;
                EPressed = false;
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;

                currentState = TutorialState.DestroyTurret;
                return;
            }

            if (!shootWallStateEntered)
            {
                shootWallStateEntered = true;
                shootWallReturnStarted = false;
                tutorialCamPoseSaved = false;
                tooltipElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                EPressed = false;
                wallDestroyedPublished = wallAlreadyDestroyed;

                BeginTutorialCameraMove(shootWallCamPos, shootWallCamTarget, true);
            }

            UpdateTutorialCameraMove(dt);
            tooltipElapsed += dt;

            if (tooltipElapsed <= tooltipMinTime && !wallDestroyedPublished)
            {
                ShowUI(pressShootID, true, dt);
                pauseForTutorial = true;
                GameState.IsPaused = true;
                return;
            }

            if (IsWallAlreadyDestroyed(currentPos, currentWallPos) && !wallDestroyedPublished)
            {
                LogMessage("[TutorialUIManager] Detect wall is destroyed");
                wallDestroyedPublished = true;
                Publish("DestructableWallDestroyed", true.ToString());
            }

            if (wallDestroyedPublished)
                ShowUI(pressShootID, false, dt);
            else
                ShowUI(pressShootID, true, dt);

            if (fadeUpElapsed >= fadeUpTime && IsKeyPressed(KeyCode.E))
                EPressed = true;

            if (EPressed)
            {
                ShowUI(pressShootID, false, dt);
                pauseForTutorial = false;
                GameState.IsPaused = false;

                if (!shootWallReturnStarted)
                    RefreshTutorialSavedCameraPoseFromLive();
            }
            else
            {
                pauseForTutorial = true;
                GameState.IsPaused = true;
            }

            if (wallDestroyedPublished && EPressed)
            {
                if (!shootWallReturnStarted)
                {
                    shootWallReturnStarted = true;
                    BeginReturnTutorialCameraMove();
                }

                UpdateTutorialCameraMove(dt);

                if (IsTutorialCameraMoveFinished() && fadeOutElapsed > switchTime)
                {
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;

                    shootWallStateEntered = false;
                    shootWallReturnStarted = false;
                    tutorialCamPoseSaved = false;

                    currentState = TutorialState.DestroyTurret;
                }
            }
        }

        private void HandleDestroyTurretState(float dt)
        {
            altUsed = false;

            if (!destroyTurretStateEntered)
            {
                destroyTurretStateEntered = true;
                destroyTurretReturnStarted = false;
                tutorialCamPoseSaved = false;
                tutorialCamMoveActive = false;
                tutorialCamMoveCompleted = false;
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                EPressed = false;

                BeginTutorialCameraMove(destroyTurretCamPos, destroyTurretCamTarget, true);
            }

            UpdateTutorialCameraMove(dt);

            if (tooltipElapsed <= tooltipMinTime)
            {
                ShowUI(destroyTurretID, true, dt);
                pauseForTutorial = true;
                GameState.IsPaused = true;
            }

            tooltipElapsed += dt;

            if (tooltipElapsed > tooltipMinTime)
            {
                if (IsKeyPressed(KeyCode.E))
                    EPressed = true;

                if (EPressed)
                {
                    ShowUI(destroyTurretID, false, dt);
                    pauseForTutorial = false;
                    GameState.IsPaused = false;

                    if (!destroyTurretReturnStarted)
                        RefreshTutorialSavedCameraPoseFromLive();
                }
                else
                {
                    pauseForTutorial = true;
                    GameState.IsPaused = true;
                }

                if (oneturretDestroyed && EPressed)
                {
                    if (!destroyTurretReturnStarted)
                    {
                        destroyTurretReturnStarted = true;
                        BeginReturnTutorialCameraMove();
                    }

                    UpdateTutorialCameraMove(dt);

                    if (IsTutorialCameraMoveFinished() && fadeOutElapsed > switchTime)
                    {
                        EPressed = false;
                        tooltipElapsed = 0.0f;
                        fadeOutElapsed = 0.0f;
                        fadeUpElapsed = 0.0f;
                        destroyTurretStateEntered = false;
                        destroyTurretReturnStarted = false;
                        tutorialCamPoseSaved = false;
                        currentState = TutorialState.DestroyEnemies;
                    }
                }
            }
        }

        private void HandleDestroyEnemiesState(float dt)
        {
            altUsed = false;
            pauseForTutorial = false;
            GameState.IsPaused = false;
            EPressed = false;

            if (!turretsDestroyed)
            {
                ShowUI(destroyEnemiesID, true, dt);
                return;
            }

            ShowUI(destroyEnemiesID, false, dt);

            if (fadeOutElapsed > fadeOutTime)
            {
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                currentState = TutorialState.Wait;
            }
        }

        private void HandleWaitState(float dt)
        {
            if (turretsDestroyed && ultCharged && !altFireShown && !altUsed)
            {
                altFireShown = true;
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                EPressed = false;
                currentState = TutorialState.AltFire;
            }
        }

        private void HandleAltFire(float dt)
        {
            pauseForTutorial = false;
            GameState.IsPaused = false;
            EPressed = false;

            if (!altUsed)
            {
                ShowUI(altFireID, true, dt);
                return;
            }

            ShowUI(altFireID, false, dt);

            if (fadeOutElapsed > fadeOutTime)
            {
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                currentState = TutorialState.Wait;
            }
        }

        private void HandleCollectUpgradeModuleState(float dt)
        {
            if (!collectUpgradeStateEntered)
            {
                collectUpgradeStateEntered = true;
                collectUpgradeReturnStarted = false;
                tutorialCamPoseSaved = false;
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                EPressed = false;

                BeginTutorialCameraMove(collectUpgradeCamPos, collectUpgradeCamTarget, true);
            }

            UpdateTutorialCameraMove(dt);

            if (IsKeyPressed(KeyCode.E))
                EPressed = true;

            if (hasCollectedUpgrade || EPressed)
            {
                ShowCollectUpgradeUI(false, dt);
                GameState.IsPaused = false;
                pauseForTutorial = false;

                if (!collectUpgradeReturnStarted)
                {
                    RefreshTutorialSavedCameraPoseFromLive();
                    collectUpgradeReturnStarted = true;
                    BeginReturnTutorialCameraMove();
                }

                UpdateTutorialCameraMove(dt);
            }
            else
            {
                ShowCollectUpgradeUI(true, dt);
                pauseForTutorial = true;
                GameState.IsPaused = true;
            }

            if (hasCollectedUpgrade &&
                collectUpgradeReturnStarted &&
                IsTutorialCameraMoveFinished() &&
                fadeOutElapsed > fadeOutTime)
            {
                fadeUpElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                EPressed = false;
                hasCollectedUpgrade = false;
                collectUpgradeStateEntered = false;
                collectUpgradeReturnStarted = false;
                tutorialCamPoseSaved = false;
                currentState = TutorialState.SummonSentry;
            }
        }

        private void HandleSummonSentryState(float dt)
        {
            pauseForTutorial = false;
            GameState.IsPaused = false;
            EPressed = false;

            if (hasSummonedSentry)
            {
                ShowSentrySummonUI(false, dt);

                if (fadeOutElapsed > sentrySummonFadeOutTime)
                {
                    SpriteRenderer.FadeIn(sentrySummonID, 1.0f, 1.0f);
                    SpriteRenderer.SetIsVisible(sentrySummonID, false);
                    fadeUpElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    hasSummonedSentry = false;
                    currentState = TutorialState.Wait;
                }
            }
            else
            {
                ShowSentrySummonUI(true, dt);
            }
        }

        private void ShowUI(uint entityID, bool value, float dt)
        {
            if (value)
            {
                if (fadeUpElapsed < fadeUpTime)
                    fadeUpElapsed += dt;

                SpriteRenderer.FadeIn(entityID, fadeUpElapsed, fadeUpTime);

                Vector3 newPos = Transform.GetPosition(entityID);
                newPos.Y = uiStartFadePos - (10.0f * fadeUpElapsed / fadeUpTime);
                Transform.SetPosition(entityID, ref newPos);

                if (SpriteRenderer.GetIsVisible(entityID) != value)
                    SpriteRenderer.SetIsVisible(entityID, value);
            }
            else
            {
                fadeOutElapsed += dt;
                SpriteRenderer.FadeOut(entityID, fadeOutElapsed, fadeOutTime);

                if (fadeOutElapsed > fadeOutTime)
                    SpriteRenderer.SetIsVisible(entityID, value);
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

        private void OnTurretDestroyed(string eventName, string payload)
        {
            oneturretDestroyed = true;
        }

        private void OnFiveTurretDestroyed(string eventName, string payload)
        {
            turretsDestroyed = true;
        }

        private void OnUltCharged(string eventName, string payload)
        {
            ultCharged = true;
        }

        private void OnAltFired(string eventName, string payload)
        {
            altUsed = true;
        }

        private void OnCoreDeath(string eventName, string payload)
        {
            SpriteRenderer.SetIsVisible(pressWASDID, false);
            SpriteRenderer.SetIsVisible(pressFlyTunnelID, false);
            SpriteRenderer.SetIsVisible(pressShootID, false);
            SpriteRenderer.SetIsVisible(destroyTurretID, false);
            SpriteRenderer.SetIsVisible(destroyEnemiesID, false);
            SpriteRenderer.SetIsVisible(altFireID, false);
            SpriteRenderer.SetIsVisible(proceedID, false);

            ResetTutorialFocusState();

            EPressed = false;
            fadeUpElapsed = 0.0f;
            fadeOutElapsed = 0.0f;
            tooltipElapsed = 0.0f;
            currentState = TutorialState.CollectUpgradeModule;
        }

        private void OnCollectPayload(string eventName, string payload)
        {
            hasCollectedUpgrade = true;
            fadeUpElapsed = 0.0f;
            fadeOutElapsed = 0.0f;
        }

        private void OnSentrySpawned(string eventName, string payload)
        {
            hasSummonedSentry = true;
            fadeUpElapsed = 0.0f;
            fadeOutElapsed = 0.0f;
        }

        private void OnFadeBlackInDone(string eventName, string payload)
        {
            blackFadeInDone = true;
        }

        private void OnFadeBlackOutDone(string eventName, string payload)
        {
            blackFadeOutDone = true;
        }

        private void OnGameResumed(string eventName, string payload)
        {
            playerPauseOnTutorial = false;
        }

        private void ShowCollectUpgradeUI(bool value, float dt)
        {
            if (value)
            {
                if (fadeUpElapsed < fadeUpTime)
                    fadeUpElapsed += dt;

                SpriteRenderer.FadeIn(collectUpgradeModuleID, fadeUpElapsed, fadeUpTime);

                Vector3 newPos = Transform.GetPosition(collectUpgradeModuleID);
                newPos.Y = uiStartFadePos - (10.0f * fadeUpElapsed / fadeUpTime);
                Transform.SetPosition(collectUpgradeModuleID, ref newPos);

                if (!SpriteRenderer.GetIsVisible(collectUpgradeModuleID))
                    SpriteRenderer.SetIsVisible(collectUpgradeModuleID, true);
            }
            else
            {
                fadeOutElapsed += dt;
                SpriteRenderer.FadeOut(collectUpgradeModuleID, fadeOutElapsed, fadeOutTime);

                if (fadeOutElapsed > fadeOutTime)
                    SpriteRenderer.SetIsVisible(collectUpgradeModuleID, false);
            }
        }

        private void ShowSentrySummonUI(bool value, float dt)
        {
            if (value)
            {
                if (fadeUpElapsed < fadeUpTime)
                    fadeUpElapsed += dt;

                SpriteRenderer.FadeIn(sentrySummonID, fadeUpElapsed, fadeUpTime);

                Vector3 newPos = Transform.GetPosition(sentrySummonID);
                newPos.Y = uiStartFadePos - (10.0f * fadeUpElapsed / fadeUpTime);
                Transform.SetPosition(sentrySummonID, ref newPos);

                if (!SpriteRenderer.GetIsVisible(sentrySummonID))
                    SpriteRenderer.SetIsVisible(sentrySummonID, true);
            }
            else
            {
                fadeOutElapsed += dt;
                SpriteRenderer.FadeOut(sentrySummonID, fadeOutElapsed, sentrySummonFadeOutTime);

                if (fadeOutElapsed > sentrySummonFadeOutTime)
                    SpriteRenderer.SetIsVisible(sentrySummonID, false);
            }
        }

        private bool IsEscapeJustPressed()
        {
            bool pressed = IsKeyPressed(KeyCode.Escape);
            bool justPressed = pressed && !prevEscapePressed;
            prevEscapePressed = pressed;
            return justPressed;
        }

        private bool IsWallAlreadyDestroyed(Vector3 currentPos, Vector3 currentWallPos)
        {
            return wallDestroyedPublished || currentPos.X < (currentWallPos.X - 2.0f);
        }

        private Vector3 GetLiveCameraTarget()
        {
            if (cameraID == 0)
                return Vector3.Zero;

            Vector3 camPos = Transform.GetPosition(cameraID);
            Quat camRot = Transform.GetRotation(cameraID);
            Vector3 camForward = camRot.Forward;

            if (camForward.SqrMagnitude < 1e-8f)
                camForward = Vector3.Forward;
            else
                camForward = camForward.Normalized;

            return camPos + camForward * flyThroughFinalLookDistance;
        }

        private void GetLiveCameraPose(out Vector3 camPos, out Vector3 camTarget)
        {
            if (cameraID == 0)
            {
                camPos = Vector3.Zero;
                camTarget = Vector3.Zero;
                return;
            }

            camPos = Transform.GetPosition(cameraID);
            camTarget = GetLiveCameraTarget();
        }

        private void ApplyCameraPose(Vector3 camPos, Vector3 camTarget)
        {
            if (cameraID == 0)
                return;

            Transform.SetPosition(cameraID, ref camPos);
            SetTarget(cameraID, ref camTarget);
        }

        private void RefreshTutorialSavedCameraPoseFromLive()
        {
            if (cameraID == 0)
                return;

            Vector3 currentCamPos;
            Vector3 currentCamTarget;
            GetLiveCameraPose(out currentCamPos, out currentCamTarget);

            tutorialCamSavedPos = currentCamPos;
            tutorialCamSavedTarget = currentCamTarget;
            tutorialCamPoseSaved = true;
        }

        private void BeginTutorialCameraMove(Vector3 targetPos, Vector3 targetLookAt, bool saveCurrentPose)
        {
            if (cameraID == 0)
            {
                tutorialCamMoveActive = false;
                tutorialCamMoveCompleted = true;
                return;
            }

            Vector3 currentCamPos;
            Vector3 currentCamTarget;
            GetLiveCameraPose(out currentCamPos, out currentCamTarget);

            if (saveCurrentPose || !tutorialCamPoseSaved)
            {
                tutorialCamSavedPos = currentCamPos;
                tutorialCamSavedTarget = currentCamTarget;
                tutorialCamPoseSaved = true;
            }

            tutorialCamMoveStartPos = currentCamPos;
            tutorialCamMoveStartTarget = currentCamTarget;
            tutorialCamMoveEndPos = targetPos;
            tutorialCamMoveEndTarget = targetLookAt;
            tutorialCamMoveElapsed = 0.0f;

            Vector3 posDelta = tutorialCamMoveEndPos - tutorialCamMoveStartPos;
            Vector3 targetDelta = tutorialCamMoveEndTarget - tutorialCamMoveStartTarget;
            float posDistance = (float)Math.Sqrt(posDelta.SqrMagnitude);
            float targetDistance = (float)Math.Sqrt(targetDelta.SqrMagnitude);
            float travelDistance = posDistance > targetDistance ? posDistance : targetDistance;
            float safeSpeed = camFlySpeed > 0.01f ? camFlySpeed : 100.0f;

            tutorialCamMoveDuration = travelDistance / safeSpeed;
            if (tutorialCamMoveDuration < 0.01f)
                tutorialCamMoveDuration = 0.01f;

            tutorialCamMoveActive = true;
            tutorialCamMoveCompleted = false;
        }

        private void BeginReturnTutorialCameraMove()
        {
            if (!tutorialCamPoseSaved)
            {
                RefreshTutorialSavedCameraPoseFromLive();
            }

            if (!tutorialCamPoseSaved)
            {
                tutorialCamMoveActive = false;
                tutorialCamMoveCompleted = true;
                return;
            }

            BeginTutorialCameraMove(tutorialCamSavedPos, tutorialCamSavedTarget, false);
        }

        private void UpdateTutorialCameraMove(float dt)
        {
            if (!tutorialCamMoveActive || cameraID == 0)
                return;

            tutorialCamMoveElapsed += dt;

            float t = tutorialCamMoveElapsed / tutorialCamMoveDuration;
            t = SimpleMath.Clamp(t, 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t);

            Vector3 camPos = Vector3.Lerp(tutorialCamMoveStartPos, tutorialCamMoveEndPos, t);
            Vector3 camTarget = Vector3.Lerp(tutorialCamMoveStartTarget, tutorialCamMoveEndTarget, t);

            ApplyCameraPose(camPos, camTarget);

            if (tutorialCamMoveElapsed >= tutorialCamMoveDuration)
            {
                tutorialCamMoveActive = false;
                tutorialCamMoveCompleted = true;
            }
        }

        private bool IsTutorialCameraMoveFinished()
        {
            return !tutorialCamMoveActive && tutorialCamMoveCompleted;
        }

        private void ResetTutorialFocusState()
        {
            tutorialCamMoveActive = false;
            tutorialCamMoveCompleted = false;
            tutorialCamPoseSaved = false;
            tutorialCamSavedPos = Vector3.Zero;
            tutorialCamSavedTarget = Vector3.Zero;
            tutorialCamMoveStartPos = Vector3.Zero;
            tutorialCamMoveStartTarget = Vector3.Zero;
            tutorialCamMoveEndPos = Vector3.Zero;
            tutorialCamMoveEndTarget = Vector3.Zero;
            tutorialCamMoveElapsed = 0.0f;
            tutorialCamMoveDuration = 0.0f;

            shootWallStateEntered = false;
            shootWallReturnStarted = false;
            destroyTurretStateEntered = false;
            destroyTurretReturnStarted = false;
            collectUpgradeStateEntered = false;
            collectUpgradeReturnStarted = false;
        }

        private void StartFlyThroughCinematic()
        {
            if (flyThroughStarted)
                return;

            if (cameraID == 0)
            {
                LogError("[TutorialUIManager] Cannot start fly-through. PlayerCam not found.");
                flyThroughStarted = true;
                flyThroughFinished = true;
                return;
            }

            if (flyThroughWaypoints.Length == 0 || flyThroughWaypoints.Length != flyThroughWaypointTimes.Length)
            {
                LogError("[TutorialUIManager] Cannot start fly-through. Waypoint/timing data is invalid.");
                flyThroughStarted = true;
                flyThroughFinished = true;
                return;
            }

            savedFlyThroughCamPos = Transform.GetPosition(cameraID);
            savedFlyThroughCamTarget = GetLiveCameraTarget();

            savedFlyThroughPlayerPos = Transform.GetPosition(playerID);
            savedFlyThroughPlayerRot = Transform.GetRotation(playerID);

            flyThroughStarted = true;
            flyThroughFinished = false;
            flyThroughSegmentIndex = 0;
            flyThroughSegmentElapsed = 0.0f;
            flyThroughBlackoutState = FlyThroughBlackoutState.None;
            blackFadeInDone = false;
            blackFadeOutDone = false;
            flyThroughTeleportFadeTriggered = false;
            flyThroughFinalFadeTriggered = false;
            flyThroughImmediateReturnRequested = false;
            flyThroughHoldCamPos = savedFlyThroughCamPos;
            flyThroughHoldCamTarget = savedFlyThroughCamTarget;

            Publish(EVENT_CINEMATIC_PLAY, "");
            LogMessage("[TutorialUIManager] Fly-through cinematic started");
        }

        private void UpdateFlyThroughCinematic(float dt)
        {
            if (!flyThroughStarted || flyThroughFinished || cameraID == 0)
                return;

            if (UpdateFlyThroughBlackoutTransition())
                return;

            if (ShouldHoldFlyThroughDuringFadeIn())
            {
                ApplyFlyThroughHoldPose();
                return;
            }

            if (flyThroughSegmentIndex >= flyThroughWaypoints.Length)
            {
                if (flyThroughFinalFadeTriggered)
                {
                    ApplyFlyThroughHoldPose();
                    return;
                }

                BeginFlyThroughFinalCut();
                return;
            }

            Vector3 segmentStart =
                (flyThroughSegmentIndex == 0)
                ? savedFlyThroughCamPos
                : flyThroughWaypoints[flyThroughSegmentIndex - 1];

            Vector3 segmentEnd = flyThroughWaypoints[flyThroughSegmentIndex];

            float segmentDuration = GetSafeFlyThroughSegmentDuration(flyThroughSegmentIndex);

            flyThroughSegmentElapsed += dt;

            float rawT = flyThroughSegmentElapsed / segmentDuration;
            rawT = SimpleMath.Clamp(rawT, 0.0f, 1.0f);

            Vector3 camPos = Vector3.Lerp(segmentStart, segmentEnd, rawT);
            Vector3 lookTarget = GetFlyThroughLookTarget(flyThroughSegmentIndex, rawT, segmentStart, segmentEnd);

            ApplyCameraPose(camPos, lookTarget);
            SaveFlyThroughHoldPose(camPos, lookTarget);

            TryBeginTeleportCutDuringMovement(flyThroughSegmentIndex, rawT);
            TryBeginFinalCutDuringMovement(flyThroughSegmentIndex, rawT);

            if (rawT >= 1.0f)
            {
                flyThroughSegmentElapsed = 0.0f;
                flyThroughSegmentIndex++;

                if (!flyThroughFinalFadeTriggered && flyThroughSegmentIndex >= flyThroughWaypoints.Length)
                    BeginFlyThroughFinalCut();
            }
        }

        private bool ShouldHoldFlyThroughDuringFadeIn()
        {
            if (flyThroughBlackoutState == FlyThroughBlackoutState.TeleportFadeIn)
                return flyThroughSegmentIndex >= flyThroughTeleportWaypointIndex;

            if (flyThroughBlackoutState == FlyThroughBlackoutState.FinalFadeIn)
                return flyThroughSegmentIndex >= flyThroughWaypoints.Length;

            return false;
        }

        private void SaveFlyThroughHoldPose(Vector3 camPos, Vector3 camTarget)
        {
            flyThroughHoldCamPos = camPos;
            flyThroughHoldCamTarget = camTarget;
        }

        private void ApplyFlyThroughHoldPose()
        {
            ApplyCameraPose(flyThroughHoldCamPos, flyThroughHoldCamTarget);
        }

        private void TryBeginTeleportCutDuringMovement(int segmentIndex, float rawT)
        {
            if (flyThroughTeleportFadeTriggered)
                return;

            if (flyThroughBlackoutState != FlyThroughBlackoutState.None)
                return;

            if (flyThroughTeleportWaypointIndex <= 0)
                return;

            if (flyThroughTeleportWaypointIndex >= flyThroughWaypoints.Length)
                return;

            int cutSegmentIndex = flyThroughTeleportWaypointIndex - 1;

            if (segmentIndex > cutSegmentIndex)
                return;

            float remainingTime = GetRemainingTimeUntilTeleportCut(segmentIndex, rawT);
            if (remainingTime <= flyThroughTeleportFadeLeadTime)
                BeginFlyThroughTeleportCut();
        }

        private void TryBeginFinalCutDuringMovement(int segmentIndex, float rawT)
        {
            if (flyThroughFinalFadeTriggered)
                return;

            if (flyThroughBlackoutState != FlyThroughBlackoutState.None)
                return;

            if (flyThroughWaypoints.Length == 0)
                return;

            int finalSegmentIndex = flyThroughWaypoints.Length - 1;

            if (segmentIndex > finalSegmentIndex)
                return;

            float remainingTime = GetRemainingTimeUntilFinalCut(segmentIndex, rawT);
            if (remainingTime <= flyThroughFinalFadeLeadTime)
                BeginFlyThroughFinalCut();
        }

        private float GetSafeFlyThroughSegmentDuration(int segmentIndex)
        {
            if (segmentIndex < 0 || segmentIndex >= flyThroughWaypointTimes.Length)
                return 0.0001f;

            float duration = flyThroughWaypointTimes[segmentIndex];
            if (duration < 0.0001f)
                duration = 0.0001f;

            return duration;
        }

        private float GetRemainingTimeUntilTeleportCut(int segmentIndex, float rawT)
        {
            if (flyThroughTeleportWaypointIndex <= 0)
                return 0.0f;

            int cutSegmentIndex = flyThroughTeleportWaypointIndex - 1;
            if (segmentIndex > cutSegmentIndex)
                return 0.0f;

            float clampedT = SimpleMath.Clamp(rawT, 0.0f, 1.0f);
            float remaining = 0.0f;

            for (int i = segmentIndex; i <= cutSegmentIndex; ++i)
            {
                float segmentDuration = GetSafeFlyThroughSegmentDuration(i);

                if (i == segmentIndex)
                    remaining += segmentDuration * (1.0f - clampedT);
                else
                    remaining += segmentDuration;
            }

            return remaining;
        }

        private float GetRemainingTimeUntilFinalCut(int segmentIndex, float rawT)
        {
            int finalSegmentIndex = flyThroughWaypoints.Length - 1;
            if (segmentIndex > finalSegmentIndex)
                return 0.0f;

            float clampedT = SimpleMath.Clamp(rawT, 0.0f, 1.0f);
            float remaining = 0.0f;

            for (int i = segmentIndex; i <= finalSegmentIndex; ++i)
            {
                float segmentDuration = GetSafeFlyThroughSegmentDuration(i);

                if (i == segmentIndex)
                    remaining += segmentDuration * (1.0f - clampedT);
                else
                    remaining += segmentDuration;
            }

            return remaining;
        }

        private void BeginFlyThroughTeleportCut()
        {
            if (flyThroughTeleportFadeTriggered)
                return;

            if (flyThroughBlackoutState != FlyThroughBlackoutState.None)
                return;

            flyThroughTeleportFadeTriggered = true;
            blackFadeInDone = false;
            blackFadeOutDone = false;

            if (fadeBlackID == 0)
            {
                LogMessage("[TutorialUIManager] FadeBlack not found. Teleporting fly-through without blackout.");
                PerformFlyThroughTeleportCut();
                return;
            }

            flyThroughBlackoutState = FlyThroughBlackoutState.TeleportFadeIn;
            Publish(EVENT_FADE_BLACK_IN, "");
        }

        private void PerformFlyThroughTeleportCut()
        {
            TeleportCameraToWaypoint(flyThroughTeleportWaypointIndex);
            flyThroughSegmentIndex = flyThroughTeleportWaypointIndex + 1;
            flyThroughSegmentElapsed = 0.0f;
            flyThroughBlackoutState = FlyThroughBlackoutState.None;

            Vector3 camPos = flyThroughWaypoints[flyThroughTeleportWaypointIndex];
            Vector3 lookTarget = GetFlyThroughResumeLookTarget(flyThroughTeleportWaypointIndex);
            SaveFlyThroughHoldPose(camPos, lookTarget);
        }

        private void BeginFlyThroughFinalCut()
        {
            if (flyThroughFinalFadeTriggered)
                return;

            if (flyThroughBlackoutState != FlyThroughBlackoutState.None)
                return;

            flyThroughFinalFadeTriggered = true;
            blackFadeInDone = false;
            blackFadeOutDone = false;

            if (fadeBlackID == 0)
            {
                LogMessage("[TutorialUIManager] FadeBlack not found. Finishing fly-through without blackout.");
                RestorePlayerAndCameraAfterCinematic();
                FinishFlyThroughCinematic();
                return;
            }

            flyThroughBlackoutState = FlyThroughBlackoutState.FinalFadeIn;
            Publish(EVENT_FADE_BLACK_IN, "");
        }

        private bool UpdateFlyThroughBlackoutTransition()
        {
            switch (flyThroughBlackoutState)
            {
                case FlyThroughBlackoutState.None:
                    return false;

                case FlyThroughBlackoutState.TeleportFadeIn:
                    if (!blackFadeInDone)
                        return false;

                    blackFadeInDone = false;
                    PerformFlyThroughTeleportCut();
                    Publish(EVENT_FADE_BLACK_OUT, "");
                    flyThroughBlackoutState = FlyThroughBlackoutState.TeleportFadeOut;
                    return true;

                case FlyThroughBlackoutState.TeleportFadeOut:
                    if (blackFadeOutDone)
                    {
                        blackFadeOutDone = false;
                        flyThroughBlackoutState = FlyThroughBlackoutState.None;
                    }
                    return false;

                case FlyThroughBlackoutState.FinalFadeIn:
                    if (!blackFadeInDone)
                        return false;

                    blackFadeInDone = false;
                    RestorePlayerAndCameraAfterCinematic();
                    Publish(EVENT_FADE_BLACK_OUT, "");
                    flyThroughBlackoutState = FlyThroughBlackoutState.FinalFadeOut;
                    return true;

                case FlyThroughBlackoutState.FinalFadeOut:
                    if (!blackFadeOutDone)
                        return false;

                    blackFadeOutDone = false;
                    flyThroughBlackoutState = FlyThroughBlackoutState.None;
                    FinishFlyThroughCinematic();
                    return true;
            }

            return false;
        }

        private void TeleportCameraToWaypoint(int waypointIndex)
        {
            if (cameraID == 0)
                return;

            if (waypointIndex < 0 || waypointIndex >= flyThroughWaypoints.Length)
                return;

            Vector3 camPos = flyThroughWaypoints[waypointIndex];
            Vector3 lookTarget = GetFlyThroughResumeLookTarget(waypointIndex);
            ApplyCameraPose(camPos, lookTarget);
        }

        private Vector3 GetFlyThroughResumeLookTarget(int waypointIndex)
        {
            int nextIndex = waypointIndex + 1;

            if (nextIndex < flyThroughWaypoints.Length)
                return flyThroughWaypoints[nextIndex];

            if (waypointIndex > 0)
            {
                Vector3 dir = flyThroughWaypoints[waypointIndex] - flyThroughWaypoints[waypointIndex - 1];
                if (dir.SqrMagnitude < 1e-8f)
                    dir = Vector3.Forward;
                else
                    dir = dir.Normalized;

                return flyThroughWaypoints[waypointIndex] + dir * flyThroughFinalLookDistance;
            }

            return flyThroughWaypoints[waypointIndex] + Vector3.Forward * flyThroughFinalLookDistance;
        }

        private Vector3 GetFlyThroughLookTarget(int segmentIndex, float rawT, Vector3 segmentStart, Vector3 segmentEnd)
        {
            int nextIndex = segmentIndex + 1;

            if (nextIndex < flyThroughWaypoints.Length)
            {
                float blendStart = SimpleMath.Clamp(flyThroughLookAheadStartT, 0.0f, 0.99f);

                if (rawT <= blendStart)
                    return segmentEnd;

                float lookT = (rawT - blendStart) / (1.0f - blendStart);
                lookT = SimpleMath.Clamp(lookT, 0.0f, 1.0f);
                lookT = lookT * lookT * (3.0f - 2.0f * lookT);

                return Vector3.Lerp(segmentEnd, flyThroughWaypoints[nextIndex], lookT);
            }

            Vector3 dir = segmentEnd - segmentStart;
            if (dir.SqrMagnitude < 1e-8f)
                dir = Vector3.Forward;
            else
                dir = dir.Normalized;

            return segmentEnd + dir * flyThroughFinalLookDistance;
        }

        private void RestorePlayerAndCameraAfterCinematic()
        {
            Vector3 playerPos = savedFlyThroughPlayerPos;
            Quat playerRot = savedFlyThroughPlayerRot;
            Vector3 camPos = savedFlyThroughCamPos;
            Vector3 camTarget = savedFlyThroughCamTarget;

            Transform.SetPosition(playerID, ref playerPos);
            Transform.SetRotation(playerID, ref playerRot);

            if (cameraID != 0)
                ApplyCameraPose(camPos, camTarget);

            SaveFlyThroughHoldPose(camPos, camTarget);
        }

        private void FinishFlyThroughCinematic()
        {
            if (cameraID != 0)
                ApplyCameraPose(savedFlyThroughCamPos, savedFlyThroughCamTarget);

            flyThroughFinished = true;
            flyThroughBlackoutState = FlyThroughBlackoutState.None;
            flyThroughTeleportFadeTriggered = false;
            flyThroughFinalFadeTriggered = false;
            flyThroughImmediateReturnRequested = false;

            Publish(EVENT_CINEMATIC_STOP, "");
            LogMessage("[TutorialUIManager] Fly-through cinematic finished");
        }

        private void ResetFlyThroughCinematicState()
        {
            flyThroughStarted = false;
            flyThroughFinished = false;
            flyThroughSegmentIndex = 0;
            flyThroughSegmentElapsed = 0.0f;

            savedFlyThroughCamPos = Vector3.Zero;
            savedFlyThroughCamTarget = Vector3.Zero;
            savedFlyThroughPlayerPos = Vector3.Zero;

            flyThroughBlackoutState = FlyThroughBlackoutState.None;
            blackFadeInDone = false;
            blackFadeOutDone = false;
            flyThroughTeleportFadeTriggered = false;
            flyThroughFinalFadeTriggered = false;
            flyThroughImmediateReturnRequested = false;
            flyThroughHoldCamPos = Vector3.Zero;
            flyThroughHoldCamTarget = Vector3.Zero;
        }

        private void BeginFlyThroughImmediateReturnToPlayer()
        {
            if (!flyThroughStarted || flyThroughFinished)
                return;

            if (flyThroughBlackoutState == FlyThroughBlackoutState.FinalFadeIn ||
                flyThroughBlackoutState == FlyThroughBlackoutState.FinalFadeOut)
            {
                flyThroughImmediateReturnRequested = true;
                return;
            }

            flyThroughImmediateReturnRequested = true;
            flyThroughFinalFadeTriggered = true;
            blackFadeInDone = false;
            blackFadeOutDone = false;

            if (fadeBlackID == 0)
            {
                LogMessage("[TutorialUIManager] FadeBlack not found. Returning to player immediately.");
                RestorePlayerAndCameraAfterCinematic();
                FinishFlyThroughCinematic();
                return;
            }

            flyThroughBlackoutState = FlyThroughBlackoutState.FinalFadeIn;
            Publish(EVENT_FADE_BLACK_IN, "");
        }
    }
}

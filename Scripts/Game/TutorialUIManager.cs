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
            Move,           // Show WASD
            FlyThrough,     // Show fly tunnel
            ShootWall,      // Show shoot wall
            DestroyTurret, // Show destroy turret
            DestroyEnemies, // Show destroy enemies
            Wait,
            AltFire,
            CollectUpgradeModule //Show collect upgrade tooltip
        }

        private TutorialState currentState = TutorialState.Move;

        // Entity IDs - UI
        private uint pressWASDID; // UI_PressWASD
        private uint pressFlyTunnelID; // UI_FlyTunnel
        private uint pressShootID; // UI_Shoot
        private uint destroyTurretID;  // UI_DestroyTurret
        private uint destroyEnemiesID; // UI_DestroyEnemies
        private uint altFireID; // UI_AltFire
        private uint proceedID; // UI_E_ToProceed
        private uint collectUpgradeModuleID; //UI_CollectUpgradeModule

        // Entity IDs - Player, Wall
        private uint playerID;
        private uint wallID;

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

        // Events
        private string EVENT_ONE_TURRET_DESTROYED = "OneTurretDestroyed";
        private string EVENT_FIVE_TURRETS_DESTROYED = "FiveTurretsDestroyed";
        private const string EVENT_ULT_CHARGED = "UltCharged";
        private const string EVENT_ALT_FIRED = "AltFired";
        private const string EVENT_CORE_DEAD = "CoreDeadTriggerPostTrenchRun";
        private const string EVENT_COLLECT_PAYLOAD = "CollectPayload";

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

        // Collect Upgrade Tooltip fading (dedicated timers to avoid interfering with state machine)
        //private bool showCollectUI = false;
        //private bool hideCollectUI = false;
        //private float collectFadeUpElapsed = 0.0f;
        //private float collectFadeOutElapsed = 0.0f;

        private bool movedWASD = false;
        private bool movedSpacebar = false;
        private bool movedShift = false;
        private bool oneturretDestroyed = false;
        private bool turretsDestroyed = false;
        private bool ultCharged = false;
        private bool altUsed = false;
        private bool altFireShown = false;
        private bool hasCollectedUpgrade = false;

        private Vector3 wallPos = new Vector3(0.0f, 0.0f, 0.0f);

        private bool wallDestroyedPublished = false;
        private bool skipTutorial = false;

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


            if (ProgressTracker.SkipTutorialLevel1)
            {
                skipTutorial = true;
                SpriteRenderer.SetIsVisible(pressWASDID, false);

            } else
            {
                skipTutorial = false;
                SpriteRenderer.SetIsVisible(pressWASDID, true);
            }

            // Show initial UI
            SpriteRenderer.SetIsVisible(pressFlyTunnelID, false);
            SpriteRenderer.SetIsVisible(pressShootID, false);
            SpriteRenderer.SetIsVisible(destroyTurretID, false);
            SpriteRenderer.SetIsVisible(destroyEnemiesID, false);
            SpriteRenderer.SetIsVisible(altFireID, false);
            SpriteRenderer.SetIsVisible(proceedID, false);
            SpriteRenderer.SetIsVisible(collectUpgradeModuleID, false);

            wallPos = Transform.GetPosition(wallID);

            Subscribe(EVENT_ONE_TURRET_DESTROYED, OnTurretDestroyed);
            Subscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
            Subscribe(EVENT_ULT_CHARGED, OnUltCharged);
            Subscribe(EVENT_ALT_FIRED, OnAltFired);
            Subscribe(EVENT_CORE_DEAD, OnCoreDeath);
            Subscribe(EVENT_COLLECT_PAYLOAD, OnCollectPayload);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (skipTutorial)
            {
                currentState = TutorialState.Move;
                return;
            }

            if (GameState.IsPaused)
                return;

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
        }

        private void HandleMoveState(Vector3 currentPos, float dt)
        {
            if (!movedWASD && (IsKeyPressed(KeyCode.W) || IsKeyPressed(KeyCode.S) || IsKeyPressed(KeyCode.D) || IsKeyPressed(KeyCode.A))){
                movedWASD = true;
            }

            if (!movedSpacebar && IsKeyPressed(KeyCode.Space)){
                movedSpacebar = true;
            }

            if (!movedShift && IsKeyPressed(KeyCode.LeftShift)){
                movedShift = true;
            }

            if (movedWASD)
            {
                // Fade out WASD
                ShowUI(pressWASDID, false, dt);

                // Fade in Fly Tunnel
                if (fadeOutElapsed > switchTime) {
                    ShowUI(pressFlyTunnelID, true, dt);
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.FlyThrough;
                }
            }
        }

        private void HandleFlyThroughState(Vector3 currentPos, Vector3 wallPos, float dt)
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
                    ShowUI(pressFlyTunnelID, false, dt);
                    ShowProceedText(false);  // Remove assistance message
                }

                // Begin fade up for next 
                if (currentPos.X < (wallPos.X + 40) && EPressed && (fadeOutElapsed > switchTime)) {
                    //ShowShootUI(true, dt);
                    ShowUI(pressShootID, true, dt);
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.ShootWall;
                }
            }
        }

        private void HandleShootWallState(Vector3 currentPos, Vector3 wallPos, float dt)
        {
            altUsed = false;

            // Fade out shoot UI after destroying wall
            if (currentPos.X < (wallPos.X - 2) && !wallDestroyedPublished) {
                LogMessage("[TutorialUIManager] Detect wall is destroyed");
                wallDestroyedPublished = true;
                Publish("DestructableWallDestroyed", true.ToString());
                ShowUI(pressShootID, false, dt);
            } else if(wallDestroyedPublished){
                ShowUI(pressShootID, false, dt);
            }

            // Fade up destroy turret UI
            if (fadeOutElapsed > switchTime) ShowUI(destroyTurretID, true, dt);

            // Ensure all fading effects are completed before moving on
            if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime) {

                // Reset all elapsed time, 'E' pressed state, and set up for next state
                EPressed = false;
                tooltipElapsed = 0.0f;
                fadeOutElapsed = 0.0f;
                fadeUpElapsed = 0.0f;
                currentState = TutorialState.DestroyTurret;
            }
        }

        private void HandleDestroyTurretState(float dt)
        {
            altUsed = false;

            // Update tooltip elapsed time
            tooltipElapsed += dt;

            // Fade out upon pressing E only AFTER tooltip exists beyond min duration
            if (tooltipElapsed > tooltipMinTime) {

                // Display assistance message: "Press "E" to proceed"
                //SetProceedTextPosition(1107.1f, 475.8f);
                ShowProceedText(true);

                // Check for 'E' input
                if (IsKeyPressed(KeyCode.E)) EPressed = true;

                // Begin fade out
                if (EPressed) {
                    ShowUI(destroyTurretID, false, dt);
                    ShowProceedText(false);  // Remove assistance message
                }

                // Begin fade up for next 
                if (EPressed && (fadeOutElapsed > switchTime)) {
                    ShowUI(destroyEnemiesID, true, dt);
                }

                // Ensure all fading effects are completed before moving on
                if (fadeOutElapsed > fadeOutTime && fadeUpElapsed > fadeUpTime) {

                    // Reset all elapsed time, 'E' pressed state, and set up for next state
                    EPressed = false;
                    tooltipElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    fadeUpElapsed = 0.0f;
                    currentState = TutorialState.DestroyEnemies;
                }

            }
        }

        private void HandleDestroyEnemiesState(float dt)
        {
            altUsed = false;

            // Fade out destroy enemies UIs
            if (turretsDestroyed) ShowUI(destroyEnemiesID, false, dt);

            // Ensure fading out effect are completed before moving on
            if (fadeOutElapsed > fadeOutTime) {

                // Reset fade out elapsed time and set up for the next state
                fadeOutElapsed = 0.0f;
                currentState = TutorialState.Wait;
            }
        }

        private void HandleWaitState(float dt)
        {
            if (ultCharged && !altFireShown && !altUsed) {
                ShowUI(altFireID, true, dt);
            }

            // Ensure fading out effect are completed before moving on
            if (fadeUpElapsed > fadeUpTime) {

                // Reset fade up elapsed time and set up for the next state
                fadeUpElapsed = 0.0f;
                altFireShown = true;
                currentState = TutorialState.AltFire;
            } else if (altUsed && !altFireShown){
                //Alt was used before tooltip finished fading in - Skip to altfire to clean up
                fadeUpElapsed = 0.0f;
                altFireShown = true;
                currentState = TutorialState.AltFire;
            }
        }

        private void HandleAltFire(float dt)
        {
            if (altUsed) ShowUI(altFireID, false, dt);

            // Ensure fading out effect are completed before moving on
            if (fadeUpElapsed > fadeUpTime) {

                // Reset fade up elapsed time and set up for the next state
                fadeUpElapsed = 0.0f;
                currentState = TutorialState.Wait;
            }
        }

        private void HandleCollectUpgradeModuleState(float dt)
        {
            if (hasCollectedUpgrade)
            {
                ShowCollectUpgradeUI(false, dt);

                if (fadeOutElapsed > fadeOutTime)
                {
                    fadeUpElapsed = 0.0f;
                    fadeOutElapsed = 0.0f;
                    hasCollectedUpgrade = false;
                    currentState = TutorialState.Wait;
                }
            }
            else
            {
                ShowCollectUpgradeUI(true, dt);
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

        //private void ShowWASD(bool value)
        //{
        //    SpriteRenderer.SetIsVisible(pressWASDID, value);
        //}

        //private void ShowFlyTunnel(bool value)
        //{
        //    SpriteRenderer.SetIsVisible(pressFlyTunnelID, value);
        //}

        //private void ShowShootUI(bool value)
        //{
        //    SpriteRenderer.SetIsVisible(pressShootID, value);
        //}

        //private void ShowDestroyTurret(bool value)
        //{
        //    SpriteRenderer.SetIsVisible(destroyTurretID, value);
        //}

        //private void ShowDestroyEnemies(bool value)
        //{
        //    SpriteRenderer.SetIsVisible(destroyEnemiesID, value);
        //}

        //private void ShowAltFire(bool value)
        //{
        //    SpriteRenderer.SetIsVisible(altFireID, value);
        //}

        private void ShowProceedText(bool value)
        {
            Engine.SpriteRenderer.SetIsVisible(proceedID, value);
        }

        private void SetProceedTextPosition(float x, float y)
        {
            Vector3 newPos = new Vector3(x, y, 0.0f);
            Transform.SetPosition(proceedID, ref newPos);
        }

        private void OnTurretDestroyed(string eventName, string payload){
            oneturretDestroyed = true;
        }

        private void OnFiveTurretDestroyed(string eventName, string payload){
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
            // Force hide all other tooltips in case player never dismissed them
            SpriteRenderer.SetIsVisible(pressWASDID, false);
            SpriteRenderer.SetIsVisible(pressFlyTunnelID, false);
            SpriteRenderer.SetIsVisible(pressShootID, false);
            SpriteRenderer.SetIsVisible(destroyTurretID, false);
            SpriteRenderer.SetIsVisible(destroyEnemiesID, false);
            SpriteRenderer.SetIsVisible(altFireID, false);
            SpriteRenderer.SetIsVisible(proceedID, false);

            // Reset timers and transition to collect upgrade state
            EPressed = false;
            fadeUpElapsed = 0.0f;
            fadeOutElapsed = 0.0f;
            currentState = TutorialState.CollectUpgradeModule;
        }

        private void OnCollectPayload(string eventName, string payload)
        {
            hasCollectedUpgrade = true;
            fadeUpElapsed = 0.0f;
            fadeOutElapsed = 0.0f;
        }

        private void ShowCollectUpgradeUI(bool value, float dt)
        {
            if (value)
            {
                if (fadeUpElapsed < fadeUpTime) fadeUpElapsed += dt;
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
    }
}
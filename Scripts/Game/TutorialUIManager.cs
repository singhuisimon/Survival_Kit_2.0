using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Camera;
using static Engine.Input;

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
            AltFire
        }

        private TutorialState currentState = TutorialState.Move;

        // Entity IDs - UI
        private uint pressWASDID; // UI_PressWASD
        private uint pressFlyTunnelID; // UI_FlyTunnel
        private uint pressShootID; // UI_Shoot
        private uint destroyTurretID;  // UI_DestroyTurret
        private uint destroyEnemiesID; // UI_DestroyEnemies
        private uint altFireID; // UI_AltFire

        // Entity IDs - Player, Wall
        private uint playerID;
        private uint wallID;

        // Entity Names
        [SerializeField] private string pressWASDName = "UI_PressWASD";
        [SerializeField] private string pressFlyTunnelName = "UI_FlyTunnel";
        [SerializeField] private string playerName = "Player";
        [SerializeField] private string wallName = "DestructableWall";
        [SerializeField] private string pressShootName = "UI_Shoot";
        [SerializeField] private string destroyTurretName = "UI_DestroyTurret";
        [SerializeField] private string destroyEnemiesName = "UI_DestroyEnemies";
        [SerializeField] private string altFireName = "UI_AltFire";

        // Events
        private string EVENT_ONE_TURRET_DESTROYED = "OneTurretDestroyed";
        private string EVENT_FIVE_TURRETS_DESTROYED = "FiveTurretsDestroyed";
        private const string EVENT_ULT_CHARGED = "UltCharged";
        private const string EVENT_ALT_FIRED = "AltFired";
        
        private bool movedWASD = false;
        private bool movedSpacebar = false;
        private bool movedShift = false;
        private bool oneturretDestroyed = false;
        private bool turretsDestroyed = false;
        private bool ultCharged = false;
        private bool altUsed = false;
        private bool altFireShown = false;

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

            // Show initial UI
            ShowWASD(true);
            ShowFlyTunnel(false);
            ShowShootUI(false);
            ShowDestroyTurret(false);
            ShowDestroyEnemies(false);
            ShowAltFire(false);

            Subscribe(EVENT_ONE_TURRET_DESTROYED, OnTurretDestroyed);
            Subscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
            Subscribe(EVENT_ULT_CHARGED, OnUltCharged);
            Subscribe(EVENT_ALT_FIRED, OnAltFired);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused)
                return;

            Vector3 currentPos = Transform.GetPosition(playerID);
            Vector3 wallPos = Transform.GetPosition(wallID);

            switch (currentState)
            {
                case TutorialState.Move:
                    HandleMoveState(currentPos);
                    break;

                case TutorialState.FlyThrough:
                    HandleFlyThroughState(currentPos, wallPos);
                    break;

                case TutorialState.ShootWall:
                    HandleShootWallState(currentPos, wallPos);
                    break;

                case TutorialState.DestroyTurret:
                    HandleDestroyTurretState();
                    break;

                case TutorialState.DestroyEnemies:
                    HandleDestroyEnemiesState();
                    break;

                case TutorialState.Wait:
                    HandleWaitState();
                    break;

                case TutorialState.AltFire:
                    HandleAltFire();
                    break;
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_ONE_TURRET_DESTROYED, OnTurretDestroyed);
            Unsubscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
            Unsubscribe(EVENT_ULT_CHARGED, OnUltCharged);
            Unsubscribe(EVENT_ALT_FIRED, OnAltFired);
        }

        private void HandleMoveState(Vector3 currentPos)
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

            if (movedWASD && movedSpacebar && movedShift)
            {
                ShowWASD(false);
                ShowFlyTunnel(true);
                currentState = TutorialState.FlyThrough;
            }
        }

        private void HandleFlyThroughState(Vector3 currentPos, Vector3 wallPos)
        {
            if (currentPos.X < (wallPos.X + 40)){
                ShowFlyTunnel(false);
                ShowShootUI(true);
                currentState = TutorialState.ShootWall;
            }
        }

        private void HandleShootWallState(Vector3 currentPos, Vector3 wallPos)
        {
            altUsed = false;

            if (currentPos.X < (wallPos.X - 2))
            {
                ShowShootUI(false);
                ShowDestroyTurret(true);
                currentState = TutorialState.DestroyTurret;
            }
        }

        private void HandleDestroyTurretState(){

            altUsed = false;

            if (oneturretDestroyed){
                ShowDestroyTurret(false);
                ShowDestroyEnemies(true);
                currentState = TutorialState.DestroyEnemies;
            }
        }

        private void HandleDestroyEnemiesState()
        {
            altUsed = false;

            if (turretsDestroyed){
                ShowDestroyEnemies(false);
                currentState = TutorialState.Wait;
            }
        }

        private void HandleWaitState()
        {
            if (ultCharged && !altFireShown && !altUsed)
            {
                ShowAltFire(true);
                altFireShown = true;
                currentState = TutorialState.AltFire;
            }
        }

        private void HandleAltFire()
        {
            if(altUsed){
                ShowAltFire(false);
                currentState = TutorialState.Wait;
            }
        }

        // UI Functions
        private void ShowWASD(bool value)
        {
            SpriteRenderer.SetIsVisible(pressWASDID, value);
        }

        private void ShowFlyTunnel(bool value)
        {
            SpriteRenderer.SetIsVisible(pressFlyTunnelID, value);
        }

        private void ShowShootUI(bool value)
        {
            SpriteRenderer.SetIsVisible(pressShootID, value);
        }

        private void ShowDestroyTurret(bool value)
        {
            SpriteRenderer.SetIsVisible(destroyTurretID, value);
        }

        private void ShowDestroyEnemies(bool value)
        {
            SpriteRenderer.SetIsVisible(destroyEnemiesID, value);
        }

        private void ShowAltFire(bool value)
        {
            SpriteRenderer.SetIsVisible(altFireID, value);
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
    }
}
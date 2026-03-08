using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Camera;

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
            Wait
        }

        private TutorialState currentState = TutorialState.Move;

        // Entity IDs - UI
        private uint pressWASDID; // UI_PressWASD
        private uint pressFlyTunnelID; // UI_FlyTunnel
        private uint pressShootID; // UI_Shoot
        private uint destroyTurretID;  // UI_DestroyTurret
        private uint destroyEnemiesID; // UI_DestroyEnemies

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

        // Events
        private string EVENT_ONE_TURRET_DESTROYED = "OneTurretDestroyed";
        private string EVENT_FIVE_TURRETS_DESTROYED = "FiveTurretsDestroyed";
        // private const string EVENT_ULT_CHARGED = "UltCharged";
        // private const string EVENT_ALT_FIRED = "AltFired";
        
        private bool oneturretDestroyed = false;
        private bool turretsDestroyed = false;
        private Vector3 startPlayerPos;

        public override void OnStart()
        {
            pressWASDID = SceneFindEntityByName(pressWASDName);
            pressFlyTunnelID = SceneFindEntityByName(pressFlyTunnelName);
            playerID = SceneFindEntityByName(playerName);
            wallID = SceneFindEntityByName(wallName);
            pressShootID = SceneFindEntityByName(pressShootName);
            destroyTurretID = SceneFindEntityByName(destroyTurretName);
            destroyEnemiesID = SceneFindEntityByName(destroyEnemiesName);

            startPlayerPos = Transform.GetPosition(playerID);

            // Show initial UI
            ShowWASD(true);
            ShowFlyTunnel(false);
            ShowShootUI(false);
            ShowDestroyTurret(false);
            ShowDestroyEnemies(false);

            Subscribe(EVENT_ONE_TURRET_DESTROYED, OnTurretDestroyed);
            Subscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
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
                    break;
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_ONE_TURRET_DESTROYED, OnTurretDestroyed);
            Unsubscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
        }

        private void HandleMoveState(Vector3 currentPos)
        {
            if (Vector3.Distance(currentPos, startPlayerPos) > 0.1f)
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
            if (currentPos.X < (wallPos.X - 2))
            {
                ShowShootUI(false);
                ShowDestroyTurret(true);
                currentState = TutorialState.DestroyTurret;
            }
        }

        private void HandleDestroyTurretState(){
            if (oneturretDestroyed){
                ShowDestroyTurret(false);
                ShowDestroyEnemies(true);
                currentState = TutorialState.DestroyEnemies;
            }
        }

        private void HandleDestroyEnemiesState()
        {
            if (turretsDestroyed){
                ShowDestroyEnemies(false);
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

        private void OnTurretDestroyed(string eventName, string payload){
            oneturretDestroyed = true;
        }

        private void OnFiveTurretDestroyed(string eventName, string payload){
            turretsDestroyed = true;
        }
    }
}
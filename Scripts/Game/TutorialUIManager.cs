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
            DestroyEnemies, // Show destroy enemies
            Done
        }

        private TutorialState currentState = TutorialState.Move;

        // Entity IDs - UI
        private uint pressWASDID;
        private uint pressFlyTunnelID;
        private uint pressShootID;
        private uint destroyWallID;
        private uint destroyEnemiesID;

        // Entity IDs - Player, Wall
        private uint playerID;
        private uint wallID;

        // Entity Names
        [SerializeField] private string pressWASDName = "UI_PressWASD";
        [SerializeField] private string pressFlyTunnelName = "UI_FlyTunnel";
        [SerializeField] private string playerName = "Player";
        [SerializeField] private string wallName = "DestructableWall";
        [SerializeField] private string pressShootName = "UI_Shoot";
        [SerializeField] private string destroyWallName = "UI_DestroyWall";
        [SerializeField] private string destroyEnemiesName = "UI_DestroyEnemies";

        private Vector3 startPlayerPos;

        public override void OnStart()
        {
            pressWASDID = SceneFindEntityByName(pressWASDName);
            pressFlyTunnelID = SceneFindEntityByName(pressFlyTunnelName);
            playerID = SceneFindEntityByName(playerName);
            wallID = SceneFindEntityByName(wallName);
            pressShootID = SceneFindEntityByName(pressShootName);
            destroyWallID = SceneFindEntityByName(destroyWallName);
            destroyEnemiesID = SceneFindEntityByName(destroyEnemiesName);

            startPlayerPos = Transform.GetPosition(playerID);

            // Show initial UI
            ShowWASD(true);
            ShowFlyTunnel(false);
            ShowShootUI(false);
            ShowDestroyEnemies(false);
        }

        public override void OnUpdate(float deltaTime)
        {
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

                case TutorialState.DestroyEnemies:
                    HandleDestroyEnemiesState(currentPos, wallPos);
                    break;

                case TutorialState.Done:
                    break;
            }
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
            //if (Vector3.Distance(currentPos, wallPos) < 40f)
                ShowFlyTunnel(false);
                ShowShootUI(true);
                currentState = TutorialState.ShootWall;
            }
        }

        private void HandleShootWallState(Vector3 currentPos, Vector3 wallPos)
        {
            if (currentPos.X < (wallPos.X - 2))
            //if (Vector3.Distance(currentPos, wallPos) > 50f)
            {
                ShowShootUI(false);
                ShowDestroyEnemies(true);
                currentState = TutorialState.DestroyEnemies;
            }
        }

        private void HandleDestroyEnemiesState(Vector3 currentPos, Vector3 wallPos)
        {
            ;
        }

        // UI Functions
        private void ShowWASD(bool value)
        {
            SpriteRenderer.SetIsVisible(pressWASDID, value);
            Text.SetIsVisible(pressWASDID, value);
        }

        private void ShowFlyTunnel(bool value)
        {
            SpriteRenderer.SetIsVisible(pressFlyTunnelID, value);
            Text.SetIsVisible(pressFlyTunnelID, value);
        }

        private void ShowShootUI(bool value)
        {
            SpriteRenderer.SetIsVisible(pressShootID, value);
            Text.SetIsVisible(pressShootID, value);
            Text.SetIsVisible(destroyWallID, value);
        }

        private void ShowDestroyEnemies(bool value)
        {
            SpriteRenderer.SetIsVisible(destroyEnemiesID, value);
            Text.SetIsVisible(destroyEnemiesID, value);
        }
    }
}
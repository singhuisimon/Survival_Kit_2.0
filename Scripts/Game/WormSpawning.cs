using Engine;
using System;
using static Engine.Transform;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Logger;

namespace Game {

    public class WormSpawning : ScriptBehaviour{

        [SerializeField] private float interval = 20.0f;
        [SerializeField] private int maxWorms = 3;

        private string wall1Name = "Wall1";
        private string wall2Name = "Wall2";
        private string wall3Name = "Wall3";

        private uint wall1ID = 0;
        private uint wall2ID = 0;
        private uint wall3ID = 0;

        private float bigwall_width = 1600.0f;
        private float smallwall_width = 1000.0f;
        private float wall_height = 600.0f;

        private string Wormprefab = "Sources/Prefabs/WormHost.prefab";

        [SerializeField] private float timer = 0.0f;
        [SerializeField] private float spawnDistance = 1.5f;
        [SerializeField] private int currentWormCount = 0;

        private int wormsSpawnedThisCycle = 0;

        private bool gameRunning = true;

        // Game lose condition
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        // Game win condition

        public override void OnStart(){
            timer = interval;

            wall1ID = SceneFindEntityByName(wall1Name);
            wall2ID = SceneFindEntityByName(wall2Name);
            wall3ID = SceneFindEntityByName(wall3Name);

            if(wall1ID == 0){
                LogMessage("[Worm Spawning] Warning: Wall1 not found!");
            }
            if(wall2ID == 0){
                LogMessage("[Worm Spawning] Warning: Wall2 not found!");
            }
            if(wall3ID == 0){
                LogMessage("[Worm Spawning] Warning: Wall3 not found!");
            }

            gameRunning = true;

            Event.Subscribe(GAMEOVER, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameOver);
        }

        public override void OnUpdate(float deltaTime){
            // Don't spawn when game is paused
            if (GameState.IsPaused)
                return;

            if (gameRunning){
                
                timer -= deltaTime;

                if(timer <= 0.0f){

                    wormsSpawnedThisCycle = 0;

                    for(int i = 0; i < maxWorms; i++){
                        try {
                            SpawnWormOnRandomWall();
                        }
                        catch(Exception e) {
                            LogMessage("[WormSpawning] ERROR during spawn: " + e.ToString());
                        }
                    }
                    
                    timer = interval;
                    
                }
            }
        }

        public override void OnDestroy(){
            Event.Unsubscribe(GAMEOVER, OnGameOver);
            Event.Unsubscribe(GAMEWIN, OnGameOver);
        }

        private void SpawnWormOnRandomWall(){
            int wallIndex = RNG.RandInt(0, 2);
            uint selectedWall = 0;
            float selectedWidth = 0.0f;

            switch(wallIndex){
                case 0:
                    selectedWall = wall1ID;
                    selectedWidth = bigwall_width;
                    break;
                case 1:
                    selectedWall = wall2ID;
                    selectedWidth = smallwall_width;
                    break;
                case 2:
                    selectedWall = wall3ID;
                    selectedWidth = smallwall_width;
                    break;
            }

            if(selectedWall == 0){
                LogMessage("WormSpawning: Selected Wall is invalid");
                return;
            }

            Vector3 spawnPosition = GetRandomPositionOnWall(selectedWall, selectedWidth, wall_height);
            Quat spawnRot = GetRotation(selectedWall);
            Vector3 spawnscale = new Vector3(0.01f, 0.01f, 0.01f);

            uint WormID = PrefabInstantiateWithTransform(Wormprefab, ref spawnPosition, ref spawnRot, ref spawnscale, false);
            if(WormID == 0){
                LogMessage("[WormSpawning] Worm fail to instantiate");
                return; 
            }

            currentWormCount++;
            LogMessage("Spawning Worm Current Count is: " + currentWormCount.ToString() +
            " on wall " + (wallIndex + 1).ToString() + " at " + spawnPosition.ToString());       
        }

        private Vector3 GetRandomPositionOnWall(uint wallEntityID, float wallWidth, float wallHeight){
            // Get wall transform
            Quat wallRotation = GetRotation(wallEntityID);
            Vector3 wallPosition = GetPosition(wallEntityID);
            
            float randomX = RNG.RandFloat(-wallWidth * 0.5f, wallWidth * 0.5f);
            float randomY = RNG.RandFloat(-wallHeight * 0.5f, wallHeight * 0.5f);
    
            Vector3 localOffset = new Vector3(randomX, randomY, spawnDistance);
            Vector3 rotatedOffset = wallRotation.RotateVector(localOffset);
            
            // Add to wall's world position
            Vector3 worldPosition = wallPosition + rotatedOffset;
            
            return worldPosition;
        }

        private void OnGameOver(string eventName, string payload){
            gameRunning = false;
        }
    }

}
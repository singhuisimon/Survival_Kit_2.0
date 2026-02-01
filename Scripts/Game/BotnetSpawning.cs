using Engine;
using System;
using static Engine.Transform;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Logger;
using static Engine.Event;

namespace Game {

    public class BotnetSpawning : ScriptBehaviour{

        [SerializeField] private float interval = 2.0f;
        [SerializeField] private int maxBotnets = 20;

        private string wall1Name = "Wall1";
        private string wall2Name = "Wall2";
        private string wall3Name = "Wall3";

        private uint wall1ID = 0;
        private uint wall2ID = 0;
        private uint wall3ID = 0;

        private float bigwall_width = 1600.0f;
        private float smallwall_width = 1000.0f;
        private float wall_height = 600.0f;

        private string botnetprefab = "Sources/Prefabs/Enemy_Botnet.prefab";

        private string GAMEOVEREVENT = "GameOver";

        [SerializeField] private float timer = 0.0f;
        [SerializeField] private float spawnDistance = 1.5f;
        [SerializeField] private int currentBotnetCount = 0;

        private bool canSpawn = true;

        //private Random random = new Random();

        public override void OnStart(){
            timer = interval;

            wall1ID = SceneFindEntityByName(wall1Name);
            wall2ID = SceneFindEntityByName(wall2Name);
            wall3ID = SceneFindEntityByName(wall3Name);

            if(wall1ID == 0){
                LogMessage("[Botnet Spawning] Warning: Wall1 not found!");
            }
            if(wall2ID == 0){
                LogMessage("[Botnet Spawning] Warning: Wall2 not found!");
            }
            if(wall3ID == 0){
                LogMessage("[Botnet Spawning] Warning: Wall3 not found!");
            }

            canSpawn = true;
            Subscribe(GAMEOVEREVENT, OnGameOver);
        }

        public override void OnUpdate(float deltaTime){
            // if(currentBotnetCount >= maxBotnets){
            //     return;
            // }

            if(!canSpawn){
                LogMessage("[BOTNETSTOPSPAWNING]");
                return;
            }

            timer -= deltaTime;

            if(timer <= 0.0f){
                try {
                    SpawnBotnetOnRandomWall();
                }
                catch(Exception e) {
                    LogMessage("[BotnetSpawning] ERROR during spawn: " + e.ToString());
                }
                finally {
                    // ALWAYS executes, even if exception thrown
                    timer = interval;
                }
            }
        }

        public override void OnDestroy(){
            Unsubscribe(GAMEOVEREVENT, OnGameOver);
        }

        private void OnGameOver(string eventName, string payload){
            LogMessage("[BotnetSpawning] GameOver detected disallowing spawning");
            canSpawn = false;
        }

        private void SpawnBotnetOnRandomWall(){
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
                LogMessage("BotnetSpawning: Selected Wall is invalid");
                return;
            }

            Vector3 spawnPosition = GetRandomPositionOnWall(selectedWall, selectedWidth, wall_height);
            Quat spawnRot = GetRotation(selectedWall);
            Vector3 spawnscale = new Vector3(0.01f, 0.01f, 0.01f);

            uint botnetID = PrefabInstantiateWithTransform(botnetprefab, ref spawnPosition, ref spawnRot, ref spawnscale, false);
            if(botnetID == 0){
                LogMessage("[BotnetSpawning] Botnet fail to instantiate");
                return; //comment this for debugging temp
            }

            currentBotnetCount++;
            LogMessage("Spawning Botnet Current Count is: " + currentBotnetCount.ToString() +
            " on wall " + (wallIndex + 1).ToString() + " at " + spawnPosition.ToString());
            
            
        }

        private Vector3 GetRandomPositionOnWall(uint wallEntityID, float wallWidth, float wallHeight){
            // Get wall transform
            Quat wallRotation = GetRotation(wallEntityID);
            Vector3 wallPosition = GetPosition(wallEntityID);
            
            // Generate random local coordinates on the wall surface
            // Range from -wallWidth/2 to +wallWidth/2
            float randomX = RNG.RandFloat(-wallWidth * 0.5f, wallWidth * 0.5f);
            float randomY = RNG.RandFloat(-wallHeight * 0.5f, wallHeight * 0.5f);
            
            // Create local position (assuming wall faces along local Z-axis)
            // Spawn slightly in front of wall surface
            Vector3 localOffset = new Vector3(randomX, randomY, spawnDistance);
            
            // Rotate the local offset by wall's rotation to get world-space offset
            Vector3 rotatedOffset = wallRotation.RotateVector(localOffset);
            
            // Add to wall's world position
            Vector3 worldPosition = wallPosition + rotatedOffset;
            
            return worldPosition;
        }
    }

}
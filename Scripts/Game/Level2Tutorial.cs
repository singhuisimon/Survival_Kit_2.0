using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;
using static Engine.Prefab;
using static Engine.MeshRenderer;
using System.Numerics;

namespace Game
{
    public class Level2Tutorial : ScriptBehaviour
    {
        // ================== Spawn Setting ============================
        [SerializeField] private float spawntimer = 0.0f;
        [SerializeField] private int currentTotalSpawnCount = 0;
        [SerializeField] private float spawnInterval = 10.0f;
        [SerializeField] private float decreaseTimer = 0.0f; // every 10s decrease
        [SerializeField] private float decreaseInterval = 10.0f; // every 10s decrease
        [SerializeField] private float gradualdecrease = 0.5f;
        [SerializeField] private int currentBotnetSpawned = 0;
        [SerializeField] private int currentWormSpawned = 0;
        [SerializeField] private int currentLoveletterSpawned = 0;
        private float minInterval = 4.0f;
        private float wormbotSpawnDist = 1.5f;
        private float loveletterSpawnDist = 200.0f;
        private int enemiesSpawnPerWave = 5;
        
        // ================== Enemy Spawn Prefab Path ============================
        private const string loveletterPrefabPath = "Sources/Prefabs/loveletterv4.prefab";
        private const string botnetPrefabPath = "Sources/Prefabs/Enemy_Botnet.prefab";
        private const string wormHostPrefabPath = "Sources/Prefabs/WormHost.prefab";
        [SerializeField] private string enemyPrefabPath;

        private uint tutorialbotnetID = 0;
        private uint tutorialwormID = 0;
        private uint tutorialloveletterID = 0;

        // ================ Audio ==========================
        private const string warpingInPrefab = "Sources/Prefabs/Loveletter_warping.prefab";

        // ======================== EVENT ================================
        private const string EVENT_GAMEOVER = "GameOver";
        private const string EVENT_GAMEWIN = "GameWin";
        private const string EVENT_WALLENABLED = "WallEnabled";
        private const string EVENT_TUTORIALOVER = "TUTORIALOVER";

        // ======================  Wall Setting ==============================
        private float smallwall_width = 1000.0f;
        private float wall_height = 600.0f;

        // ====================== STATE ======================
        private bool canSpawn = true;
        private bool initialized = false;
        private bool active = false;

        // ========================= TUTORIAL SETTING ======================
        [SerializeField] private bool tutorialover = false;
        private int tutorialstate = 0;
        private float tutorialSpawnInterval = 0.0f;
        [SerializeField] private float tutorialCountdown = 0.0f; // Have a short delay at the start
        private const uint INVALID_ENTITY = 0xffffffffu;

        // ============== RNG Setting =================
        private static uint seed = 123;

        [SerializeField] private float botnetSpawnWeight = 55.0f;
        [SerializeField] private float wormHostSpawnWeight = 40.0f;
        [SerializeField] private float loveletterSpawnWeight = 5.0f;

        public override void OnStart(){
            initialize();

            Subscribe(EVENT_GAMEOVER, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);
            Subscribe(EVENT_WALLENABLED, OnWallEnabled);
        }

        public override void OnUpdate(float deltaTime){
            
            if(!initialized){
                initialize();
                return;
            }

            // Don't spawn when game is paused
            if(GameState.IsPaused){
                return;
            }
            
            // Don't spawn if the wall is not visible / active
            if(!active){
                return;
            }

            if(!canSpawn){
                LogMessage("[WallSpawner] Spawn is disabled for wall entity: " + EntityID.ToString());
                return;
            }

            if(!tutorialover){

                // Complete tutorial if loveletter is destroyed
                if (tutorialstate > 2 && 
                   SceneFindEntityByName("botnet") == INVALID_ENTITY &&
                   SceneFindEntityByName("WormHost") == INVALID_ENTITY &&
                   SceneFindEntityByName("loveletter") == INVALID_ENTITY) {
                    tutorialover = true;
                    Publish(EVENT_TUTORIALOVER, "");
                }

                // Time delay between enemy spawns
                if (SceneFindEntityByName("botnet") == INVALID_ENTITY &&
                   SceneFindEntityByName("WormHost") == INVALID_ENTITY &&
                   SceneFindEntityByName("loveletter") == INVALID_ENTITY) {
                    tutorialCountdown -= deltaTime;
                }

                // Spawn enemy
                if(tutorialCountdown <= 0.0f){
                    for (int i = 0; i < 3; i++)
                    {
                        SpawnTutorialOnWall();
                    }
                } 

                return; // return once fin first tutorial
            }

            spawntimer -= deltaTime;
            decreaseTimer -= deltaTime;

            //checks if it can be decreased as well as hifher than the minInterval
            if(decreaseTimer <= 0.0f && spawnInterval > minInterval){
                DecreaseSpawnInterval();
            }

            if(spawntimer <= 0.0f){
                try{
                    for(int i = 0; i < enemiesSpawnPerWave; i++){
                        SpawnRandomEnemyOnWall();
                    }
                }
                catch(Exception e){
                    LogMessage("[WallSpawner] ERRROR during spawn: " + e.ToString());
                }
                finally{
                    //Always Execute even if exeption thrown
                    spawntimer = spawnInterval;
                }
            }
        }

        public override void OnDestroy(){
            Unsubscribe(EVENT_GAMEOVER, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
            Unsubscribe(EVENT_WALLENABLED, OnWallEnabled);

            canSpawn = false;
            initialized = false;
        }

        private void OnGameEnd(string eventName, string payload){
            LogMessage("[WallSpawner] GameEnd detected disallowing spawning");
            canSpawn = false;
        }

        private void OnWallEnabled(string eventName, string payload){
            LogMessage("[WallSpawner] Wall Enabled detected checking if wall is activated");
            active = GetVisible((uint)EntityID); //only enable spawning if it is active
        }

        private void SpawnRandomEnemyOnWall(){
            //int enemyDex = RNG.RandInt(0, 2);
            //int enemyDex = GetRandom012();
            int enemyDex = GetWeightedRandomEnemy();
            //string enemyPrefabPath = "";
            float selectedWidth = 0.0f;
            float spawnDistance = 0.0f;
            
            switch(enemyDex){
                case 0:
                    //botnet
                    enemyPrefabPath = botnetPrefabPath;
                    selectedWidth = smallwall_width;
                    spawnDistance = wormbotSpawnDist;
                    break;
                case 1:
                    //worm
                    enemyPrefabPath = wormHostPrefabPath;
                    spawnDistance = wormbotSpawnDist;
                    selectedWidth = smallwall_width;
                    break;
                case 2:
                    //loveletter
                    enemyPrefabPath = loveletterPrefabPath;
                    selectedWidth = smallwall_width;
                    spawnDistance = loveletterSpawnDist;
                    break;
            }
 
            Vector3 spawnPos = GetRandomPositionOnWall(selectedWidth, wall_height, spawnDistance);
            Quat spawnRot = GetSpawnRotation(spawnPos);

            uint enemyID = 0;
            enemyID = PrefabInstantiate(enemyPrefabPath);
            if(enemyID == 0){
                LogMessage("[WallSpawner] Fail to instantiate enemy for: " + enemyPrefabPath);
                return;
            }

            //Set Position and Rotation
            SetPosition(enemyID, ref spawnPos);
            SetRotation(enemyID, ref spawnRot);

            //Increment counters
            currentTotalSpawnCount++;

            //Instantiate audio if needed
            if(enemyDex == 0){
                currentBotnetSpawned++;
            }
            else if(enemyDex == 1){
                currentWormSpawned++;
            }
            else if(enemyDex == 2){
                currentLoveletterSpawned++;
                //spawn warping in audio
                Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
                uint warpingInID = PrefabInstantiateWithTransform(warpingInPrefab, ref spawnPos, ref spawnRot, ref scale, false);
                if(warpingInID == 0){
                    LogMessage("[LoveletterSpawn] loveletter warping in entity fail to instantiate");
                    return;
                }
            }
        }

        private Vector3 GetRandomPositionOnWall(float wallWidth, float wallHeight, float spawnDistance){
            //Get Wall Transform
            Quat wallRotation = GetRotation((uint)EntityID);
            Vector3 wallPosition = GetPosition((uint)EntityID);

            //Generate random local coordinates on the wall surface
            float randomX = RNG.RandFloat(-wallWidth * 0.5f, wallWidth * 0.5f);
            float randomY = RNG.RandFloat(-wallHeight * 0.5f, wallHeight * 0.5f);

            Vector3 localOffset = new Vector3(randomX, randomY, spawnDistance);

            Vector3 rotatedOffset = wallRotation.RotateVector(localOffset);

            //Add to wall's world position
            Vector3 worldPos = wallPosition + rotatedOffset;

            return worldPos;
        }

        private void initialize(){
            spawntimer = spawnInterval;
            canSpawn = true;
            initialized = true;
            tutorialover = false;
            tutorialstate = 0;
            decreaseTimer = decreaseInterval;
            active = GetVisible((uint)EntityID); //only enable spawning if it is active
        }

        private int GetWeightedRandomEnemy(){
            float totalWeight = botnetSpawnWeight + wormHostSpawnWeight + loveletterSpawnWeight;
            float roll = RandFloat() * totalWeight;

            if(roll < botnetSpawnWeight) return 0;                          // botnet
            if(roll < botnetSpawnWeight + wormHostSpawnWeight) return 1;        // worm
            return 2;                                                       // loveletter
        }

        private float RandFloat(){
            seed = (1103414245 * seed + 12345) & 0x7fffffff;
            return (float)seed / (float)0x7fffffff;
        }

        private int GetRandom012(){
            seed = (1103515245 * seed + 12345) & 0x7fffffff;

            return (int)(seed % 3);
        }

        private void DecreaseSpawnInterval(){
            spawnInterval -= gradualdecrease;
            decreaseTimer = decreaseInterval;
        }

        private void SpawnTutorialOnWall(){
            
            if(tutorialstate > 2)
            {
                return;
            }

            int enemyDex = tutorialstate;
            //string enemyPrefabPath = "";
            float selectedWidth = 0.0f;
            float spawnDistance = 0.0f;
            
            switch(enemyDex){
                case 0:
                    //botnet
                    enemyPrefabPath = botnetPrefabPath;
                    selectedWidth = smallwall_width;
                    spawnDistance = wormbotSpawnDist;
                    break;
                case 1:
                    //worm
                    enemyPrefabPath = wormHostPrefabPath;
                    spawnDistance = wormbotSpawnDist;
                    selectedWidth = smallwall_width;
                    break;
                case 2:
                    //loveletter
                    enemyPrefabPath = loveletterPrefabPath;
                    selectedWidth = smallwall_width;
                    spawnDistance = loveletterSpawnDist;
                    break;
            }
 
            Vector3 spawnPos = GetRandomPositionOnWall(selectedWidth, wall_height, spawnDistance);
            Quat spawnRot = GetSpawnRotation(spawnPos);

            uint enemyID = 0;
            enemyID = PrefabInstantiate(enemyPrefabPath);
            if(enemyID == 0){
                LogMessage("[WallSpawner] Fail to instantiate enemy for: " + enemyPrefabPath);
                return;
            }

            //Set Position and Rotation
            SetPosition(enemyID, ref spawnPos);
            SetRotation(enemyID, ref spawnRot);

            //Set tutorial state
            //tutorialCountdown = tutorialSpawnInterval;
            tutorialstate++;

            //Instantiate audio if needed
            if(enemyDex == 0){
                currentBotnetSpawned++;
                tutorialbotnetID = enemyID;
            }
            else if(enemyDex == 1){
                currentWormSpawned++;
                tutorialwormID = enemyID;
            }
            else if(enemyDex == 2){
                currentLoveletterSpawned++;
                tutorialloveletterID = enemyID;
                Publish("BotnetTutorialSpawn", tutorialbotnetID.ToString());
                Publish("WormTutorialSpawn", tutorialwormID.ToString());
                Publish("LoveletterTutorialSpawn", tutorialloveletterID.ToString());
                //spawn warping in audio
                Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
                uint warpingInID = PrefabInstantiateWithTransform(warpingInPrefab, ref spawnPos, ref spawnRot, ref scale, false);
                if (warpingInID == 0)
                {
                    LogMessage("[LoveletterSpawn] loveletter warping in entity fail to instantiate");
                    return;
                }
            }

        }

        private Quat GetSpawnRotation(Vector3 spawnPos)
        {
            uint coreID = SceneFindEntityByName("SEMICONDUCTOR");
            Quat Rot = GetRotation((uint)EntityID);
            if(coreID != 0)
            {
                Vector3 corePos = GetPosition(coreID);
                
                //Direction vector from spawn point toward core
                Vector3 dir = corePos - spawnPos;
                float len = dir.Magnitude;

                if(len > 0.001f)
                {
                    dir = dir / len; // Normalize
                    Rot = SimpleMath.LookRotation(-dir, Vector3.Up);
                }
            }

            return Rot;
        }

    }
}
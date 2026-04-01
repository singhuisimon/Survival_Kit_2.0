using Engine;
using System;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;
using static Engine.Prefab;
using static Engine.MeshRenderer;

namespace Game
{
    public class WallSpawner : ScriptBehaviour
    {
        // ================== Spawn Setting ============================
        [SerializeField] private float spawntimer = 0.0f;
        [SerializeField] private int currentTotalSpawnCount = 0;
        [SerializeField] private float spawnInterval = 10.0f;
        [SerializeField] private float decreaseTimer = 0.0f;
        [SerializeField] private float decreaseInterval = 10.0f;
        [SerializeField] private float gradualdecrease = 0.5f;
        [SerializeField] private int currentBotnetSpawned = 0;
        [SerializeField] private int currentWormSpawned = 0;
        [SerializeField] private int currentLoveletterSpawned = 0;
        private float minInterval = 4.0f;
        private float wormbotSpawnDist = 10.0f;//1.5f;
        private float loveletterSpawnDist = 350.0f;///200.0f;
        private int enemiesSpawnPerWave = 5;

        // ================== VFX Delay Setting ============================

        [SerializeField] private float botnetVFXDelay = 1.0f;
        [SerializeField] private float wormVFXDelay = 1.0f;
        [SerializeField] private float loveletterVFXDelay = 1.2f;
        private float vfxToEnemyDelay = 0.0f;

        // Struct to hold pending spawn data
        private struct PendingSpawn
        {
            public float timer;
            public string prefabPath;
            public Vector3 position;
            public Quat rotation;
            public int enemyType; // 0=botnet, 1=worm, 2=loveletter
        }

        private List<PendingSpawn> pendingSpawns = new List<PendingSpawn>();

        // ================== Enemy Spawn Prefab Path ============================
        private const string loveletterPrefabPath = "Sources/Prefabs/loveletterv4.prefab";
        private const string botnetPrefabPath = "Sources/Prefabs/Enemy_Botnet.prefab";
        private const string wormHostPrefabPath = "Sources/Prefabs/WormHost.prefab";

        private const string botnetVFXPath = "Sources/Prefabs/RootSpawnBotNet.prefab";
        private const string wormVFXPath = "Sources/Prefabs/RootWormVFX.prefab";
        private const string loveletterVFXPath = "Sources/Prefabs/RootLoveLetterSpawn.prefab";

        [SerializeField] private string enemyPrefabPath;
        [SerializeField] private string vfxPrefabPath;

        // =================== Audio ======================
        private const string warpingInPrefab = "Sources/Prefabs/Loveletter_warping.prefab";

        // ======================== EVENT ================================
        private const string EVENT_GAMEOVER = "GameOver";
        private const string EVENT_GAMEWIN = "GameWin";
        private const string EVENT_WALLENABLED = "WallEnabled";
        private const string EVENT_TUTORIALOVER = "TUTORIALOVER";
        private const string EVENT_DEBUG_TIMER = "DebugSetTimer";


        // ======================  Wall Setting ==============================
        private float smallwall_width = 1000.0f;
        private float wall_height = 600.0f;

        // ====================== STATE ======================
        private bool canSpawn = true;
        private bool initialized = false;
        private bool active = false;
        [SerializeField] private bool tutorialOver = false;

        // ============== RNG Setting =================
        private static uint seed = 123;

        [SerializeField] private float botnetSpawnWeight = 55.0f;
        [SerializeField] private float wormHostSpawnWeight = 40.0f;
        [SerializeField] private float loveletterSpawnWeight = 5.0f;

        public override void OnStart()
        {
            initialize();

            Subscribe(EVENT_GAMEOVER, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);
            Subscribe(EVENT_WALLENABLED, OnWallEnabled);
            Subscribe(EVENT_TUTORIALOVER, OnTutorialEnd);
            Subscribe(EVENT_DEBUG_TIMER, OnDebugTimer);
        }

        public override void OnUpdate(float deltaTime)
        {

            if (!initialized)
            {
                initialize();
                return;
            }

            // Don't spawn when game is paused
            if (GameState.IsPaused)
            {
                return;
            }

            // Process pending spawns (VFX delay) - always process even if spawning is disabled
            ProcessPendingSpawns(deltaTime);

            // Don't spawn if the wall is not visible / active
            if (!active)
            {
                return;
            }

            if (!tutorialOver)
            {
                return;
            }

            if (!canSpawn)
            {
                LogMessage("[WallSpawner] Spawn is disabled for wall entity: " + EntityID.ToString());
                return;
            }

            spawntimer -= deltaTime;
            decreaseTimer -= deltaTime;

            //checks if it can be decreased as well as higher than the minInterval
            if (decreaseTimer <= 0.0f && spawnInterval > minInterval)
            {
                DecreaseSpawnInterval();
            }

            if (spawntimer <= 0.0f)
            {
                try
                {
                    for (int i = 0; i < enemiesSpawnPerWave; i++)
                    {
                        SpawnRandomEnemyOnWall();
                    }
                }
                catch (Exception e)
                {
                    LogMessage("[WallSpawner] ERRROR during spawn: " + e.ToString());
                }
                finally
                {
                    //Always Execute even if exception thrown
                    spawntimer = spawnInterval;
                }
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_GAMEOVER, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
            Unsubscribe(EVENT_WALLENABLED, OnWallEnabled);
            Unsubscribe(EVENT_TUTORIALOVER, OnTutorialEnd);
            Unsubscribe(EVENT_DEBUG_TIMER, OnDebugTimer);

            canSpawn = false;
            initialized = false;
            pendingSpawns.Clear();
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("[WallSpawner] GameEnd detected disallowing spawning");
            canSpawn = false;
            pendingSpawns.Clear(); // Clear any pending spawns on game end
        }

        private void OnWallEnabled(string eventName, string payload)
        {
            LogMessage("[WallSpawner] Wall Enabled detected checking if wall is activated");
            active = GetVisible((uint)EntityID); //only enable spawning if it is active
        }

        private void OnTutorialEnd(string eventName, string payload)
        {
            LogMessage("[WallSpawner] Wall detect that tutorial ended proceeding");
            tutorialOver = true;
        }

        private void OnDebugTimer(string eventName, string payload)
        {
            spawntimer = 0;
            spawnInterval = minInterval;
        }

        private void ProcessPendingSpawns(float deltaTime)
        {
            // Iterate backwards so we can remove items safely
            for (int i = pendingSpawns.Count - 1; i >= 0; i--)
            {
                PendingSpawn spawn = pendingSpawns[i];
                spawn.timer -= deltaTime;

                if (spawn.timer <= 0.0f)
                {
                    // Time to spawn the actual enemy
                    SpawnEnemy(spawn.prefabPath, spawn.position, spawn.rotation, spawn.enemyType);
                    pendingSpawns.RemoveAt(i);
                }
                else
                {
                    // Update the timer in the list
                    pendingSpawns[i] = spawn;
                }
            }
        }

        private void SpawnEnemy(string prefabPath, Vector3 spawnPos, Quat spawnRot, int enemyType)
        {
            uint enemyID = PrefabInstantiate(prefabPath);

            if (enemyID == 0)
            {
                LogMessage("[WallSpawner] Fail to instantiate enemy for: " + prefabPath);
                return;
            }

            SetPosition(enemyID, ref spawnPos);
            SetRotation(enemyID, ref spawnRot);

            currentTotalSpawnCount++;

            if (enemyType == 0)
            {
                currentBotnetSpawned++;
            }
            else if (enemyType == 1)
            {
                currentWormSpawned++;
            }
            else if (enemyType == 2)
            {
                currentLoveletterSpawned++;
                Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
                uint warpingInID = PrefabInstantiateWithTransform(warpingInPrefab, ref spawnPos, ref spawnRot, ref scale, false);
                if (warpingInID == 0)
                {
                    LogMessage("[LoveletterSpawn] loveletter warping in entity fail to instantiate");
                }
            }
        }

        private void SpawnRandomEnemyOnWall()
        {
            int enemyDex = GetWeightedRandomEnemy();
            float selectedWidth = smallwall_width;
            float spawnDistance = 0.0f;

            switch (enemyDex)
            {
                case 0: // botnet
                    enemyPrefabPath = botnetPrefabPath;
                    vfxPrefabPath = botnetVFXPath;
                    spawnDistance = wormbotSpawnDist;
                    vfxToEnemyDelay = botnetVFXDelay;
                    break;
                case 1: // worm
                    enemyPrefabPath = wormHostPrefabPath;
                    vfxPrefabPath = wormVFXPath;
                    spawnDistance = wormbotSpawnDist;
                    vfxToEnemyDelay = wormVFXDelay;
                    break;
                case 2: // loveletter
                    enemyPrefabPath = loveletterPrefabPath;
                    vfxPrefabPath = loveletterVFXPath;
                    spawnDistance = loveletterSpawnDist;
                    vfxToEnemyDelay = loveletterVFXDelay;
                    break;
            }

            Vector3 spawnPos = GetRandomPositionOnWall(selectedWidth, wall_height, spawnDistance);
            Quat spawnRot = GetSpawnRotation(spawnPos);

            // Spawn VFX immediately
            uint VFXID = PrefabInstantiate(vfxPrefabPath);

            if (VFXID == 0)
            {
                LogMessage("[WallSpawner] Fail to instantiate VFX for: " + vfxPrefabPath);
                return;
            }

            SetPosition(VFXID, ref spawnPos);
            SetRotation(VFXID, ref spawnRot);

            // Queue the enemy spawn with delay
            PendingSpawn pending = new PendingSpawn
            {
                timer = vfxToEnemyDelay,
                prefabPath = enemyPrefabPath,
                position = spawnPos,
                rotation = spawnRot,
                enemyType = enemyDex
            };
            pendingSpawns.Add(pending);
        }

        private Vector3 GetRandomPositionOnWall(float wallWidth, float wallHeight, float spawnDistance)
        {
            Quat wallRotation = GetRotation((uint)EntityID);
            Vector3 wallPosition = GetPosition((uint)EntityID);

            float randomX = RNG.RandFloat(-wallWidth * 0.5f, wallWidth * 0.5f);
            float randomY = RNG.RandFloat(-wallHeight * 0.5f, wallHeight * 0.5f);

            Vector3 localOffset = new Vector3(randomX, randomY, spawnDistance);
            Vector3 rotatedOffset = wallRotation.RotateVector(localOffset);
            Vector3 worldPos = wallPosition + rotatedOffset;

            return worldPos;
        }

        private void initialize()
        {
            canSpawn = true;
            initialized = true;
            decreaseTimer = decreaseInterval;
            active = GetVisible((uint)EntityID);
            pendingSpawns.Clear();
        }

        private int GetWeightedRandomEnemy()
        {
            float totalWeight = botnetSpawnWeight + wormHostSpawnWeight + loveletterSpawnWeight;
            float roll = RandFloat() * totalWeight;

            if (roll < botnetSpawnWeight) return 0;
            if (roll < botnetSpawnWeight + wormHostSpawnWeight) return 1;
            return 2;
        }

        private float RandFloat()
        {
            seed = (1103414245 * seed + 12345) & 0x7fffffff;
            return (float)seed / (float)0x7fffffff;
        }

        private int GetRandom012()
        {
            seed = (1103515245 * seed + 12345) & 0x7fffffff;
            return (int)(seed % 3);
        }

        private void DecreaseSpawnInterval()
        {
            spawnInterval -= gradualdecrease;
            decreaseTimer = decreaseInterval;
        }

        private Quat GetSpawnRotation(Vector3 spawnPos)
        {
            uint coreID = SceneFindEntityByName("SEMICONDUCTOR");
            Quat Rot = GetRotation((uint)EntityID);
            if (coreID != 0)
            {
                Vector3 corePos = GetPosition(coreID);
                Vector3 dir = corePos - spawnPos;
                float len = dir.Magnitude;

                if (len > 0.001f)
                {
                    dir = dir / len;
                    Rot = SimpleMath.LookRotation(-dir, Vector3.Up);
                }
            }
            return Rot;
        }
    }
}
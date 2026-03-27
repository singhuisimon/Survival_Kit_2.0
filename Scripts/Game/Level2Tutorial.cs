using Engine;
using System;
using System.Collections.Generic;
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
        [SerializeField] private float decreaseTimer = 0.0f;
        [SerializeField] private float decreaseInterval = 10.0f;
        [SerializeField] private float gradualdecrease = 0.5f;
        [SerializeField] private int currentBotnetSpawned = 0;
        [SerializeField] private int currentWormSpawned = 0;
        [SerializeField] private int currentLoveletterSpawned = 0;
        private float minInterval = 4.0f;
        private float wormbotSpawnDist = 1.5f;
        private float loveletterSpawnDist = 200.0f;
        private int enemiesSpawnPerWave = 5;

        // ================== VFX Delay Setting ============================

        [SerializeField] private float botnetVFXDelay = 1.0f;
        [SerializeField] private float wormVFXDelay = 1.0f;
        [SerializeField] private float loveletterVFXDelay = 1.2f;
        private float vfxToEnemyDelay = 0.0f;

        // Use a class instead of struct to avoid copy issues
        private class PendingSpawn
        {
            public float timer;
            public string prefabPath;
            public Vector3 position;
            public Quat rotation;
            public int enemyType;
            public bool isTutorial; // Track if this is a tutorial spawn
        }

        private List<PendingSpawn> pendingSpawns;

        // ================== Enemy Spawn Prefab Path ============================
        private const string loveletterPrefabPath = "Sources/Prefabs/loveletterv4.prefab";
        private const string botnetPrefabPath = "Sources/Prefabs/Enemy_Botnet.prefab";
        private const string wormHostPrefabPath = "Sources/Prefabs/WormHost.prefab";

        private const string botnetVFXPath = "Sources/Prefabs/RootSpawnBotNet.prefab";
        private const string wormVFXPath = "Sources/Prefabs/RootWormVFX.prefab";
        private const string loveletterVFXPath = "Sources/Prefabs/RootLoveLetterSpawn.prefab";

        [SerializeField] private string enemyPrefabPath;
        [SerializeField] private string vfxPrefabPath;

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
        private float tutorialSpawnInterval = 5.0f;
        [SerializeField] private float tutorialCountdown = 7.5f;
        private const uint INVALID_ENTITY = 0xffffffffu;

        // ============== RNG Setting =================
        private static uint seed = 123;

        [SerializeField] private float botnetSpawnWeight = 55.0f;
        [SerializeField] private float wormHostSpawnWeight = 40.0f;
        [SerializeField] private float loveletterSpawnWeight = 5.0f;

        public override void OnStart()
        {
            pendingSpawns = new List<PendingSpawn>();

            initialize();

            Subscribe(EVENT_GAMEOVER, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);
            Subscribe(EVENT_WALLENABLED, OnWallEnabled);
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

            // Ensure list exists
            if (pendingSpawns == null)
            {
                pendingSpawns = new List<PendingSpawn>();
            }

            // Process pending spawns FIRST (always, regardless of other states)
            if (pendingSpawns.Count > 0)
            {
                ProcessPendingSpawns(deltaTime);
            }

            // Don't spawn if the wall is not visible / active
            if (!active)
            {
                return;
            }

            if (!canSpawn)
            {
                LogMessage("[Level2Tutorial] Spawn is disabled for wall entity: " + EntityID.ToString());
                return;
            }

            // TUTORIAL MODE
            if (!tutorialover)
            {

                // Complete tutorial if loveletter is destroyed
                if (tutorialstate > 2 && SceneFindEntityByName("loveletter") == INVALID_ENTITY)
                {
                    tutorialover = true;
                    Publish(EVENT_TUTORIALOVER, "");
                }

                // Time delay between enemy spawns
                if (SceneFindEntityByName("botnet") == INVALID_ENTITY &&
                   SceneFindEntityByName("WormHost") == INVALID_ENTITY &&
                   SceneFindEntityByName("loveletter") == INVALID_ENTITY)
                {
                    tutorialCountdown -= deltaTime;
                }

                // Spawn enemy
                if (tutorialCountdown <= 0.0f)
                {
                    SpawnTutorialOnWall();
                }
                return;
            }

            // NORMAL MODE (after tutorial)
            spawntimer -= deltaTime;
            decreaseTimer -= deltaTime;

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
                    LogMessage("[Level2Tutorial] ERROR during spawn: " + e.ToString());
                }
                finally
                {
                    spawntimer = spawnInterval;
                }
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_GAMEOVER, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
            Unsubscribe(EVENT_WALLENABLED, OnWallEnabled);

            canSpawn = false;
            initialized = false;

            if (pendingSpawns != null)
            {
                pendingSpawns.Clear();
            }
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("[Level2Tutorial] GameEnd detected disallowing spawning");
            canSpawn = false;

            if (pendingSpawns != null)
            {
                pendingSpawns.Clear();
            }
        }

        private void OnWallEnabled(string eventName, string payload)
        {
            LogMessage("[Level2Tutorial] Wall Enabled detected checking if wall is activated");
            active = GetVisible((uint)EntityID);
        }

        private void ProcessPendingSpawns(float deltaTime)
        {
            for (int i = pendingSpawns.Count - 1; i >= 0; i--)
            {
                PendingSpawn spawn = pendingSpawns[i];
                spawn.timer -= deltaTime;

                if (spawn.timer <= 0.0f)
                {
                    if (spawn.isTutorial)
                    {
                        SpawnTutorialEnemy(spawn.prefabPath, spawn.position, spawn.rotation, spawn.enemyType);
                    }
                    else
                    {
                        SpawnEnemy(spawn.prefabPath, spawn.position, spawn.rotation, spawn.enemyType);
                    }
                    pendingSpawns.RemoveAt(i);
                }
            }
        }

        private void SpawnEnemy(string prefabPath, Vector3 spawnPos, Quat spawnRot, int enemyType)
        {
            uint enemyID = PrefabInstantiate(prefabPath);

            if (enemyID == 0)
            {
                LogMessage("[Level2Tutorial] Fail to instantiate enemy for: " + prefabPath);
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

        private void SpawnTutorialEnemy(string prefabPath, Vector3 spawnPos, Quat spawnRot, int enemyType)
        {
            uint enemyID = PrefabInstantiate(prefabPath);

            if (enemyID == 0)
            {
                LogMessage("[Level2Tutorial] Fail to instantiate tutorial enemy for: " + prefabPath);
                return;
            }

            SetPosition(enemyID, ref spawnPos);
            SetRotation(enemyID, ref spawnRot);

            // Publish tutorial events
            if (enemyType == 0)
            {
                currentBotnetSpawned++;
                Publish("BotnetTutorialSpawn", enemyID.ToString());
            }
            else if (enemyType == 1)
            {
                currentWormSpawned++;
                Publish("WormTutorialSpawn", enemyID.ToString());
            }
            else if (enemyType == 2)
            {
                currentLoveletterSpawned++;
                Publish("LoveletterTutorialSpawn", enemyID.ToString());
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
                LogMessage("[Level2Tutorial] Fail to instantiate VFX for: " + vfxPrefabPath);
                return;
            }

            SetPosition(VFXID, ref spawnPos);
            SetRotation(VFXID, ref spawnRot);

            // Queue enemy spawn with delay
            PendingSpawn pending = new PendingSpawn();
            pending.timer = vfxToEnemyDelay;
            pending.prefabPath = enemyPrefabPath;
            pending.position = spawnPos;
            pending.rotation = spawnRot;
            pending.enemyType = enemyDex;
            pending.isTutorial = false;

            pendingSpawns.Add(pending);
        }

        private void SpawnTutorialOnWall()
        {
            int enemyDex = tutorialstate;
            float selectedWidth = smallwall_width;
            float spawnDistance = 0.0f;

            switch (enemyDex)
            {
                case 0:
                    enemyPrefabPath = botnetPrefabPath;
                    vfxPrefabPath = botnetVFXPath;
                    spawnDistance = wormbotSpawnDist;
                    break;
                case 1:
                    enemyPrefabPath = wormHostPrefabPath;
                    vfxPrefabPath = wormVFXPath;
                    spawnDistance = wormbotSpawnDist;
                    break;
                case 2:
                    enemyPrefabPath = loveletterPrefabPath;
                    vfxPrefabPath = loveletterVFXPath;
                    spawnDistance = loveletterSpawnDist;
                    break;
            }

            Vector3 spawnPos = GetRandomPositionOnWall(selectedWidth, wall_height, spawnDistance);
            Quat spawnRot = GetSpawnRotation(spawnPos);

            // Spawn VFX immediately
            uint VFXID = PrefabInstantiate(vfxPrefabPath);

            if (VFXID == 0)
            {
                LogMessage("[Level2Tutorial] Fail to instantiate VFX for: " + vfxPrefabPath);
                return;
            }

            SetPosition(VFXID, ref spawnPos);
            SetRotation(VFXID, ref spawnRot);

            // Update tutorial state immediately (so countdown resets)
            tutorialCountdown = tutorialSpawnInterval;
            tutorialstate++;

            // Queue enemy spawn with delay
            PendingSpawn pending = new PendingSpawn();
            pending.timer = vfxToEnemyDelay;
            pending.prefabPath = enemyPrefabPath;
            pending.position = spawnPos;
            pending.rotation = spawnRot;
            pending.enemyType = enemyDex;
            pending.isTutorial = true;

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
            spawntimer = spawnInterval;
            canSpawn = true;
            initialized = true;
            tutorialover = false;
            decreaseTimer = decreaseInterval;
            active = GetVisible((uint)EntityID);

            if (pendingSpawns == null)
            {
                pendingSpawns = new List<PendingSpawn>();
            }
            else
            {
                pendingSpawns.Clear();
            }
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
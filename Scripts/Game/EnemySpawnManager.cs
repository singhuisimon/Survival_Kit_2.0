using System;
using System.Collections.Generic;
using System.Collections;
using Engine;

namespace Game
{

    // Simple struct to hold spawn point transform data
    public struct SpawnPointData
    {
        public Vector3 position;
        public Vector3 rotation;   // Euler angles (as returned by Quat.ToEuler)

        public SpawnPointData(Vector3 pos, Vector3 rot)
        {
            position = pos;
            rotation = rot;
        }
    }

    public class EnemySpawnManager : ScriptBehaviour
    {
        //public static EnemySpawnManager instance;

        // Basic toggles
        [SerializeField] private bool isActive = false;
        [SerializeField] private bool spawningAllowed = false;

        // Wave tracking
        [SerializeField] private int CURRENT_WAVE = 0;
        [SerializeField] private int waveEnemiesLeftToSpawn = 0;

        // Enemy counts for current wave
        [SerializeField] private int E005_loveletter = 0;
        [SerializeField] private int E004_botnet = 0;
        [SerializeField] private int E001_worm_host = 0;
        [SerializeField] private int E003_trojan = 0;
        [SerializeField] private int E007_adware = 0;

        // Spawn timing
        [SerializeField] private float spawnRate = 2.0f;
        [SerializeField] private float spawnRateNext = 0.0f;
        [SerializeField] private bool isSpawning = false;

        // Wave timers
        [SerializeField] private float timeBetweenWave = 5.0f;

        //time per wave
        [SerializeField] private float timeWave = 180.0f;

        //shows the current time it is waiting for (180 for wave, 10 for start, 5 for between)
        [SerializeField] private float timeCurrent = 10.0f;
        [SerializeField] private float timeNext = 10.0f;

        //state
        [SerializeField] private bool BREAK = true;
        [SerializeField] private bool COMBAT = false;

        // Enemy counting
        //[SerializeField] private int enemiesLeft = 0;
        private int enemiesLeft = 0;
        // Prefab names 
        private string[] enemyPrefabNames = new string[]
        {
            "Enemy_Botnet",      // 0
            "Enemy_LoveLetter",  // 1  
            "Enemy_Worm",        // 2
            "Enemy_Trojan",      // 3
            "Enemy_Adware",      // 4
        };

        // Create spawn point of equal spacing based off wall scale
        private List<SpawnPointData> spawnPointsA = new List<SpawnPointData>();
        private List<SpawnPointData> spawnPointsB = new List<SpawnPointData>();
        private List<SpawnPointData> spawnPointsC = new List<SpawnPointData>();
        private List<SpawnPointData> spawnPointsD = new List<SpawnPointData>();
        private List<SpawnPointData> spawnPointsE = new List<SpawnPointData>();

        // Current active spawn points for this wave
        private List<SpawnPointData> activeSpawnPoints = new List<SpawnPointData>();

        // Loveletter routes for current wave
        private string[] loveletterRoutes;

        // Wall entity names (find these by name in scene)
        // Format: "Wall_A_Active", "Wall_A_Inactive", etc.
        private List<Entity> wallActiveEntities = new List<Entity>();
        private Entity[] wallAActiveEntities;
        private Entity[] wallBActiveEntities;
        private Entity[] wallCActiveEntities;
        private Entity[] wallDActiveEntities;
        private Entity[] wallEActiveEntities;
        private Entity[] wallInactiveEntities;

        private const uint INVALID_ENTITY = 0xffffffffu;

        // Light entities
        //private Entity lightPrep;
        //private Entity lightCombat;

        // Simple pseudo-random number generator
        private uint rngSeed;

        // Time tracking (since we don't have Time.time)
        [SerializeField]
        private float elapsedTime = 0f;

        private string alliesambience = "Flotilla_Gunship_Ambient.wav";
        private string coreambience = "Core_Ambient.wav";

        private string loveletterwarning = "Loveletter_Warp_Warning.wav";

        private uint spawnmanagerID;

        private bool playbgm = false;

        public override void OnStart()
        {
            // Initialize random seed
            rngSeed = (uint)DateTime.Now.Ticks;

            // init some values
            isActive = false;

            // Find all spawn points in the scene
            InitializeSpawnPoints();

            // Find wall entities
            InitializeWalls();

            // Setup walls for initial state
            WallSetup_DisableActiveWalls();
            WallSetup_InactiveWalls();

            InternalCalls.Entity_AddAudio((uint)EntityID);

            timeCurrent = 10.0f;
            timeNext = 10.0f;
            spawnRateNext = 0.0f;

            Log("EnemySpawnManager initialized - Wave " + CURRENT_WAVE);
        }

        public override void OnUpdate(float deltaTime)
        {
            //check for the trigger to start
            if (Input.IsKeyPressed(KeyCode.Enter) && !isActive)
            {
                isActive = true;
                stopmainsound();
            }

            if (!isActive)
            {
                return;
            }

            if (!playbgm)
            {
                PlayGameBGM();
                playbgm = true;
            }

            elapsedTime += deltaTime;
            float currentTime = elapsedTime;

            // Phase transitions
            if (currentTime > timeNext)
            {
                if (BREAK)
                {
                    if (CURRENT_WAVE == 0)
                    {
                        SetupWaveSpawning();
                    }
                    StartCombat();
                }
                else if (COMBAT)
                {
                    StartPrep();
                }
            }

            // Spawning logic
            if (spawningAllowed)
            {
                if (!isSpawning && waveEnemiesLeftToSpawn > 0)
                {
                    SpawnEnemyByWaves();
                }
                else
                {
                    isSpawning = false;
                    spawningAllowed = false;
                }
            }
        }

        private void InitializeSpawnPoints()
        {
            Log("=== Initializing Spawn Points ===");

            // Wall A spawn points - register each one
            RegisterSpawnPointsForWall("A", spawnPointsA, new string[] {
                "SpawnPointA_1",
                "SpawnPointA_2",
                "SpawnPointA_3",
                "SpawnPointA_4",
                "SpawnPointA_5",
                "SpawnPointA_6",
                "SpawnPointA_7",
                "SpawnPointA_8",
                "SpawnPointA_9",
                "SpawnPointA_10",
                "SpawnPointA_11",
                "SpawnPointA_12",
                "SpawnPointA_13",
                "SpawnPointA_14",
                "SpawnPointA_15"
            });

            // Wall B spawn points - register each one
            RegisterSpawnPointsForWall("B", spawnPointsB, new string[] {
                "SpawnPointB_1",
                "SpawnPointB_2",
                "SpawnPointB_3",
                "SpawnPointB_4",
                "SpawnPointB_5",
                "SpawnPointB_6",
                "SpawnPointB_7",
                "SpawnPointB_8",
                "SpawnPointB_9",
                "SpawnPointB_10",
                "SpawnPointB_11",
                "SpawnPointB_12",
                "SpawnPointB_13",
                "SpawnPointB_14",
                "SpawnPointB_15"
            });

            // Wall C spawn points - register each one
            RegisterSpawnPointsForWall("C", spawnPointsC, new string[] {
                "SpawnPointC_1",
                "SpawnPointC_2",
                "SpawnPointC_3",
                "SpawnPointC_4",
                "SpawnPointC_5",
                "SpawnPointC_6"
            });

            // Wall D spawn points - register each one
            RegisterSpawnPointsForWall("D", spawnPointsD, new string[] {
                "SpawnPointD_1",
                "SpawnPointD_2",
                "SpawnPointD_3",
                "SpawnPointD_4",
                "SpawnPointD_5",
                "SpawnPointD_6",
                "SpawnPointD_7",
                "SpawnPointD_8",
                "SpawnPointD_9",
                "SpawnPointD_10",
                "SpawnPointD_11",
                "SpawnPointD_12",
                "SpawnPointD_13",
                "SpawnPointD_14",
                "SpawnPointD_15"
            });

            // Wall E spawn points - register each one
            RegisterSpawnPointsForWall("E", spawnPointsE, new string[] {
                "SpawnPointB_1",
                "SpawnPointB_2",
                "SpawnPointB_3",
                "SpawnPointB_4",
                "SpawnPointB_5",
                "SpawnPointB_6"
            });

            Log("TODO: Initialize spawn points - find entities by name");
        }

        private void RegisterSpawnPointsForWall(string wallName, List<SpawnPointData> targetList, string[] spawnPointNames)
        {
            Log(string.Concat("Registering spawn points for Wall ", wallName));

            int successCount = 0;
            int totalCount = spawnPointNames.Length;

            foreach (string spawnPointName in spawnPointNames)
            {
                // Find the spawn point entity by name
                uint entityID = InternalCalls.Scene_FindEntityByName(spawnPointName);

                if (entityID != 0)
                {
                    Vector3 spawnPosition;
                    Vector3 spawnRotation;

                    try
                    {
                        // Native position
                        InternalCalls.Transform_GetPosition(entityID, out spawnPosition);

                        Entity tempEntity = new Entity(entityID);
                        Transform tempTransform = new Transform();
                        tempTransform.Entity = tempEntity;

                        Quat spawnQuat = tempTransform.Rotation;
                        spawnRotation = spawnQuat.ToEuler();   // store Euler for prefab instantiation

                        // Create spawn point data and add to list
                        SpawnPointData spawnData = new SpawnPointData(spawnPosition, spawnRotation);
                        targetList.Add(spawnData);
                        successCount++;

                        Log(string.Concat(
                            spawnPointName, " registered at position (",
                            spawnPosition.X.ToString(), ", ", spawnPosition.Y.ToString(), ", ", spawnPosition.Z.ToString(),
                            ") rotation (",
                            spawnRotation.X.ToString(), ", ", spawnRotation.Y.ToString(), ", ", spawnRotation.Z.ToString(), ")"
                        ));
                    }
                    catch (Exception ex)
                    {
                        Log(string.Concat("WARNING: Failed to get transform for ", spawnPointName, " - ", ex.Message));
                    }
                }
                else
                {
                    Log(string.Concat(
                        "WARNING: Spawn point not found or invalid: ",
                        spawnPointName,
                        " (EntityID: ",
                        entityID.ToString(),
                        ")"
                    ));
                }
            }

            Log(string.Concat("Wall ", wallName, " registered: ", successCount.ToString(),
                "/", totalCount.ToString(), " spawn points"));
        }

        private void InitializeWalls()
        {
            // Helper to create entity list with validation
            wallAActiveEntities = CreateValidEntityArray(new string[]{
                "WallA1", "WallA2", "WallA3", "WallA4", "WallA5", "WallA6", "WallA_Logo", "WallA_Void"
            });

            wallBActiveEntities = CreateValidEntityArray(new string[]{
                "WallB1", "WallB2", "WallB3", "WallB4", "WallB5", "WallB6", "WallB_Logo", "WallB_Void"
            });

            wallCActiveEntities = CreateValidEntityArray(new string[]{
                "WallC1", "WallC2", "WallC3", "WallC4", "WallC5", "WallC6", "WallC_Logo", "WallC_Void"
            });

            wallDActiveEntities = CreateValidEntityArray(new string[]{
                "WallD1", "WallD2", "WallD3", "WallD4", "WallD5", "WallD6", "WallD_Logo", "WallD_Void"
            });

            wallEActiveEntities = CreateValidEntityArray(new string[]{
                "WallE1", "WallE2", "WallE3", "WallE4", "WallE5", "WallE6", "WallE_Logo", "WallE_Void"
            });

            wallInactiveEntities = CreateValidEntityArray(new string[]{
                "WallA_Inactive", "WallB_Inactive", "WallC_Inactive", "WallD_Inactive", "WallE_Inactive"
            });

            // Add all valid wall entities to the master list
            if (wallAActiveEntities != null && wallAActiveEntities.Length > 0)
                wallActiveEntities.AddRange(wallAActiveEntities);
            if (wallBActiveEntities != null && wallBActiveEntities.Length > 0)
                wallActiveEntities.AddRange(wallBActiveEntities);
            if (wallCActiveEntities != null && wallCActiveEntities.Length > 0)
                wallActiveEntities.AddRange(wallCActiveEntities);
            if (wallDActiveEntities != null && wallDActiveEntities.Length > 0)
                wallActiveEntities.AddRange(wallDActiveEntities);
            if (wallEActiveEntities != null && wallEActiveEntities.Length > 0)
                wallActiveEntities.AddRange(wallEActiveEntities);

            Log(string.Concat("Initialize wall entities - found ", wallActiveEntities.Count.ToString(), " active walls"));
            Log(string.Concat("Found ", wallInactiveEntities.Length.ToString(), " inactive walls"));
        }

        private void SetupWaveSpawning()
        {
            Log("SET UP WAVE SPAWNING HERE");
            CURRENT_WAVE++;

            switch (CURRENT_WAVE)
            {
                case 1:
                    SetEnemyCount(1, 4, 0, 0, 0);
                    loveletterRoutes = new string[] { "A1" };
                    activeSpawnPoints = spawnPointsA;
                    break;
                case 2:
                    SetEnemyCount(3, 13, 0, 0, 0);
                    loveletterRoutes = new string[] { "A1" };
                    activeSpawnPoints = new List<SpawnPointData>();
                    activeSpawnPoints.AddRange(spawnPointsA);
                    break;
                case 3:
                    // not for this milestone
                    break;
                default:
                    Log("Wave " + CURRENT_WAVE + " not configured - using Wave 3 settings");
                    break;
            }

            // Calculate total enemies to spawn
            waveEnemiesLeftToSpawn = E005_loveletter + E004_botnet + E001_worm_host + E003_trojan;

            Log(string.Concat("Total enemies to spawn: ", waveEnemiesLeftToSpawn.ToString()));

            // Update walls based on loveletter routes
            WallChange(loveletterRoutes);
        }

        private void SetEnemyCount(int loveletters, int botnets, int worms, int trojans, int adwares)
        {
            E005_loveletter = loveletters;
            E004_botnet = botnets;
            E001_worm_host = worms;
            E003_trojan = trojans;
            E007_adware = adwares;
        }

        private void StartCombat()
        {
            BREAK = false;
            COMBAT = true;

            //calculate when is the next time (Accumulated) for wave
            timeNext = elapsedTime + timeWave;
            timeCurrent = timeWave;

            spawningAllowed = true;
            spawnRateNext = elapsedTime + spawnRate;

            Log(string.Concat("=== COMBAT PHASE START - Wave ", CURRENT_WAVE.ToString(), " ==="));
        }

        private void StartPrep()
        {
            BREAK = true;
            COMBAT = false;

            timeNext = elapsedTime + timeBetweenWave;
            timeCurrent = timeBetweenWave;

            spawningAllowed = false;

            // Setup next wave
            if (CURRENT_WAVE < 2)
            {
                SetupWaveSpawning();
            }
            else
            {
                Log("=== ALL WAVES COMPLETE ===");
                Log("=== AWAITING FOR PLAYERS TO KILL ALL ENEMIES");
                isActive = false;
                WallSetup_DisableActiveWalls();
                WallSetup_InactiveWalls();
                //StopBGM();
                playbgm = false;
            }
        }

        private void SpawnEnemyByWaves()
        {
            if (elapsedTime < spawnRateNext)
            {
                return;
            }

            isSpawning = true;
            spawnRateNext = elapsedTime + spawnRate;

            // Determine which enemy type to spawn based on remaining counts
            int enemyType = DetermineEnemyTypeToSpawn();

            if (enemyType == -1)
            {
                Log("No more enemies to spawn");
                isSpawning = false;
                return;
            }

            // Spawn the enemy
            SpawnEnemy(enemyType);

            waveEnemiesLeftToSpawn--;
            isSpawning = false;

            Log(string.Concat("Spawned enemy type ", enemyType.ToString(),
                " - Remaining: ", waveEnemiesLeftToSpawn.ToString()));
        }

        private int DetermineEnemyTypeToSpawn()
        {
            // Priority: Loveletter > Botnet > Worm > Trojan
            if (E005_loveletter > 0)
            {
                E005_loveletter--;
                return 1; // Loveletter
            }
            else if (E004_botnet > 0)
            {
                E004_botnet--;
                return 0; // Botnet
            }
            else if (E001_worm_host > 0)
            {
                E001_worm_host--;
                return 2; // Worm
            }
            else if (E003_trojan > 0)
            {
                E003_trojan--;
                return 3; // Trojan
            }
            else if (E007_adware > 0)
            {
                E007_adware--;
                return 4; // Adware
            }

            return -1; // No enemies left to spawn
        }

        private void SpawnEnemy(int enemyType)
        {
            // Get random spawn point
            if (activeSpawnPoints == null || activeSpawnPoints.Count == 0)
            {
                Log("ERROR: No active spawn points!");
                return;
            }

            // Create enemy from prefab
            string prefabpath = "Sources/Prefabs/" + enemyPrefabNames[enemyType] + ".prefab";

            if (enemyType == 1)
            {
                // Loveletter special spawn logic
                SpawnLoveLetter(prefabpath);
                return;
            }

            int spawnIndex = GetRandomInt(0, activeSpawnPoints.Count);
            SpawnPointData spawnPoint = activeSpawnPoints[spawnIndex];

            // Get spawn position and rotation from the spawn point
            Vector3 spawnPos = spawnPoint.position;
            Vector3 spawnRot = spawnPoint.rotation; // Euler from Quat.ToEuler
            Vector3 spawnScale = new Vector3(0.006f, 0.006f, 0.006f);

            uint enemyID = InternalCalls.Prefab_InstantiateWithTransform(prefabpath, ref spawnPos, ref spawnRot, ref spawnScale, false);

            if (enemyID == 0)
            {
                Log("INVALID ID FOR SPAWNING");
            }
            else
            {
                Log(string.Concat("Spawn type: ", enemyType.ToString(), " at position ", spawnPos.X.ToString(), ", ",
                spawnPos.Y.ToString(), ", ", spawnPos.Z.ToString(), " and at rotation ", spawnRot.X.ToString(), ", ",
                spawnRot.Y.ToString(), ", ", spawnRot.Z.ToString()));
            }

            Log(string.Concat("Spawn ", prefabpath, " at position (",
                spawnPos.X.ToString(), ", ", spawnPos.Y.ToString(), ", ", spawnPos.Z.ToString(), ")"));
        }

        private void SpawnLoveLetter(string prefabpath)
        {
            int spawnIndex = GetRandomInt(0, loveletterRoutes.Length);
            string spawnPointName = loveletterRoutes[spawnIndex];

            // Find the spawn point entity by name
            uint spawnID = InternalCalls.Scene_FindEntityByName(spawnPointName);

            Log("Spawn point name is: " + spawnPointName);

            Vector3 spawnPosition;
            Vector3 spawnRotation;

            InternalCalls.Transform_GetPosition((uint)spawnID, out spawnPosition);

            // Get rotation from Transform (Quat) and convert to Euler Vector3
            Entity spawnent = new Entity(spawnID);
            Transform spawnentrans = new Transform();
            spawnentrans.Entity = spawnent;
            Quat spawnQuat = spawnentrans.Rotation;
            spawnRotation = spawnQuat.ToEuler();

            Vector3 spawnScale = new Vector3(0.002f, 0.002f, 0.002f);

            //PLAY WARNING AUDIO ONCE
            InternalCalls.Entity_AddAudio(spawnID);
            InternalCalls.Audio_SetFile(spawnID, loveletterwarning);
            InternalCalls.Audio_SetLoop(spawnID, false);
            InternalCalls.Audio_SetIs3D(spawnID, true);
            InternalCalls.Audio_SetMinDistance(spawnID, 22.42f);
            InternalCalls.Audio_SetMaxDistance(spawnID, 300.87f);
            InternalCalls.Audio_Play(spawnID);

            uint enemyID = InternalCalls.Prefab_InstantiateWithTransform(prefabpath, ref spawnPosition, ref spawnRotation, ref spawnScale, true);

            if (enemyID == 0)
            {
                Log("LOVELETTERSPAWN FAIL");
            }
            else
            {
                Log(string.Concat("Spawn type: loveletter at position ", spawnPosition.X.ToString(), ", ",
                spawnPosition.Y.ToString(), ", ", spawnPosition.Z.ToString(), " and at rotation ", spawnRotation.X.ToString(), ", ",
                spawnRotation.Y.ToString(), ", ", spawnRotation.Z.ToString()));
            }
        }

        private void CheckForEnemiesLeft()
        {
            uint[] loveletter = InternalCalls.Scene_FindEntitiesByTag("loveletter");
            uint[] botnet = InternalCalls.Scene_FindEntitiesByTag("botnet");
            uint[] trojan = InternalCalls.Scene_FindEntitiesByTag("Enemy_Trojan");
            uint[] adware = InternalCalls.Scene_FindEntitiesByTag("Enemy_Adware");
            uint[] worm = InternalCalls.Scene_FindEntitiesByTag("Enemy_Worm");

            int totalenemiesleft = 0;

            if (loveletter != null && loveletter.Length != 0)
            {
                totalenemiesleft += loveletter.Length;
            }
            else if (botnet != null && botnet.Length != 0)
            {
                totalenemiesleft += botnet.Length;
            }
            else if (trojan != null && trojan.Length != 0)
            {
                totalenemiesleft += trojan.Length;
            }
            else if (adware != null && adware.Length != 0)
            {
                totalenemiesleft += adware.Length;
            }
            else if (worm != null && worm.Length != 0)
            {
                totalenemiesleft += worm.Length;
            }

            if (totalenemiesleft <= 0 && waveEnemiesLeftToSpawn <= 0)
            {
                enemiesLeft = 0;
                EndTime();
            }
            else
            {
                enemiesLeft = totalenemiesleft;
            }
        }

        private void EndTime()
        {
            Log("=== Wave Complete ===");
            StartPrep();
        }

        #region environment

        // Wall management
        private void WallChange(string[] routes)
        {
            // Disable all active walls
            WallSetup_DisableActiveWalls();

            // Enable all inactive walls
            WallSetup_InactiveWalls();

            // Enable specific walls based on routes
            foreach (string route in routes)
            {
                switch (route)
                {
                    case "A1": case "A2": WallEnable(0); break;
                    case "B1": case "B2": WallEnable(1); break;
                    case "C1": WallEnable(2); break;
                    case "D1": case "D2": WallEnable(3); break;
                    case "E1": WallEnable(4); break;
                }
            }
        }

        private void WallEnable(int wallIndex)
        {
            switch (wallIndex)
            {
                case 0:
                    if (wallAActiveEntities != null)
                    {
                        WallSetup_ActiveWallVisibility(wallAActiveEntities, 0);
                    }
                    break;
                case 1:
                    if (wallBActiveEntities != null)
                    {
                        WallSetup_ActiveWallVisibility(wallBActiveEntities, 1);
                    }
                    break;
                case 2:
                    if (wallCActiveEntities != null)
                    {
                        WallSetup_ActiveWallVisibility(wallCActiveEntities, 2);
                    }
                    break;
                case 3:
                    if (wallDActiveEntities != null)
                    {
                        WallSetup_ActiveWallVisibility(wallDActiveEntities, 3);
                    }
                    break;
                case 4:
                    if (wallEActiveEntities != null)
                    {
                        WallSetup_ActiveWallVisibility(wallEActiveEntities, 4);
                    }
                    break;
            }

            Log("Enable wall " + wallIndex);
        }

        private void WallSetup_ActiveWallVisibility(Entity[] entities, int wallIndex)
        {
            //activate the selected walls to spawn
            foreach (Entity entity in entities)
            {
                if (entity.EntityID != 0) // Extra safety check
                {
                    InternalCalls.MeshRenderer_SetVisible((uint)entity.EntityID, true);
                }
                else
                {
                    Log("ERROR: Skipping invalid entity (ID=0) in WallSetup_ActiveWallVisibility");
                }
            }

            //deactivate the inactive wall of the walls that are spawning
            if (wallInactiveEntities != null && wallIndex < wallInactiveEntities.Length)
            {
                uint id = wallInactiveEntities[wallIndex].EntityID;
                if (id != 0) // Extra safety check
                {
                    InternalCalls.MeshRenderer_SetVisible((uint)id, false);
                }
                else
                {
                    Log(string.Concat("ERROR: Invalid inactive wall entity at index ", wallIndex.ToString()));
                }
            }
        }

        private void WallSetup_InactiveWalls()
        {
            // Enable all inactive walls
            if (wallInactiveEntities != null)
            {
                Log(string.Concat("Enabling ", wallInactiveEntities.Length.ToString(), " inactive wall entities"));
                foreach (Entity wall in wallInactiveEntities)
                {
                    if (wall.EntityID != 0)
                    {
                        InternalCalls.MeshRenderer_SetVisible((uint)wall.EntityID, true);
                    }
                }
                Log("All inactive walls enabled");
            }
        }

        private void WallSetup_DisableActiveWalls()
        {
            // Disable all active walls
            if (wallActiveEntities != null)
            {
                foreach (Entity wall in wallActiveEntities)
                {
                    if (wall.EntityID != 0) // Extra safety check
                    {
                        Log(string.Concat("HI PLS WORK DISABLE ACTIVE WALLS - EntityID: ", wall.EntityID.ToString()));
                        InternalCalls.MeshRenderer_SetVisible((uint)wall.EntityID, false);
                    }
                    else
                    {
                        Log("ERROR: Skipping invalid entity (ID=0) in wallActiveEntities");
                    }
                }
            }
        }

        #endregion

        #region random

        // Simple pseudo-random number generator (since System.Random not available)
        private int GetRandomInt(int min, int max)
        {
            // Linear Congruential Generator
            rngSeed = (1103515245 * rngSeed + 12345) & 0x7fffffff;
            return min + (int)(rngSeed % (uint)(max - min));
        }

        #endregion

        #region others

        // Helper method to create entity arrays with validation
        private Entity[] CreateValidEntityArray(string[] entityNames)
        {
            List<Entity> validEntities = new List<Entity>();

            foreach (string name in entityNames)
            {
                uint entityID = InternalCalls.Scene_FindEntityByName(name);
                if (entityID != 0) // Only add valid entities
                {
                    validEntities.Add(new Entity(entityID));
                    Log(string.Concat("Found entity: ", name, " (ID: ", entityID.ToString(), ")"));
                }
                else
                {
                    Log(string.Concat("WARNING: Entity not found: ", name));
                }
            }

            Log(string.Concat("CreateValidEntityArray: ", validEntities.Count.ToString(),
                " / ", entityNames.Length.ToString(), " entities found"));
            return validEntities.ToArray();
        }

        private void PlayGameBGM()
        {
            spawnmanagerID = InternalCalls.Scene_FindEntityByName("Spawn Manager");
            if (spawnmanagerID != INVALID_ENTITY)
            {
                Log("YAY FOUND IT IT'S " + spawnmanagerID.ToString());
            }
            InternalCalls.Audio_Play(spawnmanagerID);
            PlayAllOtherAudio();
        }

        //FOR FUTURE PURPOSE
        private void PauseBGM()
        {
            InternalCalls.Audio_Pause((uint)EntityID);
        }

        private void StopBGM()
        {
            InternalCalls.Audio_Stop((uint)EntityID);
            StopAllOtherAudio();
        }

        #endregion

        #region other sound

        private void PlayAllOtherAudio()
        {
            uint[] Allies = InternalCalls.Scene_FindEntitiesByTag("ALLIES");

            if (Allies == null || Allies.Length <= 0)
            {
                LogWarning("SpawnManager: Allies list is null or non-existent/not found");
            }

            if (Allies != null)
            {
                for (int i = 0; i < Allies.Length; i++)
                {
                    if (Allies[i] != INVALID_ENTITY)
                    {
                        InternalCalls.Entity_AddAudio(Allies[i]);
                        InternalCalls.Audio_SetFile(Allies[i], alliesambience);
                        InternalCalls.Audio_SetLoop(Allies[i], true);
                        InternalCalls.Audio_SetIs3D(Allies[i], true);
                        InternalCalls.Audio_SetMinDistance(Allies[i], 5.42f);
                        InternalCalls.Audio_SetMaxDistance(Allies[i], 152.45f);

                        InternalCalls.Audio_Play(Allies[i]);
                        Log("SpawnManager: Playing ally audio right now - only gunship ambience");
                    }
                }
            }

            uint coreID = InternalCalls.Scene_FindEntityByName("Core");

            if (coreID != INVALID_ENTITY)
            {
                InternalCalls.Entity_AddAudio(coreID);
                InternalCalls.Audio_SetFile(coreID, coreambience);
                InternalCalls.Audio_SetLoop(coreID, true);
                InternalCalls.Audio_SetIs3D(coreID, true);
                InternalCalls.Audio_SetMinDistance(coreID, 52.42f);
                InternalCalls.Audio_SetMaxDistance(coreID, 527.87f);

                InternalCalls.Audio_Play(coreID);
                Log("SpawnManager: Playing core ambience now through core");
            }
            else
            {
                LogError("SpawnManager: Cannot find Emplacement");
            }
        }

        private void StopAllOtherAudio()
        {
            AudioManager.StopAll();
        }

        private void stopmainsound()
        {
            uint officeambi = InternalCalls.Scene_FindEntityByName("office_ambience");

            if (officeambi != INVALID_ENTITY)
            {
                InternalCalls.Audio_SetLoop(officeambi, false);
                InternalCalls.Audio_Stop(officeambi);
                Log("SpawnManager: Stopping office ambience");
            }
            else
            {
                LogError("SpawnManager: Cannot find office ambience");
            }

            uint roomambi = InternalCalls.Scene_FindEntityByName("room_ambience");

            if (roomambi != INVALID_ENTITY)
            {
                InternalCalls.Audio_SetLoop(roomambi, false);
                InternalCalls.Audio_Stop(roomambi);
                Log("SpawnManager: Stopping room ambience");
            }
            else
            {
                LogError("SpawnManager: Cannot find room ambience");
            }
        }

        #endregion
    }

}

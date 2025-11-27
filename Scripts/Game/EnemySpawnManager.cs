using System;
using System.Collections.Generic;
using System.Collections;
using Engine;

namespace Game{
    public class EnemySpawnManager : ScriptBehaviour
    {
        public static EnemySpawnManager instance;
    
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
        [SerializeField] private float timeBetweenWave = 30.0f;
        [SerializeField] private float timeWave = 60.0f;
        [SerializeField] private float timeCurrent = 10.0f;
        [SerializeField] private float timeNext = 0.0f;
        [SerializeField] private bool BREAK = true;
        [SerializeField] private bool COMBAT = false;
        
        // Enemy counting
        [SerializeField] private int enemiesLeft = 0;
        
        // Prefab names 
        private string[] enemyPrefabNames = new string[]
        {
            "Enemy_Botnet",      // 0
            "Enemy_Loveletter",  // 1  
            "Enemy_Worm",        // 2
            "Enemy_Trojan",      // 3
            "Enemy_Adware",      // 4
        };
        
        // Create spawn point of equal spacing based off wall scale
        private List<Transform> spawnPointsA = new List<Transform>();
        private List<Transform> spawnPointsB = new List<Transform>();
        private List<Transform> spawnPointsC = new List<Transform>();
        private List<Transform> spawnPointsD = new List<Transform>();
        private List<Transform> spawnPointsE = new List<Transform>();
    
        // Current active spawn points for this wave
        private List<Transform> activeSpawnPoints = new List<Transform>();
        
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
        
        // Light entities
        private Entity lightPrep;
        private Entity lightCombat;
        
        // Simple pseudo-random number generator
        private uint rngSeed;

        // Time tracking (since we don't have Time.time)
        private float elapsedTime = 0f;
        
        public override void OnStart()
        {
            // Singleton setup
            if (instance == null)
            {
                instance = this;
            }
            else
            {
                Log("DUPLICATE EnemySpawnManager. DELETING");
                // In custom engine, you'd destroy this entity
                return;
            }
            
            // Initialize random seed
            rngSeed = (uint)DateTime.Now.Ticks;

            // Find all spawn points in the scene
            InitializeSpawnPoints();
            
            // Find wall entities
            InitializeWalls();
            
            // Find light entities
            InitializeLights();
            
            // Setup lights for prep phase
            SetupLights();
            
            // Setup walls for initial state
            WallSetup_DisableActiveWalls();
            WallSetup_InactiveWalls();
            
            // Start at wave 1
            SetupWaveSpawning();
            
            isActive = true;
            
            Log("EnemySpawnManager initialized - Wave " + CURRENT_WAVE);
        }
        
        public override void OnUpdate(float deltaTime)
        {
            if (!isActive)
                return;
            
            elapsedTime += deltaTime;
            float currentTime = elapsedTime;
            
            // Phase transitions
            if (currentTime > timeNext)
            {
                if  (BREAK)
                {
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
            
            // Check if wave is complete
            if (COMBAT)
            {
                CheckForEnemiesLeft();
            }
            
            // CHEAT CODE - K to kill all enemies
            if (Input.IsKeyPressed(KeyCode.K))
            {
                enemiesLeft = 0;
                // Would destroy all enemy entities here
                Log("CHEAT: All enemies killed");
            }
        }
        
        private void InitializeSpawnPoints()
        {
            // Find spawn points by name pattern
            // You'll need to have entities named like:
            // "SpawnPoint_A_1", "SpawnPoint_A_2", "SpawnPoint_A_3", etc.
            
            // For now, we'll create arrays to hold them
            // You'll need to implement Entity.FindByName() or similar
            
            // EXAMPLE - adapt this to your engine's entity finding method:
            /*
            spawnPointsA = new Entity[] {
                Entity.FindByName("SpawnPoint_A_1"),
                Entity.FindByName("SpawnPoint_A_2"),
                Entity.FindByName("SpawnPoint_A_3")
            };
            
            spawnPointsB = new Entity[] {
                Entity.FindByName("SpawnPoint_B_1"),
                Entity.FindByName("SpawnPoint_B_2"),
                Entity.FindByName("SpawnPoint_B_3")
            };
            
            // etc. for C, D, E
            */

            
            
            Log("TODO: Initialize spawn points - find entities by name");
        }
        
        private void InitializeWalls()
        {
            // Find wall entities by name
            // Expected names: "Wall_A_Active", "Wall_A_Inactive", etc.
            
            /*
            wallActiveEntities = new Entity[] {
                Entity.FindByName("Wall_A_Active"),
                Entity.FindByName("Wall_B_Active"),
                Entity.FindByName("Wall_C_Active"),
                Entity.FindByName("Wall_D_Active"),
                Entity.FindByName("Wall_E_Active")
            };
            
            wallInactiveEntities = new Entity[] {
                Entity.FindByName("Wall_A_Inactive"),
                Entity.FindByName("Wall_B_Inactive"),
                Entity.FindByName("Wall_C_Inactive"),
                Entity.FindByName("Wall_D_Inactive"),
                Entity.FindByName("Wall_E_Inactive")
            };
            */

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
            
            Log("Initialize wall entities - found " + wallActiveEntities.Count + " active walls");
            Log("Found " + wallInactiveEntities.Length + " inactive walls");
        }
        
        private void InitializeLights()
        {
            // Find light entities
            // lightPrep = Entity.FindByName("Light_Prep");
            // lightCombat = Entity.FindByName("Light_Combat");
            
            Log("TODO: Initialize light entities");
        }
        
        private void SetupWaveSpawning()
        {
            CURRENT_WAVE++;
            
            Log("=== Setting up WAVE " + CURRENT_WAVE + " ===");
            
            // Configure wave based on current wave number
            switch (CURRENT_WAVE)
            {
                case 1:
                    SetEnemyCount(1, 4, 0, 0, 0);
                    //SetEnemyCount(1, 4, 20, 0, 0);
                    loveletterRoutes = new string[] {"A1", "A2"};
                    activeSpawnPoints = spawnPointsA;
                    break;
                case 2:
                    SetEnemyCount(3, 13, 0, 0, 0);
                    //SetEnemyCount(3, 13, 6, 7, 0);
                    loveletterRoutes = new string[] {"A1", "A2", "D1", "D2"};
                    activeSpawnPoints = new List<Transform>();
                    activeSpawnPoints.AddRange(spawnPointsA);
                    activeSpawnPoints.AddRange(spawnPointsD);
                    break;
                case 3:
                    // not for this milestone
                    // SetEnemyCount{5, 16, 7, 0, 1};
                    // loveletterRoutes = new string[] {"B1", "B2", "C1", "E1"};
                    // activeSpawnPoints = new List<Transform>();
                    // activeSpawnPoints.AddRange(spawnPointsB);
                    // activeSpawnPoints.AddRange(spawnPointsC);
                    // activeSpawnPoints.AddRange(spawnPointsE);
                    break;
                default:
                    Log("Wave " + CURRENT_WAVE + " not configured - using Wave 3 settings");
                    break;
            }
            
            // Calculate total enemies to spawn
            waveEnemiesLeftToSpawn = E005_loveletter + E004_botnet + E001_worm_host + E003_trojan;
            enemiesLeft = waveEnemiesLeftToSpawn;
            
            Log("Total enemies to spawn: " + waveEnemiesLeftToSpawn);
            
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
            
            timeNext = elapsedTime + timeWave;
            timeCurrent = timeWave;
            
            HandleLights("COMBAT", true);
            
            spawningAllowed = true;
            spawnRateNext = elapsedTime + spawnRate;
            
            Log("=== COMBAT PHASE START - Wave " + CURRENT_WAVE + " ===");
        }
        
        private void StartPrep()
        {
            BREAK = true;
            COMBAT = false;
            
            timeNext = elapsedTime + timeBetweenWave;
            timeCurrent = timeBetweenWave;
            
            HandleLights( "BREAK", true);
            
            spawningAllowed = false;
            
            // Setup next wave
            if (CURRENT_WAVE < 2)
            {
                SetupWaveSpawning();
            }
            else
            {
                Log("=== ALL WAVES COMPLETE ===");
                isActive = false;
            }
        }
        
        private void SpawnEnemyByWaves()
        {
            if (elapsedTime < spawnRateNext)
                return;
            
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
            
            Log("Spawned enemy type " + enemyType + " - Remaining: " + waveEnemiesLeftToSpawn);
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
            }else if (E007_adware > 0)
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
            
            int spawnIndex = GetRandomInt(0, activeSpawnPoints.Count);
            Transform spawnPoint = activeSpawnPoints[spawnIndex];
            
            // Get spawn position and rotation from the spawn point
            Vector3 spawnPos = spawnPoint.Position;
            Vector3 spawnRot = spawnPoint.Rotation; // Or use Euler angles
            
            // Create enemy from prefab
            string prefabpath = "Sources/Prefabs/" + enemyPrefabNames[enemyType] + ".prefab";
            
            // MANUAL INSTANTIATION - adapt to your engine's PrefabInstantiator
            /*
            Entity enemy = PrefabRegistry.InstantiatePrefab(prefabName);
            
            if (enemy.IsValid())
            {
                // Manually set transform
                enemy.Transform.Position = spawnPos;
                enemy.Transform.Rotation = spawnRot;
                
                // If it's a loveletter, set its route
                if (enemyType == 1 && loveletterRoutes.Length > 0)
                {
                    // Get random route
                    int routeIndex = GetRandomInt(0, loveletterRoutes.Length);
                    string route = loveletterRoutes[routeIndex];
                    
                    // Set the route on the enemy script
                    // enemy.GetComponent<EnemyLoveletterScript>().SetPathID(route);
                }
                
                Log("Spawned " + prefabName + " at position " + spawnPos);
            }
            else
            {
                Log("ERROR: Failed to instantiate " + prefabName);
            }
            */
            
            Log("TODO: Spawn " + prefabpath + " at position " + spawnPos);
        }
        
        private void CheckForEnemiesLeft()
        {
            // Count enemies by tag or some other method
            // For now, just track the counter
            
            if (enemiesLeft <= 0 && waveEnemiesLeftToSpawn <= 0)
            {
                enemiesLeft = 0;
                // End combat early if all enemies defeated
                EndTime();
            }
        }
        
        public void EnemyDestroyed()
        {
            if (COMBAT)
            {
                enemiesLeft--;
                Log("Enemy destroyed - Remaining: " + enemiesLeft);
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
            // Enable active wall, disable inactive wall
            /*
            if (wallActiveEntities != null && wallIndex < wallActiveEntities.Length)
            {
                wallActiveEntities[wallIndex].SetActive(true);
                wallInactiveEntities[wallIndex].SetActive(false);
            }
            */
            switch(wallIndex){
                case 0:
                    if(wallAActiveEntities != null){
                        WallSetup_ActiveWallVisibility(wallAActiveEntities, 0);
                    }
                    break;
                case 1:
                    if(wallBActiveEntities != null){
                        WallSetup_ActiveWallVisibility(wallBActiveEntities, 1);
                    }
                    break;
                case 2:
                    if(wallCActiveEntities != null){
                        WallSetup_ActiveWallVisibility(wallCActiveEntities, 2);
                    }
                    break;
                case 3:
                    if(wallDActiveEntities != null){
                        WallSetup_ActiveWallVisibility(wallDActiveEntities, 3);
                    }
                    break;
                case 4:
                    if(wallEActiveEntities != null){
                        WallSetup_ActiveWallVisibility(wallEActiveEntities, 4);
                    }
                    break;
            }
            
            Log("Enable wall " + wallIndex);
        }
        
        private void WallSetup_ActiveWallVisibility(Entity[] entities, int wallIndex){
            //activate the selected walls to spawn
            foreach(Entity entity in entities){
                if (entity.EntityID != 0) // Extra safety check
                {
                    InternalCalls.MeshRenderer_SetVisible(entity.EntityID, true);
                }
                else
                {
                    Log("ERROR: Skipping invalid entity (ID=0) in WallSetup_ActiveWallVisibility");
                }
            }

            //deactivate the inactive wall of the walls that are spawning
            if (wallInactiveEntities != null && wallIndex < wallInactiveEntities.Length)
            {
                ulong id = wallInactiveEntities[wallIndex].EntityID;
                if (id != 0) // Extra safety check
                {
                    InternalCalls.MeshRenderer_SetVisible(id, false);
                }
                else
                {
                    Log("ERROR: Invalid inactive wall entity at index " + wallIndex);
                }
            }
        }

        private void WallSetup_InactiveWalls()
        {
            // Enable all inactive walls
            /*
            if (wallInactiveEntities != null)
            {
                foreach (Entity wall in wallInactiveEntities)
                {
                    if (wall.IsValid())
                        wall.SetActive(true);
                }
            }
            */
            // Enable all inactive walls
            if(wallInactiveEntities != null){
                Log("Enabling " + wallInactiveEntities.Length + " inactive wall entities");
                foreach(Entity wall in wallInactiveEntities){
                    if (wall.EntityID != 0)
                    {
                        InternalCalls.MeshRenderer_SetVisible(wall.EntityID, true);
                    }
                }
                Log("All inactive walls enabled");
            }
        }
        
        private void WallSetup_DisableActiveWalls()
        {
            // Disable all active walls
            if(wallActiveEntities != null){
                foreach (Entity wall in wallActiveEntities){
                    if (wall.EntityID != 0) // Extra safety check
                    {
                        Log("HI PLS WORK DISABLE ACTIVE WALLS - EntityID: " + wall.EntityID);
                        InternalCalls.MeshRenderer_SetVisible(wall.EntityID, false);
                    }
                    else
                    {
                        Log("ERROR: Skipping invalid entity (ID=0) in wallActiveEntities");
                    }
                }
            }
        }
        
        // Light management
        private void SetupLights()
        {
            HandleLights( "BREAK", true);
        }
        
        private void HandleLights(string phase, bool status)
        {
            /*
            if (phase ==  BREAK")
            {
                if (lightPrep.IsValid())
                    lightPrep.SetActive(status);
                if (lightCombat.IsValid())
                    lightCombat.SetActive(!status);
            }
            else if (phase == "COMBAT")
            {
                if (lightPrep.IsValid())
                    lightPrep.SetActive(!status);
                if (lightCombat.IsValid())
                    lightCombat.SetActive(status);
            }
            */
            
            Log("TODO: Handle lights - " + phase);
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
                ulong entityID = InternalCalls.Scene_FindEntityByName(name);
                if (entityID != 0) // Only add valid entities
                {
                    validEntities.Add(new Entity(entityID));
                    Log("Found entity: " + name + " (ID: " + entityID + ")");
                }
                else
                {
                    Log("WARNING: Entity not found: " + name);
                }
            }
            
            Log("CreateValidEntityArray: " + validEntities.Count + " / " + entityNames.Length + " entities found");
            return validEntities.ToArray();
        }

        #endregion
    }

}
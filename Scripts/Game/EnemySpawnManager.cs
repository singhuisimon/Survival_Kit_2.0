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
        // Basic toggles
        [SerializeField] private bool isActive = false;
        [SerializeField] private bool spawningAllowed = false;
        [SerializeField] private bool infiniteSpawning = false;
        
        // Wave tracking
        [SerializeField] private int waveEnemiesLeftToSpawn = 0;

        // Enemy counts for current wave
        [SerializeField] private int E005_loveletter = 0;
        [SerializeField] private int E004_botnet = 0;

        // Spawn timing
        [SerializeField] private float spawnRate = 1.50f;
        [SerializeField] private float spawnRateNext = 0.0f;
        [SerializeField] private bool isSpawning = false;
        // Enemy counting
        [SerializeField] private int enemiesLeft = 0;

        // Prefab names 
        private string[] enemyPrefabNames = new string[]
        {
            "Enemy_Botnet",      // 0
            "Enemy_LoveLetter",  // 1  
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

        // Simple pseudo-random number generator
        private uint rngSeed;

        // Time tracking (since we don't have Time.time)
        [SerializeField] private float elapsedTime = 0f;
        [SerializeField] private int botnetSpawned = 0;
        [SerializeField] private int loveletterSpawned = 0;

        private string alliesambience = "Flotilla_Gunship_Ambient.wav";
        private string coreambience = "Core_Ambient.wav";
        private string loveletterwarning = "Loveletter_Warp_Warning.wav";

        private uint spawnmanagerID;

        private bool playInGameSound = false;
  
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
            
            // // Setup walls for initial state
            EnvironmentReset();
            
            // spawnmanagerID = InternalCalls.Scene_FindEntityByName("Spawn Manager");
            // if(spawnmanagerID != INVALID_ENTITY){
            //     Log("YAY FOUND IT IT'S " + spawnmanagerID.ToString());
            // }

            InternalCalls.Entity_AddAudio((uint)EntityID);

            Log("EnemySpawnManager initialized");
        }

        public override void OnUpdate(float deltaTime)
        {
            //check for the trigger to start
            //logically restart will also be via enter!
            //change of plan. change this part of the code to look for maincamera if enabled.
            if(Input.IsKeyPressed(KeyCode.Enter) && !isActive){
                isActive = true;
                stopmainsound();

                //deactivate all active wall and activate all inactive
                EnvironmentReset();

                //set the no. of enemies to spawn here
                //in the function it also activates the wall we spawning enemies from
                SetupEnemySpawning();

                //play the ingame sounds
                if(!playInGameSound){
                    PlayInGameSounds();
                } 
            }

            //check if the game has started
            if (!isActive){
                return;
            }

            //update elapsed time
            elapsedTime += deltaTime;
            float currentTime = elapsedTime;

            // Spawning logic
            if (spawningAllowed)
            {
                if (!isSpawning && waveEnemiesLeftToSpawn > 0)
                {
                    SpawnPresetEnemy();
                }
                else
                {
                    isSpawning = false;
                    //spawningAllowed = false;
                }

                if(infiniteSpawning){
                    SetupInfiniteBotnetSpawning();
                }
            }

            CheckForEnemiesLeft();
            
        }

        #region setup
        
        private void InitializeSpawnPoints()
        {
            // Find spawn points by name pattern
            // You'll need to have entities named like:
            // "SpawnPoint_A_1", "SpawnPoint_A_2", "SpawnPoint_A_3", etc.
            
            // For now, we'll create arrays to hold them
            // You'll need to implement Entity.FindByName() or similar

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
                "SpawnPointE_1",
                "SpawnPointE_2",
                "SpawnPointE_3",
                "SpawnPointE_4",
                "SpawnPointE_5",
                "SpawnPointE_6"
            });
            
            Log("SpawnManager is initializing spawnpoints for all wall");
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
            // Find wall entities by name
            // Expected names: "Wall_A_Active", "Wall_A_Inactive", etc.

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

        private void SetupEnemySpawning()
        {
            Log("SpawnManager - Setting up enemy spawning");
            
            E005_loveletter = 0;
            E004_botnet = 15;

            loveletterRoutes = new string[] {"A1"};

            activeSpawnPoints = spawnPointsA;

            // Calculate total enemies to spawn
            waveEnemiesLeftToSpawn = E005_loveletter + E004_botnet;
            
            Log(string.Concat("Total enemies to spawn: ", waveEnemiesLeftToSpawn.ToString()));

            // Update walls based on loveletter routes
            WallChange(loveletterRoutes);

            spawningAllowed = true;
            spawnRateNext = elapsedTime;
        }

        private void SetupInfiniteBotnetSpawning(){
            Log("SpawnManager - Setting up enemy spawning");
            
            E004_botnet += 1;

            // Calculate total enemies to spawn
            waveEnemiesLeftToSpawn = E005_loveletter + E004_botnet;
            
            Log(string.Concat("Total enemies to spawn: ", waveEnemiesLeftToSpawn.ToString()));
        }

        #endregion

        #region Spawning
        
        private void SpawnPresetEnemy()
        {
            if (elapsedTime < spawnRateNext){
                isSpawning = false;
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
            
            //waveEnemiesLeftToSpawn--;
            waveEnemiesLeftToSpawn = E004_botnet + E005_loveletter;
            //isSpawning = false;
            
            Log(string.Concat("Spawned enemy type ", enemyType.ToString(), 
                " - Remaining: ", waveEnemiesLeftToSpawn.ToString()));
            Log("spawned botnet count: " + botnetSpawned.ToString());
            Log("spawned loveletter count: " + loveletterSpawned.ToString());
        }

        private int DetermineEnemyTypeToSpawn()
        {
            // Priority: Loveletter > Botnet > Worm > Trojan
            if (E005_loveletter > 0)
            {
                E005_loveletter--;
                //loveletterSpawned++;
                return 1; // Loveletter
            }
            else if (E004_botnet > 0)
            {
                E004_botnet--;
                //botnetSpawned++;
                return 0; // Botnet
            }
            
            return -1; // No enemies left to spawn
        }

        private void SpawnEnemy(int enemyType)
        {
            // Get random spawn point
            if (activeSpawnPoints == null || activeSpawnPoints.Count == 0)
            {
                isSpawning = false;
                Log("ERROR: No active spawn points!");
                return;
            }

            // Create enemy from prefab
            string prefabpath = "Sources/Prefabs/" + enemyPrefabNames[enemyType] + ".prefab";
            
            if(enemyType == 1){
                //Log("HI LOVELETTER HERE");
                SpawnLoveLetter(prefabpath);
                return;
            }

            //comment this part if u want to test more than 20 botnet.
            if(botnetSpawned >= 20){
                if(enemyType == 0){
                    return;
                }
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
            } else {
                if(enemyType == 0){
                    botnetSpawned++;
                }
                InternalCalls.Entity_AddScript(enemyID, "Game.Botnet");
                Log(string.Concat("Spawn type: ", enemyType.ToString(), " at position ", spawnPos.X.ToString(), ", " ,
                spawnPos.Y.ToString(), ", " , spawnPos.Z.ToString(), " and at rotation ", spawnRot.X.ToString(), ", " ,
                spawnRot.Y.ToString(), ", " , spawnRot.Z.ToString()));
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

            // PLAY WARNING AUDIO ONCE
            InternalCalls.Entity_AddAudio(spawnID);
            InternalCalls.Audio_SetFile(spawnID, loveletterwarning);
            InternalCalls.Audio_SetLoop(spawnID, false);
            InternalCalls.Audio_SetIs3D(spawnID, true);
            InternalCalls.Audio_SetMinDistance(spawnID, 22.42f);
            InternalCalls.Audio_SetMaxDistance(spawnID, 300.87f);
            InternalCalls.Audio_Play(spawnID);

            uint enemyID = InternalCalls.Prefab_InstantiateWithTransform(
                prefabpath, ref spawnPosition, ref spawnRotation, ref spawnScale, true
            );

            if (enemyID == 0)
            {
                Log("LOVELETTERSPAWN FAIL");
            }
            else
            {
                loveletterSpawned++;
                Log(string.Concat(
                    "Spawn type: loveletter at position ",
                    spawnPosition.X.ToString(), ", ",
                    spawnPosition.Y.ToString(), ", ",
                    spawnPosition.Z.ToString(), " and at rotation ",
                    spawnRotation.X.ToString(), ", ",
                    spawnRotation.Y.ToString(), ", ",
                    spawnRotation.Z.ToString()
                ));
            }
        }

        #endregion

        private void CheckForEnemiesLeft()
        {
            uint[] loveletter = InternalCalls.Scene_FindEntitiesByTag("loveletter");
            uint[] botnet = InternalCalls.Scene_FindEntitiesByTag("botnet");

            int totalenemiesleft = 0;

            if (loveletter != null && loveletter.Length != 0)
            {
                totalenemiesleft += loveletter.Length;
                Log("adding loveletter to total enemies left. currently there is: " + loveletter.Length.ToString());
            }
            
            if (botnet != null && botnet.Length != 0){
                totalenemiesleft += botnet.Length;
                Log("adding botnet to total enemies left. currently there is: " + botnet.Length.ToString());
            }

            //keep this here first to debug - amanda
            // if(botnet == null){
            //     Log("hello botnet is null");
            // } else if (botnet.Length == 0){
            //     Log("hi this is currently 0 for botnet length");
            // } else if (botnet != null){
            //     Log("hi this is botnet length that is not null " + botnet.Length.ToString());
            // }
            
            //logic here will have to change in the future when i got player information. 
            //need detect if player destroy 15 botnets.
            if(totalenemiesleft <= 0 && waveEnemiesLeftToSpawn <= 0){
                enemiesLeft = 0;

                isActive = false;
                playInGameSound = false;
                spawningAllowed = false;
                StopAllOtherAudio();
                Log("=== Wave Complete ===");

            } else {
                enemiesLeft = totalenemiesleft;
            }
        }
        
        #region environment

        private void EnvironmentReset(){
            WallSetup_DisableActiveWalls();
            WallSetup_InactiveWalls();
        }
        
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
            switch(wallIndex){
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
            if(wallInactiveEntities != null){
                Log(string.Concat("Enabling ", wallInactiveEntities.Length.ToString(), " inactive wall entities"));
                foreach(Entity wall in wallInactiveEntities){
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
            if(wallActiveEntities != null){
                foreach (Entity wall in wallActiveEntities){
                    if (wall.EntityID != INVALID_ENTITY) // Extra safety check
                    {
                        //Log(string.Concat("HI PLS WORK DISABLE ACTIVE WALLS - EntityID: ", wall.EntityID.ToString()));
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

        private void PlayInGameSounds(){
            //Log("Playbgm hehe");
            spawnmanagerID = InternalCalls.Scene_FindEntityByName("Spawn Manager");
            if (spawnmanagerID != INVALID_ENTITY)
            {
                Log("YAY FOUND IT IT'S " + spawnmanagerID.ToString());
            }
            InternalCalls.Audio_Play(spawnmanagerID);
            PlayAllOtherAudio();

            playInGameSound = true;
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

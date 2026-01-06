using System;
using System.Collections.Generic;
using System.Collections;
using Engine;

using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Audio;
using static Engine.Transform;
using static Engine.Logger;

namespace Game
{
    // Simple struct to hold spawn point transform data
    public struct SpawnPointData
    {
        public Vector3 position;
        public Quat rotation;

        public SpawnPointData(Vector3 pos, Quat rot)
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

        private const uint INVALID_ENTITY = 0xffffffffu;

        // Simple pseudo-random number generator
        private uint rngSeed;

        // Time tracking (since we don't have Time.time)
        [SerializeField] private float elapsedTime = 0f;
        [SerializeField] private int botnetSpawned = 0;
        [SerializeField] private int loveletterSpawned = 0;
        [SerializeField] private int botnetkilled = 0;

        private string alliesambience = "Flotilla_Gunship_Ambient.wav";
        private string coreambience = "Core_Ambient.wav";
        private string loveletterwarning = "Loveletter_Warp_Warning.wav";

        //event names
        private const string BOTNETKILLED = "BotnetDeath";
        private const string LOSTDETECTED = "ChangeToLost";
        private const string GAMESTARTDETECTED = "StartingGame";

        private const string WINCAM = "WinCamera";
        private const string LOSECAM = "LoseCamera";
        private const string MENUCAM = "MainMenuCamera";

        private const int TARGETBOTNETKILLED = 15;

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

            // Setup walls for initial state
            EnvironmentReset();

            EntityAddAudio((uint)EntityID);

            Event.Subscribe(BOTNETKILLED, UpdateBotnetKilled);
            Event.Subscribe(LOSTDETECTED, DeactiveSpawnCondition);
            Event.Subscribe(GAMESTARTDETECTED, ActivateSpawnManager);

            StopInGameSounds();

            LogMessage("EnemySpawnManager initialized");
        }

        public override void OnUpdate(float deltaTime)
        {
            //Cheat code
            // if (Input.IsKeyPressed(KeyCode.U) && !isActive)
            // {
            //     // isActive = true;
            //     // stopmainsound();

            //     // // deactivate all active wall and activate all inactive
            //     // EnvironmentReset();

            //     // // set the no. of enemies to spawn here
            //     // // in the function it also activates the wall we spawning enemies from
            //     // SetupEnemySpawning();

            //     // // play the ingame sounds
            //     // if (!playInGameSound)
            //     // {
            //     //     PlayInGameSounds();
            //     // }

            //     // botnetkilled = 0;
            //     ActiveSpawn();
            //     LogMessage("Cheatcode to start spawnmanager again when deactivate from any cam!");
            // }

            // check if the game has started
            if (!isActive)
            {
                return;
            }

            // update elapsed time
            elapsedTime += deltaTime;
            float currentTime = elapsedTime;

            // Spawning LogMessageic
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

                if (infiniteSpawning)
                {
                    SetupInfiniteBotnetSpawning();
                }
            }

            CheckBotnetKilled();

            CheckForEnemiesLeft();
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(BOTNETKILLED, UpdateBotnetKilled);
            Event.Unsubscribe(LOSTDETECTED, DeactiveSpawnCondition);
            Event.Unsubscribe(GAMESTARTDETECTED, ActivateSpawnManager);
        }

        #region setup

        private void InitializeSpawnPoints()
        {
            LogMessage("=== Initializing Spawn Points ===");

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

            LogMessage("SpawnManager is initializing spawnpoints for all wall");
        }

        private void RegisterSpawnPointsForWall(string wallName, List<SpawnPointData> targetList, string[] spawnPointNames)
        {
            LogMessage(string.Concat("Registering spawn points for Wall ", wallName));

            int successCount = 0;
            int totalCount = spawnPointNames.Length;

            foreach (string spawnPointName in spawnPointNames)
            {
                // Find the spawn point entity by name
                uint entityID = SceneFindEntityByName(spawnPointName);

                if (entityID != 0)
                {
                    Vector3 spawnPosition;
                    Quat spawnRotation;

                    try
                    {
                        // Position from internal call
                        spawnPosition = GetPosition(entityID);
                        // Rotation as QUAT from your Transform API
                        spawnRotation = Transform.GetRotation(entityID);

                        // Create spawn point data and add to list
                        SpawnPointData spawnData = new SpawnPointData(spawnPosition, spawnRotation);
                        targetList.Add(spawnData);
                        successCount++;

                        LogMessage(string.Concat(
                            spawnPointName, " registered at position (",
                            spawnPosition.X.ToString(), ", ", spawnPosition.Y.ToString(), ", ", spawnPosition.Z.ToString(),
                            ") rotation (", spawnRotation.X.ToString(), ", ", spawnRotation.Y.ToString(), ", ",
                            spawnRotation.Z.ToString(), ", ", spawnRotation.W.ToString(), ")"));
                    }
                    catch (Exception ex)
                    {
                        LogMessage(string.Concat("WARNING: Failed to get transform for ", spawnPointName, " - ", ex.Message));
                    }
                }
                else
                {
                    LogMessage(string.Concat("WARNING: Spawn point not found or invalid: ", spawnPointName,
                        " (EntityID: ", entityID.ToString(), ")"));
                }
            }

            LogMessage(string.Concat("Wall ", wallName, " registered: ", successCount.ToString(),
                "/", totalCount.ToString(), " spawn points"));
        }

        private void InitializeWalls()
        {
            // Helper to create entity list with validation
            wallAActiveEntities = CreateValidEntityArray(new string[]{
                "WallA1", "WallA2", "WallA3", "WallA4", "WallA5", "WallA6", "WallA_LogMessageo", "WallA_Void"
            });

            wallBActiveEntities = CreateValidEntityArray(new string[]{
                "WallB1", "WallB2", "WallB3", "WallB4", "WallB5", "WallB6", "WallB_LogMessageo", "WallB_Void"
            });

            wallCActiveEntities = CreateValidEntityArray(new string[]{
                "WallC1", "WallC2", "WallC3", "WallC4", "WallC5", "WallC6", "WallC_LogMessageo", "WallC_Void"
            });

            wallDActiveEntities = CreateValidEntityArray(new string[]{
                "WallD1", "WallD2", "WallD3", "WallD4", "WallD5", "WallD6", "WallD_LogMessageo", "WallD_Void"
            });

            wallEActiveEntities = CreateValidEntityArray(new string[]{
                "WallE1", "WallE2", "WallE3", "WallE4", "WallE5", "WallE6", "WallE_LogMessageo", "WallE_Void"
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

            LogMessage(string.Concat("Initialize wall entities - found ", wallActiveEntities.Count.ToString(), " active walls"));
            LogMessage(string.Concat("Found ", wallInactiveEntities.Length.ToString(), " inactive walls"));
        }

        private void SetupEnemySpawning()
        {
            LogMessage("SpawnManager - Setting up enemy spawning");

            E005_loveletter = 0;
            E004_botnet = 30;

            loveletterRoutes = new string[] { "A1" };

            activeSpawnPoints = spawnPointsA;

            // Calculate total enemies to spawn
            waveEnemiesLeftToSpawn = E005_loveletter + E004_botnet;

            LogMessage(string.Concat("Total enemies to spawn: ", waveEnemiesLeftToSpawn.ToString()));

            // Update walls based on loveletter routes
            WallChange(loveletterRoutes);

            spawningAllowed = true;
            spawnRateNext = elapsedTime;
        }

        private void SetupInfiniteBotnetSpawning()
        {
            LogMessage("SpawnManager - Setting up infinite botnet spawning");

            E004_botnet += 1;

            // Calculate total enemies to spawn
            waveEnemiesLeftToSpawn = E005_loveletter + E004_botnet;

            LogMessage(string.Concat("Total enemies to spawn: ", waveEnemiesLeftToSpawn.ToString()));
        }

        #endregion

        #region Spawning

        private void SpawnPresetEnemy()
        {
            if (elapsedTime < spawnRateNext)
            {
                isSpawning = false;
                return;
            }

            isSpawning = true;
            spawnRateNext = elapsedTime + spawnRate;

            // Determine which enemy type to spawn based on remaining counts
            int enemyType = DetermineEnemyTypeToSpawn();

            if (enemyType == -1)
            {
                LogMessage("No more enemies to spawn");
                isSpawning = false;
                return;
            }

            // Spawn the enemy
            SpawnEnemy(enemyType);

            waveEnemiesLeftToSpawn = E004_botnet + E005_loveletter;

            LogMessage(string.Concat("Spawned enemy type ", enemyType.ToString(),
                " - Remaining: ", waveEnemiesLeftToSpawn.ToString()));
            LogMessage("spawned botnet count: " + botnetSpawned.ToString());
            LogMessage("spawned loveletter count: " + loveletterSpawned.ToString());
        }

        private int DetermineEnemyTypeToSpawn()
        {
            // Priority: Loveletter > Botnet
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

            return -1; // No enemies left to spawn
        }

        private void SpawnEnemy(int enemyType)
        {
            // Get random spawn point
            if (activeSpawnPoints == null || activeSpawnPoints.Count == 0)
            {
                isSpawning = false;
                LogMessage("ERROR: No active spawn points!");
                return;
            }

            // Create enemy from prefab
            string prefabpath = "Sources/Prefabs/" + enemyPrefabNames[enemyType] + ".prefab";

            if (enemyType == 1)
            {
                SpawnLoveLetter(prefabpath);
                return;
            }

            // comment this part if u want to test more than 20 botnet.
            // if (botnetSpawned >= 20)
            // {
            //     if (enemyType == 0)
            //     {
            //         return;
            //     }
            // }

            int spawnIndex = GetRandomInt(0, activeSpawnPoints.Count);
            SpawnPointData spawnPoint = activeSpawnPoints[spawnIndex];

            // Get spawn position and rotation from the spawn point
            Vector3 spawnPos = spawnPoint.position;
            Quat spawnRot = spawnPoint.rotation; // QUAT
            Vector3 spawnScale = new Vector3(0.006f, 0.006f, 0.006f);

            //PlayEnemySpawnSound(enemyType, spawnPos, spawnRot, spawnScale);

            uint enemyID = PrefabInstantiateWithTransform(
                prefabpath,
                ref spawnPos,
                ref spawnRot,
                ref spawnScale,
                false
            );

            if (enemyID == 0)
            {
                LogMessage("INVALID ID FOR SPAWNING");
            }
            else
            {
                if (enemyType == 0)
                {
                    botnetSpawned++;
                }
                EntityAddScript(enemyID, "Game.Botnet");
                LogMessage(string.Concat(
                    "Spawn type: ", enemyType.ToString(), " at position ", spawnPos.X.ToString(), ", ",
                    spawnPos.Y.ToString(), ", ", spawnPos.Z.ToString(), " and at rotation ",
                    spawnRot.X.ToString(), ", ", spawnRot.Y.ToString(), ", ",
                    spawnRot.Z.ToString(), ", ", spawnRot.W.ToString(), ")"));
            }

            LogMessage(string.Concat("Spawn ", prefabpath, " at position (",
                spawnPos.X.ToString(), ", ", spawnPos.Y.ToString(), ", ", spawnPos.Z.ToString(), ")"));
        }

        private void SpawnLoveLetter(string prefabpath)
        {
            int spawnIndex = GetRandomInt(0, loveletterRoutes.Length);
            string spawnPointName = loveletterRoutes[spawnIndex];

            // Find the spawn point entity by name
            uint spawnID = SceneFindEntityByName(spawnPointName);

            LogMessage("Spawn point name is: " + spawnPointName);

            // Get transform data directly using native calls
            Vector3 spawnPosition;
            Quat spawnRotation;

            spawnPosition = GetPosition(spawnID);
            spawnRotation = Transform.GetRotation(spawnID);

            Vector3 spawnScale = new Vector3(0.002f, 0.002f, 0.002f);

            // PLAY WARNING AUDIO ONCE
            EntityAddAudio(spawnID);
            AudioSetFile(spawnID, loveletterwarning);
            AudioSetLoop(spawnID, false);
            AudioSetIs3D(spawnID, true);
            AudioSetMinDistance(spawnID, 22.42f);
            AudioSetMaxDistance(spawnID, 300.87f);
            //AudioPlay(spawnID);

            uint enemyID = PrefabInstantiateWithTransform(
                prefabpath,
                ref spawnPosition,
                ref spawnRotation,
                ref spawnScale,
                true
            );

            if (enemyID == 0)
            {
                LogMessage("LOVELETTERSPAWN FAIL");
            }
            else
            {
                loveletterSpawned++;
                LogMessage(string.Concat(
                    "Spawn type: loveletter at position ", spawnPosition.X.ToString(), ", ",
                    spawnPosition.Y.ToString(), ", ", spawnPosition.Z.ToString(),
                    " and at rotation ", spawnRotation.X.ToString(), ", ",
                    spawnRotation.Y.ToString(), ", ", spawnRotation.Z.ToString(), ", ",
                    spawnRotation.W.ToString(), ")"));
            }
        }

        #endregion

        #region eventhandler

        private void UpdateBotnetKilled(string eventName, string payload)
        {
            if (!int.TryParse(payload, out int count))
            {
                return;
            }

            botnetkilled += count;

            LogMessage("SpawnManager detected botnet is killed current count: " + botnetkilled.ToString());
        }

        private void DeactiveSpawnCondition(string eventName, string payload)
        {

            string name = payload;

            if (name == WINCAM || name == LOSECAM || name == MENUCAM)
            {
                DisableSpawn();
                LogMessage("Detect win, lose, maincam so disabling spawn");
            }
            else
            {
                LogMessage("hey its not win or lose or mainmenu cam soo im not deactivating sadly");
            }
        }

        private void ActivateSpawnManager(string eventName, string payload)
        {
            if (!bool.TryParse(payload, out bool camactive))
                return;

            if (camactive && !isActive)
            {
                ActiveSpawn();
                LogMessage("detect cam is active for event " + eventName + "and curr spawn manager is just activated");
            }
            else
            {
                LogMessage("game cam is not active");
            }
        }

        #endregion

        private void ActiveSpawn()
        {
            LogMessage("Activating Spawn/Setting up Spawn");
            isActive = true;
            stopmainsound();

            // deactivate all active wall and activate all inactive
            EnvironmentReset();

            // set the no. of enemies to spawn here
            // in the function it also activates the wall we spawning enemies from
            SetupEnemySpawning();

            // play the ingame sounds
            if (!playInGameSound)
            {
                PlayInGameSounds();
            }

            botnetkilled = 0;

            //feel free to add any event publish here for when spawn start
            Event.Publish("SMActivated", EntityID.ToString());
        }

        private void DisableSpawn()
        {
            LogMessage("Disabling spawn rn");

            enemiesLeft = 0;

            isActive = false;
            playInGameSound = false;
            spawningAllowed = false;
            botnetkilled = 0;
            StopAllOtherAudio();
            EnvironmentReset();

            //this event is being used by botnet !
            Event.Publish("DisablingSpawn", isActive.ToString());

            //feel free to add more event for disabling spawn.
            Event.Publish("SMDeactivated", EntityID.ToString());
        }

        private void CheckBotnetKilled()
        {
            if (botnetkilled >= TARGETBOTNETKILLED)
            {
                DisableSpawn();
                LogMessage("=== YIPPEE U KILLED ENOUGH ===");

                // publish the win event to be integrated with the win screen 
                // this event is use by UI Spawn Manager
                Event.Publish("PlayerWin", botnetkilled.ToString());
            }
        }

        private void CheckForEnemiesLeft()
        {
            if (botnetkilled >= TARGETBOTNETKILLED)
            {
                return;
            }

            uint[] loveletter = SceneFindEntitiesByTag("loveletter");
            uint[] botnet = SceneFindEntitiesByTag("botnet");

            int totalenemiesleft = 0;

            if (loveletter != null && loveletter.Length != 0)
            {
                totalenemiesleft += loveletter.Length;
                LogMessage("adding loveletter to total enemies left. currently there is: " + loveletter.Length.ToString());
            }

            if (botnet != null && botnet.Length != 0)
            {
                totalenemiesleft += botnet.Length;
                //LogMessage("adding botnet to total enemies left. currently there is: " + botnet.Length.ToString());
            }

            if (totalenemiesleft <= 0 && waveEnemiesLeftToSpawn <= 0)
            {
                DisableSpawn();
                LogMessage("=== Wave Complete ===");
            }
            else
            {
                enemiesLeft = totalenemiesleft;
            }
        }

        #region environment

        private void EnvironmentReset()
        {
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
                    case "A1":
                    case "A2": WallEnable(0); break;
                    case "B1":
                    case "B2": WallEnable(1); break;
                    case "C1": WallEnable(2); break;
                    case "D1":
                    case "D2": WallEnable(3); break;
                    case "E1": WallEnable(4); break;
                }
            }
        }

        private void WallEnable(int wallIndex)
        {
            // Enable active wall, disable inactive wall
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

            LogMessage("Enable wall " + wallIndex);
        }

        private void WallSetup_ActiveWallVisibility(Entity[] entities, int wallIndex)
        {
            // activate the selected walls to spawn
            foreach (Entity entity in entities)
            {
                if (entity.EntityID != 0) // Extra safety check
                {
                    SetVisible((uint)entity.EntityID, true);
                }
                else
                {
                    LogMessage("ERROR: Skipping invalid entity (ID=0) in WallSetup_ActiveWallVisibility");
                }
            }

            // deactivate the inactive wall of the walls that are spawning
            if (wallInactiveEntities != null && wallIndex < wallInactiveEntities.Length)
            {
                uint id = wallInactiveEntities[wallIndex].EntityID;
                if (id != 0) // Extra safety check
                {
                    SetVisible(id, false);
                }
                else
                {
                    LogMessage(string.Concat("ERROR: Invalid inactive wall entity at index ", wallIndex.ToString()));
                }
            }
        }

        private void WallSetup_InactiveWalls()
        {
            // Enable all inactive walls
            if (wallInactiveEntities != null)
            {
                LogMessage(string.Concat("Enabling ", wallInactiveEntities.Length.ToString(), " inactive wall entities"));
                foreach (Entity wall in wallInactiveEntities)
                {
                    if (wall.EntityID != 0)
                    {
                        SetVisible((uint)wall.EntityID, true);
                    }
                }
                LogMessage("All inactive walls enabled");
            }
        }

        private void WallSetup_DisableActiveWalls()
        {
            // Disable all active walls
            if (wallActiveEntities != null)
            {
                foreach (Entity wall in wallActiveEntities)
                {
                    if (wall.EntityID != INVALID_ENTITY) // Extra safety check
                    {
                        SetVisible((uint)wall.EntityID, false);
                    }
                    else
                    {
                        LogMessage("ERROR: Skipping invalid entity (ID=0) in wallActiveEntities");
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
            rngSeed = (1103515245u * rngSeed + 12345u) & 0x7fffffffu;
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
                uint entityID = SceneFindEntityByName(name);
                if (entityID != 0) // Only add valid entities
                {
                    validEntities.Add(new Entity(entityID));
                    LogMessage(string.Concat("Found entity: ", name, " (ID: ", entityID.ToString(), ")"));
                }
                else
                {
                    LogMessage(string.Concat("WARNING: Entity not found: ", name));
                }
            }

            LogMessage(string.Concat("CreateValidEntityArray: ", validEntities.Count.ToString(),
                " / ", entityNames.Length.ToString(), " entities found"));
            return validEntities.ToArray();
        }

        private void PlayInGameSounds()
        {
            spawnmanagerID = SceneFindEntityByName("Spawn Manager");
            if (spawnmanagerID != INVALID_ENTITY)
            {
                LogMessage("Spawn Manager entity ID: " + spawnmanagerID.ToString());
            }
            AudioPlay(spawnmanagerID);
            PlayAllOtherGameAudio();

            playInGameSound = true;
        }

        private void StopInGameSounds()
        {
            spawnmanagerID = SceneFindEntityByName("Spawn Manager");
            if (spawnmanagerID != INVALID_ENTITY)
            {
                LogMessage("Spawn Manager entity ID: " + spawnmanagerID.ToString());
            }
            AudioStop(spawnmanagerID);
            StopAllOtherGameAudio();

            playInGameSound = false;
        }

        // FOR FUTURE PURPOSE
        private void PauseBGM()
        {
            AudioPause((uint)EntityID);
        }

        private void StopBGM()
        {
            AudioStop((uint)EntityID);
            StopAllOtherAudio();
        }

        #endregion

        #region other sound

        private void PlayAllOtherGameAudio()
        {
            uint[] Allies = SceneFindEntitiesByTag("ALLIES");

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
                        EntityAddAudio(Allies[i]);
                        AudioSetFile(Allies[i], alliesambience);
                        AudioSetLoop(Allies[i], true);
                        AudioSetIs3D(Allies[i], true);
                        AudioSetMinDistance(Allies[i], 5.42f);
                        AudioSetMaxDistance(Allies[i], 152.45f);

                        AudioPlay(Allies[i]);
                        LogMessage("SpawnManager: Playing ally audio right now - only gunship ambience");
                    }
                }
            }

            uint coreID = SceneFindEntityByName("Core");

            if (coreID != INVALID_ENTITY)
            {
                EntityAddAudio(coreID);
                AudioSetFile(coreID, coreambience);
                AudioSetLoop(coreID, true);
                AudioSetIs3D(coreID, true);
                AudioSetMinDistance(coreID, 52.42f);
                AudioSetMaxDistance(coreID, 527.87f);

                AudioPlay(coreID);
                LogMessage("SpawnManager: Playing core ambience now through core");
            }
            else
            {
                LogError("SpawnManager: Cannot find Core");
            }
        }

        private void StopAllOtherGameAudio()
        {
            uint[] Allies = SceneFindEntitiesByTag("ALLIES");

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
                        EntityAddAudio(Allies[i]);
                        AudioSetFile(Allies[i], alliesambience);

                        AudioStop(Allies[i]);
                        LogMessage("SpawnManager: stopping gunship ambience");
                    }
                }
            }

            uint coreID = SceneFindEntityByName("Core");

            if (coreID != INVALID_ENTITY)
            {
                EntityAddAudio(coreID);
                AudioSetFile(coreID, coreambience);

                AudioStop(coreID);
                LogMessage("SpawnManager: Stopping core ambience");
            }
            else
            {
                LogError("SpawnManager: Cannot find Core");
            }

            uint emplacementID = SceneFindEntityByName("EMPLACEMENT");

            if (emplacementID != INVALID_ENTITY)
            {
                EntityAddAudio(emplacementID);
                //AudioSetFile(emplacementID, coreambience);

                AudioStop(emplacementID);
                LogMessage("SpawnManager: Stop emplacement from playing");
            }
            else
            {
                LogError("SpawnManager: Cannot find Emplacement");
            }

            uint a1ID = SceneFindEntityByName("A1");

            if (a1ID != INVALID_ENTITY)
            {
                EntityAddAudio(a1ID);
                //AudioSetFile(emplacementID, coreambience);

                AudioStop(a1ID);
                LogMessage("SpawnManager: Stop A1 from playing");
            }
            else
            {
                LogError("SpawnManager: Cannot find A1");
            }
        }

        private void StopAllOtherAudio()
        {
            AudioManager.StopAll();
        }

        private void stopmainsound()
        {
            uint officeambi = SceneFindEntityByName("office_ambience");

            if (officeambi != INVALID_ENTITY)
            {
                AudioSetLoop(officeambi, false);
                AudioStop(officeambi);
                LogMessage("SpawnManager: Stopping office ambience");
            }
            else
            {
                LogError("SpawnManager: Cannot find office ambience");
            }

            uint roomambi = SceneFindEntityByName("room_ambience");

            if (roomambi != INVALID_ENTITY)
            {
                AudioSetLoop(roomambi, false);
                AudioStop(roomambi);
                LogMessage("SpawnManager: Stopping room ambience");
            }
            else
            {
                LogError("SpawnManager: Cannot find room ambience");
            }
        }

        private void startmainsound()
        {
            uint officeambi = SceneFindEntityByName("office_ambience");

            if (officeambi != INVALID_ENTITY)
            {
                AudioSetLoop(officeambi, true);
                AudioPlay(officeambi);
                LogMessage("SpawnManager: Playing office ambience");
            }
            else
            {
                LogError("SpawnManager: Cannot find office ambience");
            }

            uint roomambi = SceneFindEntityByName("room_ambience");

            if (roomambi != INVALID_ENTITY)
            {
                AudioSetLoop(roomambi, true);
                AudioPlay(roomambi);
                LogMessage("SpawnManager: Playing room ambience");
            }
            else
            {
                LogError("SpawnManager: Cannot find room ambience");
            }
        }

        #endregion

        private void SetVisible(Entity e, bool visible)
        {
            if (e.EntityID == 0 || e.EntityID == INVALID_ENTITY)
                return;

            try
            {
                MeshRenderer mr = e.GetComponent<MeshRenderer>();
                if (mr != null)
                    mr.Visible = visible;
            }
            catch
            {
                // If entity has no MeshRenderer, ignore (or log if you want)
            }
        }

        private void SetVisible(uint entityID, bool visible)
        {
            if (entityID == 0 || entityID == INVALID_ENTITY)
                return;

            SetVisible(new Entity(entityID), visible);
        }

    }
}

using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    public class LoveLetterSpawn : ScriptBehaviour
    {
        // ===== SPAWNER SETTINGS =====
       
        private string loveletterPrefabPath = "Sources/Prefabs/" + "loveletterV3.prefab";
        
        [SerializeField] private string[] spawnWallNames = new string[]
        {
            "Wall1",
            "Wall2", 
            "Wall3"
        };
        
        // ===== SPAWN TIMING =====
        [SerializeField] private float initialDelay = 5.0f;        // Wait before first spawn
        [SerializeField] private float minSpawnInterval = 2.0f;    // Min time between spawns
        [SerializeField] private float maxSpawnInterval = 4.0f;    // Max time between spawns
        
        // ===== SPAWN LIMITS =====
        // IMPORTANT: Increase this number to have more loveletters active at once!
        // Set to 10 for more constant pressure
        [SerializeField] private int maxActiveLetters = 1;        // Max letters alive at once
        [SerializeField] private int maxTotalSpawns = -1;          // -1 = infinite spawns
        
        // ===== SPAWN OFFSET =====
        [SerializeField] private float spawnOffsetFromWall = 200.0f; // Distance in front of wall
        
        // ===== WALL SPAWN AREA =====
        [SerializeField] private bool useWallScale = true;          // Use wall's actual scale for spawn area
        [SerializeField] private float wallScaleMultiplierX = 1.0f; // Multiply wall's X scale by this (if too big/small)
        [SerializeField] private float wallScaleMultiplierY = 1.0f; // Multiply wall's Y scale by this (if too big/small)
        
        // Manual override (only used if useWallScale = false)
        [SerializeField] private float manualWallWidth = 500.0f;
        [SerializeField] private float manualWallHeight = 300.0f;
        [SerializeField] private bool randomizeSpawnPosition = true; // Toggle random spawn within wall bounds
        
        // ===== STATE =====
        private uint[] wallEntityIDs;
        private float spawnTimer = 0.0f;
        private float nextSpawnTime = 0.0f;
        private int activeLetterCount = 0;
        private int totalSpawned = 0;
        private bool isInitialized = false;
        
        private static bool rngSeeded = false;

        public override void OnStart()
        {
            LogMessage("=== LoveLetterSpawner Started ===");
            
            // Seed RNG if not already done
            if (!rngSeeded)
            {
                uint timeSeed = (uint)(System.DateTime.Now.Ticks & 0xFFFFFFFF);
                RNG.Seed(timeSeed);
                rngSeeded = true;
                LogMessage("RNG seeded with: " + timeSeed);
            }
            
            // Find all wall entities
            wallEntityIDs = new uint[spawnWallNames.Length];
            int validWalls = 0;
            
            for (int i = 0; i < spawnWallNames.Length; i++)
            {
                wallEntityIDs[i] = SceneFindEntityByName(spawnWallNames[i]);
                if (wallEntityIDs[i] != 0)
                {
                    validWalls++;
                    LogMessage("Found spawn wall: " + spawnWallNames[i] + " (ID: " + wallEntityIDs[i] + ")");
                }
                else
                {
                    LogWarning("Spawn wall not found: " + spawnWallNames[i]);
                }
            }
            
            if (validWalls == 0)
            {
                LogError("No valid spawn walls found! Spawner disabled.");
                return;
            }
            
            LogMessage("Spawner initialized with " + validWalls + " valid walls");
            LogMessage("Initial delay: " + initialDelay + "s");
            LogMessage("Spawn interval: " + minSpawnInterval + "-" + maxSpawnInterval + "s");
            LogMessage("Max active letters: " + maxActiveLetters);
            LogMessage("Infinite spawning enabled - will spawn continuously until game ends");
            
            // Subscribe to LoveLetter destruction events
            Subscribe("LoveLetterDestroyed", OnLoveLetterDestroyed);
            Subscribe("LoveLetterReachedCore", OnLoveLetterReachedCore);
            
            // Set initial spawn timer
            spawnTimer = initialDelay;
            nextSpawnTime = GetRandomSpawnInterval();
            
            isInitialized = true;
            
            LogMessage("*** SPAWNER READY - Will maintain " + maxActiveLetters + " active loveletters ***");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!isInitialized) return;
            
            // Update spawn timer
            spawnTimer -= deltaTime;
            
            if (spawnTimer <= 0.0f)
            {
                // Check if we can spawn another letter
                if (activeLetterCount < maxActiveLetters)
                {
                    SpawnLoveLetter();
                    
                    // Set next spawn time
                    spawnTimer = nextSpawnTime;
                    nextSpawnTime = GetRandomSpawnInterval();
                    
                    LogMessage(">>> Next spawn in " + spawnTimer.ToString("F1") + " seconds (Active: " + activeLetterCount + "/" + maxActiveLetters + ")");
                }
                else
                {
                    // At max capacity - check again quickly
                    spawnTimer = 0.2f;
                }
            }
        }

        private void SpawnLoveLetter()
        {
            // Select random wall
            int randomWallIndex = GetRandomValidWallIndex();
            if (randomWallIndex < 0)
            {
                LogError("No valid walls available for spawning!");
                return;
            }
            
            uint selectedWallID = wallEntityIDs[randomWallIndex];
            string wallName = spawnWallNames[randomWallIndex];
            
            // Get wall position, rotation, and scale
            Engine.Vector3 wallPos = GetPosition(selectedWallID);
            Engine.Quat wallRot = GetRotation(selectedWallID);
            Engine.Vector3 wallScale = GetScale(selectedWallID);
            
            // Calculate wall dimensions based on scale
            float wallWidth, wallHeight;
            
            if (useWallScale)
            {
                // Use actual wall scale (typically X and Y, but depends on wall orientation)
                // Assuming walls are oriented with width on X and height on Y
                wallWidth = wallScale.X * wallScaleMultiplierX;
                wallHeight = wallScale.Y * wallScaleMultiplierY;
                
                LogMessage("Using wall scale: Width=" + wallWidth.ToString("F1") + ", Height=" + wallHeight.ToString("F1"));
            }
            else
            {
                // Use manual values
                wallWidth = manualWallWidth;
                wallHeight = manualWallHeight;
            }
            
            // Get wall's forward, right, and up vectors
            Engine.Vector3 wallForward = wallRot.Forward;
            Engine.Vector3 wallRight = wallRot.Right;
            Engine.Vector3 wallUp = wallRot.Up;
            
            // Calculate random offset within wall bounds
            float randomX = 0.0f;
            float randomY = 0.0f;
            
            if (randomizeSpawnPosition)
            {
                // Random position within wall area
                // X offset: -wallWidth/2 to +wallWidth/2
                randomX = RNG.RandFloat(-wallWidth * 0.5f, wallWidth * 0.5f);
                // Y offset: -wallHeight/2 to +wallHeight/2
                randomY = RNG.RandFloat(-wallHeight * 0.5f, wallHeight * 0.5f);
            }
            
            // Calculate spawn position
            // Start at wall center, add offset in wall's forward direction, then add random position
            Engine.Vector3 spawnPos = new Engine.Vector3(
                wallPos.X + wallForward.X * spawnOffsetFromWall + wallRight.X * randomX + wallUp.X * randomY,
                wallPos.Y + wallForward.Y * spawnOffsetFromWall + wallRight.Y * randomX + wallUp.Y * randomY,
                wallPos.Z + wallForward.Z * spawnOffsetFromWall + wallRight.Z * randomX + wallUp.Z * randomY
            );
            
            // Spawn the LoveLetter prefab
            uint letterID = PrefabInstantiate(loveletterPrefabPath);
            
            if (letterID == 0)
            {
                LogError("Failed to spawn LoveLetter prefab from: " + loveletterPrefabPath);
                return;
            }
            
            // Set position and rotation (use wall's rotation)
            SetPosition(letterID, ref spawnPos);
            SetRotation(letterID, ref wallRot);
            
            // Increment counters
            activeLetterCount++;
            totalSpawned++;
            
            LogMessage("LOVELETTER SPAWNED #" + totalSpawned);
            LogMessage(" Wall: " + wallName + " (Index: " + randomWallIndex + ")");
            if (randomizeSpawnPosition)
            {
                LogMessage(" Random Offset: X=" + randomX.ToString("F1") + ", Y=" + randomY.ToString("F1"));
            }
            LogMessage(" Position: (" + spawnPos.X.ToString("F1") + ", " + spawnPos.Y.ToString("F1") + ", " + spawnPos.Z.ToString("F1") + ")");
            LogMessage(" Active: " + activeLetterCount + "/" + maxActiveLetters);
   
        }

        private int GetRandomValidWallIndex()
        {
            // Create list of valid wall indices
            int validCount = 0;
            for (int i = 0; i < wallEntityIDs.Length; i++)
            {
                if (wallEntityIDs[i] != 0)
                    validCount++;
            }
            
            if (validCount == 0)
                return -1;
            
            // Build array of valid indices
            int[] validIndices = new int[validCount];
            int index = 0;
            for (int i = 0; i < wallEntityIDs.Length; i++)
            {
                if (wallEntityIDs[i] != 0)
                {
                    validIndices[index] = i;
                    index++;
                }
            }
            
            // Select random valid index
            int randomIndex = RNG.RandInt(0, validCount - 1);
            return validIndices[randomIndex];
        }

        private float GetRandomSpawnInterval()
        {
            // Generate random float between min and max interval
            return RNG.RandFloat(minSpawnInterval, maxSpawnInterval);
        }

        private void OnLoveLetterDestroyed(string eventName, string payload)
        {
            // A LoveLetter was destroyed (either killed or reached core)
            activeLetterCount--;
            if (activeLetterCount < 0) activeLetterCount = 0;
            
            LogMessage(">>> LoveLetter destroyed. Active count now: " + activeLetterCount + "/" + maxActiveLetters + " (Total spawned: " + totalSpawned + ")");
        }

        private void OnLoveLetterReachedCore(string eventName, string payload)
        {
            // Additional handling if needed when letter reaches core
            LogWarning("!!! A LoveLetter reached the core! !!!");
        }

        public override void OnDestroy()
        {
            Unsubscribe("LoveLetterDestroyed", OnLoveLetterDestroyed);
            Unsubscribe("LoveLetterReachedCore", OnLoveLetterReachedCore);
            LogMessage("=== LoveLetterSpawner Destroyed (Total spawned: " + totalSpawned + ") ===");
        }
    }
}
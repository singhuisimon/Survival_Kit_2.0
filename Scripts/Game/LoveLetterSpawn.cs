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
       
        private string loveletterPrefabPath = "Sources/Prefabs/" + "lovelettertest.prefab";
        
        [SerializeField] private string[] spawnWallNames = new string[]
        {
            "Wall1",
            "Wall2", 
            "Wall3"
        };
        
        // ===== SPAWN TIMING =====
        [SerializeField] private float initialDelay = 5.0f;        // Wait before first spawn
        [SerializeField] private float minSpawnInterval = 8.0f;    // Min time between spawns
        [SerializeField] private float maxSpawnInterval = 15.0f;   // Max time between spawns
        
        // ===== SPAWN LIMITS =====
        [SerializeField] private int maxActiveLetters = 3;         // Max letters alive at once
        // CHANGED: Set to -1 for infinite spawning until game ends
        [SerializeField] private int maxTotalSpawns = -1;          // -1 = infinite spawns
        
        // ===== SPAWN OFFSET =====
        [SerializeField] private float spawnOffsetFromWall = 50.0f; // Distance in front of wall
        
        // ===== STATE =====
        private uint[] wallEntityIDs;
        private float spawnTimer = 0.0f;
        private float nextSpawnTime = 0.0f;
        private int activeLetterCount = 0;
        private int totalSpawned = 0;
        private bool isInitialized = false;
        // CHANGED: Removed hasStarted flag that was preventing spawning
        
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
            if (maxTotalSpawns < 0)
            {
                LogMessage("Infinite spawning enabled - will spawn until game ends");
            }
            else
            {
                LogMessage("Max total spawns: " + maxTotalSpawns);
            }
            
            // Subscribe to LoveLetter destruction events
            Subscribe("LoveLetterDestroyed", OnLoveLetterDestroyed);
            Subscribe("LoveLetterReachedCore", OnLoveLetterReachedCore);
            
            // Set initial spawn timer
            spawnTimer = initialDelay;
            nextSpawnTime = GetRandomSpawnInterval();
            
            isInitialized = true;
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!isInitialized) return;
            
            // Check if we've reached max total spawns (only if limit is set)
            if (maxTotalSpawns >= 0 && totalSpawned >= maxTotalSpawns)
            {
                // Don't spawn anymore
                return;
            }
            
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
                    
                    LogMessage("Next spawn in " + spawnTimer + " seconds (Active: " + activeLetterCount + "/" + maxActiveLetters + ")");
                }
                else
                {
                    // CHANGED: Reduced wait time when at max capacity for more responsive spawning
                    spawnTimer = 0.5f;
                    // Optional: Log less frequently to reduce spam
                    // LogMessage("At max active letters (" + maxActiveLetters + "), waiting for destruction...");
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
            
            // Get wall position and rotation
            Engine.Vector3 wallPos = GetPosition(selectedWallID);
            Engine.Quat wallRot = GetRotation(selectedWallID);
            
            // Calculate spawn position (offset from wall based on wall's forward direction)
            Engine.Vector3 wallForward = wallRot.Forward;
            
            Engine.Vector3 spawnPos = new Engine.Vector3(
                wallPos.X + wallForward.X * spawnOffsetFromWall,
                wallPos.Y + wallForward.Y * spawnOffsetFromWall,
                wallPos.Z + wallForward.Z * spawnOffsetFromWall
            );
            
            // Spawn the LoveLetter prefab
            uint letterID = PrefabInstantiate(loveletterPrefabPath);
            
            if (letterID == 0)
            {
                LogError("Failed to spawn LoveLetter prefab from: " + loveletterPrefabPath);
                return;
            }
            
            // Set position and rotation
            SetPosition(letterID, ref spawnPos);
            SetRotation(letterID, ref wallRot);
            
            // Increment counters
            activeLetterCount++;
            totalSpawned++;
            
            LogMessage("=== LoveLetter Spawned ===");
            LogMessage("Spawn #" + totalSpawned);
            LogMessage("From wall: " + wallName);
            LogMessage("Position: (" + spawnPos.X + ", " + spawnPos.Y + ", " + spawnPos.Z + ")");
            LogMessage("Active letters: " + activeLetterCount + "/" + maxActiveLetters);
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
            
            LogMessage("LoveLetter destroyed. Active count: " + activeLetterCount + "/" + maxActiveLetters);
        }

        private void OnLoveLetterReachedCore(string eventName, string payload)
        {
            // Additional handling if needed when letter reaches core
            LogWarning("A LoveLetter reached the core!");
        }

        public override void OnDestroy()
        {
            Unsubscribe("LoveLetterDestroyed", OnLoveLetterDestroyed);
            Unsubscribe("LoveLetterReachedCore", OnLoveLetterReachedCore);
            LogMessage("=== LoveLetterSpawner Destroyed ===");
        }
    }
}
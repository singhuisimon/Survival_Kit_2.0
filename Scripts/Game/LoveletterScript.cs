using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Quat;
using static Engine.SimpleMath;
using static Engine.Tag;
using static Engine.Prefab;

namespace Game
{
    public class LoveLetterScript : ScriptBehaviour
    {
        // ===== NAME OF ENTITY =====
        [SerializeField] private string loveletterEntity = "loveletter";
        private uint loveletterEntityID = 0;

        // ===== CORE TAG =====
        [SerializeField] private string coreTag = "SEMICONDUCTOR";
        private uint selectedCoreEntityID = 0;

        // ===== BULLET TAG ======
        private const string TAG_PRIMARY_BULLET = "PrimaryBullet";
        private const string TAG_SECONDARY_BULLET = "PrimaryUltBullet";

        // ===== PREFAB =====
        [SerializeField] private string deathPrefab = "Sources/Prefabs/Logic_bomb_Explosion.prefab";
        [SerializeField] private string hitmarkerAudioPrefab = "Sources/Prefabs/audio_hitmarker.prefab";

        // ===== CORE DIMENSIONS =====
        [SerializeField] private float coreHalfSizeX = 37.5f;
        [SerializeField] private float coreHalfSizeY = 37.5f;
        [SerializeField] private float coreHalfSizeZ = 37.5f;
        
        [SerializeField] private float stopDistanceFromSurface = 200.0f;

        // ===== MOVEMENT SETTING ===== 
        [SerializeField] private float moveSpeed = 100.0f;
        [SerializeField] private float startDelay = 2.0f;
        [SerializeField] private float waitTimeAtSurface = 20.0f;

        // ===== SIMPLE HEALTH SYSTEM =====
        [SerializeField] private float maxHealth = 100.0f;
        [SerializeField] private float currentHealth = 100.0f;
        private bool isDead = false;

        // ===== MOVEMENT STATE =====
        private bool isMoving = false;
        private bool isWaitingAtSurface = false;
        private float delayTimer = 0.0f;
        private float waitTimer = 0.0f;
        
        // Manual position tracking to bypass physics
        private Engine.Vector3 currentPosition;
        private float totalDistanceTraveled = 0.0f;

        // ===== TARGET POSITIONS =====
        private Engine.Vector3 corePosition;
        private Engine.Vector3 startPosition;
        private Engine.Vector3 targetSurfaceCenter;
        private Engine.Vector3 directionNormalized;
        private float totalDistance = 0.0f;
        private bool targetCalculated = false;

        // ===== RNG =====
        private static uint s_RngState = 0x12345678u;
        private static bool rngSeeded = false;

        // ===== EVENTS =====
        //private const string EVENT_BULLET_HIT = "BulletHit";
        private string EVENT_BULLET_HIT = "Damage:";
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        // ==== OTHER VALUES =====
        private const uint INVALID_ENTITY = 0xffffffffu;

        public override void OnStart()
        {
            LogMessage("///////////////////////////Start of the Loveletter Script");
            loveletterEntityID = SceneFindEntityByName(loveletterEntity);

            LogMessage("=== LoveLetter Started ===");
            LogMessage("LoveLetterEntity: " + loveletterEntityID);
            LogMessage("Max Health: " + maxHealth);
            LogMessage("Core Tag: " + coreTag);
            
            if (loveletterEntityID == 0)
            {
                LogError("[loveletterEntity] cannot be found.");
                return;
            }
            
            // Initialize health system
            currentHealth = maxHealth;
            isDead = false;

            // Subscribe to bullet hit event
            //(EVENT_BULLET_HIT, OnBulletHit);

            EVENT_BULLET_HIT += EntityID.ToString();
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            Subscribe(GAMEOVER, OnGameOver);
            Subscribe(GAMEWIN, OnGameOver);
            
            // Seed RNG if not already done
            if (!rngSeeded)
            {
                uint timeSeed = (uint)(System.DateTime.Now.Ticks & 0xFFFFFFFF);
                s_RngState = timeSeed;
                rngSeeded = true;
                LogMessage("RNG seeded with: " + timeSeed);
            }
            
            // Select a random core by tag
            SelectRandomCoreByTag();
            
            if (selectedCoreEntityID != 0)
            {
                // Start movement after delay
                delayTimer = startDelay;
                LogMessage("LoveLetter initialized - waiting " + startDelay + " seconds before movement");
                LogMessage("Selected core ID: " + selectedCoreEntityID);
            }
            else
            {
                LogError("No valid core found with tag '" + coreTag + "'! LoveLetter cannot move.");
            }
        }

        public override void OnUpdate(float deltaTime)
        {
            // Don't update when game is paused
            if (GameState.IsPaused)
                return;

            // Don't update if dead or no core selected
            if (isDead || selectedCoreEntityID == 0) return;

            // Handle spawn delay
            if (delayTimer > 0.0f)
            {
                delayTimer -= deltaTime;
                
                if (delayTimer <= 0.0f)
                {
                    LogMessage("Starting movement to SEMICONDUCTOR core!");
                    FindCoreAndCalculateTarget();
                    isMoving = true;
                }
                return;
            }

            // Wait at surface before self-destruct
            if (isWaitingAtSurface)
            {
                waitTimer -= deltaTime;
                
                if (waitTimer <= 0.0f)
                {
                    LogMessage("Wait time finished - self destructing!");
                    OnReachedCore();
                }
                return;
            }

            // Movement logic using linear interpolation
            if (isMoving && targetCalculated)
            {
                MoveTowardsTargetLinear(deltaTime);
            }
        }

        private void SelectRandomCoreByTag()
        {
            // Find all entities with the SEMICONDUCTOR tag
            uint[] coreEntities = SceneFindEntitiesByTag(coreTag);
            
            if (coreEntities == null || coreEntities.Length == 0)
            {
                LogError("No cores found with tag: " + coreTag);
                selectedCoreEntityID = 0;
                return;
            }
            
            LogMessage("Found " + coreEntities.Length + " cores with tag '" + coreTag + "'");
            
            // Select a random core from the available ones
            if (coreEntities.Length == 1)
            {
                selectedCoreEntityID = coreEntities[0];
                LogMessage("Only one core available, selected ID: " + selectedCoreEntityID);
            }
            else
            {
                int randomIndex = RandomRangeInt(0, coreEntities.Length);
                selectedCoreEntityID = coreEntities[randomIndex];
                LogMessage("Randomly selected core at index " + randomIndex + ", ID: " + selectedCoreEntityID);
            }
        }

        // ===== SIMPLE DAMAGE SYSTEM =====
        private void OnBulletHit(string eventName, string payload)
        {
            // if (isDead || eventName != EVENT_BULLET_HIT)
            //     return;

            // // Parse the entity ID that was hit
            // if (!uint.TryParse(payload, out uint hitId))
            //     return;

            // // Check if this LoveLetter was the one hit
            // if (hitId != loveletterEntityID)
            // {
            //     return;
            // }

            // LogMessage("=== BULLET HIT LOVELETTER ===");
            // LogMessage("  LoveLetter ID: " + loveletterEntityID);
            
            // // Take damage
            // TakeDamage(10.0f);
            if (isDead || eventName != EVENT_BULLET_HIT)
                return;
            float damage = DamageSystem.ParseAmount(payload);
            currentHealth -= damage;
            if (currentHealth < 0.0f)
                currentHealth = 0.0f;

            uint attackerId = DamageSystem.ParseAttackerId(payload);
            if(attackerId != INVALID_ENTITY){
                string attackerTag = TagGetTag(attackerId);
                if(attackerTag == TAG_PRIMARY_BULLET || attackerTag == TAG_SECONDARY_BULLET){
                    //instantiate the hitmarker audio
                    Vector3 spawnPos = GetPosition((uint)EntityID);
                    Quat spawnRot = GetRotation((uint)EntityID);
                    Vector3 scale = new Vector3(0.1f,0.1f,0.1f);
                    uint hitmarkerID = 0;
                    hitmarkerID = PrefabInstantiateWithTransform(hitmarkerAudioPrefab, ref spawnPos, ref spawnRot, ref scale, false);
                    if(hitmarkerID == 0){
                        LogMessage("[LoveletterScript] Player Hit! But hitmarkerID fail to instantiate");
                    }
                }
            }
        }


        private void DestroyLoveLetter()
        {
            if (isDead) return;

            isDead = true;
            LogMessage("=== LOVELETTER DESTROYED ===");
            LogMessage("Health reached 0 - LoveLetter destroyed!");

            // Stop movement
            isMoving = false;
            isWaitingAtSurface = false;
            Publish("LoveLetterKilled", loveletterEntityID.ToString());
            // Publish event for game systems
            Publish("LoveLetterDestroyed", loveletterEntityID.ToString());

            // Spawn in a prefab for death audio
            Vector3 spawnPos = GetPosition((uint)EntityID);
            Quat spawnRot = GetRotation((uint)EntityID);
            Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
            uint explosion = PrefabInstantiateWithTransform(deathPrefab, ref spawnPos, ref spawnRot, ref scale, false);
            if(explosion == 0){
                LogMessage("[LoveletterScript] loveletter explosion entity fail to instantiate");
                return;
            }

            // Destroy the LoveLetter
            SceneDestroyEntity(loveletterEntityID);
        }

        // ===== MOVEMENT SYSTEM =====
        private void FindCoreAndCalculateTarget()
        {
            if (selectedCoreEntityID != 0)
            {
                corePosition = GetPosition(selectedCoreEntityID);
                CalculateTargetSurfaceCenter();
                targetCalculated = true;
                
                // Store start position and calculate direction
                startPosition = GetPosition(loveletterEntityID);
                currentPosition = startPosition;
                totalDistanceTraveled = 0.0f;
                
                // Calculate total distance
                float dx = targetSurfaceCenter.X - startPosition.X;
                float dy = targetSurfaceCenter.Y - startPosition.Y;
                float dz = targetSurfaceCenter.Z - startPosition.Z;
                totalDistance = SimpleMath.Sqrt(dx * dx + dy * dy + dz * dz);
                
                // Normalize direction
                if (totalDistance > 0.001f)
                {
                    directionNormalized = new Engine.Vector3(
                        dx / totalDistance,
                        dy / totalDistance,
                        dz / totalDistance
                    );
                }
                
                LogMessage("Total travel distance: " + totalDistance.ToString("F1") + " units");
                LogMessage("Movement will take: " + (totalDistance / moveSpeed).ToString("F1") + " seconds at " + moveSpeed + " units/sec");
            }
            else
            {
                LogMessage("Selected core entity not found!");
                targetCalculated = false;
            }
        }

        private void CalculateTargetSurfaceCenter()
        {
            Engine.Vector3 loveletterPos = GetPosition(loveletterEntityID);
            
            // Calculate direction from Core to LoveLetter
            Engine.Vector3 direction = new Engine.Vector3(
                loveletterPos.X - corePosition.X,
                loveletterPos.Y - corePosition.Y,
                loveletterPos.Z - corePosition.Z
            );
            
            // Find which axis has the largest absolute value
            float absX = direction.X > 0 ? direction.X : -direction.X;
            float absY = direction.Y > 0 ? direction.Y : -direction.Y;
            float absZ = direction.Z > 0 ? direction.Z : -direction.Z;
            
            // Determine which face is most directly in line
            if (absX >= absY && absX >= absZ)
            {
                // X-axis dominant
                if (direction.X > 0)
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X + coreHalfSizeX + stopDistanceFromSurface,
                        corePosition.Y,
                        corePosition.Z
                    );
                    LogMessage("Targeting RIGHT face center");
                }
                else
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X - coreHalfSizeX - stopDistanceFromSurface,
                        corePosition.Y,
                        corePosition.Z
                    );
                    LogMessage("Targeting LEFT face center");
                }
            }
            else if (absY >= absX && absY >= absZ)
            {
                // Y-axis dominant
                if (direction.Y > 0)
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X,
                        corePosition.Y + coreHalfSizeY + stopDistanceFromSurface,
                        corePosition.Z
                    );
                    LogMessage("Targeting TOP face center");
                }
                else
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X,
                        corePosition.Y - coreHalfSizeY - stopDistanceFromSurface,
                        corePosition.Z
                    );
                    LogMessage("Targeting BOTTOM face center");
                }
            }
            else
            {
                // Z-axis dominant
                if (direction.Z > 0)
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X,
                        corePosition.Y,
                        corePosition.Z + coreHalfSizeZ + stopDistanceFromSurface
                    );
                    LogMessage("Targeting FRONT face center");
                }
                else
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X,
                        corePosition.Y,
                        corePosition.Z - coreHalfSizeZ - stopDistanceFromSurface
                    );
                    LogMessage("Targeting BACK face center");
                }
            }
        }

        private void MoveTowardsTargetLinear(float deltaTime)
        {
            if (loveletterEntityID == 0) return;

            // Calculate how far we should move this frame (constant speed)
            float distanceThisFrame = moveSpeed * deltaTime;
            totalDistanceTraveled += distanceThisFrame;
            
            // Check if we've reached the target
            if (totalDistanceTraveled >= totalDistance)
            {
                // Snap to exact target
                SetPosition(loveletterEntityID, ref targetSurfaceCenter);
                OnReachedTarget();
                return;
            }
            
            // Calculate new position based on total distance traveled
            currentPosition = new Engine.Vector3(
                startPosition.X + directionNormalized.X * totalDistanceTraveled,
                startPosition.Y + directionNormalized.Y * totalDistanceTraveled,
                startPosition.Z + directionNormalized.Z * totalDistanceTraveled
            );
            
            // Apply the calculated position
            SetPosition(loveletterEntityID, ref currentPosition);
        }

        private void OnReachedTarget()
        {
            // Snap to exact target position
            SetPosition(loveletterEntityID, ref targetSurfaceCenter);
            
            // Stop moving and start waiting
            isMoving = false;
            isWaitingAtSurface = true;
            waitTimer = waitTimeAtSurface;
            
            LogMessage("=== REACHED TARGET SURFACE ===");
            LogMessage("Traveled " + totalDistanceTraveled.ToString("F1") + " units in total");
            LogMessage("Waiting " + waitTimeAtSurface + " seconds at surface before self-destruct...");
        }

        private void OnReachedCore()
        {
            LogMessage("///////////////////////////Start of Onreached.");
            if (isDead) return;
            LogMessage("Destroying LoveLetter | EntityID: " + loveletterEntityID);

            isDead = true;
            LogMessage("=== SELF DESTRUCTING AT CORE ===");
        
            // Deal massive damage to the core (200 damage)
            if (selectedCoreEntityID != 0)
            {
                LogMessage("Dealing 200 damage to core ID: " + selectedCoreEntityID);
                
                // Use DamageSystem like WormBullet does
                DamageSystem.DealDamage(selectedCoreEntityID, 200.0f, loveletterEntityID);
                
                LogMessage("Core damaged successfully");
            }

            // Publish event
            Publish("LoveLetterDestroyed", loveletterEntityID.ToString());
            Publish("LoveLetterReachedCore", loveletterEntityID.ToString());
            
            // Self-destruct
            SceneDestroyEntity(loveletterEntityID);
            LogMessage("///////////////////////////End of Onreached.");
        }

        // ===== HELPER FUNCTIONS =====
        private float CalculateDistance(Engine.Vector3 a, Engine.Vector3 b)
        {
            float dx = b.X - a.X;
            float dy = b.Y - a.Y;
            float dz = b.Z - a.Z;
            return SimpleMath.Sqrt(dx * dx + dy * dy + dz * dz);
        }

        // ===== RNG FUNCTIONS =====
        private static uint Nextuint()
        {
            uint x = s_RngState;
            if (x == 0)
                x = 0x12345678u;

            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            s_RngState = x;
            return x;
        }

        private static int RandomRangeInt(int minInclusive, int maxExclusive)
        {
            if (maxExclusive <= minInclusive)
                return minInclusive;

            uint range = (uint)(maxExclusive - minInclusive);
            uint r = Nextuint();
            return minInclusive + (int)(r % range);
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
            Unsubscribe(GAMEOVER, OnGameOver);
            Unsubscribe(GAMEWIN, OnGameOver);

            LogMessage("=== LoveLetter Destroyed ===");
        }

        private void OnGameOver(string eventName, string payload){
            if (isDead)
                return;
            isDead = true;
            isMoving = false;
            isWaitingAtSurface = false;
            SceneDestroyEntity(loveletterEntityID);
        }
    }
}
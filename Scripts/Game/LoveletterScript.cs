using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Quat;
using static Engine.SimpleMath;

namespace Game
{
    public class LoveLetterScript : ScriptBehaviour
    {
        // ===== NAME OF ENTITY =====
        [SerializeField] private string loveletterEntity = "loveletter";
        private uint loveletterEntityID = 0;

        // ===== MULTIPLE CORES =====
        [SerializeField] private string[] coreEntities = new string[]
        {
            "Core_1", "Core_2", "Core_3"
        };
        
        private uint[] coreEntityIDs;
        private int selectedCoreIndex = -1;
        private uint selectedCoreEntityID = 0;

        // ===== CORE DIMENSIONS =====
        [SerializeField] private float coreHalfSizeX = 37.5f;
        [SerializeField] private float coreHalfSizeY = 37.5f;
        [SerializeField] private float coreHalfSizeZ = 37.5f;
        
        [SerializeField] private float stopDistanceFromSurface = 200.0f;

        // ===== MOVEMENT SETTING ===== 
        [SerializeField] private float moveSpeed = 1200.0f;
        [SerializeField] private float startDelay = 2.0f;
        [SerializeField] private float waitTimeAtSurface = 3.0f;

        // ===== HEALTH SYSTEM =====
        [SerializeField] private int totalLogicBombs = 9;
        private int logicBombsAlive = 9;
        private bool isDead = false;

        // ===== MOVEMENT STATE =====
        private bool isMoving = false;
        private bool isWaitingAtSurface = false;
        private float delayTimer = 0.0f;
        private float waitTimer = 0.0f;

        // ===== TARGET POSITIONS =====
        private Engine.Vector3 corePosition;
        private Engine.Vector3 targetSurfaceCenter;
        private bool targetCalculated = false;

        // ===== SMOOTH MOVEMENT SETTINGS =====
        [SerializeField] private float slowDownRadius = 10.0f;
        [SerializeField] private float minSpeedFactor = 0.1f;

        private static bool rngSeeded = false;

        public override void OnStart()
        {
            LogMessage("///////////////////////////Start of the Loveletter Script");
            loveletterEntityID = SceneFindEntityByName(loveletterEntity);

            LogMessage("=== LoveLetter Started ===");
            LogMessage("LoveLetterEntity: " + loveletterEntityID);
            LogMessage("Total LogicBombs: " + totalLogicBombs);
            LogMessage("Available Cores: " + coreEntities.Length);
            
            if (loveletterEntityID == 0)
            {
                LogError("[loveletterEntity] cannot be found.");
                return;
            }
            
            // Initialize health system
            logicBombsAlive = totalLogicBombs;
            isDead = false;

            // Subscribe to LogicBomb destruction events
            Subscribe("LogicBombDestroyed", OnLogicBombDestroyed);
            
            // Initialize core IDs array
            coreEntityIDs = new uint[coreEntities.Length];
            
            
            if (!rngSeeded)
            {
                uint timeSeed = (uint)(System.DateTime.Now.Ticks & 0xFFFFFFFF);
                RNG.Seed(timeSeed);
                rngSeeded = true;
                LogMessage("RNG seeded with: " + timeSeed);
            }
            
            // Select a random core
            SelectRandomCore();
            
            if (selectedCoreIndex >= 0)
            {
                // Start movement after delay
                delayTimer = startDelay;
                LogMessage("LoveLetter initialized - waiting " + startDelay + " seconds before movement");
                LogMessage("Selected core: " + coreEntities[selectedCoreIndex] + " (ID: " + selectedCoreEntityID + ")");
                
            }
            else
            {
                LogError("No valid core found! LoveLetter cannot move.");
            }
        }

        public override void OnUpdate(float deltaTime)
        {
            
            // Don't update if dead or no core selected
            if (isDead || selectedCoreIndex < 0) return;

            // Handle spawn delay
            if (delayTimer > 0.0f)
            {
                delayTimer -= deltaTime;
                
                if (delayTimer <= 0.0f)
                {
                    LogMessage("Starting movement to " + coreEntities[selectedCoreIndex] + "!");
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

            // Movement logic
            if (isMoving && targetCalculated)
            {
                MoveTowardsTarget(deltaTime);
                CheckIfReachedTarget();
            }
        }

        private void SelectRandomCore()
        {
            int validCores = 0;
            for (int i = 0; i < coreEntities.Length; i++)
            {
                coreEntityIDs[i] = SceneFindEntityByName(coreEntities[i]);
                if (coreEntityIDs[i] != 0)
                {
                    validCores++;
                    LogMessage("Found core: " + coreEntities[i] + " (ID: " + coreEntityIDs[i] + ")");
                }
                else
                {
                    LogMessage("Core not found: " + coreEntities[i]);
                }
            }
            
            if (validCores == 0)
            {
                selectedCoreIndex = -1;
                selectedCoreEntityID = 0;
                return;
            }
            
            // Create list of available core indices
            int[] availableIndices = new int[validCores];
            int index = 0;
            for (int i = 0; i < coreEntities.Length; i++)
            {
                if (coreEntityIDs[i] != 0)
                {
                    availableIndices[index] = i;
                    index++;
                }
            }
            
            // Use engine's RNG.RandInt (inclusive: [min, max])
            int randomIndex = RNG.RandInt(0, validCores - 1);
            
            selectedCoreIndex = availableIndices[randomIndex];
            selectedCoreEntityID = coreEntityIDs[selectedCoreIndex];

            LogMessage("Randomly selected core: " + coreEntities[selectedCoreIndex] + 
                    " (index: " + randomIndex + " out of " + validCores + ")");
        }

        // ===== HEALTH SYSTEM =====
        private void OnLogicBombDestroyed(string eventName, string payload)
        {
            // Payload contains the parent LoveLetter's EntityID
            if (!uint.TryParse(payload, out uint parentID))
                return;

            // Check if this event is for THIS LoveLetter instance
            if (parentID == loveletterEntityID)
            {
                logicBombsAlive--;
                LogMessage("LogicBomb destroyed! Remaining: " + logicBombsAlive + "/" + totalLogicBombs);

                if (logicBombsAlive <= 0)
                {
                    OnAllLogicBombsDestroyed();
                }
            }
        }

        private void OnAllLogicBombsDestroyed()
        {
            if (isDead) return;

            isDead = true;
            LogMessage("=== ALL LOGICBOMBS DESTROYED ===");
            LogMessage("LoveLetter is defeated!");

            // Stop movement
            isMoving = false;
            isWaitingAtSurface = false;

            // TODO: Add destruction VFX/SFX here
            // TODO: Spawn upgrade module or rewards

            // Publish event for game systems
            Publish("LoveLetterDestroyed", loveletterEntityID.ToString());

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
            
            Engine.Vector3 startPos = GetPosition(loveletterEntityID);
            float initialDistance = CalculateDistance(startPos, targetSurfaceCenter);
            LogMessage("Starting distance to target: " + initialDistance);
        }

        private void MoveTowardsTarget(float deltaTime)
        {
            if (loveletterEntityID == 0) return;

            Engine.Vector3 currentPos = GetPosition(loveletterEntityID);
            
            // Calculate direction to target
            float dx = targetSurfaceCenter.X - currentPos.X;
            float dy = targetSurfaceCenter.Y - currentPos.Y;
            float dz = targetSurfaceCenter.Z - currentPos.Z;
            
            // Calculate distance
            float distance = SimpleMath.Sqrt(dx * dx + dy * dy + dz * dz);
            
            // If already very close, just snap to target
            if (distance < 0.5f)
            {
                SetPosition(loveletterEntityID, ref targetSurfaceCenter);
                OnReachedTarget();
                return;
            }
            
            // Normalize direction
            if (distance > 0.001f)
            {
                dx /= distance;
                dy /= distance;
                dz /= distance;
            }
            else
            {
                return;
            }
            
            // Calculate effective speed with smooth slowdown
            float effectiveSpeed = CalculateEffectiveSpeed(distance);
            
            // Calculate movement this frame
            float movementThisFrame = effectiveSpeed * deltaTime;
            
            // Don't overshoot - if we would move past the target, just go to target
            if (distance <= movementThisFrame)
            {
                SetPosition(loveletterEntityID, ref targetSurfaceCenter);
                OnReachedTarget();
                return;
            }
            
            // Calculate new position
            Engine.Vector3 newPos = new Engine.Vector3(
                currentPos.X + dx * movementThisFrame,
                currentPos.Y + dy * movementThisFrame,
                currentPos.Z + dz * movementThisFrame
            );
            
            SetPosition(loveletterEntityID, ref newPos);
        }

        private float CalculateEffectiveSpeed(float distanceToTarget)
        {
            // Base speed
            float speed = moveSpeed;
            
            // Apply smooth slowdown when approaching target
            if (distanceToTarget < slowDownRadius)
            {
                // Quadratic ease-out: speed reduces as we get closer
                float t = distanceToTarget / slowDownRadius; // 1.0 at radius, 0.0 at target
                speed *= t * t; // Quadratic slowdown (smoother)
                
                // Ensure minimum speed
                if (speed < moveSpeed * minSpeedFactor)
                {
                    speed = moveSpeed * minSpeedFactor;
                }
            }
            
            return speed;
        }

        private void CheckIfReachedTarget()
        {
            if (isWaitingAtSurface) return;
            
            Engine.Vector3 currentPos = GetPosition(loveletterEntityID);
            float distance = CalculateDistance(currentPos, targetSurfaceCenter);
            
            if (distance <= 1.0f)
            {
                LogMessage("=== REACHED TARGET SURFACE CENTER ===");
                OnReachedTarget();
            }
        }

        private void OnReachedTarget()
        {
            // Snap to exact target position
            SetPosition(loveletterEntityID, ref targetSurfaceCenter);
            
            // Stop moving and start waiting
            isMoving = false;
            isWaitingAtSurface = true;
            waitTimer = waitTimeAtSurface;
            
            LogMessage("Waiting " + waitTimeAtSurface + " seconds at surface center before self-destruct...");
        }

        private void OnReachedCore()
        {
            if (isDead) return;

            isDead = true;
            LogMessage("=== SELF DESTRUCTING AT CORE ===");
        
            // Publish event
            Publish("LoveLetterReachedCore", loveletterEntityID.ToString());
            
            // Self-destruct
            SceneDestroyEntity(loveletterEntityID);
            LogMessage("///////////////////////////End of the Loveletter movement.");
        }

        // ===== HELPER FUNCTIONS =====
        private float CalculateDistance(Engine.Vector3 a, Engine.Vector3 b)
        {
            float dx = b.X - a.X;
            float dy = b.Y - a.Y;
            float dz = b.Z - a.Z;
            return SimpleMath.Sqrt(dx * dx + dy * dy + dz * dz);
        }

        public override void OnDestroy()
        {
            Unsubscribe("LogicBombDestroyed", OnLogicBombDestroyed);
            LogMessage("=== LoveLetter Destroyed ===");
        }
    }
}
using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Quat;
using static Engine.SimpleMath;
using static Engine.Tag;
using static Engine.Prefab;
using static Engine.Rigidbody;

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

        // ===== PLAYER (for label billboard) =====
        [SerializeField] private string playerEntityName = "Player";
        private uint playerEntityID = 0;

        // ===== BULLET TAG ======
        private const string TAG_PRIMARY_BULLET = "PrimaryBullet";
        private const string TAG_SECONDARY_BULLET = "PrimaryUltBullet";

        private uint spawnedPayloadID = 0;
        //private uint spawnedLabelID = 0;  // track the label entity
        private const string EVENT_PAYLOAD_COLLECTED = "PayloadCollected";

        // ===== PREFAB =====
        [SerializeField] private string deathPrefab = "Sources/Prefabs/Logic_bomb_Explosion.prefab";
        [SerializeField] private string hitmarkerAudioPrefab = "Sources/Prefabs/audio_hitmarker.prefab";
        [SerializeField] private string playerKillPrefab = "Sources/Prefabs/audio_Player_Kill.prefab";
        [SerializeField] private string payloadPrefab = "Sources/Prefabs/Payload.prefab";
        [SerializeField] private string mainExplosionPrefab = "Sources/Prefabs/MainExplosion1.prefab";
        [SerializeField] private string upgradeModuleLabelPrefab = "Sources/Prefabs/UpgradeModuleLabel.prefab";

        // ===== CORE DIMENSIONS =====
        [SerializeField] private float coreHalfSizeX = 37.5f;
        [SerializeField] private float coreHalfSizeY = 37.5f;
        [SerializeField] private float coreHalfSizeZ = 37.5f;
        
        [SerializeField] private float stopDistanceFromSurface = 200.0f;

        // ===== MOVEMENT SETTING ===== 
        [SerializeField] private float moveSpeed = 500.0f;
        [SerializeField] private float startDelay = 2.0f;
        [SerializeField] private float waitTimeAtSurface = 3.0f;

        // ===== SIMPLE HEALTH SYSTEM =====
        [SerializeField] private float maxHealth = 60.0f;
        [SerializeField] private float currentHealth = 60.0f;
        private bool isDead = false;

        // Vampirism: track who landed the killing blow
        private string lastKillerTag = "";

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
        private string EVENT_BULLET_HIT = "Damage:";
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        // ==== OTHER VALUES =====
        private const uint INVALID_ENTITY = 0xffffffffu;

        private uint labelEntityID = 0;

        public override void OnStart()
        {
            LogMessage("///////////////////////////Start of the Loveletter Script");
            loveletterEntityID = EntityID;//SceneFindEntityByName(loveletterEntity);

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

            // FIX: was SceneFindEntityByTag — must be SceneFindEntityByName
            playerEntityID = SceneFindEntityByName(playerEntityName);
            if (playerEntityID == 0)
                LogWarning("[LoveLetterScript] Player entity not found for label billboard.");

            EVENT_BULLET_HIT += EntityID.ToString();
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            Subscribe(GAMEOVER, OnGameOver);
            Subscribe(GAMEWIN, OnGameOver);
            Subscribe(EVENT_PAYLOAD_COLLECTED, OnPayloadCollected);
            EnemyRegistry.Register(EntityID);

            // Seed RNG if not already done
            if (!rngSeeded)
            {
                uint timeSeed = (uint)(System.DateTime.Now.Ticks & 0xFFFFFFFF);
                s_RngState = timeSeed;
                rngSeeded = true;
                LogMessage("RNG seeded with: " + timeSeed);
            }
            
            SelectRandomCoreByTag();
            
            if (selectedCoreEntityID != 0)
            {
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
            if (GameState.IsPaused)
                return;

            if (isDead || selectedCoreEntityID == 0) return;

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

            if (isMoving && targetCalculated)
            {
                MoveTowardsTargetLinear(deltaTime);
            }
        }

        private void SelectRandomCoreByTag()
        {
            uint[] coreEntities = SceneFindEntitiesByTag(coreTag);
            
            if (coreEntities == null || coreEntities.Length == 0)
            {
                LogError("No cores found with tag: " + coreTag);
                selectedCoreEntityID = 0;
                return;
            }
            
            LogMessage("Found " + coreEntities.Length + " cores with tag '" + coreTag + "'");
            
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
            if (isDead || eventName != EVENT_BULLET_HIT)
                return;

            float damage = DamageSystem.ParseAmount(payload);
            currentHealth -= damage;

            if (currentHealth < 0.0f)
                currentHealth = 0.0f;

            uint attackerId = DamageSystem.ParseAttackerId(payload);
            LogMessage("[LoveLetterScript] Attacker ID is: " + attackerId.ToString());

            if (attackerId != INVALID_ENTITY)
            {
                string attackerTag = TagGetTag(attackerId);
                LogMessage("[LoveLetterScript] attacker is: " + attackerTag);

                if (attackerTag == TAG_PRIMARY_BULLET || attackerTag == TAG_SECONDARY_BULLET)
                {
                    lastKillerTag = attackerTag;

                    if (currentHealth > 0.0f)
                    {
                        LogMessage("[LoveLetterScript] Instantiating the hitmarker");
                        uint hitmarkerID = PrefabInstantiate(hitmarkerAudioPrefab);
                        if (hitmarkerID == 0)
                            LogMessage("[LoveLetterScript] Player Hit! But hitmarkerID fail to instantiate");
                    }
                }
            }

            if (currentHealth <= 0.0f)
            {
                uint playerkillID = PrefabInstantiate(playerKillPrefab);
                if (playerkillID == 0)
                    LogMessage("[LoveLetterScript] Player Kill LoveLetter! But playerkillID fail to instantiate");

                Publish("LoveLetterDeath", 1.ToString());
                DestroyLoveLetter();
            }
        }

        private void DestroyLoveLetter()
        {
            if (isDead) return;

            isDead = true;
            LogMessage("=== LOVELETTER DESTROYED ===");

            isMoving = false;
            isWaitingAtSurface = false;
            Publish("LoveLetterKilled", "killer=" + lastKillerTag);
            Publish("LoveLetterDestroyed", loveletterEntityID.ToString());

            // Capture position BEFORE destroying the entity
            Vector3 spawnPos = GetPosition((uint)EntityID);
            Quat spawnRot = GetRotation((uint)EntityID);

            // Death audio VFX
            Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
            uint explosion = PrefabInstantiateWithTransform(deathPrefab, ref spawnPos, ref spawnRot, ref scale, false);
            if (explosion == 0)
                LogMessage("[LoveletterScript] loveletter explosion entity fail to instantiate");

            // MainExplosion1 (same as EnemyTurret when killed by player)
            uint mainExplosion = PrefabInstantiate(mainExplosionPrefab);
            if (mainExplosion == 0)
            {
                LogMessage("[LoveletterScript] MainExplosion1 failed to instantiate");
            }
            else
            {
                Transform.SetPosition(mainExplosion, ref spawnPos);
                Vector3 explosionScale = new Vector3(20.0f, 20.0f, 20.0f);
                Transform.SetScale(mainExplosion, ref explosionScale);
            }

            // Spawn the payload
            Vector3 payloadScale = new Vector3(10.0f, 10.0f, 10.0f);
            uint payload = PrefabInstantiateWithTransform(payloadPrefab, ref spawnPos, ref spawnRot, ref payloadScale, false);
            if (payload == 0)
            {
                LogMessage("[LoveletterScript] loveletter payload entity fail to instantiate");
                return;
            }
            spawnedPayloadID = payload;
            RigidbodySetIsKinematic(payload, false);
            Vector3 halfboxExtend = new Vector3(20f, 25f, 20f);
            RigidbodySetBoxHalfExtents(payload, ref halfboxExtend);

            // Spawn label AFTER payload so it renders on top.
            // UpgradeModuleLabel.cs positions itself on the payload surface facing the player.
            Quat identityRot = new Quat(0f, 0f, 0f, 1f);
            Vector3 labelScale = new Vector3(50.0f, 21.0f, 1.0f);
            labelEntityID = PrefabInstantiateWithTransform(upgradeModuleLabelPrefab, ref spawnPos, ref identityRot, ref labelScale, false);
            if (labelEntityID == 0)
            {
                LogMessage("[LoveletterScript] upgradeModuleLabel failed to instantiate");
            }
            else
            {
                // Pass payload ID to the label via event since EntityGetScript is unavailable.
                // UpgradeModuleLabel listens for "UpgradeLabelInit:{labelEntityID}" and stores the payload ID.
                string initEvent = "UpgradeLabelInit:" + labelEntityID.ToString();
                Publish(initEvent, payload.ToString());
                LogMessage("[LoveletterScript] upgradeModuleLabel spawned ID: " + labelEntityID +
                    " | sent init event with payload ID: " + payload);
            }

            // Destroy the LoveLetter FIRST so it doesn't overlap spawned entities
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
                
                startPosition = GetPosition(loveletterEntityID);
                currentPosition = startPosition;
                totalDistanceTraveled = 0.0f;
                
                float dx = targetSurfaceCenter.X - startPosition.X;
                float dy = targetSurfaceCenter.Y - startPosition.Y;
                float dz = targetSurfaceCenter.Z - startPosition.Z;
                totalDistance = SimpleMath.Sqrt(dx * dx + dy * dy + dz * dz);
                
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
            
            Engine.Vector3 direction = new Engine.Vector3(
                loveletterPos.X - corePosition.X,
                loveletterPos.Y - corePosition.Y,
                loveletterPos.Z - corePosition.Z
            );
            
            float absX = direction.X > 0 ? direction.X : -direction.X;
            float absY = direction.Y > 0 ? direction.Y : -direction.Y;
            float absZ = direction.Z > 0 ? direction.Z : -direction.Z;
            
            if (absX >= absY && absX >= absZ)
            {
                if (direction.X > 0)
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X + coreHalfSizeX + stopDistanceFromSurface,
                        corePosition.Y, corePosition.Z);
                    LogMessage("Targeting RIGHT face center");
                }
                else
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X - coreHalfSizeX - stopDistanceFromSurface,
                        corePosition.Y, corePosition.Z);
                    LogMessage("Targeting LEFT face center");
                }
            }
            else if (absY >= absX && absY >= absZ)
            {
                if (direction.Y > 0)
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X,
                        corePosition.Y + coreHalfSizeY + stopDistanceFromSurface,
                        corePosition.Z);
                    LogMessage("Targeting TOP face center");
                }
                else
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X,
                        corePosition.Y - coreHalfSizeY - stopDistanceFromSurface,
                        corePosition.Z);
                    LogMessage("Targeting BOTTOM face center");
                }
            }
            else
            {
                if (direction.Z > 0)
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X, corePosition.Y,
                        corePosition.Z + coreHalfSizeZ + stopDistanceFromSurface);
                    LogMessage("Targeting FRONT face center");
                }
                else
                {
                    targetSurfaceCenter = new Engine.Vector3(
                        corePosition.X, corePosition.Y,
                        corePosition.Z - coreHalfSizeZ - stopDistanceFromSurface);
                    LogMessage("Targeting BACK face center");
                }
            }
        }

        private void MoveTowardsTargetLinear(float deltaTime)
        {
            if (loveletterEntityID == 0) return;

            float distanceThisFrame = moveSpeed * deltaTime;
            totalDistanceTraveled += distanceThisFrame;
            
            if (totalDistanceTraveled >= totalDistance)
            {
                SetPosition(loveletterEntityID, ref targetSurfaceCenter);
                OnReachedTarget();
                return;
            }
            
            currentPosition = new Engine.Vector3(
                startPosition.X + directionNormalized.X * totalDistanceTraveled,
                startPosition.Y + directionNormalized.Y * totalDistanceTraveled,
                startPosition.Z + directionNormalized.Z * totalDistanceTraveled
            );
            
            SetPosition(loveletterEntityID, ref currentPosition);
        }

        private void OnReachedTarget()
        {
            SetPosition(loveletterEntityID, ref targetSurfaceCenter);
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

            isDead = true;
            isMoving = false;
            isWaitingAtSurface = false;
            LogMessage("=== SELF DESTRUCTING AT CORE ===");
        
            if (selectedCoreEntityID != 0)
            {
                LogMessage("Dealing 200 damage to core ID: " + selectedCoreEntityID);
                DamageSystem.DealDamage(selectedCoreEntityID, 200.0f, loveletterEntityID);
                LogMessage("Core damaged successfully");
            }

            Publish("LoveLetterDestroyed", loveletterEntityID.ToString());
            Publish("LoveLetterReachedCore", loveletterEntityID.ToString());
            
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
            EnemyRegistry.Unregister(EntityID);
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
            Unsubscribe(GAMEOVER, OnGameOver);
            Unsubscribe(GAMEWIN, OnGameOver);
            Unsubscribe(EVENT_PAYLOAD_COLLECTED, OnPayloadCollected);
            LogMessage("=== LoveLetter Destroyed ===");
        }

        private void OnGameOver(string eventName, string payload)
        {
            if (isDead)
                return;
            isDead = true;
            isMoving = false;
            isWaitingAtSurface = false;

            if (labelEntityID != 0)              // <-- add this
            {
                SceneDestroyEntity(labelEntityID);
                labelEntityID = 0;
            }
            SceneDestroyEntity(loveletterEntityID);
        }

        private void OnPayloadCollected(string eventName, string payload)
        {
            if (!uint.TryParse(payload, out uint collectedID)) return;
            if (collectedID != spawnedPayloadID) return; // only care about our own payload

            if (labelEntityID != 0)
            {
                SceneDestroyEntity(labelEntityID);
                labelEntityID = 0;
            }
        }
    }
}
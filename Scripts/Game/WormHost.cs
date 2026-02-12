using Engine;
using System;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Physics;
using static Engine.Rigidbody;
using static Engine.Audio;
using static Engine.Tag;
using static Engine.Transform;

namespace Game
{
    public class WormHost : ScriptBehaviour
    {
        // Movement / AI
        [SerializeField] private bool isStationary = false;
        [SerializeField] private bool hasSplit = false;

        [SerializeField] private float speed = 100f;
        [SerializeField] private float stationaryTimer = 10.0f;
        private float timer = 0.0f;

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private uint gunshipID = INVALID_ENTITY;
        private uint currentTargetID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";
        private const string TAG_GUNSHIP = "Gunship";

        private const string TAG_PRIMARY_BULLET = "PrimaryBullet";
        private const string TAG_SECONDARY_BULLET = "PrimaryUltBullet";
        private string hitmarkerAudioPrefab = "Sources/Prefabs/audio_hitmarker.prefab";
        private string playerKillPrefab = "Sources/Prefabs/audio_Player_Kill.prefab";

        // Health
        [SerializeField] private float health = 18.0f;

        // Events
        private string EVENT_BULLET_HIT = "Damage:";
        //private const string EVENT_HOST_SPLIT = "WormHostSplit";

        // Worm Child
        private string wormChildPrefabName = "WormChild";
        [SerializeField] private int childCount = 3;

        // BARE MINIMUM
        [SerializeField] private float shootingCooldown = 0.25f;
        private float shootingTimer = 0.0f;

        // Game lose / win condition
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        // Lifecycle
        public override void OnStart()
        {
            LogMessage("======= WormHost started (EntityID = " + EntityID + ") =======");

            // Find both player and gunship
            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID + " =======");

            gunshipID = SceneFindEntityByName(TAG_GUNSHIP);
            LogMessage("======= gunshipID: " + gunshipID + " =======");

            isStationary = false;
            hasSplit = false;
            timer = 0.0f;

            RigidbodySetIsKinematic(EntityID, true);
            Vector3 extents = new Vector3(40.0f, 40.0f, 40.0f);
            RigidbodySetBoxHalfExtents(EntityID, ref extents);

            EVENT_BULLET_HIT += EntityID.ToString();
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            Subscribe(GAMEOVER, OnGameOver);
            Subscribe(GAMEWIN, OnGameOver);
        }

        public override void OnUpdate(float deltaTime)
        {
            // Don't update when game is paused
            if (GameState.IsPaused)
                return;

            // Update target - find closest valid target
            UpdateTarget();

            // If no valid targets exist, destroy self
            if (currentTargetID == INVALID_ENTITY)
            {
                LogMessage("WormHost: No valid targets found, destroying");
                SceneDestroyEntity(EntityID);
                return;
            }

            Vector3 ownPosition = GetPosition(EntityID);
            Vector3 targetPosition = GetPosition(currentTargetID);
            Vector3 direction = targetPosition - ownPosition;

            // Rotation - aim at target
            float magnitude = SimpleMath.Sqrt(
                direction.X * direction.X +
                direction.Y * direction.Y +
                direction.Z * direction.Z
            );

            if (magnitude < 0.001f)
                return;

            // Normalize direction
            float invMag = 1.0f / magnitude;
            Vector3 toTarget = new Vector3(
                direction.X * invMag,
                direction.Y * invMag,
                direction.Z * invMag
            );

            Vector3 forward = Vector3.Forward;
            Quat targetRot = QuaternionFromTo(forward, toTarget);
            SetRotation(EntityID, ref targetRot);

            if (SimpleMath.Sqrt(direction.X*direction.X + direction.Y*direction.Y + direction.Z*direction.Z) < 0.001f)
                return;

            Quat lookRot = SimpleMath.LookRotation(-direction, Vector3.Up);
            SetRotation(EntityID, ref lookRot);

            // Check if close enough to become stationary and shoot
            if (magnitude < 250.0f)
            {
                isStationary = true;
            } else {
                isStationary = false;
            }

            if (isStationary)
            {
                // Shooting logic
                shootingTimer -= deltaTime;
                if (shootingTimer <= 0.0f)
                {
                    ShootAtTarget();
                    shootingTimer = shootingCooldown;
                }
            }
            else
            {
                // Movement logic
                float inverseMag = 1.0f / magnitude;
                Vector3 normDirection = direction * inverseMag;
                Vector3 newPosition = ownPosition + normDirection * speed * deltaTime;
                SetPosition(EntityID, ref newPosition);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
            Unsubscribe(GAMEOVER, OnGameOver);
            Unsubscribe(GAMEWIN, OnGameOver);
        }

        // NEW: Update target with priority: Player first, then Gunship if player is far
        [SerializeField] private float playerPriorityRange = 250.0f; // Range within which player is always prioritized
        
        private void UpdateTarget()
        {
            Vector3 wormPos = GetPosition(EntityID);
            
            uint selectedTarget = INVALID_ENTITY;
            float playerDist = float.MaxValue;
            float gunshipDist = float.MaxValue;

            // Check player distance
            bool playerExists = false;
            if (playerID != INVALID_ENTITY && EntityExists(playerID))
            {
                Vector3 playerPos = GetPosition(playerID);
                playerDist = CalculateDistance(wormPos, playerPos);
                playerExists = true;
            }

            // Check gunship distance
            bool gunshipExists = false;
            if (gunshipID != INVALID_ENTITY && EntityExists(gunshipID))
            {
                Vector3 gunshipPos = GetPosition(gunshipID);
                gunshipDist = CalculateDistance(wormPos, gunshipPos);
                gunshipExists = true;
            }

            // PRIORITY LOGIC:
            // 1. If player is within priority range, always target player
            // 2. If player is far away, target gunship if it's closer
            // 3. If both are far, target the closest one
            
            if (playerExists && playerDist <= playerPriorityRange)
            {
                // Player is nearby - always prioritize player
                selectedTarget = playerID;
            }
            else if (playerExists && gunshipExists)
            {
                // Both exist but player is far - choose closest
                if (gunshipDist < playerDist)
                {
                    selectedTarget = gunshipID;
                }
                else
                {
                    selectedTarget = playerID;
                }
            }
            else if (playerExists)
            {
                // Only player exists
                selectedTarget = playerID;
            }
            else if (gunshipExists)
            {
                // Only gunship exists
                selectedTarget = gunshipID;
            }

            // Update current target if it changed
            if (currentTargetID != selectedTarget)
            {
                currentTargetID = selectedTarget;
                if (currentTargetID != INVALID_ENTITY)
                {
                    string targetName = (currentTargetID == playerID) ? "Player" : "Gunship";
                    LogMessage("WormHost: New target locked - " + targetName + " (EntityID: " + currentTargetID + ")");
                }
            }
        }

        // Helper method to check if entity still exists
        private bool EntityExists(uint entityID)
        {
            if (entityID == 0 || entityID == INVALID_ENTITY)
                return false;

            // Try to get the entity's tag - if it fails, entity doesn't exist
            string tag = TagGetTag(entityID);
            return !string.IsNullOrEmpty(tag);
        }

        // Helper method to calculate distance between two points
        private float CalculateDistance(Vector3 a, Vector3 b)
        {
            float dx = b.X - a.X;
            float dy = b.Y - a.Y;
            float dz = b.Z - a.Z;
            return SimpleMath.Sqrt(dx * dx + dy * dy + dz * dz);
        }

        // Combat
        private void OnBulletHit(string eventName, string payload)
        {
            Vector3 emptyVec = new Vector3(0, 0, 0);
            RigidbodySetAngularVelocity(EntityID, ref emptyVec);

            health -= 1.0f;
            LogMessage("WormHost hit! Health: " + health);

            uint attackerId = DamageSystem.ParseAttackerId(payload);
            if(attackerId != INVALID_ENTITY && health > 0.0f){
                string attackerTag = TagGetTag(attackerId);
                if(attackerTag == TAG_PRIMARY_BULLET || attackerTag == TAG_SECONDARY_BULLET){
                    //instantiate the hitmarker audio
                    uint hitmarkerID = 0;
                    hitmarkerID = PrefabInstantiate(hitmarkerAudioPrefab);
                    if(hitmarkerID == 0){
                        LogMessage("[WormHost] Player Hit! But hitmarkerID fail to instantiate");
                    }
                }
            }

            if (health <= 0)
            {
                uint playerkillID = 0;
                playerkillID = PrefabInstantiate(playerKillPrefab);
                if(playerkillID == 0){
                    LogMessage("[WormHost] Player Kill WormHost! But playerkillID fail to instantiate");
                }
                Publish("WormHostDead", EntityID.ToString());
                SceneDestroyEntity(EntityID);
            }
        }

        // Split & Spawn Children - FOR LATER
        private void OnSplit()
        {
            if (hasSplit)
                return;

            hasSplit = true;

            Vector3 spawnPos = GetPosition(EntityID);
            LogMessage("======= WormHost splitting =======");

            string wormChildPrefabPath = "Sources/Prefabs/WormChild.prefab";

            Engine.Vector3[] offset = { new Vector3(0, 40, 0), new Vector3(40, -40, 0), new Vector3(-40, -40, 0)};

            for (int i = 0; i < childCount; i++)
            {
                uint childID = PrefabInstantiate(wormChildPrefabPath);

                if (childID == 0)
                {
                    LogError("Failed to spawn WormChild prefab from: " + wormChildPrefabPath);
                    return;
                }

                Vector3 childPos = spawnPos;

                if (i < offset.Length)
                {
                    childPos += offset[i];
                }

                SetPosition(childID, ref childPos);

            }

            //Publish(EVENT_HOST_SPLIT, EntityID.ToString());

            // Remove host after split
            Vector3 zero = new Vector3(0, 0, 0);
            RigidbodySetBoxHalfExtents(EntityID, ref zero);
            SceneDestroyEntity(EntityID);
        }

        // MODIFIED: Shoot at current target (player or gunship)
        public void ShootAtTarget()
        {
            if (currentTargetID == INVALID_ENTITY)
                return;

            // Get positions
            Vector3 wormPos = GetPosition(EntityID);
            Vector3 targetPos = GetPosition(currentTargetID);

            // Calculate direction to target
            Vector3 direction = new Vector3(
                targetPos.X - wormPos.X,
                targetPos.Y - wormPos.Y,
                targetPos.Z - wormPos.Z
            );

            // Normalize direction
            float magnitude = SimpleMath.Sqrt(
                direction.X * direction.X +
                direction.Y * direction.Y +
                direction.Z * direction.Z
            );

            if (magnitude < 0.001f)
                return;

            float invMag = 1.0f / magnitude;
            Vector3 directionNorm = new Vector3(
                direction.X * invMag,
                direction.Y * invMag,
                direction.Z * invMag
            );

            // Spawn bullet slightly in front
            float spawnDist = 1.5f;
            Vector3 spawnPosition = new Vector3(
                wormPos.X + directionNorm.X * spawnDist,
                wormPos.Y + directionNorm.Y * spawnDist,
                wormPos.Z + directionNorm.Z * spawnDist
            );

            // Create bullet
            uint wormBulletID = SceneCreateEntity("WormBullet");
            if (wormBulletID == 0)
                return;

            Transform.SetPosition(wormBulletID, ref spawnPosition);

            // Set rotation to face the direction (optional, for visual)
            Vector3 forward = Vector3.Forward;
            Quat bulletRot = QuaternionFromTo(forward, directionNorm);
            Transform.SetRotation(wormBulletID, ref bulletRot);

            // Setup rigidbody
            EntityAddRigidBody(wormBulletID);
            RigidbodySetIsKinematic(wormBulletID, false);
            RigidbodySetUseGravity(wormBulletID, false);

            // Set tag
            TagSetTag(wormBulletID, "WormBullet");

            // Apply force in the direction of the target
            float bulletForce = 100.0f;
            Vector3 force = new Vector3(
                directionNorm.X * bulletForce,
                directionNorm.Y * bulletForce,
                directionNorm.Z * bulletForce
            );
            RigidbodyAddForce(wormBulletID, ref force);

            // Set collision box
            Vector3 extents = new Vector3(2.0f, 2.0f, 2.0f);
            RigidbodySetBoxHalfExtents(wormBulletID, ref extents);

            // Add visuals and script
            EntityAddMeshRenderer(wormBulletID);
            EntityAddScript(wormBulletID, "Game.WormBullet");
        }

        private static Quat QuaternionFromTo(Vector3 from, Vector3 to)
        {
            float dot = from.X * to.X + from.Y * to.Y + from.Z * to.Z;

            if (dot < -0.9999f)
            {
                Quat q180;
                q180.X = 0.0f;
                q180.Y = 1.0f;
                q180.Z = 0.0f;
                q180.W = 0.0f;
                return q180;
            }

            Vector3 cross = new Vector3(
                from.Y * to.Z - from.Z * to.Y,
                from.Z * to.X - from.X * to.Z,
                from.X * to.Y - from.Y * to.X
            );

            float s = SimpleMath.Sqrt((1.0f + dot) * 2.0f);
            float invS = 1.0f / s;

            Quat q;
            q.X = cross.X * invS;
            q.Y = cross.Y * invS;
            q.Z = cross.Z * invS;
            q.W = 0.5f * s;

            return q;
        }

        private void OnGameOver(string eventName, string payload)
        {
            SceneDestroyEntity(EntityID);
        }
    }
}
using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Physics;
using static Engine.Prefab;

namespace Game
{
    /// <summary>
    /// Deals gradual damage to the player while the player is in direct physics contact
    /// with the entity this script is attached to.
    ///
    /// This bypasses CollisionManager entirely and queries raw physics collision pairs.
    /// Also detects bullet contact against this hazard/wall and signals the bullet to die.
    /// </summary>
    public class TickDamage : ScriptBehaviour
    {
        [SerializeField] private string playerName = "Player";
        [SerializeField] private float damagePerSecond = 3.0f;
        [SerializeField] private bool debugLogging = false;

        private const string collisionaudiodamagePrefab = "Sources/Prefabs/Audio_Tick_Damage.prefab";

        private uint playerID = 0;
        private uint wallID = 0;
        private bool isInitialized = false;

        public override void OnStart()
        {
            wallID = (uint)EntityID;
            playerID = SceneFindEntityByName(playerName);

            if (wallID == 0)
            {
                LogError("[TickDamage] ERROR: Invalid wall entity ID.");
                return;
            }

            if (playerID == 0)
            {
                LogError("[TickDamage] ERROR: Player entity not found in scene!");
                return;
            }

            isInitialized = true;

            LogMessage(
                "[TickDamage] Initialized on entity " + wallID.ToString() +
                " | playerID=" + playerID.ToString() +
                " | damagePerSecond=" + damagePerSecond.ToString()
            );
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!isInitialized)
                return;

            if (GameState.IsPaused)
                return;

            if (deltaTime <= 0.0f)
                return;

            CheckDirectCollisionAndApplyDamage(deltaTime);
        }

        private void CheckDirectCollisionAndApplyDamage(float deltaTime)
        {
            int collisionCount = PhysicsGetCollisionCount();
            if (collisionCount <= 0)
                return;

            for (int i = 0; i < collisionCount; i++)
            {
                PhysicsGetCollisionPair(i, out uint entityA, out uint entityB);

                // Handle bullet-vs-wall first.
                CheckBulletCollision(entityA, entityB);

                bool playerWallCollision =
                    (entityA == playerID && entityB == wallID) ||
                    (entityA == wallID && entityB == playerID);

                if (!playerWallCollision)
                    continue;

                float damageThisTick = damagePerSecond * deltaTime;
                DamageSystem.DealDamage(playerID, damageThisTick, wallID);

                SpawnAudioPrefab();

                if (debugLogging)
                {
                    LogMessage(
                        "[TickDamage] Direct contact detected | wall=" + wallID.ToString() +
                        " | player=" + playerID.ToString() +
                        " | damage=" + damageThisTick.ToString()
                    );
                }

                // Only apply once per fixed step, even if multiple contact pairs exist.
                return;
            }
        }

        private void CheckBulletCollision(uint entityA, uint entityB)
        {
            string entityTag = string.Empty;
            bool bulletHit = false;
            uint targetEntityID = 0;

            if (entityA == wallID)
            {
                entityTag = Tag.TagGetTag(entityB);
                if (entityTag == "PrimaryBullet" || entityTag == "PrimaryUltBullet")
                {
                    bulletHit = true;
                    targetEntityID = entityB;
                }
            }
            else if (entityB == wallID)
            {
                entityTag = Tag.TagGetTag(entityA);
                if (entityTag == "PrimaryBullet" || entityTag == "PrimaryUltBullet")
                {
                    bulletHit = true;
                    targetEntityID = entityA;
                }
            }

            if (!bulletHit || targetEntityID == 0)
                return;

            if (debugLogging)
            {
                LogMessage(
                    "[TickDamage] Bullet/environment contact detected | wall=" + wallID.ToString() +
                    " | bullet=" + targetEntityID.ToString() +
                    " | tag=" + entityTag
                );
            }

            // Use DamageSystem as the signal path so the bullet can subscribe to Damage:<its id>
            // and destroy itself immediately on environment impact.
            DamageSystem.DealDamage(targetEntityID, 0.0f, wallID);
        }

        private void SpawnAudioPrefab()
        {
            uint dmgID = PrefabInstantiate(collisionaudiodamagePrefab);
            if (dmgID == 0)
            {
                LogMessage("[TickDamage] Failed to instantiate damage audio prefab");
            }
        }

        public override void OnDestroy()
        {
        }
    }
}
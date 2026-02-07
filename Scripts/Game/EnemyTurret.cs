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
    public class EnemyTurret : ScriptBehaviour
    {
        // Range
        [SerializeField] private bool inRange = false;

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private uint mainExplosionID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        // Health
        [SerializeField] private float health = 10.0f;

        // Events
        private string EVENT_BULLET_HIT = "Damage:";
        private const string GAMEOVER = "GameOver";

        // Shooting
        [SerializeField] private float shootingCooldown = 0.25f;
        private float shootingTimer = 0.0f;
        private float explosionTimer = 1.0f;
        private bool exploding = false;

        string EnemyTurretBulletPrefabPath = "Sources/Prefabs/EnemyTurretBullet.prefab";
        string EnemyTurretBulletExplosionPrefabPath = "Sources/Prefabs/EnemyTurretExplosion.prefab";
        string MainExplosionPrefabPath = "Sources/Prefabs/MainExplosion1.prefab";

        // Lifecycle
        public override void OnStart()
        {
            LogMessage("======= EnemyTurret started (EntityID = " + EntityID + ") =======");

            TagSetTag(EntityID, "EnemyTurret");
            
            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID + " =======");

            inRange = false;

            RigidbodySetIsKinematic(EntityID, true);
            RigidbodySetUseGravity(EntityID, false);

            EVENT_BULLET_HIT += EntityID.ToString();
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            Subscribe(GAMEOVER, OnGameOver);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused)
                return;

            if (playerID == INVALID_ENTITY)
                return;
            
            Vector3 ownPosition = GetPosition(EntityID);
            Vector3 targetPosition = GetPosition(playerID);
            Vector3 direction = targetPosition - ownPosition;

            // Rotation - aim at player
            float magnitude = SimpleMath.Sqrt(
                direction.X * direction.X +
                direction.Y * direction.Y +
                direction.Z * direction.Z
            );

            if (magnitude < 250.0f)
            {
                inRange = true;

            } else {
                
                inRange = false;
            }

            if (inRange && !exploding)
            {
                shootingTimer -= deltaTime;
                if (shootingTimer <= 0.0f)
                {
                    ShootAtTarget();
                    shootingTimer = shootingCooldown;
                }
            }

            if(exploding && mainExplosionID != INVALID_ENTITY){
                explosionTimer -= deltaTime;
                if (explosionTimer <= 0.0f)
                {
                    SceneDestroyEntity(mainExplosionID);
                    SceneDestroyEntity(EntityID);
                }
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
            Unsubscribe(GAMEOVER, OnGameOver);
        }

        // Combat
        private void OnBulletHit(string eventName, string payload)
        {
            if (exploding)
                return;

            LogMessage("OnBulletHit called! Payload: " + payload + " | My EntityID: " + EntityID);

            // uint hitEntityID = uint.Parse(payload.Split(',')[0]);
            // if (hitEntityID != EntityID){
            //     LogMessage("Ignoring (hitEntityID=" + hitEntityID + ")");
            //     return;
            // }
            
            Vector3 emptyVec = new Vector3(0, 0, 0);
            RigidbodySetVelocity(EntityID, ref emptyVec);
            RigidbodySetAngularVelocity(EntityID, ref emptyVec);

            health -= 1.0f;
            LogMessage("EnemyTurret hit! Health: " + health);

            if (health > 0)
                return;

            exploding = true;

            uint explosionID = PrefabInstantiate(EnemyTurretBulletExplosionPrefabPath);
            Vector3 myPos = Transform.GetPosition(EntityID);
            Transform.SetPosition(explosionID, ref myPos);
            AudioPlay(explosionID);

            mainExplosionID = PrefabInstantiate(MainExplosionPrefabPath);
            Transform.SetPosition(mainExplosionID, ref myPos);

            Vector3 newScale = new Vector3(20.0f, 20.0f, 20.0f);
            Transform.SetScale(mainExplosionID, ref newScale);

            Vector3 emptyScale = new Vector3(0.0f, 0.0f, 0.0f);
            Transform.SetScale(EntityID, ref emptyScale);

        }

        public void ShootAtTarget()
        {
            if (playerID == INVALID_ENTITY)
                return;

            // Get positions
            Vector3 wormPos = GetPosition(EntityID);
            Vector3 playerPos = GetPosition(playerID);

            // Calculate direction to player
            Vector3 direction = new Vector3(
                playerPos.X - wormPos.X,
                playerPos.Y - wormPos.Y,
                playerPos.Z - wormPos.Z
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
            uint enemyTurretBulletID = PrefabInstantiateScene(EnemyTurretBulletPrefabPath);
            if (enemyTurretBulletID == 0)
                return;

            Transform.SetPosition(enemyTurretBulletID, ref spawnPosition);

            // Set rotation to face the direction (optional, for visual)
            Vector3 forward = Vector3.Forward;
            Quat bulletRot = QuaternionFromTo(forward, directionNorm);
            Transform.SetRotation(enemyTurretBulletID, ref bulletRot);

            // Setup rigidbody
            EntityAddRigidBody(enemyTurretBulletID);
            RigidbodySetIsKinematic(enemyTurretBulletID, false);
            RigidbodySetUseGravity(enemyTurretBulletID, false);
            
            // Set tag
            TagSetTag(enemyTurretBulletID, "EnemyTurretBullet");

            // Apply force in the direction of the player
            float bulletForce = 100.0f;
            Vector3 force = new Vector3(
                directionNorm.X * bulletForce,
                directionNorm.Y * bulletForce,
                directionNorm.Z * bulletForce
            );
            RigidbodyAddForce(enemyTurretBulletID, ref force);

            // Set collision box
            Vector3 extents = new Vector3(2.0f, 2.0f, 2.0f);
            RigidbodySetBoxHalfExtents(enemyTurretBulletID, ref extents);

            // Add visuals and script - Using WormBullet
            EntityAddMeshRenderer(enemyTurretBulletID);
            EntityAddScript(enemyTurretBulletID, "Game.WormBullet");

            LogMessage("Turret fired bullet at player!");
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

        private void OnGameOver(string eventName, string payload){
            SceneDestroyEntity(EntityID);
        }
    }
}

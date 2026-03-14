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
    public class EnemyCoreShooter : ScriptBehaviour
    {
        // Range
        [SerializeField] private bool inRange = false;

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        // Events
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        // Shooting
        [SerializeField] private float shootingCooldown = 0.1f;
        private float shootingTimer = 0.0f;

        string EnemyTurretBulletPrefabPath = "Sources/Prefabs/EnemyTurretBullet.prefab";

        // Lifecycle
        public override void OnStart()
        {
            LogMessage("======= EnemyTurret started (EntityID = " + EntityID + ") =======");

            Vector3 newpos = Engine.Transform.GetPosition(EntityID);
            Engine.Transform.SetPosition(EntityID, ref newpos);
            
            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID + " =======");

            inRange = false;
            shootingTimer = 0.0f;
            Subscribe(GAMEOVER, OnGameOver);
            Subscribe(GAMEWIN, OnGameOver);
         
        }

        public override void OnUpdate(float deltaTime)
        {
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

            if (magnitude < 590.0f)
            {
                inRange = true;

            } else {
                
                inRange = false;
            }

            if (inRange)
            {
                shootingTimer -= deltaTime;
                if (shootingTimer <= 0.0f)
                {
                    ShootAtTarget();
                    shootingTimer = shootingCooldown;
                }
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(GAMEOVER, OnGameOver);
            Unsubscribe(GAMEWIN, OnGameOver);
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

            if (magnitude < 0.200f)
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
            if (enemyTurretBulletID == INVALID_ENTITY){
                return;
            }

            Transform.SetPosition(enemyTurretBulletID, ref spawnPosition);

            // Set rotation to face the direction (optional, for visual)
            Vector3 forward = Vector3.Forward;
            Quat bulletRot = QuaternionFromTo(forward, directionNorm);
            Transform.SetRotation(enemyTurretBulletID, ref bulletRot);

            Vector3 coreBulletSize = new Vector3(5.0f, 5.0f, 5.0f);
            Transform.SetScale(enemyTurretBulletID, ref coreBulletSize);
            
            // Set tag
            TagSetTag(enemyTurretBulletID, "EnemyCoreBullet");

            // Apply force in the direction of the player
            float bulletForce = 300.0f;
            Vector3 force = new Vector3(
                directionNorm.X * bulletForce,
                directionNorm.Y * bulletForce,
                directionNorm.Z * bulletForce
            );
            RigidbodyAddForce(enemyTurretBulletID, ref force);

            // Set collision box
            Vector3 extents = new Vector3(5.0f, 5.0f, 5.0f);
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

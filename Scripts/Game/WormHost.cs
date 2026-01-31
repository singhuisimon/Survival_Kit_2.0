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

        [SerializeField] private float speed = 50f;
        [SerializeField] private float stationaryTimer = 10.0f;
        private float timer = 0.0f;

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        // Health
        [SerializeField] private float health = 18.0f;

        // Events
        private const string EVENT_BULLET_HIT = "BulletHit";
        //private const string EVENT_HOST_SPLIT = "WormHostSplit";

        // Worm Child
        private string wormChildPrefabName = "WormChild";
        [SerializeField] private int childCount = 3;

        // BARE MINIMUM
        [SerializeField] private float shootingCooldown = 0.25f;
        private float shootingTimer = 0.0f;

        // Lifecycle
        public override void OnStart()
        {
            LogMessage("======= WormHost started (EntityID = " + EntityID + ") =======");

            // TODO: Set Target

            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID + " =======");

            isStationary = false;
            hasSplit = false;
            timer = 0.0f;

            RigidbodySetIsKinematic(EntityID, true);
            Vector3 extents = new Vector3(40.0f, 40.0f, 40.0f);
            RigidbodySetBoxHalfExtents(EntityID, ref extents);

            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (playerID == INVALID_ENTITY)
            {
                SceneDestroyEntity(EntityID);
                return;
            }

            Vector3 ownPosition = GetPosition(EntityID);
            Vector3 targetPosition = GetPosition(playerID);
            Vector3 direction = targetPosition - ownPosition;

            // Rotation - aim at player
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

            // Use the same method as Botnet - create rotation from forward to target
            Vector3 forward = Vector3.Forward; // This should be (0, 0, -1) based on your engine
            Quat targetRot = QuaternionFromTo(forward, toTarget);
            SetRotation(EntityID, ref targetRot);

            if (SimpleMath.Sqrt(direction.X*direction.X + direction.Y*direction.Y + direction.Z*direction.Z) < 0.001f)
                return;

            Quat lookRot = SimpleMath.LookRotation(-direction, Vector3.Up);
            SetRotation(EntityID, ref lookRot);

            if (magnitude < 250.0f)
            {
                isStationary = true;
            } else {
                isStationary = false;
            }

            if (isStationary)
            {
                // timer += deltaTime;

                // if (timer >= stationaryTimer)
                // {
                    //OnSplit();
                    // BARE MINIMUM
                    shootingTimer -= deltaTime;
                    if (shootingTimer <= 0.0f)
                    {
                        ShootAtTarget();
                        shootingTimer = shootingCooldown;
                    }
                //}
            }
            else
            {
                float inverseMag = 1.0f / magnitude;
                Vector3 normDirection = direction * inverseMag;
                Vector3 newPosition = ownPosition + normDirection * speed * deltaTime;
                SetPosition(EntityID, ref newPosition);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
        }

        // Combat
        private void OnBulletHit(string eventName, string payload)
        {
            uint hitEntityID = uint.Parse(payload.Split(',')[0]);
            if (hitEntityID != EntityID)
                return;
            
            Vector3 emptyVec = new Vector3(0, 0, 0);
            RigidbodySetAngularVelocity(EntityID, ref emptyVec);

            health -= 1.0f;
            LogMessage("WormHost hit! Health: " + health);

            if (health <= 0)
            {
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

        // BARE MINIMUM
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

            // Apply force in the direction of the player
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
    }
}

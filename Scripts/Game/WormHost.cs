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

            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (hasSplit || playerID == INVALID_ENTITY)
                return;
            
            Vector3 ownPosition = GetPosition(EntityID);
            Vector3 targetPosition = GetPosition(playerID);
            Vector3 direction = targetPosition - ownPosition;

            // Rotation
            // float yaw = SimpleMath.Atan2(direction.X, direction.Z) + SimpleMath.PI;
            // Vector3 upAxis = new Vector3(0, 1, 0);
            // Quat yawQ = Quat.FromAxisAngle(upAxis, yaw);

            // float horizLen = SimpleMath.Sqrt(direction.X * direction.X + direction.Z * direction.Z);
            // float pitch = SimpleMath.Atan2(direction.Y, horizLen);

            // Vector3 localRight = new Vector3(1, 0, 0);
            // Vector3 rightAxis = SimpleMath.QuatMultiplyVec3(yawQ, localRight);
            // Quat pitchQ = Quat.FromAxisAngle(rightAxis, -pitch);

            // Quat finalRotation = pitchQ * yawQ;
            // SetRotation(EntityID, ref finalRotation);

            if (SimpleMath.Sqrt(direction.X*direction.X + direction.Y*direction.Y + direction.Z*direction.Z) < 0.001f)
                return;

            Quat lookRot = SimpleMath.LookRotation(-direction, Vector3.Up);
            SetRotation(EntityID, ref lookRot);

            // Movement
            float magnitude = SimpleMath.Sqrt(
                direction.X * direction.X +
                direction.Y * direction.Y +
                direction.Z * direction.Z
            );

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
            Engine.Vector3 globalPosition = GetPosition(EntityID);
            Quat rot = Transform.GetRotation(EntityID);
            Engine.Vector3 directionDir = rot.Forward;

            float spawnDist = 1.5f;
            float bulletForce = 100.0f;

            Engine.Vector3 spawnPosition = globalPosition + directionDir * spawnDist;

            uint wormBulletID = SceneCreateEntity("WormBullet");
            if (wormBulletID == 0)
                return;

            Transform.SetPosition(wormBulletID, ref spawnPosition);
            Transform.SetRotation(wormBulletID, ref rot);

            EntityAddRigidBody(wormBulletID);
            RigidbodySetIsKinematic(wormBulletID, false);
            RigidbodySetUseGravity(wormBulletID, false);

            TagSetTag(wormBulletID, "WormBullet");

            Engine.Vector3 force = directionDir * bulletForce;
            RigidbodyAddForce(wormBulletID, ref force);

            Engine.Vector3 extents = new Engine.Vector3(2.0f, 2.0f, 2.0f);
            RigidbodySetBoxHalfExtents(wormBulletID, ref extents);

            EntityAddMeshRenderer(wormBulletID);
            EntityAddScript(wormBulletID, "Game.WormBullet");

        }
    }
}

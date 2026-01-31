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

        [SerializeField] private float speed = 20f;
        [SerializeField] private float stationaryTimer = 100.0f;
        private float timer = 0.0f;

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        // Health
        [SerializeField] private float health = 10.0f;

        // Events
        private const string EVENT_BULLET_HIT = "BulletHit";
        private const string EVENT_HOST_SPLIT = "WormHostSplit";

        // Worm Child
        [SerializeField] private string wormChildPrefabName = "WormChild";
        [SerializeField] private int childCount = 3;

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
            // TODO: To add finding target script

            if (hasSplit || playerID == INVALID_ENTITY)
                return;

            Vector3 ownPosition = GetPosition(EntityID);
            Vector3 targetPosition = GetPosition(playerID);
            Vector3 direction = targetPosition - ownPosition;

            // Rotation
            float yaw = SimpleMath.Atan2(direction.X, direction.Z) + SimpleMath.PI;
            Vector3 upAxis = new Vector3(0, 1, 0);
            Quat yawQ = Quat.FromAxisAngle(upAxis, yaw);

            float horizLen = SimpleMath.Sqrt(direction.X * direction.X + direction.Z * direction.Z);
            float pitch = SimpleMath.Atan2(direction.Y, horizLen);

            Vector3 localRight = new Vector3(1, 0, 0);
            Vector3 rightAxis = SimpleMath.QuatMultiplyVec3(yawQ, localRight);
            Quat pitchQ = Quat.FromAxisAngle(rightAxis, -pitch);

            Quat finalRotation = pitchQ * yawQ;
            SetRotation(EntityID, ref finalRotation);

            // Movement
            float magnitude = SimpleMath.Sqrt(
                direction.X * direction.X +
                direction.Y * direction.Y +
                direction.Z * direction.Z
            );

            if (magnitude < 50.0f)
            {
                isStationary = true;
            }

            if (isStationary)
            {
                timer += deltaTime;

                if (timer >= stationaryTimer)
                {
                    OnSplit();
                }
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

        // Split & Spawn Children
        private void OnSplit()
        {
            if (hasSplit)
                return;

            hasSplit = true;

            Vector3 spawnPos = GetPosition(EntityID);
            LogMessage("======= WormHost splitting =======");

            // for (int i = 0; i < childCount; i++)
            // {
            //     uint child = PrefabInstantiate(wormChildPrefabName);
            //     if (child == INVALID_ENTITY)
            //         continue;

            //     Vector3 offset = new Vector3(
            //         SimpleMath.Cos(i * 2.0f) * 20.0f,
            //         0.0f,
            //         SimpleMath.Sin(i * 2.0f) * 20.0f
            //     );

            //     Vector3 childPos = spawnPos + offset;
            //     SetPosition(child, ref childPos);
            // }

            // Publish(EVENT_HOST_SPLIT, EntityID.ToString());

            // Remove host after split
            Vector3 zero = new Vector3(0, 0, 0);
            RigidbodySetBoxHalfExtents(EntityID, ref zero);
            SceneDestroyEntity(EntityID);
        }
    }
}

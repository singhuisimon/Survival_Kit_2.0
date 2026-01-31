// WormChild.cs
using Engine;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Rigidbody;
using static Engine.Transform;
using static Engine.Tag;

namespace Game
{
    public class WormChild : ScriptBehaviour
    {
        [SerializeField] private float shootingCooldown = 0.25f;
        private float shootingTimer = 0.0f;

        [SerializeField] private float health = 9.0f;

        private Engine.Vector3 stillPos;

        private float OGscaleX = 0.01f;
        private float OGscaleY = 0.01f;
        private float OGscaleZ = 0.01f;

        private const string EVENT_BULLET_HIT = "BulletHit";

        public override void OnStart()
        {
            LogMessage("======= WormChild started (EntityID = " + EntityID + ") =======");

            stillPos = GetPosition(EntityID);

            // Start invisible (same as before)
            Engine.Vector3 disappearScale = new Engine.Vector3(0, 0, 0);
            SetScale(EntityID, ref disappearScale);

            Subscribe(EVENT_BULLET_HIT, OnBulletHit);

            // Replaces WormHostSplit
            ActivateChild();
        }

        public override void OnUpdate(float deltaTime)
        {
            Transform.SetPosition(EntityID, ref stillPos);

            shootingTimer -= deltaTime;
            if (shootingTimer <= 0.0f)
            {
                ShootAtTarget();
                shootingTimer = shootingCooldown;
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
        }

        private void ActivateChild()
        {
            // Appear
            Engine.Vector3 appearScale = new Engine.Vector3(OGscaleX, OGscaleY, OGscaleZ);
            SetScale(EntityID, ref appearScale);

            // Physics setup (unchanged)
            EntityAddRigidBody(EntityID);
            Engine.Vector3 newBoxHalfExtents = new Engine.Vector3(30.0f, 30.0f, 30.0f);
            RigidbodySetBoxHalfExtents(EntityID, ref newBoxHalfExtents);
            RigidbodySetIsKinematic(EntityID, false);
            RigidbodySetUseGravity(EntityID, false);
        }

        private void OnBulletHit(string eventName, string payload)
        {
            uint hitEntityID = uint.Parse(payload.Split(',')[0]);
            if (hitEntityID != EntityID)
                return;

            health -= 1.0f;
            Transform.SetPosition(EntityID, ref stillPos);

            if (health <= 0)
            {
                SceneDestroyEntity(EntityID);
            }
        }

        public void ShootAtTarget()
        {
            // Engine.Vector3 globalPosition = SimpleMath.LocalChildtoWorld(EntityID);
            // Quat rot = Transform.GetRotation(EntityID);
            // Engine.Vector3 forwardDir = rot.Forward;

            // float spawnDist = 1.5f;
            // float bulletForce = 100.0f;

            // Engine.Vector3 spawnPosition = globalPosition + forwardDir * spawnDist;

            // uint wormBulletID = SceneCreateEntity("WormBullet");
            // if (wormBulletID == 0)
            //     return;

            // Transform.SetPosition(wormBulletID, ref spawnPosition);
            // Transform.SetRotation(wormBulletID, ref rot);

            // EntityAddRigidBody(wormBulletID);
            // RigidbodySetIsKinematic(wormBulletID, false);
            // RigidbodySetUseGravity(wormBulletID, false);

            // TagSetTag(wormBulletID, "WormBullet");

            // Engine.Vector3 force = forwardDir * bulletForce;
            // RigidbodyAddForce(wormBulletID, ref force);

            // Engine.Vector3 extents = new Engine.Vector3(2.0f, 2.0f, 2.0f);
            // RigidbodySetBoxHalfExtents(wormBulletID, ref extents);

            // EntityAddMeshRenderer(wormBulletID);
            // EntityAddScript(wormBulletID, "Game.WormBullet");
        }
    }
}

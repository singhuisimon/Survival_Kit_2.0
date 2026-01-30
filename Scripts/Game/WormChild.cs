// WormChild.cs
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
using System.Numerics;

namespace Game
{
    public class WormChild : ScriptBehaviour
    {
        [SerializeField] private float targetX;
        [SerializeField] private float targetY;
        [SerializeField] private float targetZ;

        [SerializeField] private float directionX;
        [SerializeField] private float directionY;
        [SerializeField] private float directionZ;

        private float OGscaleX;
        private float OGscaleY;
        private float OGscaleZ;

        [SerializeField] private float worldLocX;
        [SerializeField] private float worldLocY;
        [SerializeField] private float worldLocZ;

        [SerializeField] private float shootLocX;
        [SerializeField] private float shootLocY;
        [SerializeField] private float shootLocZ;

        [SerializeField] private float shootingCooldown = 0.25f;
        [SerializeField] private float health;

        private Engine.Vector3 stillPos;
        private const uint INVALID_ENTITY = 0xffffffffu;
        private const string EVENT_HOST_SPLIT = "WormHostSplit";
        private const string EVENT_BULLET_HIT = "BulletHit";
        private float shootingTimer = 0.0f;
        private bool hasSplit;

        public override void OnStart()
        {
            // Check own entity
            LogMessage("======= WormChild started (EntityID = " + EntityID + ") =======");

            Engine.Vector3 ownPosition = GetPosition((uint)EntityID);

            targetX = ownPosition.X;
            targetY = ownPosition.Y;
            targetZ = ownPosition.Z;

            Engine.Vector3 ownScale = GetScale((uint)EntityID);
            OGscaleX = 0.01f;
            OGscaleY = 0.01f;
            OGscaleZ = 0.01f;

            health = 9.0f;
            Engine.Vector3 disappearScale = new Engine.Vector3(0,0,0);
            SetScale(EntityID, ref disappearScale);

            hasSplit = false;

            Subscribe("WormHostSplit", OnHostSplit);
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            LogMessage("----------------- Subscribing to WormHostSplit ----------------- ");
            
        }

        public override void OnUpdate(float deltaTime)
        {   
            if(!hasSplit){
                return;
            }

            Transform.SetPosition(EntityID, ref stillPos);
            shootingTimer -= deltaTime;

            if(shootingTimer <= 0.0f){
                ShootAtTarget();
                shootingTimer = shootingCooldown;
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_HOST_SPLIT, OnHostSplit);
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
        }

        private void OnHostSplit(string eventName, string payload)
        {
            Engine.Vector3 appearScale = new Engine.Vector3(OGscaleX,OGscaleY,OGscaleZ);
            SetScale(EntityID, ref appearScale);

            if (isParentCall(payload) == false){
                return;
            }
            
            Engine.Vector3 aim = VectorToString(payload);
            targetX = aim.X;
            targetY = aim.Y;
            targetZ = aim.Z;

            Engine.Vector3 ownPosition = GetPosition((uint)EntityID);
            stillPos.X = ownPosition.X;
            stillPos.Y = ownPosition.Y;
            stillPos.Z = ownPosition.Z;

            //Gave up on aiming for now
            Engine.Vector3 globalPosition = SimpleMath.LocalChildtoWorld((uint)EntityID);
            Engine.Vector3 distanceVec = aim - globalPosition;

            directionX = distanceVec.X;
            directionY = distanceVec.Y;
            directionZ = distanceVec.Z;
            
            EntityAddRigidBody(EntityID);
            Engine.Vector3 newBoxHalfExtents = new Engine.Vector3(30.0f, 30.0f, 30.0f);
            RigidbodySetBoxHalfExtents(EntityID, ref newBoxHalfExtents);
            RigidbodySetIsKinematic(EntityID, false);
            RigidbodySetUseGravity(EntityID, false);

            hasSplit = true;
        }

        private void OnBulletHit(string eventName, string payload){
            LogMessage("WormChild OnBulletHit: " + eventName + ", " + payload);
            health -= 1.0f;
            Transform.SetPosition(EntityID, ref stillPos);
            if(health > 0){
                float ratio = health / 9.0f;
                float currentScaleX = 0.01f * 0.5f * ratio;
                float currentScaleY = 0.01f * 0.5f * ratio;
                float currentScaleZ = 0.01f * 0.5f * ratio;
                
                Engine.Vector3 currentScale = new Engine.Vector3(currentScaleX,currentScaleY,currentScaleZ);
                SetScale(EntityID, ref currentScale);
                LogMessage("WormChild Health: " + health);
                LogMessage("WormChild ratio: " + ratio);
            } else {
                SceneDestroyEntity(EntityID);
            }
        }

        public bool isParentCall (string payload){
            
            string[] parts = payload.Split(',');

            uint parentID = uint.Parse(parts[0]);
            if(parentID == TransformGetParent((uint)EntityID)){
                return true;
            }

            return false;
            
        }

        public Engine.Vector3 VectorToString(string payload){
            
            string[] parts = payload.Split(',');

            float x = float.Parse(parts[1]);
            float y = float.Parse(parts[2]);
            float z = float.Parse(parts[3]);

            return new Engine.Vector3(x, y, z);
        }


        public void ShootAtTarget()
        {
            // Get position and Rotation
            Engine.Vector3 globalPosition = SimpleMath.LocalChildtoWorld((uint)EntityID);

            worldLocX = globalPosition.X;
            worldLocY = globalPosition.Y;
            worldLocZ = globalPosition.Z;

            Quat wormchildRot = Transform.GetRotation(TransformGetParent((uint)EntityID));

            Engine.Vector3 forwardDir = wormchildRot.Forward;

            float spawnDist = 1.5f;
            float bulletForce = 100.0f;

            Engine.Vector3 spawnPosition = new Engine.Vector3 (
                globalPosition.X + forwardDir.X * spawnDist, 
                globalPosition.Y + forwardDir.Y * spawnDist, 
                globalPosition.Z + forwardDir.Z * spawnDist
            );

            uint wormBulletID = SceneCreateEntity("WormBullet");
            if (wormBulletID == 0)
                return;

            shootLocX = spawnPosition.X;
            shootLocY = spawnPosition.Y;
            shootLocZ = spawnPosition.Z;

            Transform.SetPosition(wormBulletID, ref spawnPosition);
            Transform.SetRotation(wormBulletID, ref wormchildRot);
            
            EntityAddRigidBody(wormBulletID);
            RigidbodySetIsKinematic(wormBulletID, false);
            RigidbodySetUseGravity(wormBulletID, false);

            TagSetTag(wormBulletID, "WormBullet");
            Engine.Vector3 force = new Engine.Vector3(
                forwardDir.X * bulletForce,
                forwardDir.Y * bulletForce,
                forwardDir.Z * bulletForce
            );
            RigidbodyAddForce(wormBulletID, ref force);
            Engine.Vector3 newBoxHalfExtents = new Engine.Vector3(2.0f, 2.0f, 2.0f);
          
            RigidbodySetBoxHalfExtents(wormBulletID,ref newBoxHalfExtents);

            EntityAddMeshRenderer(wormBulletID);
            EntityAddScript(wormBulletID, "Game.WormBullet");
            LogMessage("Creating worm bullet with ID: " + wormBulletID);
            LogMessage("Bullet spawn position: X=" + spawnPosition.X + " Y=" + spawnPosition.Y + " Z=" + spawnPosition.Z);
            LogMessage("Bullet force applied: X=" + force.X + " Y=" + force.Y + " Z=" + force.Z);

        }
    }
    
}

// WormHost.cs
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
        [SerializeField] private float health;

        private float ownScaleX;
        private float ownScaleY;
        private float ownScaleZ;

        private const string EVENT_BULLET_HIT = "BulletHit";
        private const string EVENT_HOST_SPLIT = "WormHostSplit";

        public override void OnStart()
        {
            Engine.Vector3 OGscale = GetScale((uint)EntityID);
            ownScaleX = OGscale.X;
            ownScaleY = OGscale.Y;
            ownScaleZ = OGscale.Z;

            health = 18.0f;
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            Subscribe(EVENT_HOST_SPLIT, Disappear);
        }

        public override void OnUpdate(float deltaTime)
        {
            ;
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_HOST_SPLIT, Disappear);
        }

        private void OnBulletHit(string eventName, string payload){

            uint hitEntityID = uint.Parse(payload.Split(',')[0]);

            if(hitEntityID != EntityID){
                return; 
            }
            
            LogMessage("WormHost OnBulletHit: " + eventName + ", " + payload);

            health -= 1.0f;

            // To ensure it stays static

            if(health > 0){

                float ratio = health / 18.0f;

                float currentScaleX = ownScaleX * ratio;
                float currentScaleY = ownScaleY * ratio;
                float currentScaleZ = ownScaleZ * ratio;
                
                Engine.Vector3 currentScale = new Engine.Vector3(currentScaleX,currentScaleY,currentScaleZ);
                SetScale(EntityID, ref currentScale);
                LogMessage("WormHost Health: " + health);
                LogMessage("WormHost ratio: " + ratio);
            } else {
                SceneDestroyEntity(EntityID);
            }
        }

        public void Disappear(string eventName, string payload)
        {
            LogMessage("----------------- DISAPPEAR: " + eventName + ", " + payload +" ----------------- ");
            
            Vector3 newScale = new Vector3(0,0,0);
            SetScale(EntityID, ref newScale);
            Engine.Vector3 newBoxHalfExtents = new Engine.Vector3(0.0f, 0.0f, 0.0f);
            RigidbodySetBoxHalfExtents(EntityID, ref newBoxHalfExtents);

            SceneDestroyEntity(EntityID);
        }
    }
    
}

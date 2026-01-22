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

namespace Game
{
    public class WormChild : ScriptBehaviour
    {
        // [SerializeField] private float positionX;
        // [SerializeField] private float positionY;
        // [SerializeField] private float positionZ;

        [SerializeField] private float targetX;
        [SerializeField] private float targetY;
        [SerializeField] private float targetZ;

        [SerializeField] private float OGscaleX;
        [SerializeField] private float OGscaleY;
        [SerializeField] private float OGscaleZ;

        private const uint INVALID_ENTITY = 0xffffffffu;
        
        // private uint targetID = INVALID_ENTITY;
        // private uint targetID = INVALID_ENTITY;
        
        private const string EVENT_HOST_SPLIT = "WormHostSplit";

        private bool hasSplit;

        public override void OnStart()
        {
            // Check own entity
            LogMessage("======= WormChild started (EntityID = " + EntityID + ") =======");

            Vector3 ownPosition = GetPosition((uint)EntityID);
            // positionX = ownPosition.X;
            // positionY = ownPosition.Y;
            // positionZ = ownPosition.Z;

            targetX = ownPosition.X;
            targetY = ownPosition.Y;
            targetZ = ownPosition.Z;

            Vector3 ownScale = GetScale((uint)EntityID);
            OGscaleX = ownScale.X;
            OGscaleY = ownScale.Y;
            OGscaleZ = ownScale.Z;

            Vector3 disappearScale = new Vector3(0,0,0);
            SetScale(EntityID, ref disappearScale);

            hasSplit = false;

            Subscribe("WormHostSplit", OnHostSplit);
            LogMessage("----------------- Subscribing to WormHostSplit ----------------- ");
            
        }

        public override void OnUpdate(float deltaTime)
        {   
            if(hasSplit){

                LogMessage("Aiming at: " + targetX + ", " + targetY + ", " + targetZ);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_HOST_SPLIT, OnHostSplit);
        }

        private void OnHostSplit(string eventName, string payload)
        {
            Vector3 appearScale = new Vector3(OGscaleX,OGscaleY,OGscaleZ);
            SetScale(EntityID, ref appearScale);

            Vector3 aim = VectorToString(payload);
            targetX = aim.X;
            targetY = aim.Y;
            targetZ = aim.Z;
            
            hasSplit = true;
        }

        public Vector3 VectorToString(string payload){
            
            string[] parts = payload.Split(',');

            float x = float.Parse(parts[0]);
            float y = float.Parse(parts[1]);
            float z = float.Parse(parts[2]);

            return new Vector3(x, y, z);
        }
    }
    
}

// Botnet.cs
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
        [SerializeField] private float positionX;
        [SerializeField] private float positionY;
        [SerializeField] private float positionZ;

        [SerializeField] private float parentX;
        [SerializeField] private float parentY;
        [SerializeField] private float parentZ;

        private const uint INVALID_ENTITY = 0xffffffffu;
        
        private uint parentID = INVALID_ENTITY;
        private uint targetID = INVALID_ENTITY;
        
        private const string EVENT_HOST_SPLIT = "WormHostSplit";

        private bool hasSplit;

        public override void OnStart()
        {
            // Check own entity
            LogMessage("======= WormChild started (EntityID = " + EntityID + ") =======");

            Vector3 ownPosition = GetPosition((uint)EntityID);
            positionX = ownPosition.X;
            positionY = ownPosition.Y;
            positionZ = ownPosition.Z;

            parentID = TransformGetParent((uint)EntityID);
            LogMessage("======= WormChild's parent (parentID = " + parentID + ") =======");

            Vector3 parentPosition = GetPosition((uint)parentID);
            parentX = parentPosition.X;
            parentY = parentPosition.Y;
            parentZ = parentPosition.Z;

            hasSplit = false;

            Subscribe("WormHostSplit", OnHostSplit);
            LogMessage("----------------- Subscribing to WormHostSplit ----------------- ");
            
        }

        public override void OnUpdate(float deltaTime)
        {   
            if(hasSplit){
                LogMessage("Supposed to split");
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_HOST_SPLIT, OnHostSplit);
        }

        private void OnHostSplit(string eventName, string payload)
        {
            LogMessage("----------------- Triggered by WormHostSplit -----------------");
            LogMessage("----------------- Parent Position: " + payload + " -----------------");
            hasSplit = true;
        }
    }
    
}

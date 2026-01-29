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
        private const string EVENT_HOST_SPLIT = "WormHostSplit";

        public override void OnStart()
        {
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

        public void Disappear(string eventName, string payload)
        {
            LogMessage("----------------- DISAPPEAR: " + eventName + ", " + payload +" ----------------- ");
            
            Vector3 newScale = new Vector3(0,0,0);
            SetScale(EntityID, ref newScale);
            Engine.Vector3 newBoxHalfExtents = new Engine.Vector3(0.0f, 0.0f, 0.0f);
            RigidbodySetBoxHalfExtents(EntityID, ref newBoxHalfExtents);
        }
    }
    
}

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

namespace Game
{
    public class WormHost : ScriptBehaviour
    {
        private const uint INVALID_ENTITY = 0xffffffffu;
        
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        public override void OnStart()
        {
            // Check own entity
            LogMessage("======= WormHost started (EntityID = " + EntityID + ") =======");
            
            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID.ToString() + " =======");

            //FindFirstEntityWithTag();
            //SceneFindEntityByName();
            //SceneFindEntitiesByTag()

            ;
        }

        public override void OnUpdate(float deltaTime)
        {
            ;
        }

        public override void OnDestroy()
        {
            ;
        }
    }
    
}

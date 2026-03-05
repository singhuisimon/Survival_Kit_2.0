using Engine;
using System;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Scene;
using static Engine.Prefab;

namespace Game
{
    public class Payload : ScriptBehaviour
    {

        private const string EVENT_COLLECT_PAYLOAD = "CollectPayload";

         // ===== Entity =====
        private uint playerID = 0;
        private string playerName = "Player";

        public override void OnStart()
        {

            playerID = SceneFindEntityByName(playerName);

            if (playerID == 0)
            {
                LogError("[Payload] player cannot be found.");
                return;
            }

            LogMessage("Payload " + EntityID + " initialized");
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            // Don't update when game is paused
            if (GameState.IsPaused)
                return;


            CheckPlayerCollected();
        }

        public override void OnDestroy()
        {
            //Unsubscribe(EVENT_ENEMYCORE_HIT, OnDamageReceived);
            LogMessage("Payload " + EntityID + " destroyed");
        }

        private void CheckPlayerCollected(){

            bool playercollided = false;

            // Query the CollisionManager for hits
            List<uint> collision = CollisionManager.GetPayloadCollisions((uint)EntityID);
            
            if (collision != null && collision.Count > 0)
            {
                // Process all collision
                foreach (uint targetId in collision)
                {
                    //only need to check for player collision and stop checking thereafter
                    if(targetId == playerID){
                        playercollided = true;
                        break;
                    }
                }

            }
 
            if(playercollided){
                //publish event that payload is collected
                Publish(EVENT_COLLECT_PAYLOAD, "");


                //kills itself thereafter
                SceneDestroyEntity((uint)EntityID);
            }

        }
    }
}

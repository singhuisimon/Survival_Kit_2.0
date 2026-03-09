using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Camera;

namespace Game
{
    public class TurretEnemyCounter : ScriptBehaviour
    {
        private string EVENT_TURRET_DESTROYED = "EnemyTurretDestroyed";
        [SerializeField] private int turretCount = 0;
        private bool destroyed = false;

        public override void OnStart()
        {
            Subscribe(EVENT_TURRET_DESTROYED, OnTurretDestroyed);
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if(!destroyed){

                if(turretCount == 1){
                    Publish("OneTurretDestroyed", EntityID.ToString());
                }

                if(turretCount == 6){
                    Publish("FiveTurretsDestroyed", EntityID.ToString());
                    destroyed = true;
                }
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_TURRET_DESTROYED, OnTurretDestroyed);
        }

        private void OnTurretDestroyed(string eventName, string payload){
            ++turretCount;
        }
    }
}
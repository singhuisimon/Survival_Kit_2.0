using Engine;
using System;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Physics;
using static Engine.Event;
using static Engine.Scene;
using static Engine.Rigidbody;

namespace Game{
    public class OOBManager : ScriptBehaviour{

        [SerializeField] private string playerName = "Player";
        [SerializeField] private float originalCountdownOOB = 5.0f;
        [SerializeField] private float countdownOOB = 5.0f;
        [SerializeField] private bool inEnvironment = true;
        [SerializeField] private float nextDamageTime = 0.0f;
        [SerializeField] private float elapsedTime = 0.0f;
        [SerializeField] private float damageCooldown = 1.0f;

        private uint playerEntityID = 0;

        public override void OnStart(){

            playerEntityID = SceneFindEntityByName(playerName);

            if(playerEntityID == 0){
                LogMessage("[OOBManager] player entity cannot be found");
            }

            LogMessage("[OOBManager] OOB EntityID is: " + EntityID.ToString());
            LogMessage("[OOBManager] Player EntityID is: " + playerEntityID.ToString());

            countdownOOB = originalCountdownOOB;
            inEnvironment = true;

            RigidbodySetMass((uint)EntityID, 1.0f);

            //PhysicsEnableCollisionEvents();

            //subscribe to event here
        }

        //put physics/ detection here if there is something visual needed regarding
        //physics / collision
        public override void OnUpdate(float deltaTime){
            elapsedTime += deltaTime;

            if(!inEnvironment){
                CountdownStart(deltaTime);
            } else {
                //reset here
                countdownOOB = originalCountdownOOB;
            }
        }

        //put physics / detection here if nothing visual is needed/important
        public override void OnFixedUpdate(float deltaTime){
            //add the detection here 
            CheckPlayerInEnvrionemnt();

            LogMessage("HEY FIXED UPDATING!!!");
        }

        public override void OnDestroy(){
            
        }


        private void CheckPlayerInEnvrionemnt(){

            LogMessage("CHECKING");
            int count = PhysicsGetCollisionCount();

            bool playerdetected = false;

            if(count <= 0){
                LogMessage("Count is Zero");
                return;
            } 

            if(playerEntityID == 0){
                return;
            }

            uint self = (uint)EntityID;
            
            for(int i = 0; i < count; i++){
                uint a, b;
                PhysicsGetCollisionPair(i, out a, out b);

                if(a != self && b != self)
                    continue;

                uint other = (a == self) ? b : a;

                if(other == playerEntityID){
                    LogMessage("[OOBManager] Player is in Environment");
                    playerdetected = true;
                } 
            }

            if(!playerdetected){
                LogMessage("[OOBManager] Player is not in environment");
                inEnvironment = false;
            } else {
                LogMessage("[OOBManager] Player is in Environment");
                inEnvironment = true;
            }
        }

        private void CountdownStart(float deltaTime){
            countdownOOB -= deltaTime;

            if(countdownOOB <= 0){
                DealDamage();
            }
        }

        private void DealDamage(){
            //first time make sure nextdamage time is immediate
            if(nextDamageTime == 0.0f){
                nextDamageTime = elapsedTime;
            }

            if(elapsedTime >= nextDamageTime){
                //RECEIVER IS AT SPACESHIP CONTROLLER
                DamageSystem.DealDamage(playerEntityID, 20.0f, (uint)EntityID);
                nextDamageTime += damageCooldown; //its 20 per second
            }
        }

    }
}
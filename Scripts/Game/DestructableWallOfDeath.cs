using Engine;
using Game;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;

namespace Game {

    public class IndestructableColliderCube : ScriptBehaviour{

        [SerializeField] bool DamagePlayer = false;
        [SerializeField] float Damage = 9999.0f;

        [SerializeField] private float currentHP = 25.0f;
        [SerializeField] private float maxHP = 25.0f;
        [SerializeField] private bool isDead = false;

        private float EVENT_DESTRUCTABLE_WALL_OF_DEATH_HIT = "Damage:";

        private string playerName = "Player";
        private uint playerID = 0;

        public override void OnStart(){

            playerID = SceneFindEntityByName(playerName);

            if(playerID == 0){
                LogMessage("[IndestructableColliderCube] WARNING: Player entity not found in scene!");
                return;
            }

            isDead = false;
            currentHP = maxHP
            EVENT_DESTRUCTABLE_WALL_OF_DEATH_HIT += EntityID.ToString()

            Subscribe(EVENT_DESTRUCTABLE_WALL_OF_DEATH_HIT, OnDamageReceived);

            LogMessage("[IndestructableColliderCube] Initialized - EntityID: " + EntityID.ToString() + ", PlayerID: " + playerID.ToString());
            LogMessage("[IndestructableColliderCube] Lethal wall active - Damage: " + Damage.ToString());
        }

        public override void OnFixedUpdate(float deltaTime){

            // Only check collisions if we have a valid player reference
            if (playerID == 0)
                return;

            //Only need check/damage once. 9999 is instant kill
            if (DamagePlayer){
                return;
            }

            if(isDead){
                return;
            }

            CheckCollisionWithPlayer();
        }

        public override void OnDestroy(){
            Unsubscribe(EVENT_DESTRUCTABLE_WALL_OF_DEATH_HIT, OnDamageReceived);

            LogMessage("Indestructable wall of death is destroyed");
        }

        private void OnDamageReceived(string EventName, float payload){
            float damage = DamageSystem.ParseAmount(payload);

            currentHP -= damage;

            LogMessage("Destructable wall of death: " EntityID.ToString() + " is hit!. Health is currently: " currentHP.ToString() + "/" + maxHP.ToString());

            if(currentHP <= 0.0f){
                isDead = true;

                Die();
            }
        }

        private void Die(){
            if(!isDead) return;

            SceneDestroyEntity((uint)EntityID);
        }

        private void CheckCollisionWithPlayer(){
            // Query the CollisionManager for hits
            List<uint> playerCollision = CollisionManager.GetPlayerCollisions(playerID);
            
            if (playerCollision != null && playerCollision.Count > 0)
            {
                // Process all hits (in case bullet passed through multiple enemies in one frame)
                foreach (uint collidedEntityId in playerCollision)
                {
                    if(collidedEntityId == (uint)EntityID){
                        DamagePlayer = true;
                        DamageSystem.DealDamage(playerID, Damage, (uint)EntityID);

                        LogMessage("[IndestructableColliderCube] Player contacted lethal wall - dealing " + Damage + " damage");
                        
                        // Only deal damage once per frame even if multiple collision points
                        return;
                    }
                }
                
            }
        }
    }

}
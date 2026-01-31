using Engine;
using Game;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;

namespace Game {

    public class IndestructableColliderCube : ScriptBehaviour{

        [SerializeField] bool DamagePlayer = false;
        [SerializeField] float Damage = 9999.0f;

        private string playerName = "Player";
        private uint playerID = 0;

        public override void OnStart(){

            playerID = SceneFindEntityByName(playerName);

            if(playerID == 0){
                LogMessage("[IndestructableColliderCube] WARNING: Player entity not found in scene!");
                return;
            }

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

            CheckCollisionWithPlayer();
        }

        public override void OnDestroy(){
            
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
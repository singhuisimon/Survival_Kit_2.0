using Engine;
using System;
using System.Collections.Generic;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Physics;
using static Engine.Scene;
using static Engine.Tag;
using static Engine.Rigidbody;

namespace Game {

    public class PrimaryUltAOEDamage : ScriptBehaviour{

        [SerializeField] private float aoeDamage = 30.0f;
        [SerializeField] private float projectileLifetime = 2.0f;

        private List<uint> damagedEntities = new List<uint>();

        private float elapsedTime = 0.0f;

        public override void OnStart(){
            LogMessage("[PrimaryUltAOEDamage] AOE zone spawned: " + EntityID);
        }
        
        public override void OnUpdate(float deltaTime){
            elapsedTime += deltaTime;
        }

        public override void OnFixedUpdate(float deltaTime){
            if(elapsedTime >= projectileLifetime){
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            //CheckCollisions();
        }

        public override void OnDestroy(){

        }

        // -------- COLLISION HANDLING (TAG-BASED, FOR ALL BULLETS) --------

        private void CheckCollisions()
        {
            List<uint> hits = CollisionManager.GetPlayerProjectileHits((uint)EntityID);

            if (hits == null || hits.Count == 0)
                return;

            foreach (uint targetId in hits)
            {
                if (AlreadyDamaged(targetId))
                    continue;

                damagedEntities.Add(targetId);
                OnTargetHit(targetId);
            }
        }

        private bool AlreadyDamaged(uint targetId)
        {
            for (int i = 0; i < damagedEntities.Count; i++)
            {
                if (damagedEntities[i] == targetId)
                    return true;
            }
            return false;
        }


        private void OnTargetHit(uint targetEntityID)
        {
            // Deal AOE damage using DamageSystem
            DamageSystem.DealDamage(targetEntityID, aoeDamage, (uint)EntityID);

            LogMessage("[PrimaryUltAOEDamage] AOE hit target " + targetEntityID + " for " + aoeDamage + " damage");
            
            // Optional: Spawn hit effect per enemy
            // SpawnAOEHitEffect(targetEntityID);
        }


    }

}
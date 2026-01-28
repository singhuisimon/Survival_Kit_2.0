using Engine;
using System;
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
        
        private string[] targets = { "botnet", "wormhost", "wormchild", "adware"};
        private string ultTag = "PrimaryUltExplosion";

        private float elapsedTime = 0.0f;

        public float test = 0.0f;

        public override void OnStart(){

        }
        
        public override void OnUpdate(float deltaTime){
            elapsedTime += deltaTime;

            

            //CheckCollisions();
        }

        public override void OnFixedUpdate(float deltaTime){
            CheckCollisions();

            if(elapsedTime >= projectileLifetime){
                SceneDestroyEntity((uint)EntityID);
                return;
            }
        }

        public override void OnDestroy(){

        }

        // -------- COLLISION HANDLING (TAG-BASED, FOR ALL BULLETS) --------

        private void CheckCollisions()
        {
            int collisionCount = PhysicsGetCollisionCount();

            for (int i = 0; i < collisionCount; i++)
            {
                PhysicsGetCollisionPair(i, out uint entityA, out uint entityB);

                string tagA = TagGetTag(entityA);
                string tagB = TagGetTag(entityB);

                // Case 1: A is bullet, B is valid target
                if (IsBulletTag(tagA) && IsTargetTag(tagB))
                {
                    OnBulletHit(entityA, entityB);
                }
                // Case 2: B is bullet, A is valid target
                else if (IsBulletTag(tagB) && IsTargetTag(tagA))
                {
                    OnBulletHit(entityB, entityA);
                }
            }
        }

        private bool IsBulletTag(string tag)
        {
            if (string.IsNullOrEmpty(tag))
                return false;

            if (!string.IsNullOrEmpty(ultTag) &&
                string.Equals(tag, ultTag, StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }

            return false;
        }

        private bool IsTargetTag(string tag)
        {
            if (string.IsNullOrEmpty(tag) || targets == null)
                return false;

            for (int i = 0; i < targets.Length; i++)
            {
                string target = targets[i];
                if (!string.IsNullOrEmpty(target) &&
                    string.Equals(tag, target, StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }

            LogMessage("Tag: " + tag + "is not the target");
            return false;
        }

        // -------- WHEN A VALID TARGET IS HIT --------

        // bulletEntityID = entity that has a "bullet" tag
        // targetEntityID = entity that has one of TargetTags
        private void OnBulletHit(uint bulletEntityID, uint targetEntityID)
        {

            DamageSystem.DealDamage(targetEntityID, aoeDamage, (uint)EntityID);

            // Log
            LogMessage("AOE SPAWN! BulletHit: target=" + targetEntityID + " from bullet=" + bulletEntityID);

            // Destroy the bullet that actually hit
            //SceneDestroyEntity(bulletEntityID);
        }


    }

}
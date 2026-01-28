using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Rigidbody;
using static Engine.Prefab;
using static Engine.Physics;
using static Engine.Tag;


namespace Game{

    public class PrimaryUltFire : ScriptBehaviour {

        [SerializeField] private float aoeRadius = 10.0f;
        [SerializeField] private float projectileLifetime = 2.0f;
        [SerializeField] private string ultExplosionPrefab = "Sources/Prefabs/PrimaryUltExplosion.prefab";

        private string[] targets = { "botnet", "wormhost", "wormchild", "adware"};
        private string ultTag = "PrimaryUltBullet";

        private float elapsedTime = 0.0f;

        public override void OnStart(){

        }

        public override void OnUpdate(float deltaTime){
            elapsedTime += deltaTime;

            if(elapsedTime >= projectileLifetime){
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            // Collisions (for ALL bullets, decided by tag)
            //CheckCollisions();
        }

        public override void OnFixedUpdate(float deltaTime){
            CheckCollisions();
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
            // Log
            // Instantiate Prefab
            Vector3 Position = GetPosition((uint)EntityID);
            Quat Rot = GetRotation((uint)EntityID);
            Vector3 Scale = new Vector3(aoeRadius, aoeRadius, aoeRadius);
            uint bulletID = 0;
            bulletID = PrefabInstantiateWithTransform(ultExplosionPrefab, ref Position, ref Rot, ref Scale, false);
            if(bulletID == 0){
                LogMessage("[PlayerWeapon] Primary Ult AOE fail to instantiate");
                return; //comment this for debugging temp
            }

            LogMessage("AOE SPAWN! BulletHit: target=" + targetEntityID + " from bullet=" + bulletEntityID);

            // Destroy the bullet that actually hit
            SceneDestroyEntity(bulletEntityID);
        }

    }

}
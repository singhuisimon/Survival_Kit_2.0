using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Physics;
using static Engine.Tag;
using static Engine.Event;

namespace Game
{
    public class PrimaryBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileLifetime = 2.0f;

        [SerializeField]
        public float Damage = 10.0f;

        // Tags this bullet can damage
        [SerializeField]
        private string[] TargetTags = { "botnet", "loveletter", "adware" };

        // Tags that represent bullets (fill this in Inspector with your bullet tag, e.g. "primarybullet")
        [SerializeField]
        private string[] BulletTags = { "Primarybullet" };

        private float elapsedTime = 0.0f;

        public override void OnStart()
        {
        }

        public override void OnUpdate(float deltaTime)
        {
            // Lifetime
            elapsedTime += deltaTime;
            if (elapsedTime >= ProjectileLifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            // Collisions (for ALL bullets, decided by tag)
            CheckCollisions();
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
            if (string.IsNullOrEmpty(tag) || BulletTags == null)
                return false;

            for (int i = 0; i < BulletTags.Length; i++)
            {
                string bulletTag = BulletTags[i];
                if (!string.IsNullOrEmpty(bulletTag) &&
                    string.Equals(tag, bulletTag, StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }

            return false;
        }

        private bool IsTargetTag(string tag)
        {
            if (string.IsNullOrEmpty(tag) || TargetTags == null)
                return false;

            for (int i = 0; i < TargetTags.Length; i++)
            {
                string target = TargetTags[i];
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
            // Publish event
            Publish("BulletHit", targetEntityID.ToString());
            Publish("BulletHitEnemy", true.ToString());
            LogMessage("Event Published! BulletHit: target=" + targetEntityID + " from bullet=" + bulletEntityID);

            // Destroy the bullet that actually hit
            SceneDestroyEntity(bulletEntityID);
        }

        public override void OnDestroy()
        {
            // Optional cleanup hook
        }
    }
}

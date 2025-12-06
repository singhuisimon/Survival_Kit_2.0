using Engine;
using System;

namespace Game
{
    public class PrimaryBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileLifetime = 2.0f;

        [SerializeField]
        public float Damage = 10.0f;

        // Name of the shooter whose rotation we copy (e.g., "Player")
        [SerializeField]
        public string ShooterName = "Player";

        // Tags this bullet can damage
        [SerializeField]
        private string[] TargetTags = { "botnet", "loveletter", "adware" };

        // Tags that represent bullets (fill this in Inspector with your bullet tag, e.g. "primarybullet")
        [SerializeField]
        private string[] BulletTags = { "primarybullet" };

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
                InternalCalls.Scene_DestroyEntity((uint)EntityID);
                return;
            }

            // Collisions (for ALL bullets, decided by tag)
            CheckCollisions();
        }

        // -------- COLLISION HANDLING (TAG-BASED, FOR ALL BULLETS) --------

        private void CheckCollisions()
        {
            int collisionCount = InternalCalls.Physics_GetCollisionCount();

            for (int i = 0; i < collisionCount; i++)
            {
                InternalCalls.Physics_GetCollisionPair(i, out uint entityA, out uint entityB);

                string tagA = InternalCalls.Tag_GetTag(entityA);
                string tagB = InternalCalls.Tag_GetTag(entityB);

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

            InternalCalls.Log("Failed to tag target: " + tag);
            return false;
        }

        // -------- WHEN A VALID TARGET IS HIT --------

        // bulletEntityID = entity that has a "bullet" tag
        // targetEntityID = entity that has one of TargetTags
        private void OnBulletHit(uint bulletEntityID, uint targetEntityID)
        {
            // Publish event
            EventSystem.Publish("BulletHit", targetEntityID.ToString());
            Log("Event Published! BulletHit: target=" + targetEntityID + " from bullet=" + bulletEntityID);

            // Destroy the bullet that actually hit
            InternalCalls.Scene_DestroyEntity(bulletEntityID);
        }

        public override void OnDestroy()
        {
            // Optional cleanup hook
        }
    }
}

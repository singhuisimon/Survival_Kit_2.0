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

            // Collisions
            CheckCollisions();
        }

        // -------- COLLISION HANDLING (TAG-FILTERED) --------

        private void CheckCollisions()
        {
            int collisionCount = InternalCalls.Physics_GetCollisionCount();
            uint bulletID = EntityID;

            for (int i = 0; i < collisionCount; i++)
            {
                InternalCalls.Physics_GetCollisionPair(i, out uint entityA, out uint entityB);

                uint otherEntity;

                if (entityA == bulletID)
                {
                    otherEntity = entityB;
                }
                else if (entityB == bulletID)
                {
                    otherEntity = entityA;
                }
                else
                {
                    // This pair does not involve the bullet
                    continue;
                }

                // Get the tag of the other entity
                string otherTag = InternalCalls.Tag_GetTag(otherEntity);
                // Only react if the tag matches one of our TargetTags
                if (IsTargetTag(otherTag))
                {
                    OnHitEnemy(otherEntity);
                    return; // bullet will be destroyed
                }
            }
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
            return false;
        }

        // -------- WHEN A VALID TARGET IS HIT --------

        public void OnHitEnemy(uint targetEntityID)
        {
            // Publish event BEFORE destroying the bullet
            EventSystem.Publish("BulletHit", targetEntityID.ToString());
            InternalCalls.Scene_DestroyEntity((uint)EntityID);
        }

        public override void OnDestroy()
        {
            // Optional cleanup hook
        }
    }
}

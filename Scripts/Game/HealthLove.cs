using Engine;
using System;
using static Engine.Log;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Scene;

namespace Game
{
    public class HealthLove : ScriptBehaviour
    {
        private const float MaxHealth = 100.0f;
        private float CurrentHealth = 100.0f;
        private const float DamagePerHit = 50.0f;
        private bool isDead = false;

        private const float SPAWN_GRACE_TIME = 0.5f;
        private float spawnTimer = 0.0f;
        private bool isInvulnerable = true;

        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            isDead = false;
            isInvulnerable = true;
            spawnTimer = SPAWN_GRACE_TIME;

            // Subscribe to bullet hits
            Subscribe("BulletHit", OnBulletHit);

            LogMessage("Core " + EntityID + " Health initialized");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead) return;

            if (isInvulnerable)
            {
                spawnTimer -= deltaTime;
                if (spawnTimer <= 0.0f)
                {
                    isInvulnerable = false;
                    LogMessage("Core " + EntityID + " is now vulnerable!");
                }
                return;
            }
        }

        private void OnBulletHit(string eventName, string payload)
        {
            if (!uint.TryParse(payload, out uint hitEntity))
                return;

            // If this core was hit
            if (hitEntity == EntityID && !isInvulnerable)
            {
                LogMessage("Core " + EntityID + " hit by bullet!");
                TakeDamage();
            }
        }

        private void TakeDamage()
        {
            CurrentHealth -= DamagePerHit;
            LogMessage("Core " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

            if (CurrentHealth <= 0.0f)
            {
                Die();
            }
        }

        private void Die()
        {
            if (isDead) return;
            isDead = true;

            uint parentEntityID = TransformGetParent((uint)EntityID);

            if (parentEntityID != 0)
            {
                Publish("CoreDestroyed", parentEntityID.ToString());
            }

            SceneDestroyEntity((uint)EntityID);
        }

        public override void OnDestroy()
        {
            LogMessage("Core " + EntityID + " destroyed");
        }
    }
}

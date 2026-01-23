using Engine;
using System;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Scene;

namespace Game
{
    public class HealthCore : ScriptBehaviour
    {
        private const float MaxHealth = 100.0f;
        private float CurrentHealth = 100.0f;
        private const float DamagePerHit = 50.0f;
        private bool isDead = false;

        private const float SPAWN_GRACE_TIME = 0.5f;
        private float spawnTimer = 0.0f;
        private bool isInvulnerable = true;

        private const string EVENT_CORE_HIT = "CoreHit";

        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            isDead = false;
            isInvulnerable = true;
            spawnTimer = SPAWN_GRACE_TIME;

            // Subscribe to bullet hits
            Subscribe(EVENT_CORE_HIT, OnEnemyBulletHit);

            LogMessage("CoreMotherboard " + EntityID + " Health initialized");
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
                    LogMessage("CoreMotherboard " + EntityID + " is now vulnerable!");
                }
                return;
            }
        }

        private void OnEnemyBulletHit(string eventName, string coreMotherboard)
        {
            if (!uint.TryParse(coreMotherboard, out uint hitEntity))
                return;

            // If this core was hit
            if (hitEntity == EntityID && !isInvulnerable)
            {
                LogMessage("CoreMotherboard " + EntityID + " hit by bullet!");
                TakeDamage();
            }
        }

        private void TakeDamage()
        {
            CurrentHealth -= DamagePerHit;
            LogMessage("CoreMotherboard " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

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
                Publish("CoreMotherboardDestroyed", parentEntityID.ToString());
            }

            SceneDestroyEntity((uint)EntityID);
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_CORE_HIT, OnEnemyBulletHit);
            LogMessage("CoreMotherboard " + EntityID + " destroyed");
        }
    }
}

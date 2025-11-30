using Engine;
using System;

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
            Engine.EventSystem.Subscribe("BulletHit", OnBulletHit);

            Engine.InternalCalls.Log("Core " + EntityID + " Health initialized");
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
                    Engine.InternalCalls.Log("Core " + EntityID + " is now vulnerable!");
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
                Engine.InternalCalls.Log("Core " + EntityID + " hit by bullet!");
                TakeDamage();
            }
        }

        private void TakeDamage()
        {
            CurrentHealth -= DamagePerHit;
            Engine.InternalCalls.Log("Core " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

            if (CurrentHealth <= 0.0f)
            {
                Die();
            }
        }

        private void Die()
        {
            if (isDead) return;
            isDead = true;

            uint parentEntityID = Engine.InternalCalls.Transform_GetParent((uint)EntityID);

            if (parentEntityID != 0)
            {
                Engine.EventSystem.Publish("CoreDestroyed", parentEntityID.ToString());
            }

            Engine.InternalCalls.Scene_DestroyEntity((uint)EntityID);
        }

        public override void OnDestroy()
        {
            Engine.InternalCalls.Log("Core " + EntityID + " destroyed");
        }
    }
}

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

        // ===== Spawn Grace Period =====
        private const float SPAWN_GRACE_TIME = 0.5f;  // Invulnerable for 0.5 seconds after spawn
        private float spawnTimer = 0.0f;
        private bool isInvulnerable = true;

        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            isDead = false;
            isInvulnerable = true;
            spawnTimer = SPAWN_GRACE_TIME;
            Engine.InternalCalls.Log("Core " + EntityID + " Health initialized (invulnerable for " + SPAWN_GRACE_TIME + " seconds)");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead) return;

            // Handle spawn grace period
            if (isInvulnerable)
            {
                spawnTimer -= deltaTime;
                if (spawnTimer <= 0.0f)
                {
                    isInvulnerable = false;
                    Engine.InternalCalls.Log("Core " + EntityID + " is now vulnerable!");
                }
                return;  // Don't check collisions while invulnerable
            }

            CheckBulletCollision();
        }

        /// <summary>
        /// Check if this core collided with a bullet
        /// </summary>
        private void CheckBulletCollision()
        {
            int collisionCount = Engine.InternalCalls.Physics_GetCollisionCount();

            for (int i = 0; i < collisionCount; i++)
            {
                Engine.InternalCalls.Physics_GetCollisionPair(i, out uint entityA, out uint entityB);

                // Check if this core is involved in collision
                if (entityA == EntityID || entityB == EntityID)
                {
                    Engine.InternalCalls.Log("Core " + EntityID + " hit by bullet!");
                    TakeDamage();
                    return;
                }
            }
        }

        /// <summary>
        /// Take damage from bullet hit (50 per hit)
        /// </summary>
        private void TakeDamage()
        {
            CurrentHealth -= DamagePerHit;
            Engine.InternalCalls.Log("Core " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

            if (CurrentHealth <= 0.0f)
            {
                Die();
            }
        }

        /// <summary>
        /// Core destroyed after 2 hits
        /// </summary>
        private void Die()
        {
            if (isDead) return;
            isDead = true;

            uint parentEntityID = Engine.InternalCalls.Transform_GetParent((uint)EntityID);

            if (parentEntityID != 0)
            {
                // Publish event: channel="CoreDestroyed", payload=parentEntityID
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

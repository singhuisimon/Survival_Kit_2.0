using Engine;
using System;

namespace Game
{
    /// <summary>
    /// Health component for LoveLetter core sub-entities
    /// Detects bullet collisions and takes 50 damage per hit
    /// Dies after 2 hits (100 health total)
    /// Calls parent's OnCoreDestroyed when this core dies
    /// </summary>
    public class HealthLove : ScriptBehaviour
    {
        private const float MaxHealth = 100.0f;
        private float CurrentHealth = 100.0f;
        private const float DamagePerHit = 50.0f;
        private bool isDead = false;



        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            isDead = false;
            Engine.InternalCalls.Log("Core " + EntityID + " Health initialized");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead) return;

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

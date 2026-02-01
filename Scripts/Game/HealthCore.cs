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
        [SerializeField] private const float MaxHealth = 100.0f;
        [SerializeField] private float CurrentHealth = 100.0f;
        [SerializeField] private const float DamagePerHit = 50.0f;
        [SerializeField] private bool isDead = false;

        private const float SPAWN_GRACE_TIME = 0.5f;
        private float spawnTimer = 0.0f;
        private bool isInvulnerable = true;
        private bool isgameover = false;

        private string EVENT_CORE_HIT = "Damage:";
        private string EVENT_CORE_HEALTHCHANGE = "Core Health Change";
        private const string EVENT_PLAYER_DEAD = "PlayerDead";

        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            isDead = false;
            isInvulnerable = true;
            isgameover = false;
            spawnTimer = SPAWN_GRACE_TIME;
            EVENT_CORE_HIT += EntityID.ToString();

            // Subscribe to bullet hits
            Subscribe(EVENT_CORE_HIT, OnDamageReceived);
            Subscribe(EVENT_PLAYER_DEAD, OnPlayerDeath);
            Publish(EVENT_CORE_HEALTHCHANGE, CurrentHealth.ToString());  // ADD THIS


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

        private void OnPlayerDeath(string eventName, string payload){
            LogMessage("CoreMotherboard " + EntityID.ToString() + "is now immune!");
            isgameover = true;
        }

        private void OnDamageReceived(string eventName, string payload)
        {

            if(isgameover) return;

            float damage = DamageSystem.ParseAmount(payload);

            CurrentHealth -= damage;
            Publish(EVENT_CORE_HEALTHCHANGE, CurrentHealth.ToString());  // ADD THIS

            LogMessage("CoreMotherboard " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

            if(CurrentHealth <= 0.0f){
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
            Unsubscribe(EVENT_CORE_HIT, OnDamageReceived);
            LogMessage("CoreMotherboard " + EntityID + " destroyed");
        }
    }
}

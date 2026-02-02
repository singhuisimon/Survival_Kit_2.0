using Engine;
using System;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Scene;
using static Engine.Prefab;

namespace Game
{
    public class EnemyCore : ScriptBehaviour
    {
        [SerializeField] private const float MaxHealth = 500.0f;
        [SerializeField] private float CurrentHealth = 500.0f;
        [SerializeField] private bool isDead = false;

        //private const float SPAWN_GRACE_TIME = 0.5f;
        //private float spawnTimer = 0.0f;
        //private bool isInvulnerable = true;

        private string EnemyCore_ExplosionPrefab = "Sources/Prefabs/EnemyCoreExplosion.prefab";

        private string EVENT_ENEMYCORE_HIT = "Damage:";

        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            isDead = false;
            //spawnTimer = SPAWN_GRACE_TIME;
            EVENT_ENEMYCORE_HIT += EntityID.ToString();

            // Subscribe to bullet hits
            Subscribe(EVENT_ENEMYCORE_HIT, OnDamageReceived);

            LogMessage("EnemyCore " + EntityID + " Health initialized");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Don't update when game is paused
            if (GameState.IsPaused)
                return;

            //if (!isDead) return;

            // if (isInvulnerable)
            // {
            //     spawnTimer -= deltaTime;
            //     if (spawnTimer <= 0.0f)
            //     {
            //         isInvulnerable = false;
            //         LogMessage("EnemyCore " + EntityID + " is now vulnerable!");
            //     }
            //     return;
            // }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_ENEMYCORE_HIT, OnDamageReceived);
            LogMessage("EnemyCore " + EntityID + " destroyed");
        }

        private void OnDamageReceived(string eventName, string payload)
        {

            float damage = DamageSystem.ParseAmount(payload);

            CurrentHealth -= damage;
            LogMessage("EnemyCore " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

            if(CurrentHealth <= 0.0f){
                isDead = true;
                Die();
            }
        }

        private void Die()
        {
            if (!isDead) return;

            //spawn
            Vector3 spawnpos = GetPosition((uint)EntityID);
            Quat spawnrot = GetRotation((uint)EntityID);
            Vector3 spawnscale = GetScale((uint)EntityID);

            // Spawn the ult bullet
            uint explosion = PrefabInstantiateWithTransform(EnemyCore_ExplosionPrefab, ref spawnpos, ref spawnrot, ref spawnscale, false);
            if(explosion == 0){
                LogMessage("[EnemyCore] EnemyCore_ExplosionPrefab fail to instantiate");
                return;
            } else {
                LogMessage("[EnemyCore] Instantiating EnemyCore_ExplosionPrefab success! ID is: " + explosion.ToString());
            }

            SceneDestroyEntity((uint)EntityID);            
        }
    }
}

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
        [SerializeField] private const float MaxHealth = 600.0f;
        [SerializeField] private float CurrentHealth = 600.0f;
        [SerializeField] private float prevHealth = 0.0f;
        [SerializeField] private bool isDead = false;

        //private const float SPAWN_GRACE_TIME = 0.5f;
        //private float spawnTimer = 0.0f;
        //private bool isInvulnerable = true;

        private string EnemyCore_ExplosionPrefab = "Sources/Prefabs/EnemyCoreExplosion.prefab";
        private string EnemyCore_CorruptAudioPrefab = "Sources/Prefabs/Audio_CorruptCoreDestroyed.prefab";

        private string EVENT_ENEMYCORE_HIT = "Damage:";

        private const string EVENT_ENEMYCORE_HEALTHCHANGE = "EnemyCore Health Change";
        private bool skipTutorial = false;

        private string[] Core_Sub_Entities = { 
            "Decor_EnemyCore_Core", 
            "Decor_EnemyCore_CoreLines", 
            "Decor_EnemyCore_CoreLines2", 
            "Decor_EnemyCore_CoreLines3", 
            "Decor_EnemyCore_CoreLines4", 
            "Decor_EnemyCore_CoreLines5", 
            "DecorLoveletterMesh" 
        };

        private const uint INVALID_ENTITY = 0xffffffffu;

        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            prevHealth = CurrentHealth;
            isDead = false;
            //spawnTimer = SPAWN_GRACE_TIME;
            EVENT_ENEMYCORE_HIT = "Damage:" + EntityID.ToString();

            // Subscribe to bullet hits
            Subscribe(EVENT_ENEMYCORE_HIT, OnDamageReceived);

            //publish healthcore change
            Publish(EVENT_ENEMYCORE_HEALTHCHANGE, CurrentHealth.ToString());
            //Vector3 newpos = new Vector3(-5504.39f, -438.72f, 643.28f);
            //Engine.Transform.SetPosition(EntityID, ref newpos);

            LogMessage("EnemyCore " + EntityID + " Health initialized");
            LogMessage("EnemyCore " + EntityID + " Health is: " + CurrentHealth.ToString() + "/" + MaxHealth.ToString());
        }

        public override void OnUpdate(float deltaTime)
        {
            // Don't update when game is paused
            if (GameState.IsPaused)
                return;

            if(prevHealth != CurrentHealth){
                LogMessage("EnemyCore detect health change!!!");
                prevHealth = CurrentHealth;
            }

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
            if(isDead) return;

            float damage = DamageSystem.ParseAmount(payload);

            CurrentHealth -= damage;
            LogMessage("EnemyCore is being damaged by: " + damage.ToString() );
            LogMessage("EnemyCore " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

            Publish(EVENT_ENEMYCORE_HEALTHCHANGE, CurrentHealth.ToString());

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

            string data = TransformToString(spawnpos, spawnrot);

            // Spawn the explosion
            uint explosion = PrefabInstantiateWithTransform(EnemyCore_ExplosionPrefab, ref spawnpos, ref spawnrot, ref spawnscale, false);
            if(explosion == 0){
                LogMessage("[EnemyCore] EnemyCore_ExplosionPrefab fail to instantiate");
                return;
            } else {
                LogMessage("[EnemyCore] Instantiating EnemyCore_ExplosionPrefab success! ID is: " + explosion.ToString());
            }

            if (!skipTutorial)
            {
                Publish("CoreDeadTriggerPostTrenchRun", data);
            }

            //Publish("EnemyCoreDeath", "");

            foreach (string name in Core_Sub_Entities)
            {
                uint id = SceneFindEntityByName(name);

                if (id != INVALID_ENTITY)
                {
                    SceneDestroyEntity(id);   
                }
            }
            // Spawn the audio
            uint audioID = 0;
            audioID = PrefabInstantiate(EnemyCore_CorruptAudioPrefab);
            if(audioID == 0){
                LogMessage("[EnemyCore] Failed to instantiate audio after death");
            } else {
                LogMessage("[EnemyCore] Succeed in intantiating EnemyCore_CorruptAudioPrefab");
            }

            if (skipTutorial)
            {
                Publish("EnemyCoreDeath", "");   
            }

            SceneDestroyEntity((uint)EntityID);            
        }

        public static string TransformToString(Vector3 pos, Quat rot)
        {
            return pos.X + "," + pos.Y + "," + pos.Z + "|" +
                rot.X + "," + rot.Y + "," + rot.Z + "," + rot.W;
        }
    }
}

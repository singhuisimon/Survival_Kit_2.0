using Engine;
using System;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Transform;
using static Engine.Scene;
using static Engine.Audio;

namespace Game
{
    public class HealthCore : ScriptBehaviour
    {
        [SerializeField] private const float MaxHealth = 100.0f;
        [SerializeField] private float CurrentHealth = 100.0f;
        [SerializeField] private const float DamagePerHit = 50.0f;
        [SerializeField] private bool isDead = false;
        [SerializeField] private float AudioLoopDuration = 3.2f;

        private const float SPAWN_GRACE_TIME = 0.5f;
        private float spawnTimer = 0.0f;
        private bool isInvulnerable = true;
        private bool isgameover = false;

        // Audio tracking for damage loop
        private float damageTimeoutTimer = 0.0f;
        private bool damageTakenThisCycle = false;

        private string EVENT_CORE_HIT = "Damage:";
        private string EVENT_CORE_HEALTHCHANGE = "Core Health Change";
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_GAME_WIN = "GameWin";
        private const string INNERCORENAME = "InnerCore";

        private uint innercoreID = 0;

        public override void OnStart()
        {
            CurrentHealth = MaxHealth;
            isDead = false;
            isInvulnerable = true;
            isgameover = false;
            spawnTimer = SPAWN_GRACE_TIME;
            EVENT_CORE_HIT += EntityID.ToString();

            // Audio initialization
            damageTimeoutTimer = 0.0f;
            damageTakenThisCycle = false;

            innercoreID = SceneFindEntityByName(INNERCORENAME);

            if (innercoreID == 0)
            {
                LogMessage("[HealthCore] innercore entity cannot be found");
            }

            // Subscribe to bullet hits
            Subscribe(EVENT_CORE_HIT, OnDamageReceived);
            Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Subscribe(EVENT_GAME_WIN, OnGameEnd);
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

            // Handle audio looping logic - check if audio is actively playing
            if (AudioIsPlaying(innercoreID))
            {
                damageTimeoutTimer -= deltaTime;
                
                // If we've completed a full audio loop cycle
                if (damageTimeoutTimer <= 0.0f)
                {
                    // Check if damage was taken during this cycle
                    if (!damageTakenThisCycle)
                    {
                        // No damage this cycle - stop the audio
                        StopDamageAudio();
                        LogMessage("CoreMotherboard " + innercoreID + " - No damage during loop, stopping audio");
                    }
                    else
                    {
                        // Damage occurred - reset for next cycle
                        damageTakenThisCycle = false;
                        damageTimeoutTimer = AudioLoopDuration;
                        LogMessage("CoreMotherboard " + innercoreID + " - Continuing damage audio loop");
                    }
                }
            }
        }

        private void OnGameEnd(string eventName, string payload){
            LogMessage("CoreMotherboard " + EntityID.ToString() + "is now immune!");
            isgameover = true;

            // Stop audio when game ends
            if (AudioIsPlaying(innercoreID))
            {
                StopDamageAudio();
            }
        }

        private void OnDamageReceived(string eventName, string payload)
        {

            if(isgameover) return;

            float damage = DamageSystem.ParseAmount(payload);

            CurrentHealth -= damage;
            Publish(EVENT_CORE_HEALTHCHANGE, CurrentHealth.ToString());  // ADD THIS

            LogMessage("CoreMotherboard " + EntityID + " hit! Health: " + CurrentHealth + "/" + MaxHealth);

            // Handle damage audio
            if (!AudioIsPlaying((uint)EntityID))
            {
                // Start playing audio with loop
                StartDamageAudio();
                LogMessage("CoreMotherboard " + EntityID + " - Starting damage audio loop");
            }

            // Mark that damage was taken this cycle
            damageTakenThisCycle = true;
            damageTimeoutTimer = AudioLoopDuration; // Reset the timer

            if(CurrentHealth <= 0.0f){
                Die();
            }
        }

        private void StartDamageAudio()
        {
            if(innercoreID == 0){
                LogMessage("[HealthCore] Invalid ID for innercore")
                return;
            }

            // Set audio to loop and play
            AudioSetLoop(innercoreID, true);
            AudioPlay(innercoreID);
            
            damageTakenThisCycle = true;
            damageTimeoutTimer = AudioLoopDuration;
        }
        
        private void StopDamageAudio()
        {
            if(innercoreID == 0){
                LogMessage("[HealthCore] Invalid ID for innercore")
                return;
            }
            AudioStop(innercoreID);
            
            damageTakenThisCycle = false;
            damageTimeoutTimer = 0.0f;
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
            // Stop audio before cleanup
            if (AudioIsPlaying(innercoreID))
            {
                StopDamageAudio();
            }

            Unsubscribe(EVENT_CORE_HIT, OnDamageReceived);
            Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Unsubscribe(EVENT_GAME_WIN, OnGameEnd);
            LogMessage("CoreMotherboard " + EntityID + " destroyed");
        }
    }
}

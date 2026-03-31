using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;
using static Engine.Audio;
namespace Game
{
    /// <summary>
    /// AmmoBar - Visual representation of player ammo
    /// Position adjustment: Move by FULL width change (not half)
    /// Left edge = Position.X - Width (scale.X is radius, not diameter)
    /// Attach this to the ammo bar sprite entity
    /// </summary>
    public class AudioVOHealthCore : ScriptBehaviour
    {
        // ===== Event Names =====
        private string Event_CoreDamage = "Core Health Change";

        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private const string EVENT_TIMER_FINISHED = "TimerFinished";

        // ===== State =====
        private bool initialized = false;
        private bool firstchange = false;
        private bool gameend = false;
        private bool paused = false;
        [SerializeField] private bool timerstart = false;

        // ===== Timer Values =====
        private float timerCooldown = 5.0f;
        [SerializeField] private float elapsedTimer = 0.0f;

        private string basepath = "System AI_VO_Core Damage ";

        private const string EVENT_CORE_PLAY = "CorePlaying";

        public override void OnStart()
        {
            LogMessage("=== AudioVOHealthCore OnStart ===");
            LogMessage("AudioVOHealthCore EntityID: " + EntityID);

            Event.Subscribe(Event_CoreDamage, OnCoreDamage);

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameEnd);

            timerstart = false;
            initialized = true;

            LogMessage("AudioVOHealthCore initialized:");
        }

        public override void OnUpdate(float deltaTime)
        {
            if(!initialized)
                return;

            if(GameState.IsPaused){
                if(AudioIsPlaying((uint)EntityID) && !paused){
                    LogMessage("AudioVOHealthCore is paused, pausing timer countodwn and stopping audio");
                    AudioPause((uint)EntityID);
                    paused = true;
                }

                return;
            }

            if(paused){
                //should audio be paused resume
                AudioPlay((uint)EntityID);
                paused = false;
            }
            
            if(timerstart){
                if(AudioIsPlaying((uint)EntityID)){
                    return;
                }

                elapsedTimer -= deltaTime;

                if(elapsedTimer <= 0.0f){

                    if(gameend){
                        //game ended there isn't a need to play the audio.
                        return;
                    }

                    RandomizeAudioPath();
                    AudioPlay((uint)EntityID);
                    Publish(EVENT_CORE_PLAY, "");
                    timerstart = false;
                }
            }

        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(Event_CoreDamage, OnCoreDamage);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameEnd);

            LogMessage("=== AudioVOHealthCore Destroyed ===");
        }

        private void RandomizeAudioPath(){
            int randomint = RNG.RandInt(1, 3);
            string filepath = basepath + randomint.ToString() + ".wav";
            AudioSetFile((uint)EntityID, filepath);
            LogMessage("[AudioVOHealthCore] Setting audio filepath to be: " + filepath);
        }

        private void OnCoreDamage(string eventName, string payload){

            if(!firstchange){
                firstchange = true;
                return;
            }

            timerstart = true;

            if(elapsedTimer <= 0.0f){
                elapsedTimer = timerCooldown;
            } else {
                elapsedTimer = timerCooldown;
            }

            LogMessage("[AudioVOHealthCore] Detect core damage setting timer start true and timer");
        }

        private void OnGameEnd(string eventName, string payload){
            //optional i can choose to actually self destruct instead.
            gameend = true;
            LogMessage("[AudioVOHealthCore] Detect Game End Preventing VO from health to be played");
        }
    }
}
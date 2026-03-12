using System;
using Engine;
using static Engine.Logger;
using static Engine.Audio;
namespace Game
{
    /// <summary>
    /// AmmoBar - Visual representation of player ammo
    /// Position adjustment: Move by FULL width change (not half)
    /// Left edge = Position.X - Width (scale.X is radius, not diameter)
    /// Attach this to the ammo bar sprite entity
    /// </summary>
    public class AudioPause : ScriptBehaviour
    {
        // ===== EVENT =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private const string EVENT_TIMER_FINISHED = "TimerFinished";


        // ===== State =====
        private bool initialized = false;
        [SerializeField] private bool audiostarted = false;
        private bool disabled = false;

        // ===== Timer =====
        private float startaudio = 5.0f;
        [SerializeField] private float elapsedTime = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== AudioPause OnStart ===");

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameEnd);

            initialized = true;

            audiostarted = false;

            disabled = false;

            LogMessage("AudioPause initialized:");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || disabled)
                return;

            if (GameState.IsPaused){

                if(AudioIsPlaying((uint)EntityID)){
                    LogMessage("[AudioPause] Pausing audio on entity");
                    AudioPause((uint)EntityID);
                }
                return;
            }

            if(!audiostarted){
                elapsedTime += deltaTime;

                if(elapsedTime > startaudio){
                    AudioPlay((uint)EntityID);
                    LogMessage("[AudioPause] Start playing audio attached to the entity");
                    audiostarted = true;

                    return;
                }
            }

            if(!AudioIsPlaying((uint)EntityID) && audiostarted){
                AudioPlay((uint)EntityID);
                LogMessage("[AudioPause] Playing the audio attached to the entity");
            }
        }

        // ===== EVENT HANDLERS =====
        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameEnd);

            LogMessage("=== AmmoBar Destroyed ===");
        }

        private void OnGameEnd(string eventName, string payload){
            disabled = true;
            AudioStop((uint)EntityID);
            LogMessage("[AudioPause] Detect game end, stopping audio from playing");
        }
    }
}
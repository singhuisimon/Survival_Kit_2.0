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
        private const string EVENT_LEVEL2_TUTORIAL_PAUSE = "Level2TutorialPause";


        // ===== State =====
        private bool initialized = false;
        [SerializeField] private bool audiostarted = false;
        private bool disabled = false;
        private bool isPaused = false;
        private bool pauseForTutorial = false;

        // ===== Timer =====
        private float startaudio = 5.0f;
        [SerializeField] private float elapsedTime = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== AudioPause OnStart ===");

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Event.Subscribe(EVENT_LEVEL2_TUTORIAL_PAUSE, OnLevel2Pause);

            initialized = true;

            audiostarted = false;

            isPaused = false;

            disabled = false;

            LogMessage("AudioPause initialized:");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || disabled)
                return;

            if (GameState.IsPaused){
                
                // Continue audio if pause is for tutorial in level 2
                if(!pauseForTutorial) {
                    if(!isPaused && AudioIsPlaying((uint)EntityID)){
                        LogMessage("[AudioPause] Pausing audio on entity");
                        AudioPause((uint)EntityID);
                        isPaused = true;
                    }
                    return;
                }
            }

            if(isPaused && audiostarted){
                AudioPlay((uint)EntityID);
                LogMessage("[AudioPause] Resuming audio after unpause");
                isPaused = false;
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
            // Stop audio so paused channels don't leak into the next scene
            AudioStop((uint)EntityID);

            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Event.Unsubscribe(EVENT_LEVEL2_TUTORIAL_PAUSE, OnLevel2Pause);

            LogMessage("=== AmmoBar Destroyed ===");
        }

        private void OnGameEnd(string eventName, string payload){
            disabled = true;
            AudioStop((uint)EntityID);
            LogMessage("[AudioPause] Detect game end, stopping audio from playing");
        }

        private void OnLevel2Pause(string eventName, string payload) {
            if(bool.TryParse(payload, out bool state)) {
                // Update state for pause during tutorial
                pauseForTutorial = state;

                LogMessage("[AudioPause] Level 2 pause for tutorial, continue playing audio");
            }
        }
    }
}
using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;
using static Engine.Prefab;
using static Engine.Audio;
namespace Game
{
    /// <summary>
    /// AmmoBar - Visual representation of player ammo
    /// Position adjustment: Move by FULL width change (not half)
    /// Left edge = Position.X - Width (scale.X is radius, not diameter)
    /// Attach this to the ammo bar sprite entity
    /// </summary>
    public class AudioVOTimer : ScriptBehaviour
    {
        // ===== Event Names =====

        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string EVENT_2MIN = "2MIN";
        private const string EVENT_1MIN = "1MIN";
        private const string EVENT_10s = "10SEC";

        // ===== State =====
        private bool initialized = false;
        private bool firstchange = false;
        private bool gameend = false;
        private bool paused = false;
        private bool last10s = false;

        // ===== Timer Values =====

        private float countdowntimer = 0.0f;
        private float DelayFor2nd5secWarn = 2.8f;
        [SerializeField] private float elapsedTimer = 0.0f;

        // ====== Audio Prefab Path =====
        private const string Audio2minVOPrefab = "Sources/Prefabs/Audio_VO_2Min.prefab";
        private const string Audio1minVOPrefab = "Sources/Prefabs/Audio_VO_1Min.prefab";
        private const string Audio10sVOPrefab = "Sources/Prefabs/Audio_VO_5sec_1.prefab";
        private const string Audio7sVOPrefab = "Sources/Prefabs/Audio_VO_5sec_2.prefab";

        // =========== Entity Tracking ===========
        private uint currentVO = 0;

        public override void OnStart()
        {
            LogMessage("=== AudioVOTimer OnStart ===");
            LogMessage("AudioVOTimer EntityID: " + EntityID);

            Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Subscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Subscribe(EVENT_2MIN, On2Min);
            Subscribe(EVENT_1MIN, On1Min);
            Subscribe(EVENT_10s, On10s);

            initialized = true;

            LogMessage("AudioVOHealthCore initialized:");
        }

        public override void OnUpdate(float deltaTime)
        {
            if(!initialized)
                return;

            if(GameState.IsPaused){
                if(AudioIsPlaying((uint)EntityID) && !paused){
                    LogMessage("AudioVOTimer is paused, pausing timer and pausing audio");
                    AudioPause(currentVO);
                    paused = true;
                }

                return;
            }

            if(paused){
                //should audio be paused resume
                AudioPlay(currentVO);
                paused = false;
            }

            if (last10s)
            {
                countdowntimer -= deltaTime;
                if(countdowntimer <= 0.0f)
                {
                    StopPreviousAudio();
                    currentVO = PrefabInstantiate(Audio7sVOPrefab);
                    last10s = false;
                }
            }

        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Unsubscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Unsubscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Unsubscribe(EVENT_2MIN, On2Min);
            Unsubscribe(EVENT_1MIN, On1Min);
            Unsubscribe(EVENT_10s, On10s);

            LogMessage("=== AudioVOTimer Destroyed ===");
        }

        private void OnGameEnd(string eventName, string payload){
            //optional i can choose to actually self destruct instead.
            gameend = true;
            LogMessage("[AudioVOTimer] Detect Game End Preventing VO from health to be played");
        }

        private void On2Min(string eventName, string payload)
        {
            StopPreviousAudio();
            currentVO = PrefabInstantiate(Audio2minVOPrefab);
        }

        private void On1Min(string eventName, string payload)
        {
            StopPreviousAudio();
            currentVO = PrefabInstantiate(Audio1minVOPrefab);
        }

        private void On10s(string eventName, string payload)
        {
            StopPreviousAudio();
            currentVO = PrefabInstantiate(Audio10sVOPrefab);
            last10s = true;
            countdowntimer = DelayFor2nd5secWarn;
        }

        private void StopPreviousAudio()
        {
            if (AudioIsPlaying(currentVO))
            {
                AudioStop(currentVO);
            }
        }

    }
}
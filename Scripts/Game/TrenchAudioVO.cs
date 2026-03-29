using System;
using Engine;
using static Engine.Logger;
using static Engine.Audio;
using static Engine.Prefab;

namespace Game
{
    /// <summary>
    /// AmmoBar - Visual representation of player ammo
    /// Position adjustment: Move by FULL width change (not half)
    /// Left edge = Position.X - Width (scale.X is radius, not diameter)
    /// Attach this to the ammo bar sprite entity
    /// </summary>
    public class TrenchAudioVO : ScriptBehaviour
    {
        // ===== EVENT =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private const string EVENT_TIMER_FINISHED = "TimerFinished";

        private const string EVENT_DESTRUCTABLEWALL_DESTROYED = "DestructableWallDestroyed";
        private const string EVENT_TRENCH_WALL_WARNING_TRIGGERED = "TrenchWallWarningTriggered";
        private const string EVENT_TRENCH_CORE_WARNING_TRIGGERED = "TrenchCoreWarningTriggered";

        // ===== State =====
        private bool initialized = false;
        [SerializeField] private bool audiostarted = false; //this means that there is an active audio vo playing 
        private bool disabled = false;
        private bool isPaused = false;

        private bool hasPlayedSpawnGuide = false;
        private bool hasPlayedWallWarn = false;
        private bool hasPlayedEnemiesWarn = false;
        private bool hasPlayedCoreWarn = false;

        // ===== Timer =====
        private float audiotimer = 7.0f;
        [SerializeField] private float elapsedTime = 0.0f;

        // ===== Prefab =====
        private const string trenchguideVOPrefab = "Sources/Prefabs/Audio_TrenchGuide.prefab";
        private const string trenchwallwarnVOPrefab = "Sources/Prefabs/Audio_Trench_WallWarn.prefab";
        private const string trenchenemieswarnVOPrefab = "Sources/Prefabs/Audio_Trench_EnemiesWarn.prefab";
        private const string trenchenemycorewarnVOPrefab = "Sources/Prefabs/Audio_Trench_EnemyCoreWarn.prefab";
        
        // ==== Entity =====
        private uint activeVOID = 0;


        public override void OnStart()
        {
            LogMessage("=== TrenchAudioVO OnStart ===");

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Event.Subscribe(EVENT_DESTRUCTABLEWALL_DESTROYED, OnWallDestroyed);
            Event.Subscribe(EVENT_TRENCH_WALL_WARNING_TRIGGERED, OnWallWarningTriggered);
            Event.Subscribe(EVENT_TRENCH_CORE_WARNING_TRIGGERED, OnCoreWarningTriggered);

            //initialized = true;
            audiostarted = false;
            activeVOID = 0;
            elapsedTime = 0.0f;

            disabled = false;

            //Play trench guide the moment we load into the scene. (might need test this out)
            PlayTrenchGuide();

            LogMessage("TrenchAudioVO initialized:");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (disabled)
                return;

            if(!initialized){
                PlayTrenchGuide();
                initialized = true;
            }

            if (GameState.IsPaused){

                //If audio hasn't been paused, and it is started as well as still playing and active VOID isn't 0
                if (!isPaused && audiostarted && activeVOID != 0 && AudioIsPlaying(activeVOID))
                {
                    //pause the audio and set the pause for audio to be true
                    AudioPause(activeVOID);
                    isPaused = true;
                }
                return;
            } else
            {
                if (isPaused && audiostarted && activeVOID != 0)
                {
                    AudioPlay(activeVOID);
                    isPaused = false;
                }
            }

            if(!audiostarted){
                //no current active audio to resume / pause / skip
                return;
            }

            if(audiostarted && elapsedTime > 0.0f){
                elapsedTime -= deltaTime;

                if(elapsedTime <= 0.0f){
                    ResetActiveAudio();
                }
            }

        }

        // ===== EVENT HANDLERS =====
        public override void OnDestroy()
        {
            ResetActiveAudio();

            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Event.Unsubscribe(EVENT_DESTRUCTABLEWALL_DESTROYED, OnWallDestroyed);
            Event.Unsubscribe(EVENT_TRENCH_WALL_WARNING_TRIGGERED, OnWallWarningTriggered);
            Event.Unsubscribe(EVENT_TRENCH_CORE_WARNING_TRIGGERED, OnCoreWarningTriggered);

            LogMessage("=== TrenchAudioVO Destroyed ===");
        }

        private void OnGameEnd(string eventName, string payload){
            disabled = true;
            //Change it to the current active audio entityID
            ResetActiveAudio();
            
            LogMessage("[TrenchAudioVO] Detect game end, stopping audio from playing");
        }

        private void OnWallDestroyed(string eventName, string payload){
            PlayEnemiesWarning();
        }

        private void OnWallWarningTriggered(string eventName, string payload)
        {
            PlayWallWarning();
        }

        private void OnCoreWarningTriggered(string eventName, string payload)
        {
            PlayCoreWarning();
        }

        private void PlayTrenchGuide(){

            if(disabled || hasPlayedSpawnGuide){
                return;
            }

            ResetActiveAudio();

            activeVOID = PrefabInstantiate(trenchguideVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench guide audio failed to instantiate");
                return;
            }

            LogMessage("[TrenchAudioVO] Trench guide audio is playing");

            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedSpawnGuide = true;
        }

        private void PlayWallWarning(){

            if(disabled || hasPlayedWallWarn){
                return;
            }

            ResetActiveAudio();

            activeVOID = PrefabInstantiate(trenchwallwarnVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench wall warning audio failed to instantiate");
                return;
            }

            LogMessage("[TrenchAudioVO] Trench wall warning audio is playing");

            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedWallWarn = true;
        }

        private void PlayEnemiesWarning(){

            if(disabled || hasPlayedEnemiesWarn){
                return;
            }

            ResetActiveAudio();

            activeVOID = PrefabInstantiate(trenchenemieswarnVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench enemies warning audio failed to instantiate");
                return;
            }

            LogMessage("[TrenchAudioVO] Trench enemies warning audio is playing");

            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedEnemiesWarn = true;
        }

        private void PlayCoreWarning(){

            if(disabled || hasPlayedCoreWarn){
                return;
            }

            ResetActiveAudio();

            activeVOID = PrefabInstantiate(trenchenemycorewarnVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench enemycore warning audio failed to instantiate");
                return;
            }
            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedCoreWarn = true;
        }

        private void ResetActiveAudio(){

            //stop the active entity audio from playing
            if(activeVOID != 0 && AudioIsPlaying(activeVOID)){
                AudioStop(activeVOID);
                LogMessage("[TrenchAudioVO] ResetActiveAudio: Stopping active VO that is playing");
                LogMessage("[TrenchAudioVO] ResetActiveAudio: ElapsedTime is at: " + elapsedTime.ToString());
            }

            //reset the activeVOID to 0
            activeVOID = 0;

            //reset other values back to default
            elapsedTime = 0.0f;
            audiostarted = false;
            isPaused = false;
        }
    }
}
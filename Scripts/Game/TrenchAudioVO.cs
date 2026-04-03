using System;
using Engine;
using static Engine.Logger;
using static Engine.Audio;
using static Engine.Prefab;
using static Engine.Scene;
using System.Data;

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
        private const string EVENT_LEVEL2_TUTORIAL_PAUSE = "TutorialPauseAudio";
        private const string EVENT_VO_PAUSE_STATE = "GameVOPauseState";
        private const string EVENT_DESTRUCTABLEWALL_DESTROYED = "DestructableWallDestroyed";
        private const string EVENT_TRENCH_WALL_WARNING_TRIGGERED = "TrenchWallWarningTriggered";
        private const string EVENT_TRENCH_CORE_WARNING_TRIGGERED = "TrenchCoreWarningTriggered";
        private const string EVENT_TRENCH_CORRUPT_CORE_WARNING_TRIGGERED = "TrenchCorruptCoreWarningTriggered";
        private const string EVENT_TRENCH_MID_RUN_WARNING_TRIGGERED = "TrenchMidRunWarningTriggered";
        private const string EVENT_TRENCH_ENEMYCORE_DESTROYED_TRIGGERED = "TutorialEnemyCoreDestroyedTriggered";
        private const string EVENT_TRENCH_UPGRADEMOD_PICKED_TRIGGERED = "TutorialSentryModulePickedTriggered";

        // ===== State =====
        private bool initialized = false;
        [SerializeField] private bool audiostarted = false; //this means that there is an active audio vo playing 
        private bool disabled = false;
        private bool isPaused = false;
        [SerializeField] private bool pauseForTutorial = false;
        private bool hasPlayedSpawnGuide = false;
        private bool hasPlayedWallWarn = false;
        private bool hasPlayedEnemiesWarn = false;
        private bool hasPlayedCoreWarn = false;
        private bool hasPlayedCorruptCoreWarn = false;
        private bool hasPlayedMidTrenchWarn = false;
        private bool hasPlayedEnemyCoreDestroyedWarn = false;
        private bool hasPlayedUpgradeModPickupWarn = false;

        // ===== Timer =====
        private float audiotimer = 7.5f;
        [SerializeField] private float elapsedTime = 0.0f;

        // ===== Prefab =====
        private const string trenchguideVOPrefab = "Sources/Prefabs/Audio_TrenchGuide.prefab";
        private const string trenchwallwarnVOPrefab = "Sources/Prefabs/Audio_Trench_WallWarn.prefab";
        private const string trenchenemieswarnVOPrefab = "Sources/Prefabs/Audio_Trench_EnemiesWarn.prefab";
        private const string trenchenemycorewarnVOPrefab = "Sources/Prefabs/Audio_Trench_EnemyCoreWarn.prefab";
        private const string trenchenemycorewarningVOPrefab = "Sources/Prefabs/Audio_VO_CorruptCore_Warning.prefab";
        private const string trenchmidrunwarnVOPrefab = "Sources/Prefabs/Audio_VO_Mid_Trench.prefab";
        private const string trenchenemycoredestroyedVOPrefab = "Sources/Prefabs/Audio_VO_Corrupted_Core_Destroyed.prefab";
        private const string trenchupgrademodpickupVOPrefab = "Sources/Prefabs/Audio_VO_Upgrade_Module_Drop.prefab";
        
        // ==== Entity =====
        private uint activeVOID = 0;

        private uint bgmID = 0;
        private const string bgmName = "BGM";

        private float bgmOGVol = 1.0f;

        private const float bgmdivider = 0.5f;


        public override void OnStart()
        {
            LogMessage("=== TrenchAudioVO OnStart ===");

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Event.Subscribe(EVENT_LEVEL2_TUTORIAL_PAUSE, OnTutorialPause);
            Event.Subscribe(EVENT_DESTRUCTABLEWALL_DESTROYED, OnWallDestroyed);
            Event.Subscribe(EVENT_TRENCH_WALL_WARNING_TRIGGERED, OnWallWarningTriggered);
            Event.Subscribe(EVENT_TRENCH_CORE_WARNING_TRIGGERED, OnCoreWarningTriggered);
            Event.Subscribe(EVENT_TRENCH_CORRUPT_CORE_WARNING_TRIGGERED, OnCorruptCoreWarningTriggered);
            Event.Subscribe(EVENT_TRENCH_MID_RUN_WARNING_TRIGGERED, OnMidTrenchWarningTriggered);
            Event.Subscribe(EVENT_TRENCH_ENEMYCORE_DESTROYED_TRIGGERED, OnEnemyCoreDestroyedTriggered);
            Event.Subscribe(EVENT_TRENCH_UPGRADEMOD_PICKED_TRIGGERED, OnUpgradeModPickupTriggered);

            //initialized = true;
            audiostarted = false;
            activeVOID = 0;
            elapsedTime = 0.0f;

            disabled = false;

            bgmID = SceneFindEntityByName(bgmName);
            if(bgmID != 0)
            {
                bgmOGVol = AudioGetVolume(bgmID);
            }

            //Play trench guide the moment we load into the scene. (might need test this out)
            PlayTrenchGuide();

            initialized = true;

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

                if (!pauseForTutorial)
                {
                    //If audio hasn't been paused, and it is started as well as still playing and active VOID isn't 0
                    if (!isPaused && audiostarted && activeVOID != 0 && AudioIsPlaying(activeVOID))
                    {
                        //pause the audio and set the pause for audio to be true
                        AudioPause(activeVOID);
                        Event.Publish(EVENT_VO_PAUSE_STATE, activeVOID.ToString() + "|true");
                        isPaused = true;
                    }
                    return;
                }

            }

            if (isPaused && audiostarted && activeVOID != 0)
            {
                AudioPlay(activeVOID);
                Event.Publish(EVENT_VO_PAUSE_STATE, activeVOID.ToString() + "|false");
                isPaused = false;
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
            Event.Unsubscribe(EVENT_LEVEL2_TUTORIAL_PAUSE, OnTutorialPause);
            Event.Unsubscribe(EVENT_DESTRUCTABLEWALL_DESTROYED, OnWallDestroyed);
            Event.Unsubscribe(EVENT_TRENCH_WALL_WARNING_TRIGGERED, OnWallWarningTriggered);
            Event.Unsubscribe(EVENT_TRENCH_CORE_WARNING_TRIGGERED, OnCoreWarningTriggered);
            Event.Unsubscribe(EVENT_TRENCH_CORRUPT_CORE_WARNING_TRIGGERED, OnCorruptCoreWarningTriggered);
            Event.Unsubscribe(EVENT_TRENCH_MID_RUN_WARNING_TRIGGERED, OnMidTrenchWarningTriggered);
            Event.Unsubscribe(EVENT_TRENCH_ENEMYCORE_DESTROYED_TRIGGERED, OnEnemyCoreDestroyedTriggered);
            Event.Unsubscribe(EVENT_TRENCH_UPGRADEMOD_PICKED_TRIGGERED, OnUpgradeModPickupTriggered);

            LogMessage("=== TrenchAudioVO Destroyed ===");
        }

        private void OnGameEnd(string eventName, string payload){
            disabled = true;
            //Change it to the current active audio entityID
            ResetActiveAudio();
            
            LogMessage("[TrenchAudioVO] Detect game end, stopping audio from playing");
        }

        private void OnTutorialPause(string eventName, string payload)
        {
            if(bool.TryParse(payload, out bool state))
            {
                pauseForTutorial = state;
            }
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

        private void OnCorruptCoreWarningTriggered(string eventName, string payload)
        {
            PlayCorruptCoreWarning();
        }

        private void OnMidTrenchWarningTriggered(string eventName, string payload)
        {
            PlayMidTrenchWarning();
        }

        private void OnEnemyCoreDestroyedTriggered(string eventName, string payload)
        {
            PlayEnemyCoreDestroyed();
        }

        private void OnUpgradeModPickupTriggered(string eventName, string payload)
        {
            PlayUpgradeModPickup();
        }

        private void PlayTrenchGuide(){

            if(disabled || hasPlayedSpawnGuide){
                return;
            }

            ResetActiveAudio();
            LowerBGMVol();

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
            LowerBGMVol();

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
            LowerBGMVol();

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
            LowerBGMVol();

            activeVOID = PrefabInstantiate(trenchenemycorewarnVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench enemycore warning audio failed to instantiate");
                return;
            }
            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedCoreWarn = true;
        }

        private void PlayCorruptCoreWarning(){

            if(disabled || hasPlayedCorruptCoreWarn){
                return;
            }

            ResetActiveAudio();
            LowerBGMVol();

            activeVOID = PrefabInstantiate(trenchenemycorewarningVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench corrupt core warning audio failed to instantiate");
                return;
            }
            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedCorruptCoreWarn = true;
        }

        private void PlayMidTrenchWarning(){

            if(disabled || hasPlayedMidTrenchWarn){
                return;
            }

            ResetActiveAudio();
            LowerBGMVol();

            activeVOID = PrefabInstantiate(trenchmidrunwarnVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench mid trench warning audio failed to instantiate");
                return;
            }
            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedMidTrenchWarn = true;
        }

        private void PlayEnemyCoreDestroyed(){

            if(disabled || hasPlayedEnemyCoreDestroyedWarn){
                return;
            }

            ResetActiveAudio();
            LowerBGMVol();

            activeVOID = PrefabInstantiate(trenchenemycoredestroyedVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench enemy core destroyed audio failed to instantiate");
                return;
            }
            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedEnemyCoreDestroyedWarn = true;
        }

        private void PlayUpgradeModPickup(){

            if(disabled || hasPlayedUpgradeModPickupWarn){
                return;
            }

            ResetActiveAudio();
            LowerBGMVol();

            activeVOID = PrefabInstantiate(trenchupgrademodpickupVOPrefab);
            if(activeVOID == 0){
                LogMessage("[TrenchAudioVO] Trench upgrade mod pickup audio failed to instantiate");
                return;
            }
            audiostarted = true;
            elapsedTime = audiotimer;
            hasPlayedUpgradeModPickupWarn = true;
        }

        private void ResetActiveAudio(){

            if (activeVOID != 0)
            {
                Event.Publish(EVENT_VO_PAUSE_STATE, activeVOID.ToString() + "|false");
            }

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

            //reset bgm back to og volume
            AudioSetVolume(bgmID, bgmOGVol);
        }

        private void LowerBGMVol()
        {
            float newVol = bgmOGVol * bgmdivider;
            AudioSetVolume(bgmID, newVol);
        }
    }
}
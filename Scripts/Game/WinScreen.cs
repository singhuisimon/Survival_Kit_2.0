using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.AudioManager;
using static Engine.Prefab;

namespace Game
{
    /// <summary>
    /// WinScreen - Shows win screen when timer runs out
    /// Attach this to the win screen texture entity
    /// Listens to "TimerFinished" event
    /// </summary>
    public class WinScreen : ScriptBehaviour
    {
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string GAMEWIN = "GameWin";

        private bool initialized = false;
        private bool countdownstart = false;
        private bool spawnedVO = false;

        [SerializeField] private float countdown = 0.0f;

        private const float countdownlevel2 = 0.7f;
        private const float countdowntrench = 0.5f;

        private string winVOPrefab = "";
        private const string winTrenchVOPrefab = "Sources/Prefabs/Audio_Win_Trench_VO.prefab";
        private const string winLevel2VOPrefab = "Sources/Prefabs/Audio_Win_Level2_VO.prefab";

        public override void OnStart()
        {
            LogMessage("=== WinScreen OnStart ===");
            LogMessage("WinScreen EntityID: " + EntityID);

            // Subscribe to timer finished
            Event.Subscribe(EVENT_TIMER_FINISHED, OnLevel2Win);
            Event.Subscribe(ENEMY_CORE_DEATH, OnTrenchWin);

            // Start hidden
            SetIsVisible((uint)EntityID, false);

            initialized = true;
            LogMessage("[WinScreen] Initialized - waiting for timer to finish");
        }

        private void OnUpdate(float deltaTime){

            if(countdownstart){
                countdown -= deltaTime;

                if(countdown <= 0.0f && !spawnedVO){
                    uint winVOID = 0;
                    winVOID = PrefabInstantiate(winVOPrefab);
                    if(winVOID == 0){
                        LogMessage("[WinScreen] Failed to instantiate win voiceover for audio: " + winVOPrefab);
                    }

                    spawnedVO = true;
                }
            }

        }

        private void OnTrenchWin(string eventName, string payload)
        {
            LogMessage("[WinScreen] Win! Trench run finished!");
            StopGroup(AudioType.MASTER);
            winVOPrefab = winTrenchVOPrefab;
            countdownstart = true;
            countdown = countdowntrench;
            Input.SetCursorVisible(true);
            SetIsVisible((uint)EntityID, true);
            Publish(GAMEWIN, "");
        }

        private void OnLevel2Win(string eventName, string payload)
        {
            LogMessage("[WinScreen] Win! Timer finished!");
            StopGroup(AudioType.MASTER);
            winVOPrefab = winLevel2VOPrefab;
            countdownstart = true;
            countdown = countdownlevel2;
            Input.SetCursorVisible(true);
            SetIsVisible((uint)EntityID, true);
            Publish(GAMEWIN, "");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnLevel2Win);
            Event.Unsubscribe(ENEMY_CORE_DEATH, OnTrenchWin);
            LogMessage("=== WinScreen Destroyed ===");
        }

        private void AfterWin(){

        }
    }
}
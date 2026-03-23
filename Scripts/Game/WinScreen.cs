using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.AudioManager;
using static Engine.Prefab;

namespace Game
{
    public class WinScreen : ScriptBehaviour
    {
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string EVENT_ENDSOUND1 = "EndSound1Played";
        private const string GAMEWIN = "GameWin";
        //private const string WIN_SCREEN_SHOW = "WinScreenShow";


        // to differentiate different level, e.g. if in level1, tick level1.
        [SerializeField] private bool IsLevel1 = false;   // trench_run scene
        [SerializeField] private bool IsLevel2 = false;   // level2_graphic_card scene
        [SerializeField] private bool IsLevel3 = false;   // level3 scene

        private bool initialized = false;
        private bool countdownstart = false;
        private bool spawnedVO = false;
        
        [SerializeField] private float countdown = 0.0f;

        private const float countdownlevel2 = 0.7f;
        private const float countdowntrench = 0.5f;
        private const float countdownlevel3 = 0.7f;

        private string winVOPrefab = "";
        private const string winTrenchVOPrefab = "Sources/Prefabs/Audio_Win_Trench_VO.prefab";
        private const string winLevel2VOPrefab = "Sources/Prefabs/Audio_Win_Level2_VO.prefab";

        // Texture fade
        private bool isFading = false;
        private bool fadeDone = false;
        private float fadeElapsed = 0.0f;
        [SerializeField] private float fadeUpTime = 1.0f;

        // Button fade delay
        private bool pendingLoad = false;
        private float showDelay = 2.0f;
        private float showDelayTimer = 0.0f;

        // Score & Time cached 
        private const string EVENT_SHOW_SCORE = "ShowFinalScore";
        private const string EVENT_SHOW_TIME  = "ShowTimeSurvived";

        public override void OnStart()
        {
            LogMessage("=== WinScreen OnStart ===");
            LogMessage("WinScreen EntityID: " + EntityID);

            Event.Subscribe(EVENT_TIMER_FINISHED, OnTimerWin);
            Event.Subscribe(ENEMY_CORE_DEATH, OnTrenchWin);
            Event.Subscribe(EVENT_ENDSOUND1, OnTrenchAudioEnd);

            Event.Subscribe(EVENT_SHOW_SCORE, OnScoreReceived);
            Event.Subscribe(EVENT_SHOW_TIME,  OnTimeReceived);

            // Reset the context of the score at level start (from WinCutSceneContext.cs)
            WinCutSceneContext.FinalScore = "0";
            WinCutSceneContext.FinalTime  = "00 m : 00 s";

            SetIsVisible((uint)EntityID, false);

            initialized = true;
            LogMessage("[WinScreen] Initialized - waiting for win condition");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Fade in texture
            if (isFading && !fadeDone)
            {
                fadeElapsed += deltaTime;
                FadeIn((uint)EntityID, fadeElapsed, fadeUpTime);

                if (fadeElapsed >= fadeUpTime)
                {
                    fadeDone = true;
                    isFading = false;
                    LogMessage("[WinScreen] Texture fade complete");
                   // Publish(WIN_SCREEN_SHOW, "");

                }
            }

            // VO countdown
            if (countdownstart)
            {
                countdown -= deltaTime;
                if (countdown <= 0.0f && !spawnedVO)
                {
                    uint winVOID = PrefabInstantiate(winVOPrefab);
                    if (winVOID == 0)
                        LogMessage("[WinScreen] Failed to instantiate win VO: " + winVOPrefab);
                    spawnedVO = true;
                }
            }

            // Wait then show buttons/popup
            if (pendingLoad)
            {
                showDelayTimer -= deltaTime;
                if (showDelayTimer <= 0.0f)
                {
                    pendingLoad = false;
                    LogMessage("[WinScreen] Delay done - publishing WinScreenShow");
                    //Publish(WIN_SCREEN_SHOW, "");
                    LoadCutscene(); 
                }
            }
        }

        // Cache directly into WinCutSceneContext
        private void OnScoreReceived(string eventName, string payload)
        {
            WinCutSceneContext.FinalScore = payload;
            LogMessage("[WinScreen] Cached score: " + payload);
        }

        private void OnTimeReceived(string eventName, string payload)
        {
            // Convert raw seconds to "00 m : 00 s" format
            float.TryParse(payload, out float timeSurvived);
            int minutes = (int)(timeSurvived / 60);
            int seconds  = (int)(timeSurvived % 60);
            WinCutSceneContext.FinalTime = string.Format("{0:00} m : {1:00} s", minutes, seconds);
            LogMessage("[WinScreen] Cached time: " + WinCutSceneContext.FinalTime);
        }

        private void OnTrenchWin(string eventName, string payload)
        {
            LogMessage("[WinScreen] Win! Trench run finished!");
            // StopGroup(AudioType.MASTER);
            // winVOPrefab = winTrenchVOPrefab;
            // countdownstart = true;
            // countdown = countdowntrench;
            Input.SetCursorVisible(true);

            SpriteRenderer.SetColor((uint)EntityID, 1.0f, 1.0f, 1.0f, 0.0f);
            isFading = true;
            fadeElapsed = 0.0f;
            fadeDone = false;
            SetIsVisible((uint)EntityID, true);

            Publish(GAMEWIN, "");

            pendingLoad = true;
            showDelayTimer = showDelay;
        }

        private void OnTimerWin(string eventName, string payload)
        {
            LogMessage("[WinScreen] Win! Timer finished!");
            StopGroup(AudioType.MASTER);
            winVOPrefab = winLevel2VOPrefab;
            countdownstart = true;
            countdown = countdownlevel2;
            Input.SetCursorVisible(true);

            SpriteRenderer.SetColor((uint)EntityID, 1.0f, 1.0f, 1.0f, 0.0f);
            isFading = true;
            fadeElapsed = 0.0f;
            fadeDone = false;
            SetIsVisible((uint)EntityID, true);

            Publish(GAMEWIN, "");

            pendingLoad = true;
            showDelayTimer = showDelay;
        }

        // load the file from different level, read data from WinCutSceneContext.cs
        private void LoadCutscene()
        {
            if (IsLevel1)
            {
                WinCutSceneContext.NextScene    = WinCutSceneContext.LEVEL2_SCENE;
                WinCutSceneContext.IsFinalLevel = false;
            }
            else if (IsLevel2)
            {
                WinCutSceneContext.NextScene    = WinCutSceneContext.LEVEL3_SCENE;
                WinCutSceneContext.IsFinalLevel = false;
            }
            else if (IsLevel3)
            {
                WinCutSceneContext.NextScene    = WinCutSceneContext.MAIN_MENU_SCENE;
                WinCutSceneContext.IsFinalLevel = true;
            }
            else
            {
                LogError("[WinScreen] No level flag set! Defaulting to Main Menu.");
                WinCutSceneContext.NextScene    = WinCutSceneContext.MAIN_MENU_SCENE;
                WinCutSceneContext.IsFinalLevel = true;
            }

            LogMessage("[WinScreen] Loading cutscene → next: "
                       + WinCutSceneContext.NextScene
                       + " | final: " + WinCutSceneContext.IsFinalLevel);

            bool ok = Scene.SceneLoadFromFile(WinCutSceneContext.CUTSCENE_SCENE);
            if (!ok)
                LogError("[WinScreen] Failed to load: " + WinCutSceneContext.CUTSCENE_SCENE);
        }

        private void OnTrenchAudioEnd(string eventName, string payload){
            LogMessage("[WinScreen] Trench ended already stopping audio then playing");
            StopGroup(AudioType.MASTER);
            winVOPrefab = winTrenchVOPrefab;
            countdownstart = true;
            countdown = countdowntrench;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnTimerWin);
            Event.Unsubscribe(ENEMY_CORE_DEATH, OnTrenchWin);
            Event.Unsubscribe(EVENT_ENDSOUND1, OnTrenchAudioEnd);
            Event.Unsubscribe(EVENT_SHOW_SCORE,     OnScoreReceived);
            Event.Unsubscribe(EVENT_SHOW_TIME,      OnTimeReceived);
            LogMessage("=== WinScreen Destroyed ===");
        }
    }
}

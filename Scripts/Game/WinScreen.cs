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
        private const string GAMEWIN = "GameWin";
        private const string WIN_SCREEN_SHOW = "WinScreenShow";

        private bool initialized = false;
        private bool countdownstart = false;
        private bool spawnedVO = false;

        [SerializeField] private float countdown = 0.0f;

        private const float countdownlevel2 = 0.7f;
        private const float countdowntrench = 0.5f;

        private string winVOPrefab = "";
        private const string winTrenchVOPrefab = "Sources/Prefabs/Audio_Win_Trench_VO.prefab";
        private const string winLevel2VOPrefab = "Sources/Prefabs/Audio_Win_Level2_VO.prefab";

        // Texture fade
        private bool isFading = false;
        private bool fadeDone = false;
        private float fadeElapsed = 0.0f;
        [SerializeField] private float fadeUpTime = 1.0f;

        // Button fade delay
        private bool pendingShow = false;
        private float showDelay = 2.0f;
        private float showDelayTimer = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== WinScreen OnStart ===");
            LogMessage("WinScreen EntityID: " + EntityID);

            Event.Subscribe(EVENT_TIMER_FINISHED, OnLevel2Win);
            Event.Subscribe(ENEMY_CORE_DEATH, OnTrenchWin);

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
                    Publish(WIN_SCREEN_SHOW, "");

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
            if (pendingShow)
            {
                showDelayTimer -= deltaTime;
                if (showDelayTimer <= 0.0f)
                {
                    pendingShow = false;
                    LogMessage("[WinScreen] Delay done - publishing WinScreenShow");
                    Publish(WIN_SCREEN_SHOW, "");
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

            SpriteRenderer.SetColor((uint)EntityID, 1.0f, 1.0f, 1.0f, 0.0f);
            isFading = true;
            fadeElapsed = 0.0f;
            fadeDone = false;
            SetIsVisible((uint)EntityID, true);

            Publish(GAMEWIN, "");

            pendingShow = true;
            showDelayTimer = showDelay;
        }

        private void OnLevel2Win(string eventName, string payload)
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

            pendingShow = true;
            showDelayTimer = showDelay;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnLevel2Win);
            Event.Unsubscribe(ENEMY_CORE_DEATH, OnTrenchWin);
            LogMessage("=== WinScreen Destroyed ===");
        }
    }
}

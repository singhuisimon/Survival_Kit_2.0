using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.Transform;

namespace Game
{
    public class RestartButton2 : ScriptBehaviour
    {
        private const string GAME_SCENE_PATH = "Resources/Sources/Scenes/level2.json";
        private const string EVENT_WIN_SHOW = "WinScreenShow";
        private const string EVENT_BUTTONS_FADED = "WinButtonsFaded";
        private const string EVENT_LOSE_SHOW = "LoseScreenShow";

        [SerializeField] private float fadeUpTime = 1.0f;

        private bool isButtonActive = false;
        private bool isFading = false;
        private bool fadeDone = false;
        private float fadeElapsed = 0.0f;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("=== RestartButton2 OnStart ===");
            Event.Subscribe(EVENT_WIN_SHOW, OnWinCondition);
            Event.Subscribe(EVENT_LOSE_SHOW, OnLoseCondition);
            SetIsVisible((uint)EntityID, false);
            LogMessage("[RestartButton2] Initialized");
        }

        private void OnLoseCondition(string eventName, string payload)
        {
            LogMessage("[RestartButton2] Lose condition - starting fade");
            StartFade();
        }

        private void OnWinCondition(string eventName, string payload)
        {
            LogMessage("[RestartButton2] Win condition - starting fade");
            StartFade();
        }

        private void StartFade()
        {
            isFading = true;
            fadeElapsed = 0.0f;
            fadeDone = false;
            isButtonActive = false;
            SetIsVisible((uint)EntityID, true);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isFading && !fadeDone)
            {
                fadeElapsed += deltaTime;

                FadeIn((uint)EntityID, fadeElapsed, fadeUpTime);

                if (fadeElapsed >= fadeUpTime)
                {
                    fadeDone = true;
                    isFading = false;
                    isButtonActive = true;
                    Event.Publish(EVENT_BUTTONS_FADED, "");
                    LogMessage("[RestartButton2] Fade complete");
                }
            }

            if (!isButtonActive)
                return;

            HandleMouseClick();
        }

        private void HandleMouseClick()
        {
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (!mouseJustPressed) return;

            if (Collision2D.IsMouseCollidingWithEntity((uint)EntityID))
            {
                LogMessage("[RestartButton2] Clicked - reloading scene");
                AudioManager.StopGroup(AudioType.BGM);
                AudioManager.StopGroup(AudioType.SFX);
                Input.SetCursorVisible(false);
                Scene.SceneLoadFromFile(GAME_SCENE_PATH);
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_LOSE_SHOW, OnLoseCondition);
            Event.Unsubscribe(EVENT_WIN_SHOW, OnWinCondition);
            LogMessage("=== RestartButton2 Destroyed ===");
        }
    }
}

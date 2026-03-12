using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.Transform;

namespace Game
{
    public class RestartButton : ScriptBehaviour
    {
        private const string GAME_SCENE_PATH = "Resources/Sources/Scenes/trench_run.json";
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
            LogMessage("=== RestartButton OnStart ===");
            Event.Subscribe(EVENT_LOSE_SHOW, OnLoseCondition);
            SetIsVisible((uint)EntityID, false);
            LogMessage("[RestartButton] Initialized");
        }

        private void OnLoseCondition(string eventName, string payload)
        {
            LogMessage("[RestartButton] Lose condition - starting fade");
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
                    Event.Publish("LoseButtonsFaded", "");
                    LogMessage("[RestartButton] Fade complete");
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
                LogMessage("[RestartButton] Clicked - reloading scene");
                AudioManager.StopGroup(AudioType.BGM);
                AudioManager.StopGroup(AudioType.SFX);
                Input.SetCursorVisible(false);
                bool sucess = Scene.SceneLoadFromFile(GAME_SCENE_PATH);
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_LOSE_SHOW, OnLoseCondition);
            LogMessage("=== RestartButton Destroyed ===");
        }
    }
}

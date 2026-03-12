using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.Transform;

namespace Game
{
    public class NextLevelButton : ScriptBehaviour
    {
        private const string LEVEL_2_SCENE_PATH = "Resources/Sources/Scenes/level2.json";
        private const string FALLBACK_SCENE_PATH = "Resources/Sources/Scenes/MainMenu.json";
        private const string EVENT_WIN_SHOW = "WinScreenShow";
        private const string EVENT_BUTTONS_FADED = "WinButtonsFaded";

        [SerializeField("Current Level")]
        private int currentLevel = 1;

        [SerializeField] private float fadeUpTime = 1.0f;

        private bool isButtonActive = false;
        private bool wasMousePressed = false;
        private bool isFading = false;
        private bool fadeDone = false;
        private float fadeElapsed = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== NextLevelButton OnStart ===");
            Event.Subscribe(EVENT_WIN_SHOW, OnWinCondition);
            SetIsVisible((uint)EntityID, false);
            isButtonActive = false;
            LogMessage("[NextLevelButton] Initialized - Level: " + currentLevel);
        }

        private void OnWinCondition(string eventName, string payload)
        {
            LogMessage("[NextLevelButton] Win condition - starting fade");
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
                    LogMessage("[NextLevelButton] Fade complete");
                }
            }

            if (!isButtonActive) return;
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
                LogMessage("[NextLevelButton] Clicked - loading next level");
                AudioManager.StopGroup(AudioType.BGM);
                AudioManager.StopGroup(AudioType.SFX);
                Input.SetCursorVisible(false);
                Scene.SceneLoadFromFile(GetNextLevelPath());
            }
        }

        private string GetNextLevelPath()
        {
            switch (currentLevel)
            {
                case 1:
                    return LEVEL_2_SCENE_PATH;
                case 2:
                    LogMessage("[NextLevelButton] No more levels - returning to main menu");
                    Input.SetCursorVisible(true);
                    return FALLBACK_SCENE_PATH;
                default:
                    LogMessage("[NextLevelButton] Unknown level - returning to main menu");
                    Input.SetCursorVisible(true);
                    return FALLBACK_SCENE_PATH;
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_WIN_SHOW, OnWinCondition);
            LogMessage("=== NextLevelButton Destroyed ===");
        }
    }
}

using Engine;
using System;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    public class WinScreenPopup : ScriptBehaviour
    {
        private const string EVENT_WIN_SHOW = "WinScreenShow";

        [SerializeField] private float fadeUpTime = 1.0f;

        private bool isFading;
        private bool fadeDone;
        private float fadeElapsed;

        public override void OnStart()
        {
            LogMessage("=== WinScreenPopup OnStart ===");
            SetIsVisible((uint)EntityID, false);
            Event.Subscribe(EVENT_WIN_SHOW, OnShow);
            LogMessage("[WinScreenPopup] Initialized");


         isFading = false;
         fadeDone = false;
         fadeElapsed = 0.0f;
        }

        private void OnShow(string eventName, string payload)
        {
            LogMessage("[WinScreenPopup] Starting fade in");

            // Reset alpha to 0 before fading in
            SpriteRenderer.SetColor((uint)EntityID, 1.0f, 1.0f, 1.0f, 0.0f);

            isFading = true;
            fadeElapsed = 0.0f;
            fadeDone = false;
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
                    LogMessage("[WinScreenPopup] Fade complete");
                }
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_WIN_SHOW, OnShow);
            LogMessage("=== WinScreenPopup Destroyed ===");
        }
    }
}

using Engine;
using System;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.Transform;

namespace Game
{
    public class LoseScreenPopup : ScriptBehaviour
    {
        private const string EVENT_LOSE_SHOW = "LoseScreenShow";

        [SerializeField] private float fadeUpTime = 1.0f;

        private bool isFading = false;
        private bool fadeDone = false;
        private float fadeElapsed = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== LoseScreenPopup OnStart ===");
            SetIsVisible((uint)EntityID, false);
            Event.Subscribe(EVENT_LOSE_SHOW, OnShow);
            LogMessage("[LoseScreenPopup] Initialized");
        }

        private void OnShow(string eventName, string payload)
        {
            LogMessage("[LoseScreenPopup] Starting fade in");
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
                    LogMessage("[LoseScreenPopup] Fade complete");
                }
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_LOSE_SHOW, OnShow);
            LogMessage("=== LoseScreenPopup Destroyed ===");
        }
    }
}

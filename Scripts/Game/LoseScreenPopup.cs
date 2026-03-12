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
        //private const string EVENT_WIN_SHOW = "WinScreenShow";
        private const string EVENT_LOSE_SHOW = "LoseScreenShow";

        [SerializeField] private float fadeUpTime = 1.0f;
        [SerializeField] private float uiStartFadePos = 360.0f;
        private Vector3 initialPosition;

        private bool isFading = false;
        private bool fadeDone = false;
        private float fadeElapsed = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== WinScreenPopup OnStart ===");
            SetIsVisible((uint)EntityID, false);
            initialPosition = GetPosition((uint)EntityID);

            // Event.Subscribe(EVENT_WIN_SHOW, OnShow);
            Event.Subscribe(EVENT_LOSE_SHOW, OnShow);

            LogMessage("[WinScreenPopup] Initialized");
        }

        private void OnShow(string eventName, string payload)
        {
            LogMessage("[WinScreenPopup] Starting fade in - triggered by: " + eventName);

            // Capture current position as the target
            initialPosition = GetPosition((uint)EntityID);

            isFading = true;
            fadeElapsed = 0.0f;
            fadeDone = false;
            SetIsVisible((uint)EntityID, true);

            // Push 10 units below to start
            Vector3 startPos = new Vector3(
                initialPosition.X,
                initialPosition.Y - 10.0f,
                initialPosition.Z
            );
            SetPosition((uint)EntityID, ref startPos);
        }


        public override void OnUpdate(float deltaTime)
        {
            if (isFading && !fadeDone)
            {
                fadeElapsed += deltaTime;

                FadeIn((uint)EntityID, fadeElapsed, fadeUpTime);

                // Slide UP from 10 units below into original position
                float t = fadeElapsed / fadeUpTime;
                if (t > 1.0f) t = 1.0f;

                Vector3 pos = new Vector3(
                    initialPosition.X,
                    initialPosition.Y - 10.0f + (10.0f * t),  // starts -10, ends at 0 offset
                    initialPosition.Z
                );
                SetPosition((uint)EntityID, ref pos);

                if (fadeElapsed >= fadeUpTime)
                {
                    // Snap exactly back to initial position
                    SetPosition((uint)EntityID, ref initialPosition);
                    fadeDone = true;
                    isFading = false;
                    LogMessage("[" + GetType().Name + "] Fade complete");
                }
            }

        }

        public override void OnDestroy()
        {
           // Event.Unsubscribe(EVENT_WIN_SHOW, OnShow);
            Event.Unsubscribe(EVENT_LOSE_SHOW, OnShow);
            LogMessage("=== WinScreenPopup Destroyed ===");
        }
    }
}

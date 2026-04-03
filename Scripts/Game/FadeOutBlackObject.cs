using Engine;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.MeshRenderer;
using static Engine.Event;

namespace Game
{
    public class FadeOutBlackObject : ScriptBehaviour
    {
        [SerializeField("PlayerCam Name")] private string playerCamName = "PlayerCam";
        [SerializeField("Fade Duration")] private float fadeDuration = 1.5f;
        [SerializeField("Start Delay")] private float startDelay = 0.5f;

        private const string EVENT_FADE_BLACK_IN = "FadeBlackFadeIn";
        private const string EVENT_FADE_BLACK_OUT = "FadeBlackFadeOut";
        private const string EVENT_FADE_BLACK_IN_DONE = "FadeBlackFadeInDone";
        private const string EVENT_FADE_BLACK_OUT_DONE = "FadeBlackFadeOutDone";

        private uint playerCamID = 0;

        private float timer = 0.0f;
        private float currentOpacity = 1.0f;
        private float fadeFromOpacity = 1.0f;
        private float fadeToOpacity = 0.0f;

        private bool waitingForStartupDelay = true;
        private bool isFading = false;
        private string completionEventToPublish = "";

        public override void OnStart()
        {
            playerCamID = SceneFindEntityByName(playerCamName);

            timer = 0.0f;
            currentOpacity = 1.0f;
            fadeFromOpacity = 1.0f;
            fadeToOpacity = 0.0f;
            waitingForStartupDelay = true;
            isFading = false;
            completionEventToPublish = "";

            SetOpacity(EntityID, currentOpacity);

            Subscribe(EVENT_FADE_BLACK_IN, OnFadeBlackInRequested);
            Subscribe(EVENT_FADE_BLACK_OUT, OnFadeBlackOutRequested);
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_FADE_BLACK_IN, OnFadeBlackInRequested);
            Unsubscribe(EVENT_FADE_BLACK_OUT, OnFadeBlackOutRequested);
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            FollowPlayerCamera();

            if (waitingForStartupDelay)
            {
                timer += deltaTime;
                if (timer >= startDelay)
                {
                    waitingForStartupDelay = false;
                    BeginFadeTo(0.0f, "");
                }
            }

            if (!isFading)
                return;

            float duration = fadeDuration <= 0.0f ? 0.0001f : fadeDuration;

            timer += deltaTime;
            float t = timer / duration;

            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            currentOpacity = fadeFromOpacity + ((fadeToOpacity - fadeFromOpacity) * t);
            SetOpacity(EntityID, currentOpacity);

            if (t >= 1.0f)
            {
                isFading = false;
                currentOpacity = fadeToOpacity;
                SetOpacity(EntityID, currentOpacity);

                if (completionEventToPublish != "")
                {
                    string evt = completionEventToPublish;
                    completionEventToPublish = "";
                    Publish(evt, "");
                }
            }
        }

        private void FollowPlayerCamera()
        {
            if (playerCamID == 0)
                return;

            Vector3 camPos = GetPosition(playerCamID);
            Quat camRot = GetRotation(playerCamID);

            SetPosition(EntityID, ref camPos);
            SetRotation(EntityID, ref camRot);
        }

        private void BeginFadeTo(float targetOpacity, string completionEvent)
        {
            if (targetOpacity < 0.0f) targetOpacity = 0.0f;
            if (targetOpacity > 1.0f) targetOpacity = 1.0f;

            waitingForStartupDelay = false;
            timer = 0.0f;
            fadeFromOpacity = currentOpacity;
            fadeToOpacity = targetOpacity;
            completionEventToPublish = completionEvent;

            float diff = fadeToOpacity - fadeFromOpacity;
            if (diff < 0.0f) diff = -diff;

            if (diff <= 0.0001f)
            {
                currentOpacity = fadeToOpacity;
                SetOpacity(EntityID, currentOpacity);
                isFading = false;

                if (completionEventToPublish != "")
                {
                    string evt = completionEventToPublish;
                    completionEventToPublish = "";
                    Publish(evt, "");
                }

                return;
            }

            isFading = true;
        }

        private void OnFadeBlackInRequested(string eventName, string payload)
        {
            BeginFadeTo(1.0f, EVENT_FADE_BLACK_IN_DONE);
        }

        private void OnFadeBlackOutRequested(string eventName, string payload)
        {
            BeginFadeTo(0.0f, EVENT_FADE_BLACK_OUT_DONE);
        }
    }
}
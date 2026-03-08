using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game 
{
    public class SentryLifetimeBar : ScriptBehaviour
    {
        private const string FILL_NAME  = "SentryBarFill";
        private const string BG_NAME    = "SentryBarBG";
        private const string LABEL_NAME = "SentryBarLabel";

        [SerializeField] private float maxDuration  = 90.0f;
        [SerializeField] private float heightOffset = 35.0f;
        [SerializeField] private float fullScaleX   = 50.0f;
        [SerializeField] private float barScaleY    = 6.0f;
        [SerializeField] private float labelOffsetY = 10.0f;

        private float countdown   = 0f;
        private uint  sentryID    = 0;
        private uint  fillID      = 0;
        private uint  bgID        = 0;
        private uint  labelID     = 0;
        private bool  initialized = false;
        private bool  gameEnded   = false;
        private const uint INVALID_ENTITY = 0xffffffffu;

        private const string GAMEOVEREVENT = "GameOver";
        private const string GAMEWINEVENT  = "GameWin";

        public override void OnStart()
        {
            fillID  = SceneFindEntityByName(FILL_NAME);
            bgID    = SceneFindEntityByName(BG_NAME);
            labelID = SceneFindEntityByName(LABEL_NAME);

            countdown = maxDuration;

            Subscribe(GAMEOVEREVENT, OnGameEnd);
            Subscribe(GAMEWINEVENT,  OnGameEnd);
        }

        public override void OnUpdate(float deltaTime) 
        {
            if (gameEnded) return;

            // Resolve parent sentry on first valid frame
            if (!initialized)
            {
                sentryID = TransformGetParent((uint)EntityID);
                if (sentryID == 0 || sentryID == INVALID_ENTITY) return;

                if (fillID == 0)
                {
                    LogError("[SentryLifetimeBar] SentryBarFill not found!");
                    return;
                }

                Vector3 fillScale = GetScale(fillID);
                fillScale.X = fullScaleX;
                fillScale.Y = barScaleY;
                SetScale(fillID, ref fillScale);

                initialized = true;
                LogMessage("[SentryLifetimeBar] Bound to sentry: " + sentryID);
            }

            if (GameState.IsPaused) return;

            // Tick countdown

            countdown -= deltaTime;
            if (countdown <= 0f) 
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            float progress = SimpleMath.Clamp(countdown / maxDuration, 0f, 1f);

            // Follow sentry world position
            Vector3 sentryPos = GetPosition(sentryID);
            Vector3 barPos = new Vector3(
                sentryPos.X,
                sentryPos.Y + heightOffset,
                sentryPos.Z
            );
            SetPosition((uint)EntityID, ref barPos);

            // BG follows
            if (bgID != 0 && bgID != INVALID_ENTITY)
                SetPosition(bgID, ref barPos);

            // Fill drain right to left
            Vector3 fillScale2 = GetScale(fillID);
            fillScale2.X = fullScaleX * progress;
            fillScale2.Y = barScaleY;
            SetScale(fillID, ref fillScale2);

            float shift = fullScaleX * (1f - progress) * 0.5f;
            Vector3 fillPos = new Vector3(
                barPos.X - shift,
                barPos.Y,
                barPos.Z
            );
            SetPosition(fillID, ref fillPos);

            // Label above
            if (labelID != 0 && labelID != INVALID_ENTITY)
            {
                Vector3 labelPos = new Vector3(
                    barPos.X,
                    barPos.Y + labelOffsetY,
                    barPos.Z
                );
                SetPosition(labelID, ref labelPos);
            }
        }

        private void OnGameEnd(string eventName, string payload)
        {
            gameEnded = true;
            SceneDestroyEntity((uint)EntityID);
        }

        public override void OnDestroy()
        {
            Unsubscribe(GAMEOVEREVENT, OnGameEnd);
            Unsubscribe(GAMEWINEVENT,  OnGameEnd);
            LogMessage("[SentryLifetimeBar] Destroyed.");
        }
    }
}
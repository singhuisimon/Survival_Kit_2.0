using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game 
{
    public class SentryLifetimeBar : ScriptBehaviour
    {

        [SerializeField] private float maxDuration = 90.0f;
        [SerializeField] private float fullScaleX  = 20.0f;
        [SerializeField] private float barScaleY   = 2.0f;

        private float countdown   = 0f;
        private uint  sentryID    = 0;
        private bool  initialized = false;
        private bool  gameEnded   = false;
        private const uint INVALID_ENTITY = 0xffffffffu;

        private const string GAMEOVEREVENT = "GameOver";
        private const string GAMEWINEVENT  = "GameWin";

        public override void OnStart()
        {
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

                // Set initial full-scale using world scale so sentry's own
                // local scale doesn't affect the bar size
                Vector3 initScale = GetWorldScale((uint)EntityID);
                initScale.X = fullScaleX;
                initScale.Y = barScaleY;
                SetWorldScale((uint)EntityID, ref initScale);

                initialized = true;
                LogMessage("[SentryLifetimeBar] Bound to sentry: " + sentryID);
            }

            if (GameState.IsPaused) return;

            // Tick countdown

            countdown -= deltaTime;
            if (countdown <= 0f) 
            {
                LogMessage("[SentryLifetimeBar] Lifetime expired, destroying sentry: " + sentryID);
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            // Drain fill from full to empty
            float progress = SimpleMath.Clamp(countdown / maxDuration, 0f, 1f);
            Vector3 fillScale = GetWorldScale((uint)EntityID);
            fillScale.X = fullScaleX * progress;
            fillScale.Y = barScaleY;
            SetWorldScale((uint)EntityID, ref fillScale);

        }

        private void OnGameEnd(string eventName, string payload)
        {
            gameEnded = true;
        }

        public override void OnDestroy()
        {
            Unsubscribe(GAMEOVEREVENT, OnGameEnd);
            Unsubscribe(GAMEWINEVENT,  OnGameEnd);
            LogMessage("[SentryLifetimeBar] Destroyed.");
        }
    }
}
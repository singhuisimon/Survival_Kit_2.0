using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game 
{
    public class SentryLifetimeBar : ScriptBehaviour
    {

        private const string FILL_NAME = "SentryBarFill";
        private const string BG_NAME    = "SentryBarBG";
        private const string LABEL_NAME = "SentryBarLabel";

        [SerializeField] private float maxDuration = 200.0f; // original is 90.0f for 90 seconds
        [SerializeField] private float heightOffset = 10.0f;
        [SerializeField] private float fullScaleX  = 20.0f;
        [SerializeField] private float barScaleY   = 2.0f;

        private float countdown   = 0f;
        private uint  sentryID    = 0;
        private uint  fillID      = 0;
        private uint  bgID       = 0;
        private uint  labelID    = 0;

        private Vector3 fillInitialPos;
        private bool    fillInitialPosCached = false;

        private bool  initialized = false;
        private bool  gameEnded   = false;

        private string initEventName = "";
        private bool isDead      = false;

        private const uint INVALID_ENTITY = 0xffffffffu;

        private const string GAMEOVEREVENT = "GameOver";
        private const string GAMEWINEVENT  = "GameWin";
        private const string PLAYERDEADEVENT = "PlayerDead";

        public static uint NextSentryID = 0;

        public override void OnStart()
        {
            countdown = maxDuration;

            // cache children immediately
            fillID  = SceneFindEntityByName(FILL_NAME);
            bgID    = SceneFindEntityByName(BG_NAME);
            labelID = SceneFindEntityByName(LABEL_NAME);

            if (fillID == 0 || fillID == INVALID_ENTITY)
            {
                LogError("[SentryLifetimeBar] Could not find SentryBarFill!");
                return;
            }

            fillInitialPos = GetPosition(fillID);

            Subscribe(GAMEOVEREVENT, OnGameEnd);
            Subscribe(GAMEWINEVENT, OnGameEnd);
            Subscribe(PLAYERDEADEVENT, OnGameEnd);

            if (NextSentryID != 0 && NextSentryID != INVALID_ENTITY)
            {
                sentryID     = NextSentryID;
                NextSentryID = 0;  // clear so the next bar doesn't read stale data
                initialized  = true;
                LogMessage("[SentryLifetimeBar] Linked to sentry: " + sentryID);
            }
            else
            {
                LogWarning("[SentryLifetimeBar] NextSentryID was not set before instantiation!");
            }
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead || !initialized) return;

            if (GameState.IsPaused) return;

            // move root above sentry
            Vector3 sentryPos = GetPosition(sentryID);
            Vector3 barPos = new Vector3(
                sentryPos.X,
                sentryPos.Y + heightOffset,
                sentryPos.Z
            );
            SetPosition((uint)EntityID, ref barPos);

            countdown -= deltaTime;

            
            if (countdown <= 0f)
            {
                LogMessage("[SentryLifetimeBar] Lifetime expired - destroying sentry: " + sentryID);
                SceneDestroyEntity(sentryID);        // destroy sentry
                KillSelf();
                //SceneDestroyEntity((uint)EntityID);  // destroy this bar root (children follow)
                return;
            }

            // Drain the fill bar
            float progress = SimpleMath.Clamp(countdown / maxDuration, 0f, 1f);
            
            // scale the fill
            Vector3 fillScale = GetScale(fillID);
            fillScale.X = fullScaleX * progress;
            fillScale.Y = barScaleY;
            SetScale(fillID, ref fillScale);

            Vector3 fillPos = fillInitialPos;
            fillPos.X = fillInitialPos.X + (fullScaleX / 2f) * (1f - progress);
            SetPosition(fillID, ref fillPos);
        }

        // when game ends, bar should disappear immediately
        private void OnGameEnd(string eventName, string payload)
        {
            //if (gameEnded) return;
            //    gameEnded = true;

            //SceneDestroyEntity((uint)EntityID);

            if (isDead) return;
            KillSelf();
        }

        private void KillSelf()
        {
            if (isDead) return;
            isDead = true;
 
            Unsubscribe(GAMEOVEREVENT,   OnGameEnd);
            Unsubscribe(GAMEWINEVENT,    OnGameEnd);
            Unsubscribe(PLAYERDEADEVENT, OnGameEnd);
 
            // Engine does NOT auto-destroy children — do it explicitly
            if (fillID  != 0 && fillID  != INVALID_ENTITY) SceneDestroyEntity(fillID);
            if (bgID    != 0 && bgID    != INVALID_ENTITY) SceneDestroyEntity(bgID);
            if (labelID != 0 && labelID != INVALID_ENTITY) SceneDestroyEntity(labelID);
 
            SceneDestroyEntity((uint)EntityID);
            LogMessage("[SentryLifetimeBar] Killed and cleaned up.");
        }

        public override void OnDestroy()
        {

        }
    }
}
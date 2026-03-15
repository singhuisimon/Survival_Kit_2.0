using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game 
{
    public class SentryLifetimeBar : ScriptBehaviour
    {

        public static uint NextSentryID = 0;
        public static uint NextBGID     = 0;
        public static uint NextLabelID  = 0;

        // ===== Settings =====
        [SerializeField] private float maxDuration = 20.0f; // original is 90.0f for 90 seconds
        [SerializeField] private float heightOffset = 10.0f;
        [SerializeField] private float fullScaleX  = 12.0f;
        [SerializeField] private float barScaleY   = 1.0f;

        // ===== Entity IDs =====
        private uint sentryID = 0;
        private uint bgID     = 0;
        private uint labelID  = 0;

        // ===== State =====
        private float countdown   = 0f;
        private bool  initialized = false;
        private bool isDead      = false;

        private const uint INVALID_ENTITY = 0xffffffffu;

        private const string GAMEOVEREVENT = "GameOver";
        private const string GAMEWINEVENT  = "GameWin";
        private const string PLAYERDEADEVENT = "PlayerDead";

        public override void OnStart()
        {
            countdown = maxDuration;

            // Take ownership of BG and Label spawned by SentryCount
            bgID     = NextBGID;
            labelID  = NextLabelID;
            NextBGID    = 0;
            NextLabelID = 0;

            if (bgID == 0 || bgID == INVALID_ENTITY)
                LogWarning("[SentryLifetimeBar] NextBGID was not set — bar may look wrong.");


            if (NextSentryID != 0 && NextSentryID != INVALID_ENTITY)
            {
                sentryID    = NextSentryID;
                NextSentryID = 0;
                initialized  = true;
                LogMessage("[SentryLifetimeBar] Linked to sentry=" + sentryID + " bgID=" + bgID + " labelID=" + labelID);
            }
            else
            {
                LogWarning("[SentryLifetimeBar] NextSentryID was not set before instantiation!");
            }

            Vector3 sentryPos = GetPosition(sentryID);
            Vector3 barPos = new Vector3(sentryPos.X, sentryPos.Y + heightOffset, sentryPos.Z);
            if (bgID    != 0 && bgID    != INVALID_ENTITY) SetPosition(bgID, ref barPos);
            if (labelID != 0 && labelID != INVALID_ENTITY) {
                Vector3 labelPos = new Vector3(barPos.X, barPos.Y + 2.0f, barPos.Z);
                SetPosition(labelID, ref labelPos);
            }

            // Stamp fill into correct full state immediately
            Vector3 initScale = GetScale((uint)EntityID);
            initScale.X = fullScaleX;
            initScale.Y = barScaleY;
            SetScale((uint)EntityID, ref initScale);

            Subscribe(GAMEOVEREVENT,   OnGameEnd);
            Subscribe(GAMEWINEVENT,    OnGameEnd);
            Subscribe(PLAYERDEADEVENT, OnGameEnd);

        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead || !initialized) return;

            if (GameState.IsPaused) return;

            // Compute bar world position (above Sentry)
            Vector3 sentryPos = GetPosition(sentryID);
            Vector3 barPos = new Vector3(
                sentryPos.X,
                sentryPos.Y + heightOffset,
                sentryPos.Z
            );

            // Move fill (this entity)
            //SetPosition((uint)EntityID, ref barPos);

            // Move BG and Label to same world position
            if (bgID    != 0 && bgID    != INVALID_ENTITY) SetPosition(bgID,    ref barPos);
            if (labelID != 0 && labelID != INVALID_ENTITY)
            {
                Vector3 labelPos = new Vector3(barPos.X, barPos.Y + 2.0f, barPos.Z);
                SetPosition(labelID, ref labelPos);
            }
            countdown -= deltaTime;

            if (countdown <= 0f)
            {
                LogMessage("[SentryLifetimeBar] Lifetime expired — destroying sentry: " + sentryID);
                SceneDestroyEntity(sentryID);
                KillSelf();
                return;
            }

            // Draining the fillbar
            float progress = SimpleMath.Clamp(countdown / maxDuration, 0f, 1f);

            Vector3 fillScale = GetScale((uint)EntityID);
            fillScale.X = fullScaleX * progress;
            fillScale.Y = barScaleY;
            SetScale((uint)EntityID, ref fillScale);

            // Vector3 fillPos = new Vector3(
            //     barPos.X + (fullScaleX / 2f) - (fullScaleX * progress / 2f),
            //     barPos.Y,
            //     barPos.Z + 0.5f
            // );
            // SetPosition((uint)EntityID, ref fillPos);
        }

        // when game ends, bar should disappear immediately
        private void OnGameEnd(string eventName, string payload)
        {
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
 
            // Destroy BG and Label (owned by this script)
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
using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{

    public class SummonBarSentry : ScriptBehaviour
    {

        public static uint NextBGID    = 0;
        public static uint NextLabelID = 0;

        private string playerName = "Player";

        [SerializeField] private float heightOffset = 10.0f;
        [SerializeField] private float fullScaleX   = 15.3f;
        [SerializeField] private float barScaleY    = 1.0f;
        //[SerializeField] private float labelOffsetY = 4.0f;

        // ===== Entity IDs =====
        private uint playerID = 0;
        private uint bgID     = 0;
        private uint labelID  = 0;

        // ===== State =====
        private float progress    = 0f;
        private bool  initialized = false;

        private const uint INVALID_ENTITY = 0xffffffffu;

        private string progressEventName = "";
        private string killEventName     = "";

        private const string EVENT_GAMEOVER  = "GameOver";
        private const string EVENT_PLAYERDEAD = "PlayerDead";
        private const string EVENT_GAMEWIN   = "GameWin";

        public override void OnStart() 
        {
            playerID = SceneFindEntityByName(playerName);

            if (playerID == 0)
            {
                LogError("[SummonBar] Player not found!");
                return;
            }

            // Take ownership of BG and Label spawned by SentryCount
            bgID    = NextBGID;
            labelID = NextLabelID;
            NextBGID    = 0;
            NextLabelID = 0;

            Vector3 playerPos = GetPosition(playerID);
            Vector3 barPos = new Vector3(playerPos.X, playerPos.Y + heightOffset, playerPos.Z);
            if (bgID    != 0 && bgID    != INVALID_ENTITY) SetPosition(bgID, ref barPos);
            if (labelID != 0 && labelID != INVALID_ENTITY) {
                Vector3 labelPos = new Vector3(barPos.X, barPos.Y + 2.0f, barPos.Z);
                SetPosition(labelID, ref labelPos);
            }

            if (bgID == 0 || bgID == INVALID_ENTITY)
            {
                LogWarning("[SummonBar] NextBGID was not set — bar may look wrong.");
            }

            // Stamp fill into correct empty state immediately
            Vector3 initScale = GetScale((uint)EntityID);
            initScale.X = 0f;
            initScale.Y = barScaleY;
            SetScale((uint)EntityID, ref initScale);
        
            progressEventName = "SummonBarProgress:" + EntityID.ToString();
            killEventName     = "SummonBarKill:"     + EntityID.ToString();

            Subscribe(progressEventName, OnProgressUpdate);

            Subscribe(killEventName,     OnKill);
            Subscribe(EVENT_GAMEOVER,    OnKill);
            Subscribe(EVENT_PLAYERDEAD,  OnKill);
            Subscribe(EVENT_GAMEWIN,     OnKill);

            progress = 0f; // progress bar starts empty
            initialized = true;

            LogMessage("[SummonBar] Initialized. bgID=" + bgID + " labelID=" + labelID);

        }

        public override void OnUpdate(float deltaTime) 
        {
            if (!initialized) return;

            if (GameState.IsPaused) return;

            // Compute bar world position (above player)
            Vector3 playerPos = GetPosition(playerID);
            Vector3 barPos = new Vector3(
                playerPos.X,
                playerPos.Y + heightOffset,
                playerPos.Z
            );

            // Move fill 
            SetPosition((uint)EntityID, ref barPos);

            // Move BG and Label to same world position
            if (bgID    != 0 && bgID    != INVALID_ENTITY) SetPosition(bgID,    ref barPos);
            if (labelID != 0 && labelID != INVALID_ENTITY)
            {
                Vector3 labelPos = new Vector3(barPos.X, barPos.Y + 3.0f, barPos.Z);
                SetPosition(labelID, ref labelPos);
            }

            Vector3 fillScale = GetScale((uint)EntityID);
            fillScale.X = fullScaleX * progress;
            fillScale.Y = barScaleY;
            SetScale((uint)EntityID, ref fillScale);

            // Anchor to Left Edge
            // Vector3 fillPos = new Vector3(
            //     barPos.X - (fullScaleX / 2f) + (fullScaleX * progress / 2f),
            //     barPos.Y,
            //     barPos.Z + 0.5f
            // );
            // SetPosition((uint)EntityID, ref fillPos);
        }

        private void OnProgressUpdate(string eventName, string payload)
        {
            if (float.TryParse(payload, out float p)) 
                progress = SimpleMath.Clamp(p, 0f, 1f);
        }

        private void OnKill(string eventName, string payload)
        {
            if (!initialized) return;
            initialized = false; // prevent double-kill

            Unsubscribe(progressEventName, OnProgressUpdate);
            Unsubscribe(killEventName,     OnKill);
            Unsubscribe(EVENT_GAMEOVER,    OnKill);
            Unsubscribe(EVENT_PLAYERDEAD,  OnKill);
            Unsubscribe(EVENT_GAMEWIN,     OnKill);

            // Destroy BG and Label (owned by this script)
            if (bgID    != 0 && bgID    != INVALID_ENTITY) SceneDestroyEntity(bgID);
            if (labelID != 0 && labelID != INVALID_ENTITY) SceneDestroyEntity(labelID);

            SceneDestroyEntity((uint)EntityID);
            LogMessage("[SummonBar] Killed and cleaned up.");
        }

        public override void OnDestroy()
        {

        }

        

    }

}
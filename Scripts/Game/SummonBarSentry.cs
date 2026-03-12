using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{

    public class SummonBarSentry : ScriptBehaviour
    {
        private const string FILL_NAME = "SummonBarFill";
        private const string BG_NAME    = "SummonBarBG";
        private const string LABEL_NAME = "SummonBarLabel";

        private string playerName = "Player";

        [SerializeField] private float heightOffset = 10.0f;
        [SerializeField] private float fullScaleX   = 20.0f;
        [SerializeField] private float barScaleY    = 2.0f;
        //[SerializeField] private float labelOffsetY = 4.0f;

        private uint playerID = 0;
        private uint fillID   = 0;
        private uint bgID     = 0;
        private uint labelID  = 0;

        private float progress    = 0f;
        private bool  initialized = false;

        private Vector3 fillInitialPos;
        private bool    fillInitialPosCached = false;

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

            // cache children immediately in OnStart
            fillID  = SceneFindEntityByName(FILL_NAME);
            bgID    = SceneFindEntityByName(BG_NAME);
            labelID = SceneFindEntityByName(LABEL_NAME);

            if (fillID == 0 || fillID == INVALID_ENTITY)
            {
                LogError("[SummonBar] Could not find SummonBarFill!");
                return;
            }

            fillInitialPos = GetPosition(fillID);

            progressEventName = "SummonBarProgress:" + EntityID.ToString();
            killEventName     = "SummonBarKill:"     + EntityID.ToString();

            Subscribe(progressEventName, OnProgressUpdate);

            Subscribe(killEventName,     OnKill);
            Subscribe(EVENT_GAMEOVER,    OnKill);
            Subscribe(EVENT_PLAYERDEAD,  OnKill);
            Subscribe(EVENT_GAMEWIN,     OnKill);

            // Subscribe(EVENT_GAMEOVER, OnGameEnd);
            // Subscribe(EVENT_PLAYERDEAD, OnGameEnd);
            // Subscribe(EVENT_GAMEWIN, OnGameEnd);

            progress = 0f; // progress bar starts empty
            initialized = true;

            LogMessage("[SummonBar] Initialized.");
        }

        public override void OnUpdate(float deltaTime) 
        {
            if (!initialized) return;

            if (GameState.IsPaused) return;

            // move the bar root above the player
            Vector3 playerPos = GetPosition(playerID);
            Vector3 barPos = new Vector3(
                playerPos.X,
                playerPos.Y + heightOffset,
                playerPos.Z
            );
            SetPosition((uint)EntityID, ref barPos);


            Vector3 fillScale = GetScale(fillID);
            fillScale.X = fullScaleX * progress;
            fillScale.Y = barScaleY;
            SetScale(fillID, ref fillScale);

            Vector3 fillPos = fillInitialPos;
            fillPos.X = fillInitialPos.X + (fullScaleX / 2f) * (1f - progress);
            SetPosition(fillID, ref fillPos); 
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

            // Engine does NOT auto-destroy children — do it explicitly here
            // where we are guaranteed to have the cached IDs from OnUpdate
            if (fillID  != 0 && fillID  != INVALID_ENTITY) SceneDestroyEntity(fillID);
            if (bgID    != 0 && bgID    != INVALID_ENTITY) SceneDestroyEntity(bgID);
            if (labelID != 0 && labelID != INVALID_ENTITY) SceneDestroyEntity(labelID);

            SceneDestroyEntity((uint)EntityID);
            LogMessage("[SummonBar] Killed and cleaned up.");
        }

        public override void OnDestroy()
        {

        }

        // private void OnGameEnd(string eventName, string payload)
        // {
        //     SceneDestroyEntity((uint)EntityID);
        // }

        // public override void OnDestroy()
        // {
        //     Unsubscribe(progressEventName, OnProgressUpdate);
        //     Unsubscribe(EVENT_GAMEOVER, OnGameEnd);
        //     Unsubscribe(EVENT_PLAYERDEAD, OnGameEnd);
        //     Unsubscribe(EVENT_GAMEWIN, OnGameEnd);

        //     if (fillID  != 0 && fillID  != INVALID_ENTITY) SceneDestroyEntity(fillID);
        //     if (bgID    != 0 && bgID    != INVALID_ENTITY) SceneDestroyEntity(bgID);
        //     if (labelID != 0 && labelID != INVALID_ENTITY) SceneDestroyEntity(labelID);

        //     LogMessage("[SummonBar] Destroyed.");
        // }
        

    }

}
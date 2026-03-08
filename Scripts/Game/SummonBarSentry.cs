using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{

    public class SummonBar : ScriptBehaviour
    {
        private const string FILL_NAME  = "SummonBarFill";
        private const string BG_NAME    = "SummonBarBG";
        private const string LABEL_NAME = "SummonBarLabel";

        private string playerName = "Player";

        [SerializeField] private float heightOffset = 30.0f;
        [SerializeField] private float fullScaleX   = 50.0f;
        [SerializeField] private float barScaleY    = 6.0f;
        [SerializeField] private float labelOffsetY = 10.0f;

        private uint playerID = 0;
        private uint fillID   = 0;
        private uint bgID     = 0;
        private uint labelID  = 0;

        private float progress    = 0f;
        private bool  initialized = false;
        private const uint INVALID_ENTITY = 0xffffffffu;

        private const string EVENT_PROGRESS  = "SummonBarProgress";
        private const string EVENT_GAMEOVER  = "GameOver";
        private const string EVENT_GAMEOVER2 = "PlayerDead";
        private const string EVENT_GAMEWIN   = "GameWin";

        public override void OnStart() 
        {
            playerID = SceneFindEntityByName(playerName);
            fillID   = SceneFindEntityByName(FILL_NAME);
            bgID     = SceneFindEntityByName(BG_NAME);
            labelID  = SceneFindEntityByName(LABEL_NAME);

            if (playerID == 0) { LogError("[SummonBar] Player not found!"); return; }
            if (fillID   == 0) { LogError("[SummonBar] SummonBarFill not found!"); return; }
        
            // Set Initial fill scale
            Vector3 fillScale = GetScale(fillID);
            fillScale.X = fullScaleX;
            fillScale.Y = barScaleY;
            SetScale(fillID, ref fillScale);

            Subscribe(EVENT_PROGRESS,  OnProgressUpdate);
            Subscribe(EVENT_GAMEOVER,  OnGameEnd);
            Subscribe(EVENT_GAMEOVER2, OnGameEnd);
            Subscribe(EVENT_GAMEWIN,   OnGameEnd);

            initialized = true;
            LogMessage("[SummonBar] Initialized.");
        }

        public override void OnUpdate(float deltaTime) 
        {
            if (!initialized) return;
            if (GameState.IsPaused) return;

            // Follow player with Y offset
            Vector3 playerPos = GetPosition(playerID);
            Vector3 barPos = new Vector3(
                playerPos.X,
                playerPos.Y + heightOffset,
                playerPos.Z
            );
            SetPosition((uint)EntityID, ref barPos);

            // Position the bar BG (in full width)
            if (bgID != 0 && bgID != INVALID_ENTITY)
                SetPosition(bgID, ref barPos);

            // Fill: Shrink from right to left
            // Shrinking X scale by progress
            Vector3 fillScale = GetScale(fillID);
            fillScale.X = fullScaleX * progress;
            fillScale.Y = barScaleY;
            SetScale(fillID, ref fillScale);

            // Shift enter left so left edge stays anchored
            // Billboard faces camera so world X = screen horizontal
            float shift = fullScaleX * (1f - progress) * 0.5f;
            Vector3 fillPos = new Vector3(
                barPos.X - shift,
                barPos.Y,
                barPos.Z
            );
            SetPosition(fillID, ref fillPos);

            // Label floats above
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

        private void OnProgressUpdate(string eventName, string payload)
        {
            if (float.TryParse(payload, out float p)) 
                progress = SimpleMath.Clamp(p, 0f, 1f);
        }

        private void OnGameEnd(string eventName, string payload)
        {
            SceneDestroyEntity((uint)EntityID);
        }

                public override void OnDestroy()
        {
            Unsubscribe(EVENT_PROGRESS,  OnProgressUpdate);
            Unsubscribe(EVENT_GAMEOVER,  OnGameEnd);
            Unsubscribe(EVENT_GAMEOVER2, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN,   OnGameEnd);
            LogMessage("[SummonBar] Destroyed.");
        }
    }

}
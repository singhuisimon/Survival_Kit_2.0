using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;
using static Engine.Input;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Transform;
namespace Game
{
    /// <summary>
    /// SentryCount UI - display how many payload player picked up
    /// </summary>
    public class SentryCount : ScriptBehaviour
    {
        // ===== Settings =====
        [SerializeField] private int payloadCount = 0;

        // ===== Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";

        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string GAMEWIN = "GameWin";

        private const string EVENT_COLLECT_PAYLOAD = "CollectPayload";

        // ===== State =====
        private bool initialized = false;
        private int initialcount = 0;
        private bool spawnAndLifted = true;

        // ==== Timer =====
        [SerializeField] private float countdown = 0.0f;
        private float holdTimer = 3.0f;

        // ===== Entity =====
        private uint playerID = 0;
        private string playerName = "Player";

        // ===== Prefab =====
        private string sentryPrefab = "Sources/Prefabs/NormalSentry.prefab";
        private string sentrySpawnPrefab = "Sources/Prefabs/SentrySpawn.prefab";

        public override void OnStart()
        {
            LogMessage("=== SentryCount OnStart ===");
            LogMessage("SentryCount EntityID: " + EntityID);

            // Subscribe to lose conditions
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);

            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameOver);

            Event.Subscribe(EVENT_COLLECT_PAYLOAD, OnPayloadCollection);

            playerID = SceneFindEntityByName(playerName);

            if (playerID == 0)
            {
                LogError("[SentryCount] player cannot be found.");
                return;
            }

            // Initialize with starting count
            payloadCount = initialcount;

            countdown = holdTimer;

            spawnAndLifted = true;

            // Display initial time
            UpdateCountDisplay();

            initialized = true;
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // Don't count down when game is paused
            if (GameState.IsPaused)
                return;

            if(IsKeyPressed(KeyCode.D1) && spawnAndLifted){
                countdown -= deltaTime;

                if(countdown <= 0.0f){

                    LogMessage("[SentryCount] Detected sentry to be spawn!");
                    
                    SpawnSentry();

                    spawnAndLifted = false;
                }
            }

            // Check if player released before countdown is 0
            if (IsKeyReleased(KeyCode.D1))// && countdown > 0.0f)
            {
                //reset timer
                countdown = holdTimer;

                spawnAndLifted = true;
            }

        }

        private void SpawnSentry(){

            if(payloadCount <= 0){
                LogMessage("[SentryCount] Failed to spawn Sentry as out of payload");
                return;
            }

            uint sentryID = 0;
            sentryID = PrefabInstantiate(sentryPrefab);

            if(sentryID == 0){
                LogMessage("[SentryCount] Failed to spawn Sentry");
                return;
            }

            Vector3 spawnPos = GetPosition(playerID);
            Quat spawnRot = GetRotation(playerID);

            SetPosition(sentryID, ref spawnPos);
            SetRotation(sentryID, ref spawnRot);

            payloadCount -= 1;

            LogMessage("[SentryCount] Spawn Sentry, current payload left after deduction: " + payloadCount.ToString());

            // Update display
            UpdateCountDisplay();

            // Spawn Audio effect
            uint sentrySpawnID = 0;
            sentrySpawnID = PrefabInstantiate(sentrySpawnPrefab);

            if(sentrySpawnID == 0){
                LogMessage("[SentryCount] Failed to spawn Sentry spawning audio");
                return;
            }

            SetPosition(sentrySpawnID, ref spawnPos);
            SetRotation(sentrySpawnID, ref spawnRot);
        }

        private void OnPayloadCollection(string eventName, string payload){
            LogMessage("[SentryCount] Payload collection detected");
            payloadCount += 1;
            UpdateCountDisplay();
        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[SentryCount] Game over triggered by: " + eventName + " - Timer stopped");
            Text.SetIsVisible((uint)EntityID, false);
        }

        private void UpdateCountDisplay()
        {
            string timeText = "X" + payloadCount.ToString();

            // Update the text display
            SetText((uint)EntityID, timeText);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Unsubscribe(GAMEWIN, OnGameOver);
            Event.Unsubscribe(EVENT_COLLECT_PAYLOAD, OnPayloadCollection);
            LogMessage("=== SentryCount Destroyed ===");
        }
    }
}
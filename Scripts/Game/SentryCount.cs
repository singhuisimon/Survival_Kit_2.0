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
        private bool allowedspawn = true;

        // ==== Timer =====
        [SerializeField] private float countdown = 0.0f;
        private float holdTimer = 3.0f;

        // ===== Entity =====
        private uint playerID = 0;
        private string playerName = "Player";

        // ===== Sentry Prefabs =====
        private string sentryPrefab = "Sources/Prefabs/NormalSentry.prefab";
        private string sentrySpawnPrefab = "Sources/Prefabs/SentrySpawn.prefab";

        // ===== Prefabs for Loading Bar and Summon Bar for Sentries =====
        private string summonBarBG = "Sources/Prefabs/SummonBarBG.prefab";
        private string summonBarFill = "Sources/Prefabs/SummonBarFill.prefab";
        private string summonBarLabel = "Sources/Prefabs/SummonBarLabel.prefab";

        private string sentryBarBG = "Sources/Prefabs/SentryBarBG.prefab";
        private string sentryBarFill = "Sources/Prefabs/SentryBarFill.prefab";
        private string sentryBarLabel = "Sources/Prefabs/SentryBarLabel.prefab";

        // ===== Summon Bar State =====
        private uint   barEntityID     = 0;
        private bool   isHoldingKey    = false;

        [SerializeField] private float barHeightOffset = 10.0f;

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

            allowedspawn = true;

            // Display initial time
            UpdateCountDisplay();

            initialized = true;
        }


        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // Don't count down when game is paused
            if (GameState.IsPaused){
                if(GetIsVisible((uint)EntityID)){
                    SetIsVisible((uint)EntityID, false);
                }
                return;
            }

            if(!allowedspawn){
                return;
            }

            if(!GetIsVisible((uint)EntityID)){
                SetIsVisible((uint)EntityID, true);
            }

            if(IsKeyPressed(KeyCode.D1) && spawnAndLifted)
            {
                // Spawn bar on the very first frame of press (only if we have payload)
                if (!isHoldingKey && payloadCount > 0)
                {
                    isHoldingKey  = true;
                    SpawnSummonBar();
                }

                countdown -= deltaTime;

                // Publish progress (0 = just started, 1 = complete)
                if (barEntityID != 0)
                {
                    float progress = 1f - (countdown / holdTimer);
                    progress = SimpleMath.Clamp(progress, 0f, 1f);

                    Publish("SummonBarProgress:" + barEntityID.ToString(), progress.ToString("F4"));
                }

                if(countdown <= 0.0f)
                {

                    LogMessage("[SentryCount] - Sentry spawning!");

                    KillSummonBar(); 

                    SpawnSentry();

                    countdown = holdTimer;

                    spawnAndLifted = false;
                    isHoldingKey   = false;
                }
            }

            // Check if player released before countdown is 0
            if (IsKeyReleased(KeyCode.D1))// && countdown > 0.0f)
            {
                //reset timer
                countdown = holdTimer;

                spawnAndLifted = true;

                isHoldingKey   = false;

                KillSummonBar(); // tells bar to destroy itself + children

                // Cancelled early — destroy bar
                //if (barEntityID != 0)
                // {
                //     SceneDestroyEntity(barEntityID);
                //     barEntityID = 0;
                // }
            }

        }

        private void SpawnSummonBar()
        {
            Vector3 playerPos = GetPosition(playerID);
            Vector3 spawnPos = new Vector3 (playerPos.X, playerPos.Y + barHeightOffset, playerPos.Z);
            Quat identityRot = new Quat(0f, 0f, 0f, 1f);
            Vector3 defaultScale = new Vector3(1f, 1f, 1f);

            // Spawn BG and Label first
            uint bgID    = PrefabInstantiateWithTransform(summonBarBG,    ref spawnPos, ref identityRot, ref defaultScale, false);
            uint labelID = PrefabInstantiateWithTransform(summonBarLabel, ref spawnPos, ref identityRot, ref defaultScale, false);

            SummonBarSentry.NextBGID = bgID;
            SummonBarSentry.NextLabelID = labelID;

            barEntityID = PrefabInstantiateWithTransform(summonBarFill, ref spawnPos, ref identityRot, ref defaultScale, false);

            if (barEntityID == 0)
            {
                LogWarning("[SentryCount] Failed to spawn SummonBarFill.");
                
                // Clean up BG and Label if failed
                if (bgID    != 0) SceneDestroyEntity(bgID);
                if (labelID != 0) SceneDestroyEntity(labelID);
                SummonBarSentry.NextBGID    = 0;
                SummonBarSentry.NextLabelID = 0;
            } 
            else
            {
                LogMessage("[SentryCount] Spawned SummonBar fill=" + barEntityID + " bg=" + bgID + " label=" + labelID);

            }
        }

        private void KillSummonBar()
        {
            if (barEntityID != 0)
            {
                Publish("SummonBarKill:" + barEntityID.ToString(), "");
                barEntityID = 0;
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

            // Spawn sentrybar ( 3 independent prefabs )
            Vector3 sentryPos    = GetPosition(sentryID);
            Vector3 barSpawnPos  = new Vector3(sentryPos.X, sentryPos.Y + barHeightOffset, sentryPos.Z);
            Quat    identityRot  = new Quat(0f, 0f, 0f, 1f);
            Vector3 defaultScale = new Vector3(1f, 1f, 1f);

            uint sentryBGID    = PrefabInstantiateWithTransform(sentryBarBG,    ref barSpawnPos, ref identityRot, ref defaultScale, false);
            uint sentryLabelID = PrefabInstantiateWithTransform(sentryBarLabel, ref barSpawnPos, ref identityRot, ref defaultScale, false);

            // Pass all IDs to fill script via statics before instantiating fill
            SentryLifetimeBar.NextSentryID = sentryID;
            SentryLifetimeBar.NextBGID     = sentryBGID;
            SentryLifetimeBar.NextLabelID  = sentryLabelID;

            uint sentryBarFillID = PrefabInstantiateWithTransform(sentryBarFill, ref barSpawnPos, ref identityRot, ref defaultScale, false);

            if (sentryBarFillID == 0)
            {
                LogWarning("[SentryCount] Failed to spawn SentryBarFill.");
                if (sentryBGID    != 0) SceneDestroyEntity(sentryBGID);
                if (sentryLabelID != 0) SceneDestroyEntity(sentryLabelID);
                SentryLifetimeBar.NextSentryID = 0;
                SentryLifetimeBar.NextBGID     = 0;
                SentryLifetimeBar.NextLabelID  = 0;
            }
            else
            {
                LogMessage("[SentryCount] Spawned SentryBar fill=" + sentryBarFillID + " bg=" + sentryBGID + " for sentry=" + sentryID);
            }



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
            LogMessage("[SentryCount] Game over triggered by: " + eventName);
            SetText((uint)EntityID, "");
            Text.SetIsVisible((uint)EntityID, false);
            allowedspawn = false;

            // Clean up if player was mid-hold
            KillSummonBar(); 

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
using Engine;
using System;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Physics;
using static Engine.Rigidbody;
using static Engine.Audio;
using static Engine.Tag;
using static Engine.Transform;

namespace Game
{
    public class TrenchRunEndTutorial : ScriptBehaviour
    {
        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;

        private const string TAG_PLAYER = "Player";

        // Event
        private const string COREDEAD = "CoreDeadTriggerPostTrenchRun";
        private const string EVENT_COLLECT_PAYLOAD = "CollectPayload";
         private const string EVENT_END_COOLDOWN = "DemoSentryCooldown";
        private const string EVENT_END_FIRE = "DemoSentryFinishCooldown";

        [SerializeField] private string payloadPrefab = "Sources/Prefabs/Payload.prefab";
        [SerializeField] private string upgradeModuleLabelPrefab = "Sources/Prefabs/UpgradeModuleLabel.prefab";
        private string sentryPrefab = "Sources/Prefabs/NormalSentry.prefab";
        private string sentrySpawnPrefab = "Sources/Prefabs/SentrySpawn.prefab";
        private string turretPrefab = "Sources/Prefabs/Keylogger_TrenchEnd.prefab";
        private uint spawnedPayloadID = INVALID_ENTITY;
        private uint labelEntityID = INVALID_ENTITY;

        private bool hasSpawnedUpgrade = false;
        private bool hasCollectedUpgrade = false;
        private bool hasSpawnedSentry = false;
        private bool hasSpawnedTurret = false;
        private float countdown = 3.0f;
        private float turretWaitCountdown = 5.0f;

        // Lifecycle
       public override void OnStart()
        {
            playerID = SceneFindEntityByName(TAG_PLAYER);

            Subscribe(COREDEAD, OnCoreDeath);
            Subscribe(EVENT_COLLECT_PAYLOAD, OnCollectPayload);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused)
                    return;

            if (hasCollectedUpgrade && !hasSpawnedSentry)
            {

                SpawnSentry();
                
            }

            if (hasSpawnedSentry && !hasSpawnedTurret)
            {
                turretWaitCountdown -= deltaTime;

                if (turretWaitCountdown <= 0.0f)
                {
                    SpawnTurret();
                }
                
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(COREDEAD, OnCoreDeath);
            Unsubscribe(EVENT_COLLECT_PAYLOAD, OnCollectPayload);
        }

        // Event
        private void OnCoreDeath(string eventName, string payload)
        {
            if (hasSpawnedUpgrade)
                return;

            hasSpawnedUpgrade = true;

            Vector3 spawnPos;
            Quat spawnRot;

            StringToTransform(payload, out spawnPos, out spawnRot);

            SpawnUpgrade(spawnPos, spawnRot);
        }

        // Spawn Logic
        private void SpawnUpgrade(Vector3 spawnPos, Quat spawnRot)
        {
            // Spawn payload
            Vector3 payloadScale = new Vector3(10.0f, 10.0f, 10.0f);

            uint payload = PrefabInstantiateWithTransform(
                payloadPrefab,
                ref spawnPos,
                ref spawnRot,
                ref payloadScale,
                false
            );

            if (payload == 0)
            {
                LogMessage("[TrenchRunEndTutorial] Payload failed to instantiate");
                return;
            }

            spawnedPayloadID = payload;

            RigidbodySetIsKinematic(payload, false);

            Vector3 halfboxExtend = new Vector3(20f, 25f, 20f);
            RigidbodySetBoxHalfExtents(payload, ref halfboxExtend);

            // Spawn label
            Vector3 labelPos = new Vector3(spawnPos.X, -375.182f , spawnPos.Z);
            Quat identityRot = new Quat(0f, 0f, 0f, 1f);
            Vector3 labelScale = new Vector3(50.0f, 21.0f, 1.0f);

            labelEntityID = PrefabInstantiateWithTransform(
                upgradeModuleLabelPrefab,
                ref labelPos,
                ref identityRot,
                ref labelScale,
                false
            );

            if (labelEntityID == 0)
            {
                LogMessage("[TrenchRunEndTutorial] UpgradeModuleLabel failed to instantiate");
            }
        }

        private void OnCollectPayload(string eventName, string payload)
        {
            // Destroy label if it exists
            if (labelEntityID != INVALID_ENTITY)
            {
                SceneDestroyEntity(labelEntityID);
                labelEntityID = INVALID_ENTITY;
            }

            // Reset payload ID since Payload.cs already self-destructs
            spawnedPayloadID = INVALID_ENTITY;
            
            hasCollectedUpgrade = true;
        }

        private void SpawnSentry()
        {
            uint sentryID = PrefabInstantiate(sentryPrefab);
            if (sentryID == 0)
            {
                LogMessage("[TrenchRunEndTutorial] Failed to spawn Sentry");
                return;
            }

            Vector3 spawnPos = GetPosition(playerID);
            Quat spawnRot = GetRotation(playerID);
            SetPosition(sentryID, ref spawnPos);
            SetRotation(sentryID, ref spawnRot);

            // Spawn audio
            uint sentrySpawnID = PrefabInstantiate(sentrySpawnPrefab);
            if (sentrySpawnID != 0)
            {
                SetPosition(sentrySpawnID, ref spawnPos);
                SetRotation(sentrySpawnID, ref spawnRot);
            }

            hasSpawnedSentry = true;
            LogMessage("[TrenchRunEndTutorial] Sentry spawned successfully");
        }

        private void SpawnTurret()
        {
            uint turretID = PrefabInstantiate(turretPrefab);
            if (turretID == 0)
            {
                LogMessage("[TrenchRunEndTutorial] Failed to spawn Turret");
                return;
            }

            Vector3 spawnPos = GetPosition(playerID);
            Quat spawnRot = GetRotation(playerID);
            SetPosition(turretID, ref spawnPos);
            SetRotation(turretID, ref spawnRot);

            hasSpawnedTurret = true;
            LogMessage("[TrenchRunEndTutorial] Turret spawned successfully");
        }

        // Transform Serialization
        public static void StringToTransform(string data, out Vector3 pos, out Quat rot)
        {
            string[] parts = data.Split('|');

            string[] posParts = parts[0].Split(',');
            string[] rotParts = parts[1].Split(',');

            pos = new Vector3(
                float.Parse(posParts[0]),
                float.Parse(posParts[1]),
                float.Parse(posParts[2])
            );

            rot = new Quat(
                float.Parse(rotParts[0]),
                float.Parse(rotParts[1]),
                float.Parse(rotParts[2]),
                float.Parse(rotParts[3])
            );
        }
    }
}
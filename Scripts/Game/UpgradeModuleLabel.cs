using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.SimpleMath;
using static Engine.Event;

namespace Game
{

    public class UpgradeModuleLabel : ScriptBehaviour
    {
        [SerializeField] private string playerEntityName = "Player";

        // Half-size of the payload cube — label sits on its surface
        [SerializeField] private float payloadHalfSize = 25.0f;

        // Small extra gap so the label isn't z-fighting with the payload face
        [SerializeField] private float surfaceGap = 15.0f;

        private uint playerEntityID = 0;
        private uint payloadEntityID = 0;
        private bool isDead = false;

        private string initEventName = "";
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        public override void OnStart()
        {
            LogMessage("[UpgradeModuleLabel] Started, EntityID: " + EntityID);

            playerEntityID = SceneFindEntityByName(playerEntityName);
            if (playerEntityID == 0)
                LogWarning("[UpgradeModuleLabel] Could not find player '" + playerEntityName + "'");

            // Subscribe to our own unique init event so LoveletterScript can pass the payload ID
            initEventName = "UpgradeLabelInit:" + EntityID.ToString();
            Subscribe(initEventName, OnInit);

            Subscribe(GAMEOVER, OnGameOver);
            Subscribe(GAMEWIN, OnGameOver);
        }

        private void OnInit(string eventName, string payload)
        {
            if (uint.TryParse(payload, out uint id))
            {
                payloadEntityID = id;
                LogMessage("[UpgradeModuleLabel] Linked to payload entity ID: " + payloadEntityID);
            }
            else
            {
                LogWarning("[UpgradeModuleLabel] Failed to parse payload ID from init event: " + payload);
            }

            // Unsubscribe — only need this once
            Unsubscribe(initEventName, OnInit);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead || playerEntityID == 0 || payloadEntityID == 0)
                return;

            if (GameState.IsPaused)
                return;

            UpdateSurfacePosition();
        }

        private void UpdateSurfacePosition()
        {
            Engine.Vector3 payloadPos = GetPosition(payloadEntityID);
            Engine.Vector3 playerPos  = GetPosition(playerEntityID);

            // Place label directly above the payload surface
            float offset = payloadHalfSize + surfaceGap;
            Engine.Vector3 labelPos = new Engine.Vector3(
                payloadPos.X,
                payloadPos.Y + offset,
                payloadPos.Z
            );

            SetPosition((uint)EntityID, ref labelPos);

            // Still rotate to face the player on Y-axis
            float dx = playerPos.X - payloadPos.X;
            float dz = playerPos.Z - payloadPos.Z;
            float mag = Sqrt(dx * dx + dz * dz);

            if (mag < 0.001f)
                return;

            float nx = dx / mag;
            float nz = dz / mag;

            float angle = Atan2(nx, nz);
            float halfAngle = angle * 0.5f;
            Engine.Quat faceRot = new Engine.Quat(
                0f,
                Sin(halfAngle),
                0f,
                Cos(halfAngle)
            );
            SetRotation((uint)EntityID, ref faceRot);
        }

        public override void OnDestroy()
        {
            Unsubscribe(initEventName, OnInit);
            Unsubscribe(GAMEOVER, OnGameOver);
            Unsubscribe(GAMEWIN, OnGameOver);
            LogMessage("[UpgradeModuleLabel] Destroyed");
        }

        private void OnGameOver(string eventName, string payload)
        {
            if (isDead) return;
            isDead = true;
            SceneDestroyEntity((uint)EntityID);
        }
    }
}
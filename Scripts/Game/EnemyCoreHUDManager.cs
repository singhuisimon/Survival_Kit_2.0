using System;
using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    public class EnemyCoreHUDManager : ScriptBehaviour
    {
        [SerializeField] private float visibilityRange = 200.0f;
        [SerializeField] private float pollInterval = 0.5f;

        private const string PLAYER_NAME = "Player";
        private const string CORE_NAME = "Enemy_Core";

        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string GAMEWIN = "GameWin";

        private uint playerID = 0;
        private uint coreID = 0;

        // HUD entity IDs
        private uint hpCountID = 0;
        private uint hpBarID = 0;
        private uint hpFrameID = 0;
        private uint hpRefillID = 0;
        private uint hpRedFillID = 0;

        private float pollTimer = 0.0f;
        private bool isVisible = false;
        private bool gameEnded = false;

        public override void OnStart()
        {
            playerID = SceneFindEntityByName(PLAYER_NAME);
            coreID = SceneFindEntityByTag(CORE_NAME);

            hpCountID = SceneFindEntityByName("EnemyCoreHPCount");
            hpBarID = SceneFindEntityByName("EnemyCoreHP");
            hpFrameID = SceneFindEntityByName("EnemyCoreHPFrame");
            hpRefillID = SceneFindEntityByName("EnemyCoreHPRefill");
            hpRedFillID = SceneFindEntityByName("EnemyCoreHPRedFill");

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Subscribe(ENEMY_CORE_DEATH, OnGameEnd);
            Event.Subscribe(GAMEWIN, OnGameEnd);

            SetHUDVisible(false);
            LogMessage("[EnemyCoreHUDManager] Initialized. VisibilityRange: " + visibilityRange);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (gameEnded) return;
            if (GameState.IsPaused) return;

            pollTimer -= deltaTime;
            if (pollTimer > 0.0f) return;
            pollTimer = pollInterval;

            if (playerID == 0 || coreID == 0)
            {
                playerID = SceneFindEntityByName(PLAYER_NAME);
                coreID = SceneFindEntityByTag(CORE_NAME);
                return;
            }

            Vector3 playerPos = GetPosition(playerID);
            Vector3 corePos = GetPosition(coreID);

            float dx = playerPos.X - corePos.X;
            float dy = playerPos.Y - corePos.Y;
            float dz = playerPos.Z - corePos.Z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float rangeSq = visibilityRange * visibilityRange;

            bool shouldBeVisible = distSq <= rangeSq;

            if (shouldBeVisible != isVisible)
            {
                isVisible = shouldBeVisible;
                SetHUDVisible(isVisible);
                LogMessage("[EnemyCoreHUDManager] HUD " + (isVisible ? "shown" : "hidden") + " dist²=" + distSq);
            }
        }

        private void SetHUDVisible(bool visible)
        {
            if (hpCountID != 0) Text.SetIsVisible(hpCountID, visible);
            if (hpBarID != 0) SpriteRenderer.SetIsVisible(hpBarID, visible);
            if (hpFrameID != 0) SpriteRenderer.SetIsVisible(hpFrameID, visible);
            if (hpRefillID != 0) SpriteRenderer.SetIsVisible(hpRefillID, visible);
            if (hpRedFillID != 0) SpriteRenderer.SetIsVisible(hpRedFillID, visible);
        }

        private void OnGameEnd(string eventName, string payload)
        {
            gameEnded = true;
            SetHUDVisible(false);
            LogMessage("[EnemyCoreHUDManager] Game ended - HUD hidden");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Unsubscribe(ENEMY_CORE_DEATH, OnGameEnd);
            Event.Unsubscribe(GAMEWIN, OnGameEnd);
        }
    }
}
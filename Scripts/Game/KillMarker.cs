using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    public class KillMarker : ScriptBehaviour
    {
        private const string EVENT_BOTNET_DEAD = "BotnetDeath";
        private const string EVENT_LOVELETTER_DEAD = "LoveLetterDeath";
        private const string EVENT_WORMHOST_DEAD = "WormHostDead";

        private float displayTime = 0.15f;

        private bool isShowing = false;
        private float currentDisplayTimer = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== KillMarker OnStart ===");
            LogMessage("KillMarker EntityID: " + EntityID);

            Event.Subscribe(EVENT_BOTNET_DEAD, OnEnemyKilled);
            Event.Subscribe(EVENT_LOVELETTER_DEAD, OnEnemyKilled);
            Event.Subscribe(EVENT_WORMHOST_DEAD, OnEnemyKilled);

            SetIsVisible((uint)EntityID, false);

            LogMessage("[KillMarker] Initialized");
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!isShowing)
                return;

            currentDisplayTimer -= deltaTime;
            if (currentDisplayTimer <= 0.0f)
                HideKillMarker();
        }

        private void OnEnemyKilled(string eventName, string payload)
        {
            LogMessage("[KillMarker] Death event received: " + eventName + " | payload: " + payload);

            // Only show if the killing blow came from a player bullet
            // Botnet publishes "killer=PrimaryBullet" or "killer=PrimaryUltBullet"
            if (!payload.Contains("PrimaryBullet") && !payload.Contains("PrimaryUltBullet"))
            {
                LogMessage("[KillMarker] Not a player kill - ignoring");
                return;
            }

            LogMessage("[KillMarker] Player kill confirmed - showing marker");
            ShowKillMarker();
        }

        private void ShowKillMarker()
        {
            SetIsVisible((uint)EntityID, true);
            isShowing = true;
            currentDisplayTimer = displayTime;
        }

        private void HideKillMarker()
        {
            SetIsVisible((uint)EntityID, false);
            isShowing = false;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_BOTNET_DEAD, OnEnemyKilled);
            Event.Unsubscribe(EVENT_LOVELETTER_DEAD, OnEnemyKilled);
            Event.Unsubscribe(EVENT_WORMHOST_DEAD, OnEnemyKilled);

            LogMessage("=== KillMarker Destroyed ===");
        }
    }
}

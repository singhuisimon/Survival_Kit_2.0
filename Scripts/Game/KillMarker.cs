using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// KillMarker - Flashes kill marker sprite when any enemy is killed
    /// Attach this to the kill marker sprite entity
    /// Listens to: "BotnetDeath", "LoveLetterDestroyed", "WormHostDead"
    /// Same flash pattern as Hitmarker
    /// </summary>
    public class KillMarker : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_BOTNET_DEAD = "BotnetDeath";
        private const string EVENT_LOVELETTER_DEAD = "LoveLetterDestroyed";
        private const string EVENT_WORMHOST_DEAD = "WormHostDead";

        // ===== Settings =====
        private float displayTime = 0.15f;

        // ===== State =====
        private bool isShowing = false;
        private float currentDisplayTimer = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== KillMarker OnStart ===");
            LogMessage("KillMarker EntityID: " + EntityID);

            // Subscribe to all enemy death events
            Event.Subscribe(EVENT_BOTNET_DEAD, OnEnemyKilled);
            Event.Subscribe(EVENT_LOVELETTER_DEAD, OnEnemyKilled);
            Event.Subscribe(EVENT_WORMHOST_DEAD, OnEnemyKilled);

            // Start hidden
            SetIsVisible((uint)EntityID, false);

            LogMessage("[KillMarker] Initialized - listening for enemy kills");
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!isShowing)
                return;

            // Timer countdown
            currentDisplayTimer -= deltaTime;
            if (currentDisplayTimer <= 0.0f)
            {
                HideKillMarker();
            }
        }

        // ===== EVENT HANDLER =====

        private void OnEnemyKilled(string eventName, string payload)
        {
            LogMessage("[KillMarker] Enemy killed! Event: " + eventName);
            ShowKillMarker();
        }

        // ===== HELPERS =====

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
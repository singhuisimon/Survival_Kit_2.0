using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// ScoreBoard - Tracks and displays player score
    /// Attach this to the score text entity
    /// Displays as "SCORE 0000000" (7 digit zero-padded)
    /// 
    /// Points:
    ///   Botnet       = 10
    ///   WormHost     = 100
    ///   LoveLetter   = 1111
    /// </summary>
    public class ScoreBoard : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_BOTNET_DEAD = "BotnetDeath";
        private const string EVENT_LOVELETTER_DEAD = "LoveLetterKilled";
        private const string EVENT_WORMHOST_DEAD = "WormHostDead";

        // ===== Points =====
        private const int POINTS_BOTNET = 10;
        private const int POINTS_WORMHOST = 100;
        private const int POINTS_LOVELETTER = 1111;

        // ===== State =====
        private int score = 0;

        public override void OnStart()
        {
            LogMessage("=== ScoreBoard OnStart ===");
            LogMessage("ScoreBoard EntityID: " + EntityID);

            // Subscribe to all enemy death events
            Event.Subscribe(EVENT_BOTNET_DEAD, OnBotnetKilled);
            Event.Subscribe(EVENT_LOVELETTER_DEAD, OnLoveLetterKilled);
            Event.Subscribe(EVENT_WORMHOST_DEAD, OnWormHostKilled);

            // Initialize score
            score = 0;
            UpdateScoreDisplay();

            LogMessage("[ScoreBoard] Initialized");
        }

        // ===== EVENT HANDLERS =====

        private void OnBotnetKilled(string eventName, string payload)
        {
            score += POINTS_BOTNET;
            LogMessage("[ScoreBoard] Botnet killed! +" + POINTS_BOTNET + " | Score: " + score);
            UpdateScoreDisplay();
        }

        private void OnLoveLetterKilled(string eventName, string payload)
        {
            score += POINTS_LOVELETTER;
            LogMessage("[ScoreBoard] LoveLetter killed! +" + POINTS_LOVELETTER + " | Score: " + score);
            UpdateScoreDisplay();
        }

        private void OnWormHostKilled(string eventName, string payload)
        {
            score += POINTS_WORMHOST;
            LogMessage("[ScoreBoard] WormHost killed! +" + POINTS_WORMHOST + " | Score: " + score);
            UpdateScoreDisplay();
        }

        // ===== DISPLAY =====

        private void UpdateScoreDisplay()
        {
            // Format: "SCORE 0000000" (7 digits, zero-padded)
            string scoreText = "SCORE " + score.ToString("D7");
            SetText((uint)EntityID, scoreText);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_BOTNET_DEAD, OnBotnetKilled);
            Event.Unsubscribe(EVENT_LOVELETTER_DEAD, OnLoveLetterKilled);
            Event.Unsubscribe(EVENT_WORMHOST_DEAD, OnWormHostKilled);

            LogMessage("=== ScoreBoard Destroyed ===");
        }
    }
}
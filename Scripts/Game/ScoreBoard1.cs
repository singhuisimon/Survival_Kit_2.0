using System;
using System.Collections.Generic;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// ScoreBoard1 - Tracks and displays player score for Level 1
    /// Attach this to the score text entity
    /// Displays as "SCORE 0000000" (7 digit zero-padded)
    /// Hides on win or lose
    /// Saves top 10 scores with timestamps to JSON
    ///
    /// Points:
    ///   Botnet       = 10
    ///   WormHost     = 100
    ///   LoveLetter   = 1111
    ///
    /// JSON format:
    ///   { "Level1": [{"score":5000,"date":"2025-02-23 14:30:45"}, ...] }
    /// </summary>
    public class ScoreBoard1 : ScriptBehaviour
    {
        // ===== Enemy Death Events =====
        private const string EVENT_BOTNET_DEAD = "BotnetDeath";
        private const string EVENT_LOVELETTER_DEAD = "LoveLetterKilled";
        private const string EVENT_WORMHOST_DEAD = "WormHostDead";

        // ===== Game State Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string EVENT_TURRET_DEAD = "EnemyTurretDestroyed";
        private const string GAMEWIN = "GameWin";

        // ===== Points =====
        private const int POINTS_BOTNET = 30;
        private const int POINTS_WORMHOST = 35;
        private const int POINTS_LOVELETTER = 50;
        private const int POINTS_TURRET = 25;

        // ===== Score Persistence =====
        private const string SAVE_FILE = "Resources/Sources/SaveData/ScoreData1.json";
        private const int MAX_SCORES = 10;

        // ===== State =====
        private int score = 0;
        private bool scoreSaved = false;    // Guard: only save once per session
        private string currentLevel = "trench_run";

        public override void OnStart()
        {
            LogMessage("=== ScoreBoard1 OnStart ===");
            LogMessage("ScoreBoard1 EntityID: " + EntityID);

            // Subscribe to enemy death events
            Event.Subscribe(EVENT_BOTNET_DEAD, OnBotnetKilled);
            Event.Subscribe(EVENT_LOVELETTER_DEAD, OnLoveLetterKilled);
            Event.Subscribe(EVENT_WORMHOST_DEAD, OnWormHostKilled);
            Event.Subscribe(EVENT_TURRET_DEAD, OnTurretKilled);

            // Subscribe to game state events
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Subscribe(ENEMY_CORE_DEATH, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameWin);

            // Initialize score
            score = 0;
            scoreSaved = false;
            UpdateScoreDisplay();

            LogMessage("[ScoreBoard1] Initialized for level: " + currentLevel);
        }

        // ===== ENEMY KILL HANDLERS =====

        private void OnBotnetKilled(string eventName, string payload)
        {
            score += POINTS_BOTNET;
            LogMessage("[ScoreBoard1] Botnet killed! +" + POINTS_BOTNET + " | Score: " + score);
            UpdateScoreDisplay();
        }

        private void OnLoveLetterKilled(string eventName, string payload)
        {
            score += POINTS_LOVELETTER;
            LogMessage("[ScoreBoard1] LoveLetter killed! +" + POINTS_LOVELETTER + " | Score: " + score);
            UpdateScoreDisplay();
        }

        private void OnWormHostKilled(string eventName, string payload)
        {
            score += POINTS_WORMHOST;
            LogMessage("[ScoreBoard1] WormHost killed! +" + POINTS_WORMHOST + " | Score: " + score);
            UpdateScoreDisplay();
        }

        private void OnTurretKilled(string eventName, string payload)
        {
            score += POINTS_TURRET;
            LogMessage("[ScoreBoard1] Turret killed! +" + POINTS_TURRET + " | Score: " + score);
            UpdateScoreDisplay();
        }

        // ===== GAME STATE HANDLERS =====

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[ScoreBoard1] Game over event: " + eventName);
            SaveScore();
            Text.SetIsVisible((uint)EntityID, false);
        }

        private void OnGameWin(string eventName, string payload)
        {
            LogMessage("[ScoreBoard1] Game win event");
            score += 1000;
            LogMessage("[ScoreBoard1] Win bonus +1000 | Score: " + score);
            UpdateScoreDisplay();
            SaveScore();
            Engine.ProgressTracker.MarkLevelWon("trench_run");
            Text.SetIsVisible((uint)EntityID, false);
        }

        // ===== DISPLAY =====

        private void UpdateScoreDisplay()
        {
            // Format: "SCORE 0000000" (7 digits, zero-padded)
            string scoreText = "SCORE " + score.ToString("D7");
            SetText((uint)EntityID, scoreText);
        }

        // ===== SCORE PERSISTENCE =====

        private void SaveScore()
        {
            // Guard: only save once per game session
            if (scoreSaved)
            {
                LogMessage("[ScoreBoard1] Score already saved this session, skipping");
                return;
            }

            try
            {
                // Get current timestamp
                string timestamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

                // Load all existing scores from file
                Dictionary<string, List<ScoreEntry>> allScores = LoadAllScores();

                // Get scores for this level, or start fresh
                if (!allScores.ContainsKey(currentLevel))
                    allScores[currentLevel] = new List<ScoreEntry>();

                List<ScoreEntry> levelScores = allScores[currentLevel];

                // Add new score with timestamp
                levelScores.Add(new ScoreEntry { score = score, date = timestamp });

                // Sort by score descending, keep top 10
                levelScores.Sort((a, b) => b.score.CompareTo(a.score));
                if (levelScores.Count > MAX_SCORES)
                    levelScores.RemoveRange(MAX_SCORES, levelScores.Count - MAX_SCORES);

                allScores[currentLevel] = levelScores;

                // Write back to file
                string json = SerializeScores(allScores);
                if (FileIO.WriteAllText(SAVE_FILE, json))
                {
                    scoreSaved = true;
                    Engine.ProgressTracker.AddCumulativeScore(score);
                    LogMessage("[ScoreBoard1] Score " + score + " saved for " + currentLevel + " at " + timestamp);
                }
                else
                {
                    LogError("[ScoreBoard1] Failed to write score file");
                }
            }
            catch (Exception e)
            {
                LogError("[ScoreBoard1] SaveScore error: " + e.Message);
            }
        }

        private Dictionary<string, List<ScoreEntry>> LoadAllScores()
        {
            var result = new Dictionary<string, List<ScoreEntry>>();

            try
            {
                if (!FileIO.FileExists(SAVE_FILE))
                {
                    LogMessage("[ScoreBoard1] No save file found, starting fresh");
                    return result;
                }

                string json = FileIO.ReadAllText(SAVE_FILE);
                json = json.Trim().TrimStart('{').TrimEnd('}');

                // Split entries by "],"
                string[] entries = json.Split(new string[] { "]," }, StringSplitOptions.RemoveEmptyEntries);

                foreach (string entry in entries)
                {
                    string cleaned = entry.Trim().TrimEnd(']');
                    string[] parts = cleaned.Split(new char[] { ':' }, 2);
                    if (parts.Length != 2) continue;

                    string key = parts[0].Trim().Trim('"');
                    string arrayContent = parts[1].Trim().TrimStart('[');

                    var scores = new List<ScoreEntry>();

                    // Parse score objects: {"score":5000,"date":"2025-02-23 14:30:45"}
                    string[] objects = arrayContent.Split(new string[] { "},{" }, StringSplitOptions.RemoveEmptyEntries);
                    foreach (string obj in objects)
                    {
                        string objClean = obj.Trim().TrimStart('{').TrimEnd('}');
                        string[] fields = objClean.Split(',');

                        int scoreVal = 0;
                        string dateVal = "";

                        foreach (string field in fields)
                        {
                            string[] kv = field.Split(':');
                            if (kv.Length != 2) continue;

                            string k = kv[0].Trim().Trim('"');
                            string v = kv[1].Trim().Trim('"');

                            if (k == "score")
                                int.TryParse(v, out scoreVal);
                            else if (k == "date")
                                dateVal = v;
                        }

                        if (scoreVal > 0)
                            scores.Add(new ScoreEntry { score = scoreVal, date = dateVal });
                    }

                    result[key] = scores;
                    LogMessage("[ScoreBoard1] Loaded " + scores.Count + " scores for " + key);
                }
            }
            catch (Exception e)
            {
                LogError("[ScoreBoard1] LoadAllScores error: " + e.Message);
            }

            return result;
        }

        private string SerializeScores(Dictionary<string, List<ScoreEntry>> allScores)
        {
            string json = "{\n";
            bool firstLevel = true;

            foreach (var kvp in allScores)
            {
                if (!firstLevel) json += ",\n";
                firstLevel = false;

                json += "  \"" + kvp.Key + "\": [\n";
                for (int i = 0; i < kvp.Value.Count; i++)
                {
                    if (i > 0) json += ",\n";
                    json += "    {\"score\":" + kvp.Value[i].score + ",\"date\":\"" + kvp.Value[i].date + "\"}";
                }
                json += "\n  ]";
            }

            json += "\n}";
            return json;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_BOTNET_DEAD, OnBotnetKilled);
            Event.Unsubscribe(EVENT_LOVELETTER_DEAD, OnLoveLetterKilled);
            Event.Unsubscribe(EVENT_WORMHOST_DEAD, OnWormHostKilled);
            Event.Unsubscribe(EVENT_TURRET_DEAD, OnTurretKilled);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Unsubscribe(ENEMY_CORE_DEATH, OnGameOver);
            Event.Unsubscribe(GAMEWIN, OnGameWin);

            LogMessage("=== ScoreBoard1 Destroyed ===");
        }

        // ===== HELPER CLASS =====
        private class ScoreEntry
        {
            public int score;
            public string date;
        }
    }
}
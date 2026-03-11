using System;
using static Engine.Logger;
using static Engine.Event;

namespace Engine
{
    /// <summary>
    /// Tracks which levels the player has won.
    /// Persists across sessions via progress.json.
    /// </summary>
    public static class ProgressTracker
    {
        private const string SAVE_FILE = "Resources/Sources/SaveData/progress.json";

        public static bool HasWonTrenchRun { get; private set; } = false;
        public static bool HasWonLevel2 { get; private set; } = false;
        public static int CumulativeScore { get; set; } = 0;

        // Shop state: skin purchases and equipped skin
        // 0 = default, 1 = skin 1, 2 = skin 2
        public static bool Skin1Purchased { get; set; } = false;
        public static bool Skin2Purchased { get; set; } = false;
        public static bool Skin3Purchased { get; set; } = false;
        public static int EquippedSkin { get; set; } = 0;
        public static int ByteChips { get; set; } = 0;

        // ByteChips pack purchase tracking
        public static bool BytePack1Bought { get; set; } = false;
        public static bool BytePack2Bought { get; set; } = false;
        public static bool BytePack3Bought { get; set; } = false;
        public static bool BytePack4Bought { get; set; } = false;

        /// <summary>
        /// Loads progress from disk. Call on main menu load.
        /// </summary>
        public static void LoadProgress()
        {
            HasWonTrenchRun = false;
            HasWonLevel2 = false;
            CumulativeScore = 0;
            Skin1Purchased = false;
            Skin2Purchased = false;
            Skin3Purchased = false;
            EquippedSkin = 0;
            ByteChips = 0;
            BytePack1Bought = false;
            BytePack2Bought = false;
            BytePack3Bought = false;
            BytePack4Bought = false;

            try
            {
                if (!FileIO.FileExists(SAVE_FILE))
                {
                    LogMessage("[ProgressTracker] No progress file found, starting fresh");
                    return;
                }

                string json = FileIO.ReadAllText(SAVE_FILE);
                LogMessage("[ProgressTracker] Loaded progress: " + json);

                // Parse simple JSON: {"trench_run_won":true,"level2_won":false}
                if (json.Contains("\"trench_run_won\":true") || json.Contains("\"trench_run_won\": true"))
                    HasWonTrenchRun = true;

                if (json.Contains("\"level2_won\":true") || json.Contains("\"level2_won\": true"))
                    HasWonLevel2 = true;

                // Parse cumulative_score
                int csIdx = json.IndexOf("\"cumulative_score\"");
                if (csIdx >= 0)
                {
                    int colonIdx = json.IndexOf(':', csIdx);
                    if (colonIdx >= 0)
                    {
                        int start = colonIdx + 1;
                        // Find the end of the number (next comma, brace, or end)
                        int end = start;
                        while (end < json.Length && json[end] != ',' && json[end] != '}')
                            end++;
                        string numStr = json.Substring(start, end - start).Trim();
                        int.TryParse(numStr, out int parsed);
                        CumulativeScore = parsed;
                    }
                }

                // Parse shop state
                if (json.Contains("\"skin1_purchased\":true") || json.Contains("\"skin1_purchased\": true"))
                    Skin1Purchased = true;
                if (json.Contains("\"skin2_purchased\":true") || json.Contains("\"skin2_purchased\": true"))
                    Skin2Purchased = true;
                if (json.Contains("\"skin3_purchased\":true") || json.Contains("\"skin3_purchased\": true"))
                    Skin3Purchased = true;

                int esIdx = json.IndexOf("\"equipped_skin\"");
                if (esIdx >= 0)
                {
                    int eColonIdx = json.IndexOf(':', esIdx);
                    if (eColonIdx >= 0)
                    {
                        int eStart = eColonIdx + 1;
                        int eEnd = eStart;
                        while (eEnd < json.Length && json[eEnd] != ',' && json[eEnd] != '}')
                            eEnd++;
                        string esStr = json.Substring(eStart, eEnd - eStart).Trim();
                        int.TryParse(esStr, out int esParsed);
                        EquippedSkin = esParsed;
                    }
                }

                // Parse byte_chips
                int bcIdx = json.IndexOf("\"byte_chips\"");
                if (bcIdx >= 0)
                {
                    int bcColonIdx = json.IndexOf(':', bcIdx);
                    if (bcColonIdx >= 0)
                    {
                        int bcStart = bcColonIdx + 1;
                        int bcEnd = bcStart;
                        while (bcEnd < json.Length && json[bcEnd] != ',' && json[bcEnd] != '}')
                            bcEnd++;
                        string bcStr = json.Substring(bcStart, bcEnd - bcStart).Trim();
                        int.TryParse(bcStr, out int bcParsed);
                        ByteChips = bcParsed;
                    }
                }

                // Parse byte pack purchases
                if (json.Contains("\"byte_pack1_bought\":true") || json.Contains("\"byte_pack1_bought\": true"))
                    BytePack1Bought = true;
                if (json.Contains("\"byte_pack2_bought\":true") || json.Contains("\"byte_pack2_bought\": true"))
                    BytePack2Bought = true;
                if (json.Contains("\"byte_pack3_bought\":true") || json.Contains("\"byte_pack3_bought\": true"))
                    BytePack3Bought = true;
                if (json.Contains("\"byte_pack4_bought\":true") || json.Contains("\"byte_pack4_bought\": true"))
                    BytePack4Bought = true;

                LogMessage("[ProgressTracker] HasWonTrenchRun=" + HasWonTrenchRun + " HasWonLevel2=" + HasWonLevel2 + " CumulativeScore=" + CumulativeScore + " ByteChips=" + ByteChips + " EquippedSkin=" + EquippedSkin);
            }
            catch (Exception e)
            {
                LogError("[ProgressTracker] LoadProgress error: " + e.Message);
            }
        }

        /// <summary>
        /// Marks a level as won and saves to disk.
        /// </summary>
        public static void MarkLevelWon(string levelName)
        {
            LogMessage("[ProgressTracker] Marking level won: " + levelName);

            // Load existing progress first so we don't overwrite other level wins
            LoadProgress();

            if (levelName == "trench_run")
                HasWonTrenchRun = true;
            else if (levelName == "level2")
                HasWonLevel2 = true;
            else
                LogMessage("[ProgressTracker] Unknown level name: " + levelName);

            SaveProgress();
        }

        /// <summary>
        /// Resets cumulative score to 0 and saves to disk.
        /// </summary>
        public static void ResetCumulativeScore()
        {
            LoadProgress();
            CumulativeScore = 0;
            LogMessage("[ProgressTracker] Cumulative score reset to 0");
            SaveProgress();
        }

        /// <summary>
        /// Resets all progress (levels, scores, shop, bytechips) but not high scores.
        /// </summary>
        public static void ResetAllProgress()
        {
            HasWonTrenchRun = false;
            HasWonLevel2 = false;
            CumulativeScore = 0;
            Skin1Purchased = false;
            Skin2Purchased = false;
            Skin3Purchased = false;
            EquippedSkin = 0;
            ByteChips = 0;
            BytePack1Bought = false;
            BytePack2Bought = false;
            BytePack3Bought = false;
            BytePack4Bought = false;
            LogMessage("[ProgressTracker] All progress reset");
            SaveProgress();
            Publish("ProgressReset", "");
        }

        /// <summary>
        /// Adds points to the cumulative score and saves to disk.
        /// </summary>
        public static void AddCumulativeScore(int amount)
        {
            LoadProgress();
            CumulativeScore += amount;
            LogMessage("[ProgressTracker] Added " + amount + " to cumulative score. Total: " + CumulativeScore);
            SaveProgress();
        }

        public static void SaveProgress()
        {
            try
            {
                string json = "{\n" +
                    "  \"trench_run_won\": " + (HasWonTrenchRun ? "true" : "false") + ",\n" +
                    "  \"level2_won\": " + (HasWonLevel2 ? "true" : "false") + ",\n" +
                    "  \"cumulative_score\": " + CumulativeScore + ",\n" +
                    "  \"skin1_purchased\": " + (Skin1Purchased ? "true" : "false") + ",\n" +
                    "  \"skin2_purchased\": " + (Skin2Purchased ? "true" : "false") + ",\n" +
                    "  \"skin3_purchased\": " + (Skin3Purchased ? "true" : "false") + ",\n" +
                    "  \"equipped_skin\": " + EquippedSkin + ",\n" +
                    "  \"byte_chips\": " + ByteChips + ",\n" +
                    "  \"byte_pack1_bought\": " + (BytePack1Bought ? "true" : "false") + ",\n" +
                    "  \"byte_pack2_bought\": " + (BytePack2Bought ? "true" : "false") + ",\n" +
                    "  \"byte_pack3_bought\": " + (BytePack3Bought ? "true" : "false") + ",\n" +
                    "  \"byte_pack4_bought\": " + (BytePack4Bought ? "true" : "false") + "\n" +
                    "}";

                if (FileIO.WriteAllText(SAVE_FILE, json))
                {
                    LogMessage("[ProgressTracker] Progress saved successfully");
                }
                else
                {
                    LogError("[ProgressTracker] Failed to write progress file");
                }
            }
            catch (Exception e)
            {
                LogError("[ProgressTracker] SaveProgress error: " + e.Message);
            }
        }
    }
}

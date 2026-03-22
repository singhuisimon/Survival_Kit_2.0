using System;
using static Engine.Logger;
using static Engine.Event;

namespace Engine
{
    /// <summary>
    /// Tracks which levels the player has won. Persists across sessions via progress.json.
    /// </summary>
    public static class ProgressTracker
    {
        private const string SAVEFILE = "ResourcesSources/SaveData/progress.json";

        public static bool HasWonTrenchRun { get; private set; } = false;
        public static bool HasWonLevel2 { get; private set; } = false;
        public static bool HasWonLevel3 { get; private set; } = false;

        public static int CumulativeScore { get; set; } = 0;

        // Shop state - skin purchases and equipped skin (0 = default, 1 = skin 1, 2 = skin 2)
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

        /// <summary>Loads progress from disk. Call on main menu load.</summary>
        public static void LoadProgress()
        {
            HasWonTrenchRun = false;
            HasWonLevel2 = false;
            HasWonLevel3 = false;
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
                if (!FileIO.FileExists(SAVEFILE))
                {
                    LogMessage("ProgressTracker No progress file found, starting fresh");
                    return;
                }

                string json = FileIO.ReadAllText(SAVEFILE);
                LogMessage("ProgressTracker Loaded progress: " + json);

                // Parse simple JSON
                if (json.Contains("trenchrunwon:true") || json.Contains("trenchrunwon: true"))
                    HasWonTrenchRun = true;
                if (json.Contains("level2won:true") || json.Contains("level2won: true"))
                    HasWonLevel2 = true;
                if (json.Contains("level3won:true") || json.Contains("level3won: true"))
                    HasWonLevel3 = true;

                // Parse cumulativescore
                int csIdx = json.IndexOf("cumulativescore");
                if (csIdx >= 0)
                {
                    int colonIdx = json.IndexOf(':', csIdx);
                    if (colonIdx >= 0)
                    {
                        int start = colonIdx + 1;
                        int end = start;
                        while (end < json.Length && json[end] != ',' && json[end] != '}') end++;
                        string numStr = json.Substring(start, end - start).Trim();
                        if (int.TryParse(numStr, out int parsed)) CumulativeScore = parsed;
                    }
                }

                // Parse shop state
                if (json.Contains("skin1purchased:true") || json.Contains("skin1purchased: true")) Skin1Purchased = true;
                if (json.Contains("skin2purchased:true") || json.Contains("skin2purchased: true")) Skin2Purchased = true;
                if (json.Contains("skin3purchased:true") || json.Contains("skin3purchased: true")) Skin3Purchased = true;

                int esIdx = json.IndexOf("equippedskin");
                if (esIdx >= 0)
                {
                    int eColonIdx = json.IndexOf(':', esIdx);
                    if (eColonIdx >= 0)
                    {
                        int eStart = eColonIdx + 1;
                        int eEnd = eStart;
                        while (eEnd < json.Length && json[eEnd] != ',' && json[eEnd] != '}') eEnd++;
                        string esStr = json.Substring(eStart, eEnd - eStart).Trim();
                        if (int.TryParse(esStr, out int esParsed)) EquippedSkin = esParsed;
                    }
                }

                // Parse bytechips
                int bcIdx = json.IndexOf("bytechips");
                if (bcIdx >= 0)
                {
                    int bcColonIdx = json.IndexOf(':', bcIdx);
                    if (bcColonIdx >= 0)
                    {
                        int bcStart = bcColonIdx + 1;
                        int bcEnd = bcStart;
                        while (bcEnd < json.Length && json[bcEnd] != ',' && json[bcEnd] != '}') bcEnd++;
                        string bcStr = json.Substring(bcStart, bcEnd - bcStart).Trim();
                        if (int.TryParse(bcStr, out int bcParsed)) ByteChips = bcParsed;
                    }
                }

                // Parse byte pack first-purchase flags
                if (json.Contains("bytepack1bought:true") || json.Contains("bytepack1bought: true")) BytePack1Bought = true;
                if (json.Contains("bytepack2bought:true") || json.Contains("bytepack2bought: true")) BytePack2Bought = true;
                if (json.Contains("bytepack3bought:true") || json.Contains("bytepack3bought: true")) BytePack3Bought = true;
                if (json.Contains("bytepack4bought:true") || json.Contains("bytepack4bought: true")) BytePack4Bought = true;

                LogMessage("ProgressTracker HasWonTrenchRun: " + HasWonTrenchRun +
                           " HasWonLevel2: " + HasWonLevel2 +
                           " HasWonLevel3: " + HasWonLevel3 +
                           " CumulativeScore: " + CumulativeScore +
                           " ByteChips: " + ByteChips +
                           " EquippedSkin: " + EquippedSkin);
            }
            catch (Exception e)
            {
                LogError("ProgressTracker LoadProgress error: " + e.Message);
            }
        }

        /// <summary>Marks a level as won and saves to disk.</summary>
        public static void MarkLevelWon(string levelName)
        {
            LogMessage("ProgressTracker Marking level won: " + levelName);
            LoadProgress(); // Load existing progress first so we don't overwrite other level wins

            if (levelName == "trenchrun") HasWonTrenchRun = true;
            else if (levelName == "level2") HasWonLevel2 = true;
            else if (levelName == "level3") HasWonLevel3 = true;
            else LogMessage("ProgressTracker Unknown level name: " + levelName);

            SaveProgress();
        }

        /// <summary>Resets cumulative score to 0 and saves to disk.</summary>
        public static void ResetCumulativeScore()
        {
            LoadProgress();
            CumulativeScore = 0;
            LogMessage("ProgressTracker Cumulative score reset to 0");
            SaveProgress();
        }

        /// <summary>Resets all progress (levels, scores, shop, bytechips) but not high scores.</summary>
        public static void ResetAllProgress()
        {
            HasWonTrenchRun = false;
            HasWonLevel2 = false;
            HasWonLevel3 = false;
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

            LogMessage("ProgressTracker All progress reset");
            SaveProgress();
            Event.Publish("ProgressReset", "");

        }

        /// <summary>Adds points to the cumulative score and saves to disk.</summary>
        public static void AddCumulativeScore(int amount)
        {
            LoadProgress();
            CumulativeScore += amount;
            LogMessage("ProgressTracker Added " + amount + " to cumulative score. Total: " + CumulativeScore);
            SaveProgress();
        }

        public static void SaveProgress()
        {
            try
            {
                string json =
                    "{" +
                    "\"trenchrunwon\":" + (HasWonTrenchRun ? "true" : "false") + "," +
                    "\"level2won\":" + (HasWonLevel2 ? "true" : "false") + "," +
                    "\"level3won\":" + (HasWonLevel3 ? "true" : "false") + "," +
                    "\"cumulativescore\":" + CumulativeScore + "," +
                    "\"skin1purchased\":" + (Skin1Purchased ? "true" : "false") + "," +
                    "\"skin2purchased\":" + (Skin2Purchased ? "true" : "false") + "," +
                    "\"skin3purchased\":" + (Skin3Purchased ? "true" : "false") + "," +
                    "\"equippedskin\":" + EquippedSkin + "," +
                    "\"bytechips\":" + ByteChips + "," +
                    "\"bytepack1bought\":" + (BytePack1Bought ? "true" : "false") + "," +
                    "\"bytepack2bought\":" + (BytePack2Bought ? "true" : "false") + "," +
                    "\"bytepack3bought\":" + (BytePack3Bought ? "true" : "false") + "," +
                    "\"bytepack4bought\":" + (BytePack4Bought ? "true" : "false") +
                    "}";

                if (FileIO.WriteAllText(SAVEFILE, json))
                    LogMessage("ProgressTracker Progress saved successfully");
                else
                    LogError("ProgressTracker Failed to write progress file");
            }
            catch (Exception e)
            {
                LogError("ProgressTracker SaveProgress error: " + e.Message);
            }
        }
    }
}

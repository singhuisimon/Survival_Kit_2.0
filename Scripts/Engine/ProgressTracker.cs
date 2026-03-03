using System;
using static Engine.Logger;

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

        /// <summary>
        /// Loads progress from disk. Call on main menu load.
        /// </summary>
        public static void LoadProgress()
        {
            HasWonTrenchRun = false;
            HasWonLevel2 = false;

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

                LogMessage("[ProgressTracker] HasWonTrenchRun=" + HasWonTrenchRun + " HasWonLevel2=" + HasWonLevel2);
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

            if (levelName == "trench_run")
                HasWonTrenchRun = true;
            else if (levelName == "level2")
                HasWonLevel2 = true;
            else
                LogMessage("[ProgressTracker] Unknown level name: " + levelName);

            SaveProgress();
        }

        private static void SaveProgress()
        {
            try
            {
                string json = "{\n" +
                    "  \"trench_run_won\": " + (HasWonTrenchRun ? "true" : "false") + ",\n" +
                    "  \"level2_won\": " + (HasWonLevel2 ? "true" : "false") + "\n" +
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

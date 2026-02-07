namespace Engine
{
    /// <summary>
    /// Global game state that can be checked by any script.
    /// Used for pause functionality.
    /// </summary>
    public static class GameState
    {
        /// <summary>
        /// When true, gameplay should be paused.
        /// Scripts should check this in OnUpdate and return early if paused.
        /// </summary>
        public static bool IsPaused { get; set; } = false;

        /// <summary>
        /// The scene path currently loaded. Used by PauseMenuPopup for restart.
        /// </summary>
        public static string CurrentScenePath { get; set; } = "Resources/Sources/Scenes/Level1_NewPlayer.json";
    }
}

using Engine;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Input;

namespace Game
{
    /// <summary>
    /// Debug cheats for testing:
    ///   Comma (,)      = Kill player instantly
    ///   Period (.)     = Kill enemy core (win level)
    ///   Slash (/)      = Set timer to 10 seconds
    ///   Semicolon (;)  = Add 10,000 research points
    ///   Apostrophe (') = Reset all progress (not high scores)
    /// Attach this script to any entity in the gameplay scene.
    /// </summary>
    public class DebugCheats : ScriptBehaviour
    {
        private bool wasCommaPressed = false;
        private bool wasPeriodPressed = false;
        private bool wasSlashPressed = false;
        private bool wasSemicolonPressed = false;
        private bool wasApostrophePressed = false;

        public override void OnStart()
        {
            LogMessage("[DebugCheats] Debug cheats active!");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Comma = Kill player
            bool commaPressed = IsKeyPressed(KeyCode.Comma);
            if (commaPressed && !wasCommaPressed)
            {
                LogMessage("[DebugCheats] Comma pressed - killing player");
                Publish("PlayerDead", "");
            }
            wasCommaPressed = commaPressed;

            // Period = Kill enemy core
            bool periodPressed = IsKeyPressed(KeyCode.Period);
            if (periodPressed && !wasPeriodPressed)
            {
                LogMessage("[DebugCheats] Period pressed - killing enemy core");
                Publish("EnemyCoreDeath", "");
            }
            wasPeriodPressed = periodPressed;

            // Slash = Set timer to 10 seconds
            bool slashPressed = IsKeyPressed(KeyCode.Slash);
            if (slashPressed && !wasSlashPressed)
            {
                LogMessage("[DebugCheats] Slash pressed - setting timer to 10 seconds");
                Publish("DebugSetTimer", "10");
            }
            wasSlashPressed = slashPressed;

            // Semicolon = Add 10,000 research points
            bool semiPressed = IsKeyPressed(KeyCode.Semicolon);
            if (semiPressed && !wasSemicolonPressed)
            {
                ProgressTracker.AddCumulativeScore(10000);
                LogMessage("[DebugCheats] Semicolon pressed - added 10,000 research points. Total: " + ProgressTracker.CumulativeScore);
            }
            wasSemicolonPressed = semiPressed;

            // Apostrophe = Reset all progress
            bool apostrophePressed = IsKeyPressed(KeyCode.Apostrophe);
            if (apostrophePressed && !wasApostrophePressed)
            {
                ProgressTracker.ResetAllProgress();
                LogMessage("[DebugCheats] Apostrophe pressed - all progress reset");
            }
            wasApostrophePressed = apostrophePressed;
        }

        public override void OnDestroy()
        {
            LogMessage("[DebugCheats] Destroyed");
        }
    }
}

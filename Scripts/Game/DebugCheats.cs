using Engine;
using static Engine.Logger;
using static Engine.Event;
using static Engine.Input;
using System.Security.Cryptography;

namespace Game
{
    /// <summary>
    /// Debug cheats for testing:
    /// 
    /// EXISTING
    ///   Comma (,)      = Kill player instantly
    ///   Period (.)     = Kill enemy core (win level)
    ///   Slash (/)      = Set timer to 10 seconds
    ///   Semicolon (;)  = Add 10,000 research points (in ShopPopup.cs)
    ///   Apostrophe (') = Reset all progress (not high scores)
    /// 
    /// NEW
    ///    Alt + H             = Add +10 Player health (publishes "PlayerHeal")
    ///    Alt + J             = Deal -10 Player damage (publishes "PlayerDamage")
    ///    Alt + U             = Add 1 upgrade module payload (publishes "CollectPayload")
    ///    Alt + G             = Fully recharge alt fire (publishes "GainUlt" x30)
    ///    Alt + Z             = Go to Main Menu (loads MainMenu.json)
    ///    Alt + X             = Go to Level 1 (loads trench_run.json)
    ///    Alt + V             = Go to Level 2 (name.json)
    ///    Alt + C             = Go to Level 3  (loads level2.json)
    ///    
    /// Note: cheatcode to move to level2 is moved from PlayerWeapon.cs, was T key previously
    /// 
    /// Attach this script to any entity in the gameplay scene.
    /// </summary>
    public class DebugCheats : ScriptBehaviour
    {

        //Scene paths
         private const string SCENE_MAIN_MENU = "Resources/Sources/Scenes/MainMenu.json";
        private const string SCENE_LEVEL1    = "Resources/Sources/Scenes/trench_run.json";
        private const string SCENE_LEVEL2    = "Resources/Sources/Scenes/level2_graphic_card.json";
        private const string SCENE_LEVEL3 = "Resources/Sources/Scenes/level3_motherboard.json";

        // ===== Event Names =====
        private const string EVENT_PLAYER_DEAD    = "PlayerDead";
        private const string EVENT_ENEMY_CORE     = "EnemyCoreDeath";
        private const string EVENT_DEBUG_TIMER    = "DebugSetTimer";
        private const string EVENT_PLAYER_HEAL    = "PlayerHeal";
        private const string EVENT_PLAYER_DAMAGE  = "PlayerDamage";
        private const string EVENT_COLLECT_PAYLOAD = "CollectPayload";
        private const string EVENT_GAIN_ULT       = "GainUlt";


        //alt fire seting
        private const int ALT_FIRE_MAX_CHARGE = 30;

        private bool wasCommaPressed = false;
        private bool wasPeriodPressed = false;
        private bool wasSlashPressed = false;
        private bool wasApostrophePressed = false;

        

        private bool wasHPressed = false;
        private bool wasJPressed = false;
        private bool wasUPressed = false;
        private bool wasGPressed = false;
        private bool wasZPressed = false;
        private bool wasXPressed = false;
        private bool wasCPressed = false;

        private bool wasVPressed = false;

        public override void OnStart()
        {
            LogMessage("[DebugCheats] Debug cheats active!");
            LogMessage("[DebugCheats]  ,  = Kill player");
            LogMessage("[DebugCheats]  .  = Kill enemy core");
            LogMessage("[DebugCheats]  /  = Set timer to 10s");
            LogMessage("[DebugCheats]  ;  = +10,000 research points");
            LogMessage("[DebugCheats]  '  = Reset all progress");
            LogMessage("[DebugCheats]  H  = +10 player health");
            LogMessage("[DebugCheats]  J  = -10 player health (damage)");
            LogMessage("[DebugCheats]  U  = Add 1 upgrade module");
            LogMessage("[DebugCheats]  G  = Fully recharge alt fire");
            LogMessage("[DebugCheats]  Z  = Go to Main Menu");
            LogMessage("[DebugCheats]  X  = Go to Level 1");
            LogMessage("[DebugCheats]  V  = Go to Level 2");
            LogMessage("[DebugCheats]  C  = Go to Level 3");
        }

        public override void OnUpdate(float deltaTime)
        {

           
            bool altHeld = IsKeyPressed(KeyCode.LeftAlt) || IsKeyPressed(KeyCode.RightAlt);

            // Comma = Kill player
            bool commaPressed = IsKeyPressed(KeyCode.Comma);
            if (altHeld && commaPressed && !wasCommaPressed)
            {
                LogMessage("[DebugCheats] Comma pressed - killing player");
                Publish("PlayerDead", "");
            }
            wasCommaPressed = commaPressed;

            // Period = Kill enemy core
            bool periodPressed = IsKeyPressed(KeyCode.Period);
            if (altHeld && periodPressed && !wasPeriodPressed)
            {
                LogMessage("[DebugCheats] Period pressed - killing enemy core");
                Publish("EnemyCoreDeath", "");
            }
            wasPeriodPressed = periodPressed;

            // Slash = Set timer to 10 seconds
            bool slashPressed = IsKeyPressed(KeyCode.Slash);
            if (altHeld && slashPressed && !wasSlashPressed)
            {
                LogMessage("[DebugCheats] Slash pressed - setting timer to 10 seconds");
                Publish("DebugSetTimer", "10");
            }
            wasSlashPressed = slashPressed;

            // Apostrophe = Reset all progress
            bool apostrophePressed = IsKeyPressed(KeyCode.Apostrophe);
            if (altHeld && apostrophePressed && !wasApostrophePressed)
            {
                ProgressTracker.ResetAllProgress();
                LogMessage("[DebugCheats] Apostrophe pressed - all progress reset");
            }
            wasApostrophePressed = apostrophePressed;


        // H = Add +10 player health
            bool hPressed = IsKeyPressed(KeyCode.H);
            if (altHeld && hPressed && !wasHPressed)
            {
                Publish(EVENT_PLAYER_HEAL, "10");
                LogMessage("[DebugCheats] H - PlayerHeal +10 sent to SpaceshipController");
            }
            wasHPressed = hPressed;
 
            // J = Deal -10 damage to player
            bool jPressed = IsKeyPressed(KeyCode.J);
            if (altHeld && jPressed && !wasJPressed)
            {
                Publish(EVENT_PLAYER_DAMAGE, "10");
                LogMessage("[DebugCheats] J - PlayerDamage -10 sent to SpaceshipController");
            }
            wasJPressed = jPressed;
 
            // U = Add 1 upgrade module (payload)
            bool uPressed = IsKeyPressed(KeyCode.U);
            if (altHeld && uPressed && !wasUPressed)
            {
                Publish(EVENT_COLLECT_PAYLOAD, "");
                LogMessage("[DebugCheats] U - added 1 upgrade module (CollectPayload)");
            }
            wasUPressed = uPressed;
 
            // G = Fully recharge alt fire (publish GainUlt enough times to fill bar)
            bool gPressed = IsKeyPressed(KeyCode.G);
            if (altHeld && gPressed && !wasGPressed)
            {
                for (int i = 0; i < ALT_FIRE_MAX_CHARGE; i++)
                {
                    Publish(EVENT_GAIN_ULT, "1");
                }
                LogMessage("[DebugCheats] G - alt fire fully recharged (published GainUlt x" + ALT_FIRE_MAX_CHARGE + ")");
            }
            wasGPressed = gPressed;



            // Z = Go to Main Menu
            bool zPressed = IsKeyPressed(KeyCode.Z);
            if (altHeld && zPressed && !wasZPressed)
            {
                LogMessage("[DebugCheats] Z - loading Main Menu");
                GameState.IsPaused = false;
                Input.SetCursorVisible(true);
                Scene.SceneLoadFromFile(SCENE_MAIN_MENU);
            }
            wasZPressed = zPressed;
 
            // X = Go to Level 1
            bool xPressed = IsKeyPressed(KeyCode.X);
            if (altHeld && xPressed && !wasXPressed)
            {
                LogMessage("[DebugCheats] X - loading Level 1");
                GameState.IsPaused = false;
                Input.SetCursorVisible(false);
                Scene.SceneLoadFromFile(SCENE_LEVEL1);
            }
            wasXPressed = xPressed;

            // V = Go to Level 2
            bool vPressed = IsKeyPressed(KeyCode.V);
            if (altHeld && vPressed && !wasVPressed)
            {
                LogMessage("[DebugCheats] V - loading level 2");
                GameState.IsPaused = false;
                Input.SetCursorVisible(false);
                Scene.SceneLoadFromFile(SCENE_LEVEL2);
            }
            wasVPressed = vPressed;

            // C = Go to Level 3 (moved from PlayerWeapon.cs, was T key)
            bool cPressed = IsKeyPressed(KeyCode.C);
            if (altHeld && cPressed && !wasCPressed)
            {
                LogMessage("[DebugCheats] C - loading Level 3");
                GameState.IsPaused = false;
                Input.SetCursorVisible(false);
                Scene.SceneLoadFromFile(SCENE_LEVEL3);
            }
            wasCPressed = cPressed;


        }

        public override void OnDestroy()
        {
            LogMessage("[DebugCheats] Destroyed");
        }
    }
}

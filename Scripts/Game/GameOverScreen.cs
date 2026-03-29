using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;
using static Engine.Scene;
using static Engine.Audio;
using static Engine.AudioManager;
using static Engine.Transform;
using static Engine.Prefab;
using System.Security.Cryptography;

namespace Game
{
    /// <summary>
    /// GameOverScreen - Shows lose screen when player or core dies
    /// Attach this to the lose screen texture entity
    /// Listens to "PlayerDead" and "CoreMotherboardDestroyed" events
    /// </summary>
    public class GameOverScreen : ScriptBehaviour
    {
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string GAMEOVER = "GameOver";
        private const string PlayerDeadName = "PlayerDeath";
        private const string CoreDestructionName = "CoreDestruction";
        private const string core = "SEMICONDUCTOR";
        private const string LOSE_SCREEN_SHOW = "LoseScreenShow";
        private const string GameOverVOPrefab = "Sources/Prefabs/Audio_Lose_VO.prefab";
        private const string ScreamVOBasePath = "Lose Screen Yell reverb/Office Worker_Reverb_";

        private const float countdowntimer = 1.5f;
        private const float screamdelay = 0.3f;

        private float screamcountdown = 0.0f;
        private float countdown = 0.0f;

        private bool countdownstart = false;
        private bool voInstantiated = false;
        private bool screamPlayed = false;

        private uint playerdeadID = 0;
        private uint coredestructionID = 0;
        private uint coreID = 0;

        private bool initialized = false;
        private bool playaudio = false;
        private string currEvent = "";

        // Button fade delay
        private bool pendingShow = false;
        private float showDelay = 2.0f;
        private float showDelayTimer = 0.0f;

        // Texture fade
        private bool isFading = false;
        private bool fadeDone = false;
        private float fadeElapsed = 0.0f;

        [SerializeField] private float fadeUpTime = 1.0f;

        private bool gameOverTriggered = false;

        public override void OnStart()
        {
            LogMessage("=== GameOverScreen OnStart ===");
            LogMessage("GameOverScreen EntityID: " + EntityID);

            playerdeadID = SceneFindEntityByName(PlayerDeadName);
            coredestructionID = SceneFindEntityByName(CoreDestructionName);
            coreID = SceneFindEntityByName(core);

            if (playerdeadID == 0) LogMessage("[GameOverScreen] playerdead entity cannot be found");
            if (coredestructionID == 0) LogMessage("[GameOverScreen] coredestruction entity cannot be found");
            if (coreID == 0) LogMessage("[GameOverScreen] core entity cannot be found");

            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);

            SetIsVisible((uint)EntityID, false);

            initialized = true;
            screamcountdown = screamdelay;
            LogMessage("[GameOverScreen] Initialized - waiting for lose condition");

            Vector3 corepos = GetPosition(coreID);
            LogMessage("SEMICONDUCTOR IS AT x: " + corepos.X.ToString() + ", y: " + corepos.Y.ToString() + ", z: " + corepos.Z.ToString());
        }

        public override void OnUpdate(float deltaTime)
        {
            // Fade in the lose screen texture (alpha only, no position movement)
            if (isFading && !fadeDone)
            {
                fadeElapsed += deltaTime;

                FadeIn((uint)EntityID, fadeElapsed, fadeUpTime);

                if (fadeElapsed >= fadeUpTime)
                {
                    fadeDone = true;
                    isFading = false;
                    LogMessage("[GameOverScreen] Texture fade complete");
                }
            }

            // Play audio
            if (playaudio)
            {
                if (currEvent == EVENT_PLAYER_DEAD)
                {
                    AudioPlay(playerdeadID);
                    LogMessage("[GameOverScreen] player dead audio is playing");
                }
                if (currEvent == EVENT_CORE_DESTROYED)
                {
                    AudioPlay(coredestructionID);
                    LogMessage("[GameOverScreen] core destruction audio is playing");
                }
                playaudio = false;
                StopGroup(AudioType.MASTER);
                countdownstart = true;
                countdown = countdowntimer;
                screamcountdown = screamdelay;
            }

            // VO countdown
            if (countdownstart)
            {
                if (!screamPlayed)
                {
                    screamcountdown -= deltaTime;
                    if(screamcountdown <= 0.0f && !screamPlayed)
                    {
                        RandomizeAudioPath();
                        AudioPlay((uint)EntityID);
                        screamPlayed = true;
                    }
                }
                countdown -= deltaTime;
                if (countdown <= 0.0f && !voInstantiated)
                {
                    uint loseVOID = PrefabInstantiate(GameOverVOPrefab);
                    if (loseVOID == 0)
                    {
                        LogMessage("[GameOverScreen] failure to spawn gameover VO");
                        return;
                    }
                    voInstantiated = true;
                }
            }

            // Wait 2 seconds after texture appears, then fade in buttons
            if (pendingShow)
            {
                showDelayTimer -= deltaTime;
                if (showDelayTimer <= 0.0f)
                {
                    pendingShow = false;
                    LogMessage("[GameOverScreen] Delay done - publishing LoseScreenShow");
                    Publish(LOSE_SCREEN_SHOW, "");
                }
            }
        }

        private void OnGameOver(string eventName, string payload)
        {

            if (gameOverTriggered) return;
            gameOverTriggered = true;
            
            LogMessage("[GameOverScreen] Game Over triggered by: " + eventName);

            //StopGroup(AudioType.MASTER);
            //countdownstart = true;
            //countdown = countdowntimer;

            // Reset alpha to 0 before fading in
            SpriteRenderer.SetColor((uint)EntityID, 1.0f, 1.0f, 1.0f, 0.0f);

            // Start fading in texture
            isFading = true;
            fadeElapsed = 0.0f;
            fadeDone = false;
            SetIsVisible((uint)EntityID, true);

            Input.SetCursorVisible(true);
            currEvent = eventName;
            playaudio = true;
            Publish(GAMEOVER, "");

            // Start 2 second delay before fading in buttons
            pendingShow = true;
            showDelayTimer = showDelay;
        }

        private void RandomizeAudioPath()
        {
            int randomint = RNG.RandInt(1,5);
            string filepath = ScreamVOBasePath + randomint.ToString() + ".wav";
            AudioSetFile((uint)EntityID, filepath);
            LogMessage("[GameOverScreen] Setting audio filepath to be: " + filepath);
        }

        public override void OnDestroy()
        {
            AudioStop((uint)EntityID);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            LogMessage("=== GameOverScreen Destroyed ===");
        }
    }
}

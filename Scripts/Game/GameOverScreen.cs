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

        private const string GameOverVOPrefab = "Sources/Prefabs/Audio_Lose_VO.prefab";

        private const float countdowntimer = 0.5f;
        private float countdown = 0.0f;

        private bool countdownstart = false;
        private bool voInstantiated = false;

        private uint playerdeadID = 0;
        private uint coredestructionID = 0;
        private uint coreID = 0;

        private bool initialized = false;
        private bool playaudio = false;

        private string currEvent = "";

        public override void OnStart()
        {
            LogMessage("=== GameOverScreen OnStart ===");
            LogMessage("GameOverScreen EntityID: " + EntityID);

            playerdeadID = SceneFindEntityByName(PlayerDeadName);
            coredestructionID = SceneFindEntityByName(CoreDestructionName);
            coreID = SceneFindEntityByName(core);

            if (playerdeadID == 0){
                LogMessage("[GameOverScreen] playerdead entity cannot be found");
            }

            if (coredestructionID == 0)
            {
                LogMessage("[GameOverScreen] coredestruction entity cannot be found");
            }
            if (coreID == 0)
            {
                LogMessage("[GameOverScreen] core entity cannot be found");
            }

            // Subscribe to both lose conditions
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);

            // Start hidden
            SetIsVisible((uint)EntityID, false);

            initialized = true;
            LogMessage("[GameOverScreen] Initialized - waiting for lose condition");

            Vector3 corepos = GetPosition(coreID);
            LogMessage("SEMICONDUCTOR IS AT x: " + corepos.X.ToString() + ", y: " + corepos.Y.ToString() + ", z: " + corepos.Z.ToString());
        }

        public override void OnUpdate(float deltaTime){
            if(playaudio){
                if(currEvent == EVENT_PLAYER_DEAD){
                    AudioPlay(playerdeadID);
                    LogMessage("[GameOverScreen] player dead audio is playing");
                }

                if(currEvent == EVENT_CORE_DESTROYED){
                    AudioPlay(coredestructionID);
                    LogMessage("[GameOverScreen] core destruction audio is playing");
                }

                playaudio = false;
            }

            if(countdownstart){
                countdown -= deltaTime;
                if(countdown <= 0.0f && !voInstantiated){
                    uint loseVOID = 0;
                    loseVOID = PrefabInstantiate(GameOverVOPrefab);
                    if(loseVOID == 0){
                        LogMessage("[GameOverScreen] failure to spawn gameover VO");
                        return;
                    }
                    voInstantiated = true;
                }
            }

        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[GameOverScreen] Game Over triggered by: " + eventName);

            StopGroup(AudioType.MASTER);
            countdownstart = true;
            countdown = countdowntimer;

            SetIsVisible((uint)EntityID, true);
            Input.SetCursorVisible(true);
            currEvent = eventName;
            playaudio = true;
            Publish(GAMEOVER, "");
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
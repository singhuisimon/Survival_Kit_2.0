using Engine;
using System;
using static Engine.Scene;
using static Engine.Audio;
using static Engine.Event;

namespace Game
{
    public class GameDelaySoundPrefab : ScriptBehaviour
    {
        [SerializeField]
        public float Lifetime = 5.0f;

        [SerializeField]
        public float delayedTime = 0.3f;

        //for debug purpose
        [SerializeField]
        private float elapsedTime = 0.0f;

        private float savedTime = 0.0f;
        private bool wasPaused = false;
        private bool wasPlayingBeforePause = false;

        private bool played = false;

        // Game lose / win condition
        private const string GAMEOVER = "GameOver";
        private const string GAMEWIN = "GameWin";

        public override void OnStart()
        {
            Subscribe(GAMEOVER, OnGameOver);
            Subscribe(GAMEWIN, OnGameOver);
        }

        public override void OnUpdate(float deltaTime)
        {

            // Handle pause - pause timer
            if (GameState.IsPaused)
            {
                if(!wasPaused){
                    savedTime = elapsedTime;
                    wasPaused = true;
                    wasPlayingBeforePause = AudioIsPlaying((uint)EntityID);
                    if(wasPlayingBeforePause){
                        AudioPause((uint)EntityID);
                    }
                }
                return;
            }
            else if (wasPaused)
            {
                // Just unpaused - restore timer
                elapsedTime = savedTime;
                wasPaused = false;
                if(played && wasPlayingBeforePause){
                    AudioPlay((uint)EntityID);
                }
            }


            // Lifetime
            elapsedTime += deltaTime;

            if(elapsedTime >= delayedTime){
                if(!played){
                    AudioPlay((uint)EntityID);
                    played = true;
                }
            }

            if (elapsedTime >= Lifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(GAMEOVER, OnGameOver);
            Unsubscribe(GAMEWIN, OnGameOver);
        }

        private void OnGameOver(string eventName, string payload)
        {
            SceneDestroyEntity(EntityID);
        }
    }
}

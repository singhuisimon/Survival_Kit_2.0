using Engine;
using System;
using static Engine.Scene;
using static Engine.Audio;

namespace Game
{
    public class GameSoundPrefabLongerLifetime : ScriptBehaviour
    {
        [SerializeField]
        public float Lifetime = 8.0f;

        //for debug purpose
        [SerializeField]
        private float elapsedTime = 0.0f;

        private float savedTime = 0.0f;
        private bool wasPaused = false;
        private bool wasPlayingBeforePause = false;

        public override void OnStart()
        {
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
                if(wasPlayingBeforePause){
                    AudioPlay((uint)EntityID);
                }
            }

            // Lifetime
            elapsedTime += deltaTime;
            if (elapsedTime >= Lifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }
        }

        public override void OnDestroy()
        {
            AudioStop((uint)EntityID);
        }
    }
}

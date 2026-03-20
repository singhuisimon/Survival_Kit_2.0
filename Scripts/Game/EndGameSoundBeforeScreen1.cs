using Engine;
using System;
using static Engine.Scene;
using static Engine.Audio;
using static Engine.Event;

namespace Game
{
    public class EndGameSoundBeforeScreen1 : ScriptBehaviour
    {
        [SerializeField]
        public float Lifetime = 3.0f;

        //for debug purpose
        [SerializeField]
        private float elapsedTime = 0.0f;

        private float savedTime = 0.0f;
        private float publishTime = 0.0f;
        private bool wasPaused = false;

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
                    if(AudioIsPlaying((uint)EntityID)){
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
                AudioPlay((uint)EntityID);
            }


            // Lifetime
            elapsedTime += deltaTime;
            publishTime += deltaTime;
            if(publishTime >= 1.0f){
                Publish("EndSound1Played", "");
            }
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

using Engine;
using System;
using static Engine.Scene;
using static Engine.Audio;

namespace Game
{
    public class GameDelaySoundPrefab : ScriptBehaviour
    {
        [SerializeField]
        public float Lifetime = 5.0f;

        [SerializeField]
        public float delayedTime = 0.5f;

        //for debug purpose
        [SerializeField]
        private float elapsedTime = 0.0f;

        private float savedTime = 0.0f;
        private bool wasPaused = false;

        private bool played = false;

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
                }
                return;
            }
            else if (wasPaused)
            {
                // Just unpaused - restore timer
                elapsedTime = savedTime;
                wasPaused = false;
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
        }
    }
}

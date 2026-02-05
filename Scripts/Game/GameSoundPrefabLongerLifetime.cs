using Engine;
using System;
using static Engine.Scene;

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

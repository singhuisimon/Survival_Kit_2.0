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

        public override void OnStart()
        {
        }

        public override void OnUpdate(float deltaTime)
        {
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

using Engine;
using System;

namespace Game
{
    public class GameSoundPrefabLifetime : ScriptBehaviour
    {
        [SerializeField]
        public float Lifetime = 2.0f;

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
                InternalCalls.Scene_DestroyEntity((uint)EntityID);
                return;
            }
        }

        public override void OnDestroy()
        {
        }
    }
}

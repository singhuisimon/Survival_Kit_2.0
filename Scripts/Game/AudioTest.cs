using Engine;
using System;

namespace Game
{
    /// <summary>
    /// Example player controller script
    /// Attach this to a entity to play
    /// </summary>
    public class AudioTest : ScriptBehaviour
    {
        public override void OnStart()
        {
            AudioManager.SetGroupVolume(AudioType.SFX, 0.8f);
            Log("Audio API working!");
        }

        public override void OnUpdate(float deltaTime)
        {
            float vol = AudioManager.GetGroupVolume(AudioType.SFX);
            Log("Audio API working! Groupvol is: " + vol.ToString());
        }

        public override void OnDestroy()
        {
            Log("PlayerController destroyed!");
        }
    }
}

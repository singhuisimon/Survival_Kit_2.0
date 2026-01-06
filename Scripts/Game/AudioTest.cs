using Engine;
using System;
using static Engine.Logger;

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
            LogMessage("Audio API working!");
        }

        public override void OnUpdate(float deltaTime)
        {
            float vol = AudioManager.GetGroupVolume(AudioType.SFX);
            LogMessage("Audio API working! Groupvol is: " + vol.ToString());
        }

        public override void OnDestroy()
        {
            LogMessage("PlayerController destroyed!");
        }
    }
}

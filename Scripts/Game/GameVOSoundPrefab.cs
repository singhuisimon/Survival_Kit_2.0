using System;
using Engine;
using static Engine.Audio;
using static Engine.Scene;
using static Engine.Event;

namespace Game
{
    public class GameVOSoundPrefab : ScriptBehaviour
    {
        private const string EVENT_VO_PAUSE_STATE = "GameVOPauseState";

        [SerializeField] private bool hasStartedPlaying = false;
        [SerializeField] private bool isExternallyPaused = false;

        public override void OnStart()
        {
            Subscribe(EVENT_VO_PAUSE_STATE, OnPauseStateChanged);
        }

        public override void OnUpdate(float deltaTime)
        {
            uint entityID = (uint)EntityID;
            bool isPlaying = AudioIsPlaying(entityID);

            // Do not destroy immediately on spawn before audio has actually started once.
            if (!hasStartedPlaying)
            {
                if (isPlaying)
                {
                    hasStartedPlaying = true;
                }
                return;
            }

            // If the audio reports false because it is paused, keep it alive.
            if (!isPlaying && isExternallyPaused)
            {
                return;
            }

            // If it has played before, and is no longer playing, and is not paused,
            // then it has ended or was stopped intentionally.
            if (!isPlaying)
            {
                SceneDestroyEntity(entityID);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_VO_PAUSE_STATE, OnPauseStateChanged);
            AudioStop((uint)EntityID);
        }

        private void OnPauseStateChanged(string eventName, string payload)
        {
            // Expected payload format:
            // "<entityID>|<true/false>"
            // Example: "12345|true"

            if (string.IsNullOrEmpty(payload))
                return;

            string[] parts = payload.Split('|');
            if (parts.Length != 2)
                return;

            if (!uint.TryParse(parts[0], out uint targetEntityID))
                return;

            if (targetEntityID != (uint)EntityID)
                return;

            if (!bool.TryParse(parts[1], out bool paused))
                return;

            isExternallyPaused = paused;
        }
    }
}
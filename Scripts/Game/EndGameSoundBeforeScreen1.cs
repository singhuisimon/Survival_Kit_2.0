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
        private bool wasAudioPausedByUs = false;

        // Tracks whether the current pause is a tutorial pause (not a real pause menu).
        // Audio should keep playing during tutorial pauses, matching GameVOSoundPrefab behaviour.
        private bool isTutorialPause = false;
        private const string EVENT_TUTORIAL_PAUSE_AUDIO = "TutorialPauseAudio";

        public override void OnStart()
        {
            Subscribe(EVENT_TUTORIAL_PAUSE_AUDIO, OnTutorialPauseAudio);
        }

        public override void OnUpdate(float deltaTime)
        {
            // Only pause for a real pause menu, not a tutorial pause.
            bool isRealPause = GameState.IsPaused && !isTutorialPause;

            if (isRealPause)
            {
                if (!wasPaused)
                {
                    savedTime = elapsedTime;
                    wasPaused = true;
                    if (AudioIsPlaying((uint)EntityID))
                    {
                        AudioPause((uint)EntityID);
                        wasAudioPausedByUs = true;
                    }
                }
                return;
            }
            else if (wasPaused)
            {
                // Just unpaused - restore timer
                elapsedTime = savedTime;
                wasPaused = false;
                // Only resume if we actually paused it — prevents restarting audio
                // that had already finished playing before the pause menu opened.
                if (wasAudioPausedByUs)
                {
                    AudioPlay((uint)EntityID);
                    wasAudioPausedByUs = false;
                }
            }

            // Lifetime
            elapsedTime += deltaTime;
            publishTime += deltaTime;
            if (publishTime >= 1.0f)
            {
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
            Unsubscribe(EVENT_TUTORIAL_PAUSE_AUDIO, OnTutorialPauseAudio);
            AudioStop((uint)EntityID);
        }

        private void OnTutorialPauseAudio(string eventName, string payload)
        {
            if (bool.TryParse(payload, out bool paused))
                isTutorialPause = paused;
        }
    }
}

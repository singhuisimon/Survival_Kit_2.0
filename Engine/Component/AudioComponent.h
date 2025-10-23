#pragma once
#include <fmod.hpp>
#include <string>

namespace Engine {

    enum class AudioType {MASTER, SFX, BGM, UI};
    enum class PlayState {PLAY, PAUSE, STOP};

    /**
     * @brief Audio playback component for SFX, BGM, and UI sounds
     */
    struct AudioComponent {
        // --- Serialized Data ---
        std::string AudioFilePath;   // Path to audio asset
        AudioType Type;              // SFX, BGM, UI, Master
        PlayState State;             // Play / Pause / Stop
        float Volume;                // 0.0 - 1.0
        float Pitch;                 // 0.5 - 2.0 (general range)
        bool Loop;                   // Loop playback
        bool Mute;                   // Mute toggle
        bool Is3D;                   // Enable 3D positional audio
        float MinDistance;           // 3D attenuation min
        float MaxDistance;           // 3D attenuation max
        float ReverbProperties;      // Wet level for reverb itself

        // --- Runtime Only (Not Serialized) ---
        FMOD::Channel* Channel;      // Active FMOD channel instance
        bool IsDirty;                // True when FMOD needs to be updated
        std::string PreviousPath;    // For stopping & switching audio files
  
        // --- Constructor ---
        AudioComponent()
            : AudioFilePath("")
            , Type(AudioType::SFX)
            , State(PlayState::STOP)
            , Volume(1.0f)
            , Pitch(1.0f)
            , Loop(false)
            , Mute(false)
            , Is3D(true)
            , MinDistance(1.0f)
            , MaxDistance(100.0f)
            , ReverbProperties(1.0f)
            , Channel(nullptr)
            , IsDirty(true)          // Initial push to FMOD on first update
            , PreviousPath("")
        {
        }

        // --- Constructor with filepath ---
        AudioComponent(const std::string& filepath)
            : AudioFilePath(filepath)
            , Type(AudioType::SFX)
            , State(PlayState::STOP)
            , Volume(1.0f)
            , Pitch(1.0f)
            , Loop(false)
            , Mute(false)
            , Is3D(true)
            , MinDistance(1.0f)
            , MaxDistance(100.0f)
            , ReverbProperties(1.0f)
            , Channel(nullptr)
            , IsDirty(true)          // Initial push to FMOD on first update
            , PreviousPath("")
        {
        }

        // --- Setters that mark component dirty ---
        void SetState(PlayState state) {
            State = state;
            IsDirty = true;
        }

        void SetVolume(float vol) {
            Volume = vol;
            IsDirty = true;
        }

        void SetPitch(float pitch) {
            Pitch = pitch;
            IsDirty = true;
        }

        void SetLoop(bool loop) {
            Loop = loop;
            IsDirty = true;
        }

        void SetMute(bool mute) {
            Mute = mute;
            IsDirty = true;
        }

        void SetReverbProperties(float reverbproperties) {
            ReverbProperties = reverbproperties;
            IsDirty = true;
        }

        void SetAudioFile(const std::string& path) {
            AudioFilePath = path;
            IsDirty = true;
        }
    };

}
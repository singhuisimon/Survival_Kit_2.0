/**
 * @file AudioComponent.h
 * @brief Represent the audio parts of an entity in ECS
 * @author Amanda Leow Boon Suan (100%)
 * @date 20/10/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#pragma once
#include <fmod.hpp>
#include <string>
#include "../Serialization/ComponentRegistry.h"

namespace Engine {

    enum class AudioType {MASTER, SFX, BGM, UI, VO, GAMESFX};
    enum class PlayState {PLAY, PAUSE, STOP};
    enum class AudioRolloffMode{INVERSE, LINEAR, LINEARSQUARE};

    /**
     * @brief Audio playback component for SFX, BGM, and UI sounds
     */
    struct AudioComponent {
        static constexpr ComponentTypeID TypeID = ComponentTypeID::Audio;
        static constexpr const char* TypeName = "AudioComponent";

        xresource::instance_guid ComponentGUID;
        // --- Serialized Data ---
        std::string AudioFilePath;   // Path to audio asset
        AudioType Type;              // SFX, BGM, UI, Master, VO, GAMESFX
        PlayState State;             // Play / Pause / Stop
        float Volume;                // 0.0 - 1.0
        float Pitch;                 // 0.5 - 2.0 (general range)
        bool Loop;                   // Loop playback
        bool Mute;                   // Mute toggle

        bool Is3D;                   // Enable 3D positional audio
        float MinDistance;           // 3D attenuation min
        float MaxDistance;           // 3D attenuation max
        float ReverbProperties;      // Wet level for reverb zones

        //NEW M3
        float Pan2D;                 // Stereo: -1.0 for left to 1.0 for right
        float DopplerLevel;          // Doppler effect intensity (0-5, default 1)
        AudioRolloffMode RolloffMode;       // Inverse(default), Linear, Linearsquare


        // --- Runtime Only (Not Serialized) ---
        FMOD::Channel* Channel;      // Active FMOD channel instance
        bool IsDirty;                // True when FMOD needs to be updated
        std::string PreviousPath;    // For stopping & switching audio files
  
        // --- Constructor ---
        AudioComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()),
            AudioFilePath("")
            , Type(AudioType::GAMESFX)
            , State(PlayState::STOP)
            , Volume(1.0f)
            , Pitch(1.0f)
            , Loop(false)
            , Mute(false)
            , Is3D(true)
            , MinDistance(1.0f)
            , MaxDistance(100.0f)
            , Pan2D(0.0f)
            , DopplerLevel(1.0f)
            , ReverbProperties(1.0f)
            , RolloffMode(AudioRolloffMode::INVERSE)
            , Channel(nullptr)
            , IsDirty(true)          // Initial push to FMOD on first update
            , PreviousPath("")
        {
        }

        // --- Constructor with filepath ---
        AudioComponent(const std::string& filepath)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , AudioFilePath(filepath)
            , Type(AudioType::GAMESFX)
            , State(PlayState::STOP)
            , Volume(1.0f)
            , Pitch(1.0f)
            , Loop(false)
            , Mute(false)
            , Is3D(true)
            , MinDistance(1.0f)
            , MaxDistance(100.0f)
            , Pan2D(0.0f)
            , DopplerLevel(1.0f)
            , ReverbProperties(1.0f)
            , RolloffMode(AudioRolloffMode::INVERSE)
            , Channel(nullptr)
            , IsDirty(true)          // Initial push to FMOD on first update
            , PreviousPath("")
        {
        }

        // --- Setters that mark component dirty ---

        void SetAudioType(AudioType type) {
            Type = type;
            IsDirty = true;
        }

        void Set3D(bool is3d) {
            Is3D = is3d;
            IsDirty = true;
        }

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

        void SetMinDistance(float min) {
            MinDistance = min;
            IsDirty = true;
        }

        void SetMaxDistance(float max){
            MaxDistance = max;
            IsDirty = true;
        }

        void SetRolloffMode(AudioRolloffMode mode) {
            RolloffMode = mode;
            IsDirty = true;
        }

        void SetDopplerLevel(float level) {
            DopplerLevel = level;
            IsDirty = true;
        }

        void SetPan(float pan) {
            //Clamp to valid range
            Pan2D = (pan < -1.0f) ? -1.0f : (pan > 1.0f) ? 1.0f : pan;
            IsDirty = true;
        }
    };

}
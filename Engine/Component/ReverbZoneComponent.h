#pragma once
#include <fmod.hpp>
#include <string>

namespace Engine {

    enum class ReverbPreset { Custom, Generic, Bathroom, Room, Cave, Arena };

    /**
     * @brief Component that defines a 3D reverb zone in the scene.
     * @details
     * - Reverb affects any AudioComponents that have Reverb = true.
     * - The AudioEffectSystem or AudioSystem should manage FMOD::Reverb3D lifetimes,
     *   apply preset values, and update the 3D position/attenuation.
     * - This struct stays as POD data (no logic), as ECS best practices recommend.
     */
    struct ReverbZoneComponent {
        // Serialized values
        ReverbPreset Preset;  // For easy presets (Generic, Cave, Room, etc.)
        float MinDistance;    // Reverb fade-in start distance
        float MaxDistance;    // Reverb fade-out end distance
        float DecayTime;      // Reverb duration (ms)
        float HfDecayRatio;   // High-frequency decay ratio
        float Diffusion;      // Echo density (0–100)
        float Density;        // Reverb density (0–100)
        float WetLevel;       // Wet mix level (negative dB in FMOD)

        // Runtime only (NOT serialized)
        FMOD::Reverb3D* ReverbZone;
        bool IsDirty;
  
        // --- Constructor ---
        ReverbZoneComponent()
            : Preset(ReverbPreset::Generic)
            , MinDistance(1.0f)
            , MaxDistance(20.0f)
            , DecayTime(1500.0f)      // FMOD default-ish values
            , HfDecayRatio(50.0f)
            , Diffusion(50.0f)
            , Density(100.0f)
            , WetLevel(-6.0f)
            , ReverbZone(nullptr)
            , IsDirty(true)
        {
        }

        // --- Constructor with filepath ---
        ReverbZoneComponent(ReverbPreset preset)
            : Preset(preset)
            , MinDistance(1.0f)
            , MaxDistance(20.0f)
            , DecayTime(1500.0f)      // FMOD default-ish values
            , HfDecayRatio(50.0f)
            , Diffusion(50.0f)
            , Density(100.0f)
            , WetLevel(-6.0f)
            , ReverbZone(nullptr)
            , IsDirty(true)
        {
        }

        // --- Setters that mark component dirty ---
        void SetPreset(ReverbPreset p) { Preset = p; IsDirty = true; }
        void SetMinDistance(float v) { MinDistance = v; IsDirty = true; }
        void SetMaxDistance(float v) { MaxDistance = v; IsDirty = true; }
        void SetDecayTime(float v) { DecayTime = v; IsDirty = true; }
        void SetHfDecayRatio(float v) { HfDecayRatio = v; IsDirty = true; }
        void SetDiffusion(float v) { Diffusion = v; IsDirty = true; }
        void SetDensity(float v) { Density = v; IsDirty = true; }
        void SetWetLevel(float v) { WetLevel = v; IsDirty = true; }
    };

}
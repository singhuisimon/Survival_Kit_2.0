#pragma once
#include "ECS/Scene.h"
#include "ECS/System.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "AudioManager.h"
#include <fmod.hpp>
#include <fmod_errors.h>
#include <unordered_map>
#include <string>
#include <mutex>

namespace Engine {

    /**
     * @brief Enum for supported DSP effects in the engine.
     * @details
     * This maps directly to FMOD DSP types or custom wrappers.
     */
    enum class DSPEffectType {
        None = 0,
        LowPass,
        HighPass,
        Echo,
        Reverb,
        Chorus,
        Distortion,
        Flange,
        Tremolo,
        PitchShift,
        Compressor
        //Add more as needed
    };

    /**
     * @brief Utility functions for DSPEffectType.
     */
    struct DSPEffectUtil {
        static std::string ToString(DSPEffectType type) {
            switch (type) {
            case DSPEffectType::LowPass: return "LowPass";
            case DSPEffectType::HighPass: return "HighPass";
            case DSPEffectType::Echo: return "Echo";
            case DSPEffectType::Reverb: return "Reverb";
            case DSPEffectType::Chorus: return "Chorus";
            case DSPEffectType::Distortion: return "Distortion";
            case DSPEffectType::Flange: return "Flange";
            case DSPEffectType::Tremolo: return "Tremolo";
            case DSPEffectType::PitchShift: return "PitchShift";
            case DSPEffectType::Compressor: return "Compressor";
            default: return "None";
            }
        }

        static FMOD_DSP_TYPE ToFMODType(DSPEffectType type) {
            switch (type) {
            case DSPEffectType::LowPass: return FMOD_DSP_TYPE_LOWPASS;
            case DSPEffectType::HighPass: return FMOD_DSP_TYPE_HIGHPASS;
            case DSPEffectType::Echo: return FMOD_DSP_TYPE_ECHO;
            case DSPEffectType::Reverb: return FMOD_DSP_TYPE_SFXREVERB;
            case DSPEffectType::Chorus: return FMOD_DSP_TYPE_CHORUS;
            case DSPEffectType::Distortion: return FMOD_DSP_TYPE_DISTORTION;
            case DSPEffectType::Flange: return FMOD_DSP_TYPE_FLANGE;
            case DSPEffectType::Tremolo: return FMOD_DSP_TYPE_TREMOLO;
            case DSPEffectType::PitchShift: return FMOD_DSP_TYPE_PITCHSHIFT;
            case DSPEffectType::Compressor: return FMOD_DSP_TYPE_COMPRESSOR;
            default: return FMOD_DSP_TYPE_UNKNOWN;
            }
        }
    };

} // namespace Engine
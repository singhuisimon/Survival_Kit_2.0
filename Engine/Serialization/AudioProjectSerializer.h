/**
 * @file AudioProjectSerializer.h
 * @brief Serialization for project-wide audio mixer settings (editor caps).
 *        Replaces per-scene audio settings with a single shared project file.
 */
#pragma once
#include <string>

namespace Engine {

    struct AudioProjectSettings {
        float masterVolume  = 1.0f;
        float sfxVolume     = 1.0f;
        float bgmVolume     = 1.0f;
        float uiVolume      = 1.0f;
        float voVolume      = 1.0f;
        float gameSFXVolume = 1.0f;
    };

    class AudioProjectSerializer {
    public:
        // Write current settings to a JSON file. Creates the file if it does not exist.
        static bool Serialize(const AudioProjectSettings& settings, const std::string& filepath);

        // Read settings from a JSON file into outSettings. Missing keys fall back to defaults.
        static bool Deserialize(const std::string& filepath, AudioProjectSettings& outSettings);
    };

} // namespace Engine

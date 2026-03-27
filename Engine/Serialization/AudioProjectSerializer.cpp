/**
 * @file AudioProjectSerializer.cpp
 * @brief Serialization for project-wide audio mixer settings (editor caps).
 *        Replaces per-scene audio settings with a single shared project file.
 */
#include "AudioProjectSerializer.h"
#include "../Utility/Logger.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <filesystem>

namespace Engine {

    bool AudioProjectSerializer::Serialize(const AudioProjectSettings& settings, const std::string& filepath)
    {
        // Ensure parent directory exists
        std::filesystem::path p(filepath);
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());

        rapidjson::Document doc;
        doc.SetObject();
        auto& alloc = doc.GetAllocator();

        doc.AddMember("masterVolume",  settings.masterVolume,  alloc);
        doc.AddMember("sfxVolume",     settings.sfxVolume,     alloc);
        doc.AddMember("bgmVolume",     settings.bgmVolume,     alloc);
        doc.AddMember("uiVolume",      settings.uiVolume,      alloc);
        doc.AddMember("voVolume",      settings.voVolume,      alloc);
        doc.AddMember("gameSFXVolume", settings.gameSFXVolume, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::ofstream file(filepath);
        if (!file.is_open())
        {
            LOG_ERROR("AudioProjectSerializer::Serialize - failed to open file: ", filepath);
            return false;
        }
        file << buffer.GetString();
        LOG_INFO("AudioProjectSerializer: saved project audio settings to ", filepath);
        return true;
    }

    bool AudioProjectSerializer::Deserialize(const std::string& filepath, AudioProjectSettings& outSettings)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            LOG_WARNING("AudioProjectSerializer::Deserialize - file not found, using defaults: ", filepath);
            return false;
        }

        std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        rapidjson::Document doc;
        doc.Parse(json.c_str());
        if (doc.HasParseError())
        {
            LOG_ERROR("AudioProjectSerializer::Deserialize - JSON parse error in: ", filepath);
            return false;
        }

        if (doc.HasMember("masterVolume")  && doc["masterVolume"].IsFloat())
            outSettings.masterVolume  = doc["masterVolume"].GetFloat();
        if (doc.HasMember("sfxVolume")     && doc["sfxVolume"].IsFloat())
            outSettings.sfxVolume     = doc["sfxVolume"].GetFloat();
        if (doc.HasMember("bgmVolume")     && doc["bgmVolume"].IsFloat())
            outSettings.bgmVolume     = doc["bgmVolume"].GetFloat();
        if (doc.HasMember("uiVolume")      && doc["uiVolume"].IsFloat())
            outSettings.uiVolume      = doc["uiVolume"].GetFloat();
        if (doc.HasMember("voVolume")      && doc["voVolume"].IsFloat())
            outSettings.voVolume      = doc["voVolume"].GetFloat();
        if (doc.HasMember("gameSFXVolume") && doc["gameSFXVolume"].IsFloat())
            outSettings.gameSFXVolume = doc["gameSFXVolume"].GetFloat();

        LOG_INFO("AudioProjectSerializer: loaded project audio settings from ", filepath);
        return true;
    }

} // namespace Engine

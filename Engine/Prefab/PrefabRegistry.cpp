/**
 * @file PrefabRegistry.cpp
 * @brief Implementation of PrefabRegistry singleton
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "PrefabRegistry.h"
#include "../Utility/Logger.h"
#include "../ECS/Entity.h"
#include "../Serialization/PrefabSerializer.h"
#include "../xresource_guid/include/xresource_guid.h"

#include <algorithm>
namespace Engine {
    PrefabRegistry& PrefabRegistry::Get() {
        static PrefabRegistry instance;
        return instance;
    }

    void PrefabRegistry::RegisterPrefab(xresource::instance_guid guid, const std::string& filePath, const std::string& name) {
        u64 guidValue = guid.m_Value;

        // Extract name from path if not provided
        std::string prefabName = name;
        if (prefabName.empty()) {
            // Extract filename from path (without extension)
            size_t lastSlash = filePath.find_last_of("/\\");
            size_t lastDot = filePath.find_last_of(".");

            if (lastSlash != std::string::npos && lastDot != std::string::npos) {
                prefabName = filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
            }
            else if (lastDot != std::string::npos) {
                prefabName = filePath.substr(0, lastDot);
            }
            else {
                prefabName = filePath;
            }
        }

        m_PrefabRegistry[guidValue] = { filePath, prefabName };
        LOG_INFO("Registered prefab: ", prefabName.c_str(), " (GUID: ", static_cast<u64>(guidValue), ")");
    }

    void PrefabRegistry::UnregisterPrefab(xresource::instance_guid guid) {
        u64 guidValue = guid.m_Value;

        auto it = m_PrefabRegistry.find(guidValue);
        if (it != m_PrefabRegistry.end()) {
            LOG_INFO("Unregistered prefab: ", it->second.second.c_str());
            m_PrefabRegistry.erase(it);
        }
    }

    bool PrefabRegistry::LoadPrefab(xresource::instance_guid guid, Prefab& outPrefab) {
        u64 guidValue = guid.m_Value;

        auto it = m_PrefabRegistry.find(guidValue);
        if (it == m_PrefabRegistry.end()) {
            LOG_ERROR("Prefab not registered with GUID: ", static_cast<u64>(guidValue));
            return false;
        }

        const std::string& filePath = it->second.first;
        return LoadPrefabFromFile(filePath, outPrefab);
    }

    bool PrefabRegistry::LoadPrefabFromFile(const std::string& filePath, Prefab& outPrefab) {
        if (!PrefabSerializer::DeserializePrefab(filePath, outPrefab)) {
            LOG_ERROR("Failed to load prefab from file: ", filePath.c_str());
            return false;
        }

        LOG_DEBUG("Successfully loaded prefab: ", outPrefab.name.c_str());
        return true;
    }

    std::string PrefabRegistry::GetPrefabPath(xresource::instance_guid guid) const {
        u64 guidValue = guid.m_Value;

        auto it = m_PrefabRegistry.find(guidValue);
        if (it != m_PrefabRegistry.end()) {
            return it->second.first;
        }

        return "";
    }

    std::string PrefabRegistry::GetPrefabName(xresource::instance_guid guid) const {
        u64 guidValue = guid.m_Value;

        auto it = m_PrefabRegistry.find(guidValue);
        if (it != m_PrefabRegistry.end()) {
            return it->second.second;
        }

        return "";
    }

    bool PrefabRegistry::IsPrefabRegistered(xresource::instance_guid guid) const {
        u64 guidValue = guid.m_Value;
        return m_PrefabRegistry.find(guidValue) != m_PrefabRegistry.end();
    }

    const std::unordered_map<u64, std::pair<std::string, std::string>>& PrefabRegistry::GetAllPrefabs() const {
        return m_PrefabRegistry;
    }

    void PrefabRegistry::Clear() {
        m_PrefabRegistry.clear();
        LOG_INFO("Cleared prefab registry");
    }

    bool PrefabRegistry::SavePrefabToFile(const Prefab& prefab, const std::string& filePath) {
        if (!PrefabSerializer::SerializePrefab(prefab, filePath)) {
            LOG_ERROR("Failed to save prefab to file: ", filePath.c_str());
            return false;
        }

        LOG_INFO("Successfully saved prefab: ", prefab.name.c_str(), " to: ", filePath.c_str());
        return true;
    }

 
} // namespace Engine
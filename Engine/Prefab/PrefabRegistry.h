/**
 * @file PrefabRegistry.h
 * @brief Singleton registry for managing prefab resources
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#ifndef __PREFAB_REGISTRY_H__
#define __PREFAB_REGISTRY_H__

#include "Prefab.h"
#include "../xresource_guid/include/xresource_guid.h"

#include <unordered_map>
#include <memory>
#include <string>


namespace Engine {

	class Entity;
    class Scene;
    class PrefabRegistry {
    public:
        // Get singleton instance
        static PrefabRegistry& Get();

        // Register a prefab in memory
        void RegisterPrefab(xresource::instance_guid guid, const std::string& filePath, const std::string& name);

        // Unregister a prefab
        void UnregisterPrefab(xresource::instance_guid guid);

        // Load a prefab from disk by GUID
        bool LoadPrefab(xresource::instance_guid guid, Prefab& outPrefab);

        // Load a prefab from file path
        bool LoadPrefabFromFile(const std::string& filePath, Prefab& outPrefab);

        // Get prefab file path by GUID
        std::string GetPrefabPath(xresource::instance_guid guid) const;

        // Get prefab name by GUID
        std::string GetPrefabName(xresource::instance_guid guid) const;

        // Check if prefab is registered
        bool IsPrefabRegistered(xresource::instance_guid guid) const;

        // Get all registered prefabs
        const std::unordered_map<u64, std::pair<std::string, std::string>>& GetAllPrefabs() const;

        bool SavePrefabToFile(const Prefab& prefab, const std::string& filePath);

        // Clear registry
        void Clear();

    private:
        PrefabRegistry() = default;
        ~PrefabRegistry() = default;

        // Prevent copying
        PrefabRegistry(const PrefabRegistry&) = delete;
        PrefabRegistry& operator=(const PrefabRegistry&) = delete;

        // Map of GUID -> (FilePath, PrefabName)
        std::unordered_map<u64, std::pair<std::string, std::string>> m_PrefabRegistry;
    };
} // namespace Engine

#endif // __PREFAB_REGISTRY_H__
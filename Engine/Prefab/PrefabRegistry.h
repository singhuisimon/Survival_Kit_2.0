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
#include <unordered_map>
#include <memory>
#include <string>

namespace Engine {

    /**
     * @brief Singleton registry for managing loaded prefabs
     * @details Provides centralized access to all prefab resources
     */
    class PrefabRegistry {
    public:
        static PrefabRegistry& Get() {
            static PrefabRegistry instance;
            return instance;
        }

        PrefabRegistry(const PrefabRegistry&) = delete;
        PrefabRegistry& operator=(const PrefabRegistry&) = delete;

        /**
         * @brief Register a prefab in the registry
         * @param prefab Shared pointer to the prefab
         */
        void RegisterPrefab(std::shared_ptr<Prefab> prefab);

        /**
         * @brief Unregister a prefab from the registry
         * @param guid GUID of the prefab to unregister
         */
        void UnregisterPrefab(xresource::instance_guid guid);

        /**
         * @brief Get a prefab by GUID
         * @param guid GUID of the prefab
         * @return Shared pointer to prefab, or nullptr if not found
         */
        std::shared_ptr<Prefab> GetPrefab(xresource::instance_guid guid) const;

        /**
         * @brief Get a prefab by name
         * @param name Name of the prefab
         * @return Shared pointer to prefab, or nullptr if not found
         */
        std::shared_ptr<Prefab> GetPrefabByName(const std::string& name) const;

        /**
         * @brief Check if a prefab is loaded
         * @param guid GUID of the prefab
         * @return True if prefab is loaded
         */
        bool IsPrefabLoaded(xresource::instance_guid guid) const;

        /**
         * @brief Clear all prefabs from registry
         */
        void Clear();

        /**
         * @brief Get all registered prefabs
         * @return Map of all prefabs (GUID -> Prefab)
         */
        const std::unordered_map<xresource::instance_guid, std::shared_ptr<Prefab>>& GetAllPrefabs() const {
            return m_Prefabs;
        }

    private:
        PrefabRegistry() = default;
        ~PrefabRegistry() = default;

        std::unordered_map<xresource::instance_guid, std::shared_ptr<Prefab>> m_Prefabs;
        std::unordered_map<std::string, xresource::instance_guid> m_PrefabsByName;
    };

} // namespace Engine

#endif // __PREFAB_REGISTRY_H__
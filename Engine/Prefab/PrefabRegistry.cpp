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

namespace Engine {

    void PrefabRegistry::RegisterPrefab(std::shared_ptr<Prefab> prefab) {
        if (!prefab) {
            LOG_ERROR("PrefabRegistry: Attempted to register null prefab");
            return;
        }

        xresource::instance_guid guid = prefab->GetGUID();

        // Check if already registered
        if (m_Prefabs.find(guid) != m_Prefabs.end()) {
            LOG_WARNING("PrefabRegistry: Prefab already registered: ", prefab->GetName());
            return;
        }

        m_Prefabs[guid] = prefab;
        m_PrefabsByName[prefab->GetName()] = guid;

        LOG_INFO("PrefabRegistry: Registered prefab '", prefab->GetName(),
            "' (GUID: 0x", std::hex, guid.m_Value, std::dec, ")");
    }

    void PrefabRegistry::UnregisterPrefab(xresource::instance_guid guid) {
        auto it = m_Prefabs.find(guid);
        if (it == m_Prefabs.end()) {
            LOG_WARNING("PrefabRegistry: Attempted to unregister non-existent prefab");
            return;
        }

        std::string name = it->second->GetName();
        m_PrefabsByName.erase(name);
        m_Prefabs.erase(it);

        LOG_INFO("PrefabRegistry: Unregistered prefab '", name, "'");
    }

    std::shared_ptr<Prefab> PrefabRegistry::GetPrefab(xresource::instance_guid guid) const {
        auto it = m_Prefabs.find(guid);
        if (it != m_Prefabs.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::shared_ptr<Prefab> PrefabRegistry::GetPrefabByName(const std::string& name) const {
        auto it = m_PrefabsByName.find(name);
        if (it != m_PrefabsByName.end()) {
            return GetPrefab(it->second);
        }
        return nullptr;
    }

    bool PrefabRegistry::IsPrefabLoaded(xresource::instance_guid guid) const {
        return m_Prefabs.find(guid) != m_Prefabs.end();
    }

    void PrefabRegistry::Clear() {
        LOG_INFO("PrefabRegistry: Clearing all prefabs (", m_Prefabs.size(), " prefabs)");
        m_Prefabs.clear();
        m_PrefabsByName.clear();
    }

} // namespace Engine
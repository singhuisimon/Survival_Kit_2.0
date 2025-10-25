/**
 * @file Prefab.h
 * @brief Prefab resource class for entity and scene templates
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#ifndef __PREFAB_H__
#define __PREFAB_H__

#include <string>
#include <memory>
#include "../xresource_guid/include/xresource_guid.h"

namespace Engine {

    /**
     * @brief Type of prefab
     */
    enum class PrefabType {
        Entity,  // Single entity prefab
        Scene    // Multi-entity scene prefab
    };

    /**
     * @brief Prefab resource - template for creating entities
     * @details Stores serialized entity/scene data and metadata
     */
    class Prefab {
    public:
        Prefab(PrefabType type = PrefabType::Entity);
        ~Prefab() = default;

        // Resource identification
        xresource::instance_guid GetGUID() const { return m_GUID; }
        void SetGUID(xresource::instance_guid guid) { m_GUID = guid; }

        PrefabType GetType() const { return m_Type; }
        void SetType(PrefabType type) { m_Type = type; }

        // For Entity Prefabs
        void SetEntityData(const std::string& jsonData);
        const std::string& GetEntityData() const { return m_EntityData; }

        // For Scene Prefabs
        void SetSceneData(const std::string& jsonData);
        const std::string& GetSceneData() const { return m_SceneData; }

        xresource::instance_guid GetRootEntityGUID() const { return m_RootEntityGUID; }
        void SetRootEntityGUID(xresource::instance_guid guid) { m_RootEntityGUID = guid; }

        // Metadata
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        const std::string& GetSourcePath() const { return m_SourcePath; }
        void SetSourcePath(const std::string& path) { m_SourcePath = path; }

    private:
        xresource::instance_guid m_GUID;
        PrefabType m_Type;
        std::string m_Name;
        std::string m_SourcePath;

        // Serialized entity/scene data (JSON format)
        std::string m_EntityData;  // For entity prefabs
        std::string m_SceneData;   // For scene prefabs

        // For scene prefabs - track the root entity
        xresource::instance_guid m_RootEntityGUID;
    };

} // namespace Engine

#endif // __PREFAB_H__
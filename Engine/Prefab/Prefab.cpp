/**
 * @file Prefab.cpp
 * @brief Implementation of Prefab class
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "Prefab.h"

namespace Engine {

    Prefab::Prefab(PrefabType type)
        : m_GUID(xresource::instance_guid::GenerateGUIDCopy())
        , m_Type(type)
        , m_Name("Untitled Prefab")
        , m_RootEntityGUID(xresource::instance_guid{}) {
    }

    void Prefab::SetEntityData(const std::string& jsonData) {
        m_EntityData = jsonData;
    }

    void Prefab::SetSceneData(const std::string& jsonData) {
        m_SceneData = jsonData;
    }

} // namespace Engine
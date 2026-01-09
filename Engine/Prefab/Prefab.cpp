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
    const PrefabEntityData* Prefab::GetRootEntity() const {
        if (entities.empty()) {
            return nullptr;
        }

        for (const auto& entity : entities) {
            if (entity.parentLocalID == 0) {
                return &entity;
            }
        }

        return &entities[0];
    }

    const PrefabEntityData* Prefab::FindEntityByLocalID(u64 localID) const {
        for (const auto& entity : entities) {
            if (entity.localID == localID) {
                return &entity;
            }
        }
        return nullptr;
    }

    std::vector<const PrefabEntityData*> Prefab::GetChildren(u64 parentLocalID) const {
        std::vector<const PrefabEntityData*> children;

        for (const auto& entity : entities) {
            if (entity.parentLocalID == parentLocalID && entity.localID != parentLocalID) {
                children.push_back(&entity);
            }
        }

        return children;
    }

    bool Prefab::IsValid() const {
        return m_IsValid && !entities.empty();
    }

    void Prefab::Clear() {
        entities.clear();
        name.clear();
        version = 1;
        guid = 0;
        m_IsValid = false;
    }

    const PrefabComponentData* Prefab::GetComponent(u64 entityLocalID, ComponentTypeID type) const {
        const PrefabEntityData* entity = FindEntityByLocalID(entityLocalID);
        if (!entity) {
            return nullptr;
        }

        for (const auto& component : entity->components) {
            if (component.type == type) {
                return &component;
            }
        }

        return nullptr;
    }
} // namespace Engine
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
#include "../Serialization/ComponentRegistry.h"
#include "../Asset/ResourceTypes.h"

namespace Engine {

	struct PrefabComponentData
	{
		ComponentTypeID type;
		std::string typeName;
		std::vector<u8> serializedData;
		PrefabComponentData() : type(ComponentTypeID::None) {}
	};
	struct PrefabEntityData {
		std::string name;
		u64 localID;
		u64 parentLocalID;
		std::vector<PrefabComponentData> components;

		PrefabEntityData() : localID(0), parentLocalID(0) {}
	};

    class Prefab 
    {
 
    public:
        xresource::instance_guid guid;
        std::string name;
        u32 version;
        std::vector<PrefabEntityData> entities;
        bool m_IsValid;
        Prefab() : guid(0),
            version(1),
            m_IsValid(false) {
        }

        ~Prefab() { Clear(); }

        const PrefabEntityData* GetRootEntity() const;
        const PrefabEntityData* FindEntityByLocalID(u64 localID) const;
        std::vector<const PrefabEntityData*> GetChildren(u64 parentLocalID) const;

        bool IsValid() const;
        void Clear();

        const PrefabComponentData* GetComponent(u64 entityLocalID, ComponentTypeID type) const;
    };

    struct PrefabResource {
        Prefab prefabData;
        std::string sourcePath;
        xresource::instance_guid guid;

        PrefabResource() = default;
    };
} // namespace Engine

#endif // __PREFAB_H__
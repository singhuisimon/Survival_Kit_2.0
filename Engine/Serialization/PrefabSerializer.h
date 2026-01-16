#pragma once
#ifndef __PREFAB_SERIALIZER_H__
#define __PREFAB_SERIALIZER_H__

#include "../Prefab/Prefab.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include <string>
#include <vector>
#include <memory>
#include <entt/entt.hpp>
#include <rapidjson/document.h>

namespace Engine
{
    class PrefabSerializer {
    public:
        // Serialize prefab to file (JSON format)
        static bool SerializePrefab(const Prefab& prefab, const std::string& filepath);

        // Deserialize prefab from file
        static bool DeserializePrefab(const std::string& filepath, Prefab& outPrefab);

        // Create prefab from entity hierarchy
        static bool CreatePrefabFromEntity(Entity rootEntity, Prefab& outPrefab, const std::string& prefabName);

        // Serialize an entity and all its children to a prefab file
        static bool SerializeEntityToPrefabFile(Entity rootEntity, const std::string& prefabName,
            const std::string& filePath);


        static PrefabComponentData SerializeEntityComponent(Entity entity, ComponentTypeID type);

    private:
        static void SerializeEntityHierarchy(Entity entity, Prefab& prefab, u32& nextLocalID, u32 parentLocalID);


    };
}
#endif // end of __PREFAB_SERIALIZER_H__
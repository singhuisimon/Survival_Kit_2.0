#pragma once
#include "../Serialization/ComponentRegistry.h"
#include "../ECS/Entity.h"


#include <string>

namespace Engine
{
    class ComponentSerializer {
    public:
        // Serialize any component to JSON string
        static std::string SerializeComponent(Entity entity, ComponentTypeID type);

        // Deserialize any component from JSON string
        static bool DeserializeComponent(Entity entity, ComponentTypeID type, const std::string& jsonString);

        // Get component type name
        static const char* GetComponentTypeName(ComponentTypeID type);
       

    };
}
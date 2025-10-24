#include "RigidbodyComponent.hxx"
#include "../Engine/Core/ScriptingBridge.h"
#pragma comment(lib, "Engine.lib")

namespace ScriptAPI
{
    void RigidbodyComponent::AddImpulse(float3 i)
    {
        Core::ScriptingBridge::Physics_AddImpulse(entityId, Core::Float3{ i.X,i.Y,i.Z });
    }
    void RigidbodyComponent::SetLinearVelocity(float3 v)
    {
        Core::ScriptingBridge::Physics_SetLinearVelocity(entityId, Core::Float3{ v.X,v.Y,v.Z });
    }
    float3 RigidbodyComponent::GetLinearVelocity()
    {
        auto v = Core::ScriptingBridge::Physics_GetLinearVelocity(entityId);
        return float3(v.x, v.y, v.z);
    }
}

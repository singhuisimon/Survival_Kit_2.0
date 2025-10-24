#include "TransformComponent.hxx"
#include "../Engine/Core/ScriptingBridge.h"
#pragma comment(lib, "Engine.lib")

namespace ScriptAPI
{
    float3 TransformComponent::Position::get()
    {
        Core::Float3 out{};
        Core::ScriptingBridge::GetPosition(entityId, out);
        return float3(out.x, out.y, out.z);
    }
    void TransformComponent::Position::set(float3 v)
    {
        Core::ScriptingBridge::SetPosition(entityId, Core::Float3{ v.X, v.Y, v.Z });
    }

    float3 TransformComponent::Rotation::get()
    {
        Core::Float3 out{};
        Core::ScriptingBridge::GetRotationEuler(entityId, out);
        return float3(out.x, out.y, out.z);
    }
    void TransformComponent::Rotation::set(float3 v)
    {
        Core::ScriptingBridge::SetRotationEuler(entityId, Core::Float3{ v.X, v.Y, v.Z });
    }

    float3 TransformComponent::Scale::get()
    {
        Core::Float3 out{};
        Core::ScriptingBridge::GetScale(entityId, out);
        return float3(out.x, out.y, out.z);
    }
    void TransformComponent::Scale::set(float3 v)
    {
        Core::ScriptingBridge::SetScale(entityId, Core::Float3{ v.X, v.Y, v.Z });
    }
}

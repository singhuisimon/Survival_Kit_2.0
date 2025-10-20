#include "TransformComponent.hxx"
#include "../ScriptCore/ScriptBridge.h"

namespace ScriptAPI
{
    float TransformComponent::X::get()
    {
        if (Core::ScriptBridge::GetPosition) {
            glm::vec3 pos = Core::ScriptBridge::GetPosition(entityId);
            return pos.x;
        }
        return 0.0f;
    }

    void TransformComponent::X::set(float value)
    {
        if (Core::ScriptBridge::GetPosition && Core::ScriptBridge::SetPosition) {
            glm::vec3 pos = Core::ScriptBridge::GetPosition(entityId);
            pos.x = value;
            Core::ScriptBridge::SetPosition(entityId, pos);
        }
    }

    float TransformComponent::Y::get()
    {
        if (Core::ScriptBridge::GetPosition) {
            glm::vec3 pos = Core::ScriptBridge::GetPosition(entityId);
            return pos.y;
        }
        return 0.0f;
    }

    void TransformComponent::Y::set(float value)
    {
        if (Core::ScriptBridge::GetPosition && Core::ScriptBridge::SetPosition) {
            glm::vec3 pos = Core::ScriptBridge::GetPosition(entityId);
            pos.y = value;
            Core::ScriptBridge::SetPosition(entityId, pos);
        }
    }

    float TransformComponent::Z::get()
    {
        if (Core::ScriptBridge::GetPosition) {
            glm::vec3 pos = Core::ScriptBridge::GetPosition(entityId);
            return pos.z;
        }
        return 0.0f;
    }

    void TransformComponent::Z::set(float value)
    {
        if (Core::ScriptBridge::GetPosition && Core::ScriptBridge::SetPosition) {
            glm::vec3 pos = Core::ScriptBridge::GetPosition(entityId);
            pos.z = value;
            Core::ScriptBridge::SetPosition(entityId, pos);
        }
    }

    TransformComponent::TransformComponent(int id)
        : entityId{ id }
    {
    }
}
#pragma once
#include <cstdint>
#include "ECSBridge.h"
namespace ScriptAPI {
    class ScriptTransform {
    public:
        ScriptTransform(uint32_t entityId) : m_EntityID(entityId) {}
        float GetX() const {
            float x, y, z;
            ScriptCore::ECSBridge::GetInstance().GetPosition(m_EntityID, x, y, z);
            return x;
        }
        float GetY() const {
            float x, y, z;
            ScriptCore::ECSBridge::GetInstance().GetPosition(m_EntityID, x, y, z);
            return y;
        }
        float GetZ() const {
            float x, y, z;
            ScriptCore::ECSBridge::GetInstance().GetPosition(m_EntityID, x, y, z);
            return z;
        }
    private:
        uint32_t m_EntityID;
    };
}

#pragma once
#include <cstdint>

namespace ScriptAPI {
    class ScriptEntity {
    public:
        ScriptEntity(uint32_t id) : m_EntityID(id) {}
        uint32_t GetID() const { return m_EntityID; }
    private:
        uint32_t m_EntityID;
    };
}

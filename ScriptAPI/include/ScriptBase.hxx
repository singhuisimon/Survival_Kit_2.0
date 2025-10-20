#pragma once
#include <cstdint>

namespace ScriptAPI {
    class ScriptBase {
    public:
        virtual ~ScriptBase() = default;
        virtual void OnCreate() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnDestroy() {}
        void SetEntityID(uint32_t id) { m_EntityID = id; }
        uint32_t GetEntityID() const { return m_EntityID; }
    protected:
        uint32_t m_EntityID = 0;
    };
}

#include "ECSBridge.h"

namespace ScriptCore {
    ECSBridge& ECSBridge::GetInstance() {
        static ECSBridge instance;
        return instance;
    }

    void ECSBridge::RegisterGetPosition(GetPositionCallback cb) {
        m_GetPosition = cb;
    }

    void ECSBridge::GetPosition(uint32_t entityId, float& x, float& y, float& z) {
        if (m_GetPosition) {
            m_GetPosition(entityId, x, y, z);
        }
        else {
            x = y = z = 0.0f;
        }
    }
}

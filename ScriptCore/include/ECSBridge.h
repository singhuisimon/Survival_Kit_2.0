#pragma once
#include "ImportExport.h"
#include <cstdint>
#include <functional>

namespace ScriptCore {
    using GetPositionCallback = std::function<void(uint32_t entityId, float& x, float& y, float& z)>;

    class SCRIPTCORE_API ECSBridge {
    public:
        static ECSBridge& GetInstance();
        void RegisterGetPosition(GetPositionCallback cb);
        void GetPosition(uint32_t entityId, float& x, float& y, float& z);
    private:
        GetPositionCallback m_GetPosition;
    };
}

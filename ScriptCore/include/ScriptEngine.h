#pragma once
#include "ImportExport.h"
#include <string>

namespace ScriptCore {
    class SCRIPTCORE_API ScriptEngine {
    public:
        static bool LoadManagedAssembly(const std::string& dllPath);
        static void Shutdown();
    };
}

#pragma once
#include "Script.hxx"
#include "TransformComponent.hxx"
#include "RigidbodyComponent.hxx"
#include "MathTypes.hxx"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Reflection;
using namespace System::Runtime::Loader;

namespace ScriptAPI
{
    using ScriptList = List<Script ^>;

    public ref class EngineInterface abstract sealed
    {
    public:
        static void Init();                          // load ManagedScripts.dll into collectible ALC
        static void Shutdown();                      // clear and unload
        static void ExecuteUpdate(float dt);         // tick all scripts safely
        static bool AddScriptViaName(int entityId, String ^scriptName);
        static void Reload();                        // hot reload: unload ALC and reload it

    private:
        static AssemblyLoadContext ^s_ctx;
        static Assembly ^s_asm;
        static List<ScriptList ^> ^s_entityScripts;
        static Dictionary<String ^, Type ^> ^s_scriptTypes;

        static void BuildTypeMap();
        static void ClearState();
    };
}

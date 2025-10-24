#include "EngineInterface.hxx"
#include "Debug.hxx"
#include "../Engine/Core/ScriptingBridge.h"
#pragma comment(lib, "Engine.lib")

using namespace System;
using namespace System::IO;
using namespace System::Reflection;
using namespace System::Runtime::Loader;
using namespace System::Collections::Generic;

namespace ScriptAPI
{
    AssemblyLoadContext ^EngineInterface::s_ctx = nullptr;
    Assembly ^EngineInterface::s_asm = nullptr;
    List<ScriptList ^> ^EngineInterface::s_entityScripts = nullptr;
    Dictionary<String ^, Type ^> ^EngineInterface::s_scriptTypes = nullptr;

    static void Safe(Action ^a)
    {
        SAFE_NATIVE_CALL_BEGIN
            a();
        SAFE_NATIVE_CALL_END
    }

    void EngineInterface::Init()
    {
        s_ctx = gcnew AssemblyLoadContext(nullptr, true); // collectible for unload
        // Load the latest ManagedScripts.dll (engine copies it to CWD)
        FileStream ^fs = File::Open("ManagedScripts.dll", FileMode::Open, FileAccess::Read, FileShare::Read);
        s_asm = s_ctx->LoadFromStream(fs);
        fs->Close();

        const int cap = Core::ScriptingBridge::GetMaxEntities();
        s_entityScripts = gcnew List<ScriptList ^>(cap);
        for (int i = 0; i < cap; ++i) s_entityScripts->Add(gcnew ScriptList());

        BuildTypeMap();
    }

    void EngineInterface::Shutdown()
    {
        ClearState();
        if (s_ctx) { s_ctx->Unload(); s_ctx = nullptr; }
        GC::Collect(); GC::WaitForPendingFinalizers();
    }

    void EngineInterface::ClearState()
    {
        if (s_entityScripts) { for each(ScriptList ^ l in s_entityScripts) l->Clear(); s_entityScripts->Clear(); }
        s_scriptTypes = nullptr;
        s_asm = nullptr;
    }

    void EngineInterface::BuildTypeMap()
    {
        s_scriptTypes = gcnew Dictionary<String ^, Type ^>();
        for each(Type ^ t in s_asm->GetTypes())
        {
            if (!t->IsAbstract && t->IsClass && t->IsSubclassOf(Script::typeid))
                s_scriptTypes->Add(t->FullName, t);
        }
    }

    bool EngineInterface::AddScriptViaName(int entityId, String ^fullName)
    {
        bool ok = false;
        Safe(gcnew Action([&]()
            {
                Type ^type = nullptr;
                if (!s_scriptTypes->TryGetValue(fullName, type))
                {
                    System::Console::WriteLine("Script type not found: " + fullName);
                    return;
                }
                Script ^s = safe_cast<Script ^>(Activator::CreateInstance(type));
                s->SetEntityId(entityId);
                s_entityScripts[entityId]->Add(s);
                ok = true;
            }));
        return ok;
    }

    void EngineInterface::ExecuteUpdate(float dt)
    {
        for each(ScriptList ^ list in s_entityScripts)
        {
            for each(Script ^ s in list)
            {
                Safe(gcnew Action([&]() { s->Update(dt); }));
            }
        }
    }

    void EngineInterface::Reload()
    {
        ClearState();
        if (s_ctx) { s_ctx->Unload(); s_ctx = nullptr; }
        GC::Collect(); GC::WaitForPendingFinalizers();
        Init();
    }
}

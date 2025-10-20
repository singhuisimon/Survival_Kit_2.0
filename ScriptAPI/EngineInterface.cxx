/**
 * @file EngineInterface.cxx
 * @brief Implementation of the EngineInterface class managing the C++/CLI bridge
 *        between native engine and managed C# scripts.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Handles assembly loading and unloading through AssemblyLoadContext,
 *          script instantiation and registration to entities, script type
 *          discovery via reflection, and update execution for all scripts or
 *          individual entities. Supports hot-reload functionality.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "EngineInterface.hxx"
#include "../ScriptCore/Application.h"
#include "Debug.hxx"

using namespace System;
#pragma comment (lib, "ScriptCore.lib")

namespace ScriptAPI
{
    void EngineInterface::HelloWorld()
    {
        System::Console::WriteLine("Hello Managed World!");
        Core::Application::HelloWorld();
    }
    void EngineInterface::Init()
    {
        // Load assembly
        using namespace System::IO;
        loadContext =
            gcnew System::Runtime::Loader::AssemblyLoadContext(nullptr, true);
        // Load assembly
        FileStream^ managedLibFile = File::Open
        (
            "ManagedScripts.dll",
            FileMode::Open, FileAccess::Read, FileShare::Read
        );
        loadContext->LoadFromStream(managedLibFile);
        managedLibFile->Close();

        // Create our script storage
        scripts = gcnew System::Collections::Generic::List<ScriptList^>();
        for (int i = 0; i < Core::Application::ENTITY_COUNT; ++i)
        {
            scripts->Add(gcnew ScriptList());
        }

        // Populate list of types of scripts
        updateScriptTypeList();
    }
    void EngineInterface::Reload()
    {
        // Clear all references to types in the script assembly we are going to unload
        scripts->Clear();
        scriptTypeList = nullptr;
        // Unload
        loadContext->Unload();
        loadContext = nullptr;
        // Wait for unloading to finish
        System::GC::Collect();
        System::GC::WaitForPendingFinalizers();
        // Load the assembly again
        Init();
    }

    bool EngineInterface::AddScriptViaName(int entityId, System::String^ scriptName)
    {
        SAFE_NATIVE_CALL_BEGIN

        // Check if valid entity
        if (entityId < Core::Application::MIN_ENTITY_ID || entityId > Core::Application::MAX_ENTITY_ID)
            return false;

        // Remove any whitespaces just in case
        scriptName = scriptName->Trim();

        // Look for the correct script
        System::Type^ scriptType = nullptr;
        for each (System::Type ^ type in scriptTypeList)
        {
            if (type->FullName == scriptName || type->Name == scriptName)
            {
                scriptType = type;
                break;
            }
        }

        // Failed to get any script
        if (scriptType == nullptr) {
            System::Console::WriteLine("yur");
            return false;

        }
           

        // Create the script
        Script^ script = safe_cast<Script^>(System::Activator::CreateInstance(scriptType));
        script->SetEntityId(entityId);


        // Add the script
        scripts[entityId]->Add(script);
        return true;
        SAFE_NATIVE_CALL_END
            return false;

    }
    void EngineInterface::ExecuteUpdate()
    {
        //std::cout << "ExecuteUpdate called, scripts count: " << scripts->Count << std::endl;
        //System::Console::WriteLine("ExecuteUpdate called, scripts count:");

        for each (ScriptList ^ entityScriptList in scripts)
        {
            //std::cout << "Entity has " << entityScriptList->Count << " scripts" << std::endl;
           // System::Console::WriteLine("entityScriptList");

            for each (Script ^ script in entityScriptList)
            {
                //std::cout << "Calling Update on script: " << script->GetType()->Name << std::endl;
                //System::Console::WriteLine("yur");
                SAFE_NATIVE_CALL_BEGIN
                    script->Update();
                SAFE_NATIVE_CALL_END
                //script->Update();
                //std::cout << "Update completed" << std::endl;
            }
        }
    }

    namespace
    {
        /* Select Many */
        ref struct Pair
        {
            System::Reflection::Assembly^ assembly;
            System::Type^ type;
        };

        System::Collections::Generic::IEnumerable<System::Type^>^ selectorFunc(System::Reflection::Assembly^ assembly)
        {
            return assembly->GetExportedTypes();
        }
        Pair^ resultSelectorFunc(System::Reflection::Assembly^ assembly, System::Type^ type)
        {
            Pair^ p = gcnew Pair();
            p->assembly = assembly;
            p->type = type;
            return p;
        }

        /* Where */
        bool predicateFunc(Pair^ pair)
        {
            return pair->type->IsSubclassOf(Script::typeid) && !pair->type->IsAbstract;
        }

        /* Select */
        System::Type^ selectorFunc(Pair^ pair)
        {
            return pair->type;
        }
    }


    void EngineInterface::updateScriptTypeList()
    {
        using namespace System;
        using namespace System::Reflection;
        using namespace System::Linq;
        using namespace System::Collections::Generic;

        // Debug: Check loaded assemblies
        IEnumerable<Assembly^>^ assemblies = AppDomain::CurrentDomain->GetAssemblies();
        Console::WriteLine("Loaded assemblies:");
        for each (Assembly ^ assembly in assemblies)
        {
            Console::WriteLine("  - {0}", assembly->GetName()->Name);
        }

        /* Select Many: Types in Loaded Assemblies */
        Func<Assembly^, IEnumerable<Type^>^>^ collectionSelector = gcnew Func<Assembly^, IEnumerable<Type^>^>(selectorFunc);
        Func<Assembly^, Type^, Pair^>^ resultSelector = gcnew Func<Assembly^, Type^, Pair^>(resultSelectorFunc);
        IEnumerable<Pair^>^ selectManyResult = Enumerable::SelectMany(assemblies, collectionSelector, resultSelector);

        /* Where: Are concrete Scripts */
        Func<Pair^, bool>^ predicate = gcnew Func<Pair^, bool>(predicateFunc);
        IEnumerable<Pair^>^ whereResult = Enumerable::Where(selectManyResult, predicate);

        /* Select: Select them all */
        Func<Pair^, Type^>^ selector = gcnew Func<Pair^, Type^>(selectorFunc);
        scriptTypeList = Enumerable::Select(whereResult, selector);

        // Debug: Show discovered script types
        Console::WriteLine("Found script types:");
        for each (Type ^ type in scriptTypeList)
        {
            Console::WriteLine("  - Name: {0}, FullName: {1}", type->Name, type->FullName);
        }
    }

    void EngineInterface::ExecuteUpdateForEntity(int entityId)
    {
        SAFE_NATIVE_CALL_BEGIN

            // Validate entity ID
            if (entityId < 0 || entityId >= scripts->Count)
            {
                System::Console::WriteLine("ExecuteUpdateForEntity: Invalid entity ID {0}", entityId);
                return;
            }

        // Get the script list for this specific entity
        ScriptList^ entityScriptList = scripts[entityId];

        // Check if the entity has any scripts
        if (entityScriptList->Count == 0)
        {
            return;  // No scripts to update for this entity
        }

        // Update all scripts attached to this entity
        for each (Script ^ script in entityScriptList)
        {
            script->Update();
        }

        SAFE_NATIVE_CALL_END
    }



}
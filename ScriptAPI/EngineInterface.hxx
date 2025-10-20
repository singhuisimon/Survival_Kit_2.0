// EngineInterface.hxx
#pragma once

/**
 * @file EngineInterface.hxx
 * @brief Declaration of the EngineInterface class providing the C++/CLI bridge
 *        between native C++ engine code and managed C# scripts.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Manages script lifecycle including initialization, reloading, script
 *          attachment to entities, and update execution. Handles assembly loading
 *          through AssemblyLoadContext for hot-reload support.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "Script.hxx"
namespace ScriptAPI
{
    // ref classes are classes in C#, value classes are structs in C#
    public ref class EngineInterface
    {
    public:
        static void HelloWorld();
        static void Init();
        static void Reload();
        static bool AddScriptViaName(int entityId, System::String^ scriptName);
        static void ExecuteUpdate();
        // NEW METHOD - Updates only specific entity's scripts

        static void ExecuteUpdateForEntity(int entityId);

    private:
        using ScriptList = System::Collections::Generic::List<Script^>;
        static System::Runtime::Loader::AssemblyLoadContext^ loadContext;

        static System::Collections::Generic::List<ScriptList^>^ scripts;
        static System::Collections::Generic::IEnumerable<System::Type^>^ scriptTypeList;

        static void updateScriptTypeList();
    };
}
#pragma once
/**
 * @file Application.h
 * @brief Declaration of the Application class for managing the game engine's
 *        core systems including scripting, hot-reload, and entity management.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include <Windows.h>        // HMODULE
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <filesystem>

//file watching includes
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>

// Relative include as this file will be included from other projects later
#include "../External_Libraries/include/dotnet/coreclrhost.h"    // coreclr_*

#include "ImportExport.h"
#include "MonoBehaviour.h"


namespace Core
{

    class DLL_API TransformComponent
    {
    public:
        float x = 0.0f;
        float y = 0.0f;
    };

#pragma warning(push)
#pragma warning(disable:4251) //disable dll-interface warning for stl types

    class DLL_API Application
    {
    public:

        //need to replace with entity Manager
        static constexpr int ENTITY_COUNT = 64;
        static constexpr int MIN_ENTITY_ID = 0;
        static constexpr int MAX_ENTITY_ID = ENTITY_COUNT - 1;
        static TransformComponent* GetComponent(int entityId);
        void InitializeScripting();
        bool AddScript(int entityId, const char* scriptName);
        void UpdateScripts();
        void ShutdownScripting();
        void ReloadScripts();
        void Run(); // not using currently
        void CheckAndReloadScripts();

        static void HelloWorld();

        //file creation
        static bool CreateMonoBehaviourScript(const std::string& scriptName);
        static bool CreateScriptableObjectScript(const std::string& scriptName);
        static bool CreateScriptFromTemplate(const std::string& scriptName, const std::string& templateType);
        // File opening
        static bool OpenScriptInEditor(const std::string& scriptName);

        // Template management
        static std::string GetTemplatesDirectory();
        static bool InitializeTemplates();
        static std::vector<std::string> GetAvailableTemplateTypes();


        // Validation and utility methods
        static bool ValidateScriptName(const std::string& scriptName);
        static std::string GetManagedScriptsDirectory();
        static bool DoesScriptExist(const std::string& scriptName);
        static std::vector<std::string> GetExistingScriptFiles();

        // Utility methods
        static void ListExistingScripts();
        static void ShowScriptCreationHelp();

        void UpdateScriptForEntity(int entityId);  // NEW method



    private:
        static std::array<TransformComponent, ENTITY_COUNT> nativeData;
        static void compileScriptAssembly();
        void(*initFunc)() = nullptr;
        bool(*addScriptFunc)(int, const char*) = nullptr;
        void(*executeUpdateFunc)() = nullptr;
        void(*reloadScriptsFunc)() = nullptr;  // Add this
        void(*executeUpdateForEntityFunc)(int) = nullptr;  // NEW function pointer

        void startScriptEngine();
        void stopScriptEngine();
        std::string buildTpaList(const std::string& directory);



        // File watching members
        std::thread fileWatcherThread;
        std::atomic<bool> shouldStopWatching{ false };
        std::atomic<bool> scriptsNeedReload{ false };
        std::string scriptDirectory;
        std::unordered_map<std::string, std::filesystem::file_time_type> fileTimestamps;
        std::chrono::steady_clock::time_point lastCheck;

        // File watching methods
        void startFileWatcher();
        void stopFileWatcher();
        void fileWatcherLoop();
        bool checkForScriptChanges();
        void updateFileTimestamps();

        // References to CoreCLR key components
        HMODULE coreClr = nullptr;
        void* hostHandle = nullptr;
        unsigned int domainId = 0;
        // Function Pointers to CoreCLR functions
        coreclr_initialize_ptr      initializeCoreClr = nullptr;
        coreclr_create_delegate_ptr createManagedDelegate = nullptr;
        coreclr_shutdown_ptr        shutdownCoreClr = nullptr;

        // Helper Functions
        template<typename FunctType>
        FunctType getCoreClrFuncPtr(const std::string& functionName)
        {
            auto fPtr = reinterpret_cast<FunctType>(GetProcAddress(coreClr, functionName.c_str()));
            if (!fPtr)
                throw std::runtime_error("Unable to get pointer to function.");

            return fPtr;
        }
        template<typename FunctionType>
        FunctionType GetFunctionPtr(const std::string_view& assemblyName, const std::string_view& typeName, const std::string_view& functionName)
        {
            FunctionType managedDelegate = nullptr;
            int result = createManagedDelegate
            (
                hostHandle,
                domainId,
                assemblyName.data(),
                typeName.data(),
                functionName.data(),
                reinterpret_cast<void**>(&managedDelegate)
            );

            // Check if it failed
            if (result < 0)
            {

                std::ostringstream oss;
                oss << std::hex << std::setfill('0') << std::setw(8)
                    << "[DotNetRuntime] Failed to get pointer to function \""
                    << typeName << "." << functionName << "\" in assembly (" << assemblyName << "). "
                    << "Error 0x" << result << "\n";
                throw std::runtime_error(oss.str());
            }

            return managedDelegate;
        }
    };

#pragma warning(pop)

}
#pragma once
#include <string>
#include <filesystem>
#include <chrono>
#include <vector>
#include <unordered_map>

namespace Engine {

    class ScriptReloader {
    public:
        static ScriptReloader& GetInstance();

        // Initialize with paths
        void Initialize(
            const std::string& scriptsSourcePath,    // e.g., "Scripts/"
            const std::string& scriptProjectPath,     // e.g., "Scripts/GameScripts.csproj"
            const std::string& outputDllPath          // e.g., "GameScripts.dll"
        );

        // Call every frame to check for changes
        void Update();

        // Check if reload is needed
        bool IsReloadRequested() const { return m_ReloadRequested; }
        void ClearReloadRequest() { m_ReloadRequested = false; }

        // Check if build is in progress
        bool IsBuilding() const { return m_IsBuilding; }
        void FinalizeDllSwap();  //
        std::string GetTempDllPath() const { return m_TempDllPath; }
        void ClearTempDllPath() { m_TempDllPath.clear(); }


        void ClearReloadFlag() { m_ReloadRequested = false; }
    private:
        ScriptReloader() = default;
        ~ScriptReloader() = default;
        std::string m_TempDllPath;  // NEW: Store temp DLL path
        bool m_ReloadRequested = false;  // Add this if not already there

        void CheckForFileChanges();
        void StartBuild();
        void CheckBuildStatus();
        void CopyDllToOutput();

        std::string m_ScriptsSourcePath;
        std::string m_ScriptProjectPath;
        std::string m_OutputDllPath;

        // Track file modification times
        std::unordered_map<std::string, std::filesystem::file_time_type> m_FileModTimes;

        // Build state
        bool m_IsBuilding = false;
        float m_BuildStartTime = 0.0f;

        // Process handle for async build (platform-specific)
#ifdef _WIN32
        void* m_BuildProcessHandle = nullptr;  // HANDLE on Windows
#else
        int m_BuildProcessPid = -1;            // pid_t on Unix
#endif
    };

} // namespace Engine

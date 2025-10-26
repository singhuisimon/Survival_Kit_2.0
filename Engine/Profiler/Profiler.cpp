#include "Profiler.h"
#include <cstdlib>
#include <filesystem>

namespace Engine {

    TracyProfiler::~TracyProfiler() {
        if (m_processHandle) {

            // Ask Windows to kill the Tracy GUI process (force-kills)
            // remove should u want to test what happends when shutdown
            TerminateProcess(m_processHandle, 0);

            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }
        m_running = false;
    }

    void TracyProfiler::SetTracyPath(const std::string& exename) {
        m_tracyPath = exename;// getTracyFilePath(exename);
    }

    void TracyProfiler::LaunchTracy() {
        LOG_DEBUG("LaunchTracy() Running");

#ifdef TRACY_ENABLE
        m_TracyProfiler->LaunchTracy(); // Optional: auto-launch GUI
        LOG_INFO("  -> Tracy profiler launched successfully");
#else
        LOG_WARNING("  -> TRACY_ENABLE not defined. Skipping profiler launch.");
#endif

#ifndef TRACY_ENABLE
        return;
#else
        // Prevent multiple concurrent profiler instances
        if (m_running && m_processHandle) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode == STILL_ACTIVE) {
                return; // still running
            }
            else {
                // Process was closed
                CloseHandle(m_processHandle);
                m_processHandle = nullptr;
                m_running = false;
            }
        }

        // Verify that the executable path is valid
        if (m_tracyPath.empty() || !std::filesystem::exists(m_tracyPath)) {
            return;
        }

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        // Auto-connect to localhost game
        std::string args = "\"" + m_tracyPath + "\" -a 127.0.0.1 -capture";

        if (CreateProcessA(
            nullptr, (LPSTR)args.c_str(),
            nullptr, nullptr, FALSE,
            0, nullptr, nullptr,
            &si, &pi))
        {
            
            CloseHandle(pi.hThread);
            m_processHandle = pi.hProcess;
            m_running = true;
        }
#endif
    }

    void TracyProfiler::OnUpdate() {
        if (m_running && m_processHandle) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode != STILL_ACTIVE) {
                // Process has ended
                CloseHandle(m_processHandle);
                m_processHandle = nullptr;
                m_running = false;
            }
        }
    }

}

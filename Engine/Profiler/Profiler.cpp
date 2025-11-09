/**
* @file Profiler.h
* @brief Implementation of the functions of the TracyProfiler class for running Tracy.exe.
* @author Amanda Leow Boon Suan (70%), Liliana Hanawardani (30%)
* @date September 8, 2025
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/
#include "Profiler.h"
#include <cstdlib>
#include <filesystem>

namespace Engine {

    TracyProfiler::~TracyProfiler() {
        LOG_DEBUG("TracyProfiler destructor called!");
        Shutdown();
    }

    void TracyProfiler::SetTracyPath(const std::string& exename) {
        // getTracyFilePath(exename);
        m_tracyPath = exename;
        LOG_DEBUG("m_tracyPath: %s", m_tracyPath);
    }

    void TracyProfiler::LaunchTracy() {
        LOG_DEBUG("LaunchTracy() Running");
#ifndef TRACY_ENABLE
        return;
#else
        // Prevent multiple profiler instances
        if (m_running && m_processHandle) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode == STILL_ACTIVE) {
                return; // still running
            }
            else {
                // Process closed
                CloseHandle(m_processHandle);
                m_processHandle = nullptr;
                m_running = false;
            }
        }

        // Verify valid executable path
        if (m_tracyPath.empty() || !std::filesystem::exists(m_tracyPath)) {
            return;
        }

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        // Auto-connect to localhost game
        std::string args = "\"" + m_tracyPath + "\" -a 127.0.0.1 -capture";

        if (!CreateProcessA(
            nullptr, (LPSTR)args.c_str(),
            nullptr, nullptr, FALSE,
            0, nullptr, nullptr,
            &si, &pi))
        {
            LOG_ERROR("LaunchTracy() Failed to launch Tracy Profiler at: %s", m_tracyPath.c_str());
            return;
        }

        LOG_DEBUG("LaunchTracy() Successfully launched Tracy Profiler.");
        CloseHandle(pi.hThread);
        m_processHandle = pi.hProcess;
        m_running = true;

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

    void TracyProfiler::Shutdown() {
        if (m_processHandle) {

            // Force terminates Tracy GUI process
            TerminateProcess(m_processHandle, 0);

            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }
        m_running = false;
    }

}

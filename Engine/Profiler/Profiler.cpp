#include "Profiler.h"
#include <cstdlib>
#include <filesystem>
//#include <tlhelp32.h> // for CreateToolhelp32Snapshot

namespace Engine {

    TracyProfiler::~TracyProfiler() {
        LOG_DEBUG("TracyProfiler destructor called!");
        Shutdown();
    }

    void TracyProfiler::SetTracyPath(const std::string& exename) {
        m_tracyPath = exename;// getTracyFilePath(exename);
        LOG_DEBUG("m_tracyPath: %s", m_tracyPath);
    }

    void TracyProfiler::LaunchTracy() {
        LOG_DEBUG("LaunchTracy() Running");
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


    /*void TracyProfiler::Shutdown()
    {
        if (m_processHandle)
        {
            DWORD launcherPid = GetProcessId(m_processHandle);

            // Try to find any Tracy.exe processes spawned by this one
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot != INVALID_HANDLE_VALUE)
            {
                PROCESSENTRY32 entry = { sizeof(PROCESSENTRY32) };
                if (Process32First(snapshot, &entry))
                {
                    do
                    {
                        if (_stricmp(entry.szExeFile, "Tracy.exe") == 0)
                        {
                            // Check if this process was spawned by the launcher PID
                            if (entry.th32ParentProcessID == launcherPid)
                            {
                                HANDLE child = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                                if (child)
                                {
                                    LOG_DEBUG("Terminating Tracy child process PID: %lu (parent %lu)", entry.th32ProcessID, launcherPid);
                                    TerminateProcess(child, 0);
                                    CloseHandle(child);
                                }
                            }
                        }
                    } while (Process32Next(snapshot, &entry));
                }
                CloseHandle(snapshot);
            }

            // Kill the launcher process (if still active)
            LOG_DEBUG("Terminating Tracy launcher process PID: %lu", launcherPid);
            TerminateProcess(m_processHandle, 0);
            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }

        m_running = false;
        LOG_DEBUG("TracyProfiler::Shutdown() complete");
    }*/


    void TracyProfiler::Shutdown() {
        if (m_processHandle) {

            // Ask Windows to kill the Tracy GUI process (force-kills)
            // remove should u want to test what happends when shutdown
            TerminateProcess(m_processHandle, 0);

            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }
        m_running = false;
    }


}

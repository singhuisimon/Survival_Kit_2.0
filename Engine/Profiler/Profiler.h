#pragma once
#ifndef __TRACY_PROFILER_H__
#define __TRACY_PROFILER_H__

#include <string>
#include "../Utility/Logger.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Engine {

    /**
     * @class TracyProfiler
     * @brief Handles lifecycle management of the Tracy Profiler.
     * @details
     * The TracyProfiler is responsible for launching the Tracy GUI as an external
     * process and monitoring its state during the engine runtime. It uses Windows API
     * functions to create, track, and terminate the profiler process.
     */
    class TracyProfiler {
    private:

        //---------------------- Internal Data Members ----------------------//

        std::string m_tracyPath;    ///< Path to the Tracy profiler executable.
        bool m_running = false;     ///< Indicates if Tracy profiler is currently running.
        HANDLE m_processHandle;     ///< Handle to the Tracy profiler process (Windows-specific).

    public:

        /**************************************************************************
        * @brief
        * Default constructor (private for singleton pattern).
        **************************************************************************/
        TracyProfiler() : m_processHandle(nullptr) {};

        /**************************************************************************
        * @brief
        * Destructor that ensures profiler process is properly terminated.
        **************************************************************************/
        ~TracyProfiler();

        // Delete copy and assignment operators to enforce singleton behavior
        TracyProfiler(const TracyProfiler&) = delete;
        TracyProfiler& operator=(const TracyProfiler&) = delete;

        /**************************************************************************
        * @brief Sets the file path to the Tracy profiler executable.
        * @param exename
        * Absolute or relative path to the Tracy executable file.
        **************************************************************************/
        void SetTracyPath(const std::string& exename);

        /**************************************************************************
        * @brief
        * Launches the Tracy profiler as an external process if enabled and not running.
        * @note
        * This function only executes if `TRACY_ENABLE` is defined in the build configuration.
        **************************************************************************/
        void LaunchTracy();

        /**************************************************************************
        * @brief Monitors the profiler process and resets state if the process closes.
        **************************************************************************/
        void OnUpdate();

        void Shutdown();

        /**************************************************************************
        * @brief
        * Checks if the Tracy profiler process is currently active.
        * @return
        * True if the profiler is running; otherwise, false.
        **************************************************************************/
        bool isRunning() const { return m_running; }

    };

} // end of gam300

#endif 
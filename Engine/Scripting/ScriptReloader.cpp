#include "ScriptReloader.h"
#include "../Utility/Logger.h"
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#include "../Core/Timestep.h"

#endif

namespace Engine
{

	ScriptReloader &ScriptReloader::GetInstance()
	{
		static ScriptReloader instance;
		return instance;
	}

	void ScriptReloader::Initialize(
		const std::string &scriptsSourcePath,
		const std::string &scriptProjectPath,
		const std::string &outputDllPath)
	{
		m_ScriptsSourcePath = scriptsSourcePath;
		m_ScriptProjectPath = scriptProjectPath;
		m_OutputDllPath = outputDllPath;

		LOG_INFO("[ScriptReloader] === INITIALIZATION ===");
		LOG_INFO("  Source path: ", m_ScriptsSourcePath);
		LOG_INFO("  Project: ", m_ScriptProjectPath);
		LOG_INFO("  Output DLL: ", m_OutputDllPath);

		// Convert to absolute path for debugging
		std::filesystem::path absoluteSourcePath = std::filesystem::absolute(m_ScriptsSourcePath);
		LOG_INFO("  Absolute source path: ", absoluteSourcePath.string());

		// Check if source path exists
		if (!std::filesystem::exists(m_ScriptsSourcePath))
		{
			LOG_ERROR("   Source path does NOT exist!");
			return;
		}

		LOG_INFO("   Source path exists");

		// List ALL files in the directory (for debugging)
		LOG_INFO("  Listing all files in Scripts directory:");
		try
		{
			int totalFiles = 0;
			for (const auto &entry : std::filesystem::recursive_directory_iterator(m_ScriptsSourcePath))
			{
				totalFiles++;
				std::string ext = entry.path().extension().string();
				LOG_INFO("    - ", entry.path().string(), " (ext: ", ext, ")");
			}
			LOG_INFO("  Total files found (all types): ", totalFiles);
		}
		catch (const std::exception &e)
		{
			LOG_ERROR("  Error listing directory: ", e.what());
		}

		// Now scan for .cs files specifically
		int csFileCount = 0;
		try
		{
			for (const auto &entry : std::filesystem::recursive_directory_iterator(m_ScriptsSourcePath))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".cs")
				{
					std::string filepath = entry.path().string();
					auto modTime = std::filesystem::last_write_time(entry.path());

					m_FileModTimes[filepath] = modTime;
					csFileCount++;

					LOG_INFO("     [", csFileCount, "] Watching: ", filepath);
				}
			}
		}
		catch (const std::exception &e)
		{
			LOG_ERROR("  Error scanning for .cs files: ", e.what());
		}

		LOG_INFO("  === Total .cs files found: ", csFileCount, " ===");

		if (csFileCount == 0)
		{
			LOG_WARNING("    No .cs files found! Check directory structure.");
		}
	}


	void ScriptReloader::Update()
	{
		// Debug: Log every 5 seconds
		static int updateCount = 0;
		updateCount++;

		if (updateCount == 1)
		{
			LOG_INFO("[ScriptReloader] Update() is being called!");
		}

		if (updateCount % 300 == 0)
		{  // Every 5 sec at 60fps
			LOG_INFO("[ScriptReloader] Still checking... (", m_FileModTimes.size(), " files tracked)");
		}

		// If building, check if build finished
		if (m_IsBuilding)
		{
			CheckBuildStatus();
			return;
		}

		// Check for file changes
		CheckForFileChanges();
	}


	void ScriptReloader::CheckForFileChanges()
	{
		if (!std::filesystem::exists(m_ScriptsSourcePath))
		{
			return;
		}

		bool anyFileChanged = false;

		// Check all tracked files
		for (auto &[filepath, lastModTime] : m_FileModTimes)
		{
			if (!std::filesystem::exists(filepath))
			{
				continue;
			}

			try
			{
				auto currentModTime = std::filesystem::last_write_time(filepath);
				if (currentModTime != lastModTime)
				{
					LOG_INFO("[Hot-Reload] File changed: ", filepath);
					lastModTime = currentModTime;
					anyFileChanged = true;
				}
			}
			catch (const std::exception &e)
			{
				// File might be locked, skip
				(void)e;
			}
		}

		// Check for new files
		for (const auto &entry : std::filesystem::recursive_directory_iterator(m_ScriptsSourcePath))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".cs")
			{
				std::string filepath = entry.path().string();
				if (m_FileModTimes.find(filepath) == m_FileModTimes.end())
				{
					LOG_INFO("[Hot-Reload] New file detected: ", filepath);
					m_FileModTimes[filepath] = std::filesystem::last_write_time(entry.path());
					anyFileChanged = true;
				}
			}
		}

		if (anyFileChanged)
		{
			LOG_INFO("[Hot-Reload] C# files changed, starting build...");
			StartBuild();
		}
	}

	void ScriptReloader::StartBuild()
	{
		m_IsBuilding = true;

#ifdef _WIN32
		// Windows: Use CreateProcess to run dotnet build asynchronously
		std::string buildCommand = "dotnet build \"" + m_ScriptProjectPath + "\" --configuration Debug";

		STARTUPINFOA si = { sizeof(si) };
		PROCESS_INFORMATION pi = {};

		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;  // Hide console window

		if (!CreateProcessA(
			nullptr,
			const_cast<char *>(buildCommand.c_str()),
			nullptr, nullptr, FALSE,
			CREATE_NO_WINDOW,
			nullptr, nullptr,
			&si, &pi))
		{
			LOG_ERROR("[Hot-Reload] Failed to start build process");
			m_IsBuilding = false;
			return;
		}

		m_BuildProcessHandle = pi.hProcess;
		CloseHandle(pi.hThread);  // Don't need thread handle

		LOG_INFO("[Hot-Reload] Build started (async)...");
#else
		// Unix: Use fork/exec
		pid_t pid = fork();
		if (pid == 0)
		{
			// Child process
			execlp("dotnet", "dotnet", "build", m_ScriptProjectPath.c_str(), "--configuration", "Debug", nullptr);
			exit(1);  // If exec fails
		}
		else if (pid > 0)
		{
			m_BuildProcessPid = pid;
			LOG_INFO("[Hot-Reload] Build started (async)...");
		}
		else
		{
			LOG_ERROR("[Hot-Reload] Failed to fork build process");
			m_IsBuilding = false;
		}
#endif
	}

	void ScriptReloader::CheckBuildStatus()
	{
#ifdef _WIN32
		if (!m_BuildProcessHandle)
		{
			return;
		}

		DWORD exitCode;
		if (GetExitCodeProcess(m_BuildProcessHandle, &exitCode))
		{
			if (exitCode == STILL_ACTIVE)
			{
				return;  // Still building
			}

			// Build finished
			CloseHandle(m_BuildProcessHandle);
			m_BuildProcessHandle = nullptr;
			m_IsBuilding = false;

			if (exitCode == 0)
			{
				LOG_INFO("[Hot-Reload] Build succeeded!");
				CopyDllToOutput();
				m_ReloadRequested = true;
			}
			else
			{
				LOG_ERROR("[Hot-Reload] Build failed with code: ", exitCode);
			}
		}
#else
		if (m_BuildProcessPid < 0)
		{
			return;
		}

		int status;
		pid_t result = waitpid(m_BuildProcessPid, &status, WNOHANG);

		if (result == 0)
		{
			return;  // Still building
		}

		if (result > 0)
		{
			m_BuildProcessPid = -1;
			m_IsBuilding = false;

			if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			{
				LOG_INFO("[Hot-Reload] Build succeeded!");
				CopyDllToOutput();
				m_ReloadRequested = true;
			}
			else
			{
				LOG_ERROR("[Hot-Reload] Build failed");
			}
		}
#endif
	}

	void ScriptReloader::CopyDllToOutput()
	{

		// Base dirs
		const auto scriptProj = std::filesystem::weakly_canonical(m_ScriptProjectPath);
		const auto scriptsDir = scriptProj.parent_path();                 // .../Scripts
		const auto exeDir = std::filesystem::current_path();          // or your GetExecutableDir()

#if defined(_DEBUG) || defined(DEBUG)
		constexpr const char *CFG = "Debug";
#else
		constexpr const char *CFG = "Release";
#endif

		std::vector<std::filesystem::path> possiblePaths = {
			scriptsDir / "obj" / CFG / "GameScripts.dll",  // Scripts/obj/Debug-or-Release/GameScripts.dll
			scriptsDir / "obj" / "Debug" / "GameScripts.dll",
			scriptsDir / "obj" / "Release" / "GameScripts.dll",
			exeDir / "GameScripts.dll"                 // fallback if you copy beside the exe
		};

		std::filesystem::path sourceDll;
		bool found = false;

		for (const auto &path : possiblePaths)
		{
			if (std::filesystem::exists(path))
			{
				sourceDll = path;
				found = true;
				LOG_INFO("[Hot-Reload] Found DLL at: ", path.string());
				break;
			}
		}

		if (!found)
		{
			LOG_ERROR("[Hot-Reload] Built DLL not found at: ", sourceDll.string(), "!");
			return;
		}

		try
		{
			// Copy to TEMP file (avoid locking issues with Mono)
			std::string tempDllPath = m_OutputDllPath + ".tmp";
			std::filesystem::copy_file(sourceDll, tempDllPath,
				std::filesystem::copy_options::overwrite_existing);

			LOG_INFO("[Hot-Reload] DLL copied to temp: ", tempDllPath);

			// Store temp path for swapping AFTER assembly unload
			m_TempDllPath = tempDllPath;
			
		}
		catch (const std::exception &e)
		{
			LOG_ERROR("[Hot-Reload] Failed to copy DLL: ", e.what());
		}
	}





} // namespace Engine

#include "ScriptingSystem.h"
#include "ScriptComponent.h"

#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>

#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

using AddScriptFn = bool(*)(int, const wchar_t *);
static constexpr const char *ASM_SCRIPTAPI = "ScriptAPI";
static constexpr const char *TYPE_ENGINEIF = "ScriptAPI.EngineInterface";

namespace
{
    template<typename FnT>
    FnT GetProc(HMODULE mod, const char *name)
    {
        auto p = reinterpret_cast<FnT>(::GetProcAddress(mod, name));
        if (!p) throw std::runtime_error(std::string("Missing export: ") + name);
        return p;
    }

    std::string GetExeDir()
    {
        char path[MAX_PATH]{};
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        PathRemoveFileSpecA(path);
        return std::string(path);
    }

    std::string BuildTPAList(const std::string &dir)
    {
        std::ostringstream oss;
        WIN32_FIND_DATAA fd{};
        HANDLE h = FindFirstFileA((dir + "\\*.dll").c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            do oss << dir << '\\' << fd.cFileName << ';';
            while (FindNextFileA(h, &fd));
            FindClose(h);
        }
        return oss.str();
    }

    template<typename Fn>
    Fn GetManaged(void *host, unsigned domain, sk2::ScriptingSystem::create_delegate_t createDel,
        const char *asmName, const char *typeName, const char *fnName)
    {
        Fn f = nullptr;
        const int hr = createDel(host, domain, asmName, typeName, fnName, reinterpret_cast<void **>(&f));
        if (hr < 0)
        {
            std::ostringstream oss;
            oss << "create_delegate failed for " << typeName << "::" << fnName
                << " (hr=0x" << std::hex << hr << ")";
            throw std::runtime_error(oss.str());
        }
        return f;
    }
}

namespace sk2
{
    void ScriptingSystem::Init(const std::string &runtimeDir, const std::string &scriptsCsproj)
    {
        m_runtimeDir = runtimeDir.empty() ? GetExeDir() : runtimeDir;
        m_scriptsProj = scriptsCsproj;

        startCoreClr();   // CoreCLR hosting per tutorial Part 3
        bindDelegates();  // Resolve ScriptAPI.EngineInterface static methods
        if (m_EngineInit) m_EngineInit();
    }

    void ScriptingSystem::Shutdown()
    {
        if (m_EngineShutdown) m_EngineShutdown();
        stopCoreClr();
    }

    void ScriptingSystem::Update(entt::registry &reg, float dt)
    {
        auto view = reg.view<sk2::ScriptComponent>();
        for (auto e : view)
        {
            auto &sc = view.get<sk2::ScriptComponent>(e);
            if (!sc.Bound && addScriptsForEntity(e, reg))
                sc.Bound = true;
        }
        if (m_EngineExecuteUpdate) m_EngineExecuteUpdate(dt);
    }

    bool ScriptingSystem::HotReload()
    {
        compileScripts();         // invoke dotnet build & copy out managed DLL (Part 7)
        if (m_EngineReload) m_EngineReload(); // collectible ALC unload + reload (Part 7)
        return true;
    }

    // ---------------- CoreCLR host ----------------
    void ScriptingSystem::startCoreClr()
    {
        const std::string coreclrPath = m_runtimeDir + "\\coreclr.dll";
        HMODULE mod = ::LoadLibraryA(coreclrPath.c_str());
        if (!mod) throw std::runtime_error("Failed to load coreclr.dll from " + m_runtimeDir);
        m_coreclr = mod;

        m_initialize = GetProc<initialize_fn_t>(mod, "coreclr_initialize");
        m_createDelegate = GetProc<create_delegate_t>(mod, "coreclr_create_delegate");
        m_shutdown = GetProc<shutdown_fn_t>(mod, "coreclr_shutdown");

        const std::string tpa = BuildTPAList(m_runtimeDir);
        const std::string appPath = GetExeDir();
        const char *keys[] = { "TRUSTED_PLATFORM_ASSEMBLIES", "APP_PATHS" };
        const char *vals[] = { tpa.c_str(), appPath.c_str() };

        int hr = m_initialize(appPath.c_str(), "SK2Host",
            2, keys, vals, &m_host, &m_domain);
        if (hr < 0)
        {
            std::ostringstream oss;
            oss << "coreclr_initialize failed, hr=0x" << std::hex << hr;
            throw std::runtime_error(oss.str());
        }
    }

    void ScriptingSystem::stopCoreClr()
    {
        if (m_shutdown && m_host) m_shutdown(m_host, m_domain);
        if (m_coreclr) { ::FreeLibrary(static_cast<HMODULE>(m_coreclr)); m_coreclr = nullptr; }
        m_host = nullptr; m_domain = 0;
    }

    void ScriptingSystem::bindDelegates()
    {
        m_EngineInit = GetManaged<decltype(m_EngineInit)>(m_host, m_domain, m_createDelegate, ASM_SCRIPTAPI, TYPE_ENGINEIF, "Init");
        m_EngineShutdown = GetManaged<decltype(m_EngineShutdown)>(m_host, m_domain, m_createDelegate, ASM_SCRIPTAPI, TYPE_ENGINEIF, "Shutdown");
        m_EngineExecuteUpdate = GetManaged<decltype(m_EngineExecuteUpdate)>(m_host, m_domain, m_createDelegate, ASM_SCRIPTAPI, TYPE_ENGINEIF, "ExecuteUpdate");
        m_EngineReload = GetManaged<decltype(m_EngineReload)>(m_host, m_domain, m_createDelegate, ASM_SCRIPTAPI, TYPE_ENGINEIF, "Reload");
        m_EngineAddScriptByName = GetManaged<AddScriptFn>(m_host, m_domain, m_createDelegate, ASM_SCRIPTAPI, TYPE_ENGINEIF, "AddScriptViaName");
    }

    bool ScriptingSystem::addScriptsForEntity(entt::entity e, entt::registry &reg)
    {
        auto id = static_cast<int>(e);
        auto &sc = reg.get<sk2::ScriptComponent>(e);
        bool allOK = true;
        for (const std::string &name : sc.Scripts)
        {
            std::wstring wname(name.begin(), name.end());
            bool ok = m_EngineAddScriptByName ? m_EngineAddScriptByName(id, wname.c_str()) : false;
            allOK = allOK && ok;
        }
        return allOK;
    }

    void ScriptingSystem::compileScripts()
    {
        if (m_scriptsProj.empty()) return;

        std::wstring cmd = L" build \"";
        cmd += std::filesystem::absolute(m_scriptsProj).wstring();
        cmd += L"\" -c Debug --no-self-contained -o \"./tmp_build/\" -r \"win-x64\"";

        STARTUPINFOW si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);

        std::wstring full = L"\"C:\\Program Files\\dotnet\\dotnet.exe\"" + cmd; // matches tutorial guidance (Part 7)
        if (!CreateProcessW(nullptr, full.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
            throw std::runtime_error("Failed to launch dotnet build");

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);

        if (exitCode != 0) throw std::runtime_error("ManagedScripts build failed");

        std::filesystem::copy_file("./tmp_build/ManagedScripts.dll", "./ManagedScripts.dll",
            std::filesystem::copy_options::overwrite_existing);
    }
}

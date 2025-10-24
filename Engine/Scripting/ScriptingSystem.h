#pragma once
#include <string>
#include <entt/entt.hpp>

namespace sk2
{
    class ScriptingSystem
    {
    public:
        // runtimeDir: folder containing coreclr.dll (or use installed runtime variant)
        // scriptsCsproj: path to Scripting/ManagedScripts/ManagedScripts.csproj
        void Init(const std::string &runtimeDir, const std::string &scriptsCsproj);
        void Shutdown();

        void Update(entt::registry &reg, float dt);
        bool HotReload(); // dotnet build + collectible ALC reload

    private:
        using initialize_fn_t = int(*)(const char *, const char *, int, const char *[], const char *[], void **, unsigned int *);
        using create_delegate_t = int(*)(void *, unsigned, const char *, const char *, const char *, void **);
        using shutdown_fn_t = int(*)(void *, unsigned);

        void *m_coreclr{ nullptr };
        void *m_host{ nullptr };
        unsigned m_domain{ 0 };
        initialize_fn_t   m_initialize{ nullptr };
        create_delegate_t m_createDelegate{ nullptr };
        shutdown_fn_t     m_shutdown{ nullptr };

        std::string m_runtimeDir;
        std::string m_scriptsProj;

        // ScriptAPI.EngineInterface (managed) delegates
        void (*m_EngineInit)(void) { nullptr };
        void (*m_EngineShutdown)(void) { nullptr };
        void (*m_EngineExecuteUpdate)(float) { nullptr };
        bool (*m_EngineAddScriptByName)(int, const wchar_t *) { nullptr };
        void (*m_EngineReload)(void) { nullptr };

        void startCoreClr();
        void stopCoreClr();
        void bindDelegates();
        bool addScriptsForEntity(entt::entity e, entt::registry &reg);
        void compileScripts(); // runs: dotnet build ManagedScripts
    };
}

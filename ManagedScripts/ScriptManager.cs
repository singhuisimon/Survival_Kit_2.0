using System;
using System.Collections.Generic;
using ScriptAPI;

namespace ManagedScripts
{
    public static class ScriptManager
    {
        private static Dictionary<uint, ScriptBase> s_ActiveScripts = new();
        private static Dictionary<string, Type> s_ScriptTypes = new();

        static ScriptManager()
        {
            // Register available script types
            s_ScriptTypes["PlayerController"] = typeof(Scripts.PlayerController);
            s_ScriptTypes["RotatorScript"] = typeof(Scripts.RotatorScript);
        }

        public static void OnScriptCreate(uint entityId, string scriptName)
        {
            if (s_ScriptTypes.TryGetValue(scriptName, out var type))
            {
                var script = (ScriptBase)Activator.CreateInstance(type);
                script.SetEntityID(entityId);
                s_ActiveScripts[entityId] = script;
                script.OnCreate();
            }
        }

        public static void OnScriptUpdate(uint entityId, float deltaTime)
        {
            if (s_ActiveScripts.TryGetValue(entityId, out var script))
            {
                script.OnUpdate(deltaTime);
            }
        }

        public static void OnScriptDestroy(uint entityId)
        {
            if (s_ActiveScripts.TryGetValue(entityId, out var script))
            {
                script.OnDestroy();
                s_ActiveScripts.Remove(entityId);
            }
        }
    }
}

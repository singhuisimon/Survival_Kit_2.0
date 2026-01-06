using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Managed event system facade for scripts.
    /// Scripts can subscribe to named events and publish events
    /// that are visible to both scripts and native engine systems.
    /// </summary>
    public static class Event
    {
        public delegate void ScriptEventHandler(string name, string payload);

        // Native binding (register as: "Engine.EventSystem::Event_Publish")
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Event_Publish(string name, string payload);

        // Managed subscribers per event name
        private static readonly Dictionary<string, List<ScriptEventHandler>> s_Handlers =
            new Dictionary<string, List<ScriptEventHandler>>();

        public static void Subscribe(string eventName, ScriptEventHandler handler)
        {
            if (string.IsNullOrEmpty(eventName) || handler == null)
                return;

            if (!s_Handlers.TryGetValue(eventName, out var list))
            {
                list = new List<ScriptEventHandler>();
                s_Handlers[eventName] = list;
            }

            list.Add(handler);
        }

        public static void Unsubscribe(string eventName, ScriptEventHandler handler)
        {
            if (string.IsNullOrEmpty(eventName) || handler == null)
                return;

            if (s_Handlers.TryGetValue(eventName, out var list))
                list.Remove(handler);
        }

        /// <summary>
        /// Publish a script event into the native event system.
        /// </summary>
        public static void Publish(string eventName, string payload = "")
        {
            if (eventName == null) eventName = string.Empty;
            if (payload == null) payload = string.Empty;

            Event_Publish(eventName, payload);
        }

        /// <summary>
        /// Called by the engine for each ScriptEvent that was dispatched.
        /// Invoked from native code via Mono. Do not call this directly.
        /// </summary>
        public static void RaiseFromNative(string eventName, string payload)
        {
            if (string.IsNullOrEmpty(eventName))
                return;

            if (!s_Handlers.TryGetValue(eventName, out var list) || list.Count == 0)
                return;

            // Work on a snapshot in case handlers mutate subscription lists
            var snapshot = list.ToArray();
            for (int i = 0; i < snapshot.Length; ++i)
                snapshot[i]?.Invoke(eventName, payload);
        }
    }
}

using System;
using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Internal calls to C++ engine functions
    /// These are implemented in C++ and registered via mono_add_internal_call
    /// </summary>
    public static class InternalCalls
    {
        // Logging
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LogError(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LogWarning(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Scene_FindEntityByName(string name);

        // Transform
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_GetPosition(uint entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_SetPosition(uint entityID, ref Vector3 position);
        // Add this line:
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Transform_Move(uint entityID, float deltaX, float deltaY, float deltaZ);
    }

}

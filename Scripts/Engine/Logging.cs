using System;
using System.Runtime.CompilerServices;

namespace Engine
{
    public static class Log
    {
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogMessage(string message);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogError(string message);
        [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogWarning(string message);
    }
}

using System.Runtime.CompilerServices;

namespace Engine
{
    public class TrailSystem : Component
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Trail_SetStartColor(uint entityID, ref Vector4 color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Trail_SetEndColor(uint entityID, ref Vector4 color);

        public static void SetStartColor(uint entityID, ref Vector4 color) => Trail_SetStartColor(entityID, ref color);
        public static void SetEndColor(uint entityID, ref Vector4 color) => Trail_SetEndColor(entityID, ref color);
    }
}

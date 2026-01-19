using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Engine
{
    public static class Collision2D
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsPointInEntity(uint entityId, ref Vector2 point);

        public static bool IsMouseCollidingWithEntity(uint entityId)
        {
            Vector2 mousePos = Input.GetMousePosition();
            return IsPointInEntity(entityId, ref mousePos);
        }
    }
}
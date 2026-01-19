using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Engine
{
    public static class Collision2D
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool CollisionSystem2D_IsPointInEntity(ulong entityId, ref Vector2 point);

        public static bool IsMouseCollidingWithEntity(Entity e)
        {
            Vector2 mousePos = Input.GetMousePosition();
            return CollisionSystem2D_IsPointInEntity(e.EntityID, ref mousePos);
        }
    }
}
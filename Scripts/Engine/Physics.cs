using System.Runtime.CompilerServices;

namespace Engine
{
    public static class Physics
    {
        // Register as: "Engine.Physics::Physics_*"
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Physics_EnableCollisionEvents();
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Physics_BeginCollisionFrame();
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern int Physics_GetCollisionCount();
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Physics_GetCollisionPair(int index, out uint a, out uint b);

        public static void PhysicsEnableCollisionEvents() => Physics_EnableCollisionEvents();
        public static void PhysicsBeginCollisionFrame() => Physics_BeginCollisionFrame();
        public static int PhysicsGetCollisionCount() => Physics_GetCollisionCount();
        public static void PhysicsGetCollisionPair(int index, out uint a, out uint b) => Physics_GetCollisionPair(index, out a, out b);
    }
}

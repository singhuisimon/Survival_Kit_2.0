using System.Runtime.CompilerServices;

namespace Engine
{
    public struct SphereCastHit
    {
        public uint    EntityID;  ///< Entity that was hit
        public Vector3 Point;     ///< World-space contact point on the hit surface
        public Vector3 Normal;    ///< World-space surface normal pointing toward the caster
        public float   Fraction;  ///< [0..1] fraction along (direction * maxDistance)
    }

    public static class Physics
    {
        // Register as: "Engine.Physics::Physics_*"
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Physics_EnableCollisionEvents();
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Physics_BeginCollisionFrame();
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern int Physics_GetCollisionCount();
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Physics_GetCollisionPair(int index, out uint a, out uint b);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Physics_SphereCast(
            ref Vector3 origin, ref Vector3 direction, float radius, float maxDistance,
            uint excludeEntityID,
            out uint outEntityID, out Vector3 outPoint, out Vector3 outNormal, out float outFraction);

        public static void PhysicsEnableCollisionEvents() => Physics_EnableCollisionEvents();
        public static void PhysicsBeginCollisionFrame() => Physics_BeginCollisionFrame();
        public static int PhysicsGetCollisionCount() => Physics_GetCollisionCount();
        public static void PhysicsGetCollisionPair(int index, out uint a, out uint b) => Physics_GetCollisionPair(index, out a, out b);

        /// <summary>
        /// Sweeps a sphere from <paramref name="origin"/> along <paramref name="direction"/>,
        /// returning the closest hit within <paramref name="maxDistance"/>.
        /// Pass <paramref name="excludeEntityID"/> to ignore a specific entity (e.g. the caster).
        /// </summary>
        public static bool SphereCast(Vector3 origin, Vector3 direction, float radius, float maxDistance,
                                      uint excludeEntityID, out SphereCastHit hit)
        {
            bool result = Physics_SphereCast(ref origin, ref direction, radius, maxDistance,
                excludeEntityID,
                out uint entityID, out Vector3 point, out Vector3 normal, out float fraction);
            hit = new SphereCastHit { EntityID = entityID, Point = point, Normal = normal, Fraction = fraction };
            return result;
        }

        /// <summary>Overload without entity exclusion.</summary>
        public static bool SphereCast(Vector3 origin, Vector3 direction, float radius, float maxDistance,
                                      out SphereCastHit hit)
        {
            return SphereCast(origin, direction, radius, maxDistance, uint.MaxValue, out hit);
        }
    }
}

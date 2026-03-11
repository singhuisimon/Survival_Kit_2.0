using System;
using System.Runtime.CompilerServices;

namespace Engine
{
    public class ParticleSystem : Component
    {
        [MethodImpl(MethodImplOptions.InternalCall)] 
        public static extern void SetEmitterVelocity(uint entityID, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetEmissionRate(uint entityID, float rate);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetColorMin(uint entityID, ref Vector4 color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetColorMax(uint entityID, ref Vector4 color);
    }

}
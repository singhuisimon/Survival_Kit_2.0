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
    }

}
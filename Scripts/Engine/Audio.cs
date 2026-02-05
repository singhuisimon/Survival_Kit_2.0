using System.Runtime.CompilerServices;

namespace Engine
{
    public class Audio : Component
    {
        // Audio bindings (register as: "Engine.Audio::Audio_*")
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_Play(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_Stop(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_Pause(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Audio_IsPlaying(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Audio_GetVolume(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetVolume(uint entityID, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Audio_GetPitch(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetPitch(uint entityID, float pitch);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Audio_GetLoop(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetLoop(uint entityID, bool loop);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Audio_GetMute(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetMute(uint entityID, bool mute);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Audio_GetIs3D(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetIs3D(uint entityID, bool is3d);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetFile(uint entityID, string path);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Audio_GetMinDistance(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetMinDistance(uint entityID, float minDist);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Audio_GetMaxDistance(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetMaxDistance(uint entityID, float maxDist);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern int Audio_GetRolloffMode(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetRolloffMode(uint entityID, int mode);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Audio_GetDopplerLevel(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetDopplerLevel(uint entityID, float level);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Audio_GetPan2D(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetPan2D(uint entityID, float pan);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Audio_GetReverbMix(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Audio_SetReverbMix(uint entityID, float mix);

        public static void AudioPlay(uint entityID) => Audio_Play(entityID);
        public static void AudioStop(uint entityID) => Audio_Stop(entityID);
        public static void AudioPause(uint entityID) => Audio_Pause(entityID);

        public static bool AudioIsPlaying(uint entityID) => Audio_IsPlaying(entityID);

        public static void AudioSetFile(uint entityID, string path) => Audio_SetFile(entityID, path ?? string.Empty);
        public static void AudioSetLoop(uint entityID, bool loop) => Audio_SetLoop(entityID, loop);
        public static void AudioSetIs3D(uint entityID, bool is3d) => Audio_SetIs3D(entityID, is3d);
        public static void AudioSetMinDistance(uint entityID, float minDist) => Audio_SetMinDistance(entityID, minDist);
        public static void AudioSetMaxDistance(uint entityID, float maxDist) => Audio_SetMaxDistance(entityID, maxDist);

        // ===== Instance API (keep whatever you already had) =====
        private uint ID => Entity.EntityID;

        public float Volume { get => Audio_GetVolume(ID); set => Audio_SetVolume(ID, value); }
        public float Pitch { get => Audio_GetPitch(ID); set => Audio_SetPitch(ID, value); }
        public bool Loop { get => Audio_GetLoop(ID); set => Audio_SetLoop(ID, value); }
        public bool Mute { get => Audio_GetMute(ID); set => Audio_SetMute(ID, value); }
        public bool Is3D { get => Audio_GetIs3D(ID); set => Audio_SetIs3D(ID, value); }

        public float MinDistance { get => Audio_GetMinDistance(ID); set => Audio_SetMinDistance(ID, value); }
        public float MaxDistance { get => Audio_GetMaxDistance(ID); set => Audio_SetMaxDistance(ID, value); }

        public string File { set => Audio_SetFile(ID, value ?? string.Empty); }

        public void Play() => Audio_Play(ID);
        public void Stop() => Audio_Stop(ID);
        public void Pause() => Audio_Pause(ID);
    }
}

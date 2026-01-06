using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Audio channel group types matching C++ AudioType enum
    /// </summary>
    public enum AudioType
    {
        MASTER = 0,
        SFX = 1,
        BGM = 2,
        UI = 3
    }

    /// <summary>
    /// DSP effect types matching C++ DSPEffectType enum
    /// </summary>
    public enum DSPEffectType
    {
        None = 0,
        LowPass,
        HighPass,
        Echo,
        Reverb,
        Chorus,
        Distortion,
        Flange,
        Tremolo,
        PitchShift,
        Compressor
    }

    /// <summary>
    /// Audio rolloff modes for 3D sound distance attenuation
    /// </summary>
    public enum AudioRolloffMode
    {
        INVERSE = 0,
        LINEAR = 1,
        LINEARSQUARE = 2
    }

    public static class AudioManager
    {
        // =========================
        // Native bindings (private)
        // Register as "Engine.AudioManager::AudioManager_*"
        // =========================
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float AudioManager_GetGroupVolume(int groupType);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_SetGroupVolume(int groupType, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float AudioManager_GetGroupPitch(int groupType);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_SetGroupPitch(int groupType, float pitch);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool AudioManager_IsGroupMuted(int groupType);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_SetGroupMute(int groupType, bool mute);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_PauseGroup(int groupType, bool pause);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_PauseAll(bool pause);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_StopByType(int groupType);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_StopAll();

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_CreateDSP(int groupType, int effectType);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_EnableDSP(int groupType, int effectType, bool enable);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_SetDSPParameter(int groupType, int effectType, int paramIndex, float value);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_ReleaseSpecificDSPinGroup(int groupType, int effectType);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_ReleaseDSPByGroup(int groupType);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void AudioManager_ReleaseAllDSPs();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_SetListenerAttributes(
            ref Vector3 position,
            ref Vector3 forward,
            ref Vector3 up,
            ref Vector3 velocity
        );

        // =========================
        // Public Controls
        // =========================
        public static void SetGroupVolume(AudioType group, float volume)
            => AudioManager_SetGroupVolume((int)group, volume);

        public static float GetGroupVolume(AudioType group)
            => AudioManager_GetGroupVolume((int)group);

        public static void SetGroupPitch(AudioType group, float pitch)
            => AudioManager_SetGroupPitch((int)group, pitch);

        public static float GetGroupPitch(AudioType group)
            => AudioManager_GetGroupPitch((int)group);

        public static void SetMuteGroup(AudioType group, bool mute)
            => AudioManager_SetGroupMute((int)group, mute);

        public static bool IsGroupMuted(AudioType group)
            => AudioManager_IsGroupMuted((int)group);

        // =========================
        // Group Pause/Stop Controls
        // =========================
        public static void PauseGroup(AudioType group, bool pause)
            => AudioManager_PauseGroup((int)group, pause);

        public static void PauseAll(bool pause)
            => AudioManager_PauseAll(pause);

        public static void StopGroup(AudioType group)
            => AudioManager_StopByType((int)group);

        public static void StopAll()
            => AudioManager_StopAll();

        // =========================
        // DSP Effect Management
        // =========================
        public static void CreateDSP(AudioType group, DSPEffectType effect)
            => AudioManager_CreateDSP((int)group, (int)effect);

        public static void EnableDSP(AudioType group, DSPEffectType effect, bool enable)
            => AudioManager_EnableDSP((int)group, (int)effect, enable);

        public static void SetDSPParameter(AudioType group, DSPEffectType effect, int paramIndex, float value)
            => AudioManager_SetDSPParameter((int)group, (int)effect, paramIndex, value);

        public static void ReleaseSpecificDSPinGroup(AudioType group, DSPEffectType effect)
            => AudioManager_ReleaseSpecificDSPinGroup((int)group, (int)effect);

        public static void ReleaseDSPByGroup(AudioType group)
            => AudioManager_ReleaseDSPByGroup((int)group);

        public static void ReleaseAllDSPs()
            => AudioManager_ReleaseAllDSPs();

        // =========================
        // Listener Controls (3D Audio)
        // =========================

        // If you want the name "SetListenerPosition", keep it, but the signature must be ref Vector3.
        public static void SetListenerPosition(ref Vector3 position, ref Vector3 forward, ref Vector3 up, ref Vector3 velocity)
            => AudioManager_SetListenerAttributes(ref position, ref forward, ref up, ref velocity);

        // Optional clearer alias:
        public static void SetListenerAttributes(ref Vector3 position, ref Vector3 forward, ref Vector3 up, ref Vector3 velocity)
            => AudioManager_SetListenerAttributes(ref position, ref forward, ref up, ref velocity);

        // =========================
        // Convenience Methods
        // =========================
        public static void SetupLowPassFilter(AudioType group, float cutoffFrequency)
        {
            CreateDSP(group, DSPEffectType.LowPass);
            SetDSPParameter(group, DSPEffectType.LowPass, 0, cutoffFrequency);
            EnableDSP(group, DSPEffectType.LowPass, true);
        }

        public static void SetupEcho(AudioType group, float delay, float feedback)
        {
            CreateDSP(group, DSPEffectType.Echo);
            SetDSPParameter(group, DSPEffectType.Echo, 0, delay);
            SetDSPParameter(group, DSPEffectType.Echo, 1, feedback);
            EnableDSP(group, DSPEffectType.Echo, true);
        }

        public static void SetupReverb(AudioType group, float roomSize, float damping)
        {
            CreateDSP(group, DSPEffectType.Reverb);
            SetDSPParameter(group, DSPEffectType.Reverb, 0, roomSize);
            SetDSPParameter(group, DSPEffectType.Reverb, 1, damping);
            EnableDSP(group, DSPEffectType.Reverb, true);
        }

        public static void FadeOutAll(float duration)
        {
            // Needs native smooth fade if you want time-based fade.
            SetGroupVolume(AudioType.MASTER, 0.0f);
        }

        public static void FadeInAll(float duration)
        {
            SetGroupVolume(AudioType.MASTER, 1.0f);
        }
    }
}

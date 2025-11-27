using System;
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
        INVERSE = 0,      // Default inverse rolloff
        LINEAR = 1,       // Linear rolloff
        LINEARSQUARE = 2  // Linear square rolloff
    }

    public static class AudioManager
    {

        //Audio Manager implementation - new amanda
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float AudioManager_GetGroupVolume(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_SetGroupVolume(int groupType, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float AudioManager_GetGroupPitch(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_SetGroupPitch(int groupType, float pitch);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool AudioManager_IsGroupMuted(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_SetGroupMute(int groupType, bool mute);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_PauseGroup(int groupType, bool pause);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_PauseAll(bool pause);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_StopByType(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_StopAll();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_CreateDSP(int groupType, int effectType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_EnableDSP(int groupType, int effectType, bool enable);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_SetDSPParameter(int groupType, int effectType, int paramIndex, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_ReleaseSpecificDSPinGroup(int groupType, int effectType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_ReleaseDSPByGroup(int groupType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_ReleaseAllDSPs();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AudioManager_SetListenerAttributes(ref Vector3 position, ref Vector3 forward, ref Vector3 up, ref Vector3 velocity);

        // ===== Public Controls =====

        public static void SetGroupVolume(AudioType group, float volume)
        {
            AudioManager_SetGroupVolume((int)group, volume);
        }

        public static float GetGroupVolume(AudioType group)
        {
            return AudioManager_GetGroupVolume((int)group);
        }

        public static void SetGroupPitch(AudioType group, float pitch)
        {
            AudioManager_SetGroupPitch((int)group, pitch);
        }

        public static float GetGroupPitch(AudioType group)
        {
            return AudioManager_GetGroupPitch((int)group);
        }

        public static void SetMuteGroup(AudioType group, bool mute)
        {
            AudioManager_SetGroupMute((int)group, mute);
        }

        public static bool IsGroupMuted(AudioType group)
        {
            return AudioManager_IsGroupMuted((int)group);
        }

        // ===== Group Pause/Stop Controls =====

        public static void PauseGroup(AudioType group, bool pause)
        {
            AudioManager_PauseGroup((int)group, pause);
        }

        public static void PauseAll(bool pause)
        {
            AudioManager_PauseAll(pause);
        }

        public static void StopGroup(AudioType group)
        {
            AudioManager_StopByType((int)group);
        }

        public static void StopAll()
        {
            AudioManager_StopAll();
        }

        // ===== DSP Effect Management =====

        /// <summary>
        /// Create a DSP effect for a specific channel group.
        /// The DSP starts disabled (bypassed) by default.
        /// </summary>
        public static void CreateDSP(AudioType group, DSPEffectType effect)
        {
            AudioManager_CreateDSP((int)group, (int)effect);
        }

        /// <summary>
        /// Enable or disable (bypass) a DSP effect on a channel group.
        /// </summary>
        public static void EnableDSP(AudioType group, DSPEffectType effect, bool enable)
        {
            AudioManager_EnableDSP((int)group, (int)effect, enable);
        }

        /// <summary>
        /// Set a parameter value for a DSP effect.
        /// Parameter indices and valid ranges depend on the DSP type.
        /// Refer to FMOD documentation for DSP parameter details.
        /// </summary>
        public static void SetDSPParameter(AudioType group, DSPEffectType effect, int paramIndex, float value)
        {
            AudioManager_SetDSPParameter((int)group, (int)effect, paramIndex, value);
        }

        /// <summary>
        /// Release a specific DSP effect from a channel group.
        /// </summary>
        public static void ReleaseSpecificDSPinGroup(AudioType group, DSPEffectType effect)
        {
            AudioManager_ReleaseSpecificDSPinGroup((int)group, (int)effect);
        }

        /// <summary>
        /// Release all DSP effects from a specific channel group.
        /// </summary>
        public static void ReleaseDSPByGroup(AudioType group)
        {
            AudioManager_ReleaseDSPByGroup((int)group);
        }

        /// <summary>
        /// Release all DSP effects from all channel groups.
        /// </summary>
        public static void ReleaseAllDSPs()
        {
            AudioManager_ReleaseAllDSPs();
        }

        // ===== Listener Controls (3D Audio) =====

        /// <summary>
        /// Set the 3D audio listener attributes for spatial audio.
        /// This is typically called once per frame with the camera/player position.
        /// </summary>
        public static void SetListenerPosition(Vector3 position, Vector3 forward, Vector3 up, Vector3 velocity)
        {
            AudioManager_SetListenerAttributes(
                ref position, 
                ref forward, 
                ref up, 
                ref velocity
            );
        }

        // ===== Convenience Methods =====

        /// <summary>
        /// Quick setup for common low-pass filter effect
        /// </summary>
        public static void SetupLowPassFilter(AudioType group, float cutoffFrequency)
        {
            CreateDSP(group, DSPEffectType.LowPass);
            SetDSPParameter(group, DSPEffectType.LowPass, 0, cutoffFrequency); // FMOD_DSP_LOWPASS_CUTOFF
            EnableDSP(group, DSPEffectType.LowPass, true);
        }

        /// <summary>
        /// Quick setup for echo effect
        /// </summary>
        public static void SetupEcho(AudioType group, float delay, float feedback)
        {
            CreateDSP(group, DSPEffectType.Echo);
            SetDSPParameter(group, DSPEffectType.Echo, 0, delay);    // FMOD_DSP_ECHO_DELAY
            SetDSPParameter(group, DSPEffectType.Echo, 1, feedback); // FMOD_DSP_ECHO_FEEDBACK
            EnableDSP(group, DSPEffectType.Echo, true);
        }

        /// <summary>
        /// Quick setup for reverb effect
        /// </summary>
        public static void SetupReverb(AudioType group, float roomSize, float damping)
        {
            CreateDSP(group, DSPEffectType.Reverb);
            SetDSPParameter(group, DSPEffectType.Reverb, 0, roomSize); // Room size
            SetDSPParameter(group, DSPEffectType.Reverb, 1, damping);  // Damping
            EnableDSP(group, DSPEffectType.Reverb, true);
        }

        /// <summary>
        /// Fade out all audio over specified duration
        /// </summary>
        public static void FadeOutAll(float duration)
        {
            // Note: This would need additional C++ implementation for smooth fade
            // For now, we can approximate with volume lerp in Update loop
            SetGroupVolume(AudioType.MASTER, 0.0f);
        }

        /// <summary>
        /// Fade in all audio over specified duration
        /// </summary>
        public static void FadeInAll(float duration)
        {
            SetGroupVolume(AudioType.MASTER, 1.0f);
        }
    }
}

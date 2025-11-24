using System;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over AudioComponent � simple audio source control.
    /// </summary>
    public class Audio : Component
    {
        public float Volume
        {
            get { return InternalCalls.Audio_GetVolume(Entity.EntityID); }
            set { InternalCalls.Audio_SetVolume(Entity.EntityID, value); }
        }

        public float Pitch
        {
            get { return InternalCalls.Audio_GetPitch(Entity.EntityID); }
            set { InternalCalls.Audio_SetPitch(Entity.EntityID, value); }
        }

        public bool Loop
        {
            get { return InternalCalls.Audio_GetLoop(Entity.EntityID); }
            set { InternalCalls.Audio_SetLoop(Entity.EntityID, value); }
        }

        public bool Mute
        {
            get { return InternalCalls.Audio_GetMute(Entity.EntityID); }
            set { InternalCalls.Audio_SetMute(Entity.EntityID, value); }
        }

        public bool Is3D
        {
            get { return InternalCalls.Audio_GetIs3D(Entity.EntityID); }
            set { InternalCalls.Audio_SetIs3D(Entity.EntityID, value); }
        }

        public float MinDistance{
            get{ return InternalCalls.Audio_GetMinDistance(Entity.EntityID); }
            set{ InternalCalls.Audio_SetMinDistance(Entity.EntityID, value); }
        }

        public float MaxDistance{
            get{ return InternalCalls.Audio_GetMaxDistance(Entity.EntityID); }
            set{ InternalCalls.Audio_SetMaxDistance(Entity.EntityID, value); }
        }

        public AudioRolloffMode RolloffMode
        {
            get { return (AudioRolloffMode)InternalCalls.Audio_GetRolloffMode(Entity.EntityID); }
            set { InternalCalls.Audio_SetRolloffMode(Entity.EntityID, (int)value); }
        }

        public float DopplerLevel
        {
            get { return InternalCalls.Audio_GetDopplerLevel(Entity.EntityID); }
            set { InternalCalls.Audio_SetDopplerLevel(Entity.EntityID, value); }
        }

        public float Pan2D
        {
            get { return InternalCalls.Audio_GetPan2D(Entity.EntityID); }
            set { InternalCalls.Audio_SetPan2D(Entity.EntityID, value); }
        }

        public float ReverbMix
        {
            get { return InternalCalls.Audio_GetReverbMix(Entity.EntityID); }
            set { InternalCalls.Audio_SetReverbMix(Entity.EntityID, value); }
        }

        /// <summary>
        /// Change the audio asset used by this source.
        /// </summary>
        public string File
        {
            set { InternalCalls.Audio_SetFile(Entity.EntityID, value); }
        }

        public void Play()
        {
            InternalCalls.Audio_Play(Entity.EntityID);
        }

        public void Stop()
        {
            InternalCalls.Audio_Stop(Entity.EntityID);
        }

        public void Pause()
        {
            InternalCalls.Audio_Pause(Entity.EntityID);
        }
    }
}

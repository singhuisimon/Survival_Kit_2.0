using Engine;
using System;

namespace Game
{
    /// <summary>
    /// Example player controller script
    /// Attach this to a entity to play
    /// </summary>
    public class OverallAudioControl : ScriptBehaviour
    {
        [SerializeField]
        private float masterVolume = 1.0f;

        [SerializeField]
        private float masterPitch = 1.0f;

        [SerializeField]
        private bool masterMute = false;

        [SerializeField]
        private float bgmVolume = 1.0f;

        [SerializeField]
        private float bgmPitch = 1.0f;

        [SerializeField]
        private bool bgmMute = false;

        [SerializeField]
        private float sfxVolume = 1.0f;

        [SerializeField]
        private float sfxPitch = 1.0f;

        [SerializeField]
        private bool sfxMute = false;

        [SerializeField]
        private float uiVolume = 1.0f;

        [SerializeField]
        private float uiPitch = 1.0f;

        [SerializeField]
        private bool uiMute = false;

        // Store previous values to detect changes
        private float prevMasterVolume;
        private float prevMasterPitch;
        private bool prevMasterMute;

        private float prevBgmVolume;
        private float prevBgmPitch;
        private bool prevBgmMute;

        private float prevSfxVolume;
        private float prevSfxPitch;
        private bool prevSfxMute;

        private float prevUiVolume;
        private float prevUiPitch;
        private bool prevUiMute;

        public override void OnStart()
        {
            //apply initial setting
            ApplyAllSettings();
            
            //Store initial value
            StoreCurrentValue();

            InternalCalls.Log("OverallAudioControl intiialized - all audio settings applied");
        }

        public override void OnUpdate(float deltaTime)
        {
            bool changed = false;

            if(masterVolume != prevMasterVolume){
                Engine.AudioManager.SetGroupVolume(AudioType.MASTER, masterVolume);
                prevMasterVolume = masterVolume;
                changed = true;
            }

            if(masterPitch != prevMasterPitch){
                Engine.AudioManager.SetGroupPitch(AudioType.MASTER, masterPitch);
                prevMasterPitch = masterPitch;
                changed = true;
            }

            if(masterMute != prevMasterMute){
                Engine.AudioManager.SetMuteGroup(AudioType.MASTER, masterMute);
                prevMasterMute = masterMute;
                changed = true;
            }

            if(bgmVolume != prevBgmVolume){
                Engine.AudioManager.SetGroupVolume(AudioType.BGM, bgmVolume);
                prevBgmVolume = bgmVolume;
                changed = true;
            }

            if(bgmPitch != prevBgmPitch){
                Engine.AudioManager.SetGroupPitch(AudioType.BGM, bgmPitch);
                prevBgmPitch = bgmPitch;
                changed = true;
            }

            if(bgmMute != prevBgmMute){
                Engine.AudioManager.SetMuteGroup(AudioType.BGM, bgmMute);
                prevBgmMute = bgmMute;
                changed = true;
            }

            if(sfxVolume != prevSfxVolume){
                Engine.AudioManager.SetGroupVolume(AudioType.SFX, sfxVolume);
                prevSfxVolume = sfxVolume;
                changed = true;
            }

            if(sfxPitch != prevSfxPitch){
                Engine.AudioManager.SetGroupPitch(AudioType.SFX, sfxPitch);
                prevSfxPitch = sfxPitch;
                changed = true;
            }

            if(sfxMute != prevSfxMute){
                Engine.AudioManager.SetMuteGroup(AudioType.SFX, sfxMute);
                prevSfxMute = sfxMute;
                changed = true;
            }

            if(uiVolume != prevUiVolume){
                Engine.AudioManager.SetGroupVolume(AudioType.UI, uiVolume);
                prevUiVolume = uiVolume;
                changed = true;
            }

            if(uiPitch != prevUiPitch){
                Engine.AudioManager.SetGroupPitch(AudioType.UI, uiPitch);
                prevUiPitch = uiPitch;
                changed = true;
            }

            if(uiMute != prevUiMute){
                Engine.AudioManager.SetMuteGroup(AudioType.UI, uiMute);
                prevUiMute = uiMute;
                changed = true;
            }

            if(changed){
                InternalCalls.Log("OverallAudioControl Update - audio settings changes applied");
            }
        }

        public override void OnDestroy()
        {
            Log("PlayerController destroyed!");
        }

        private void ApplyAllSettings(){
            Engine.AudioManager.SetGroupVolume(AudioType.MASTER, masterVolume);
            Engine.AudioManager.SetGroupVolume(AudioType.BGM, bgmVolume);
            Engine.AudioManager.SetGroupVolume(AudioType.SFX, sfxVolume);
            Engine.AudioManager.SetGroupVolume(AudioType.UI, uiVolume);

            Engine.AudioManager.SetGroupPitch(AudioType.MASTER, masterPitch);
            Engine.AudioManager.SetGroupPitch(AudioType.BGM, bgmPitch);
            Engine.AudioManager.SetGroupPitch(AudioType.SFX, sfxPitch);
            Engine.AudioManager.SetGroupPitch(AudioType.UI, uiPitch);

            Engine.AudioManager.SetMuteGroup(AudioType.MASTER, masterMute);
            Engine.AudioManager.SetMuteGroup(AudioType.BGM, bgmMute);
            Engine.AudioManager.SetMuteGroup(AudioType.SFX, sfxMute);
            Engine.AudioManager.SetMuteGroup(AudioType.UI, uiMute);
        }

        private void StoreCurrentValue(){
            prevMasterVolume = masterVolume;
            prevMasterPitch = masterPitch;
            prevMasterMute = masterMute;

            prevBgmVolume = bgmVolume;
            prevBgmPitch = bgmPitch;
            prevBgmMute = bgmMute;

            prevSfxVolume = sfxVolume;
            prevSfxPitch = sfxPitch;
            prevSfxMute = sfxMute;

            prevUiVolume = uiVolume;
            prevUiPitch = uiPitch;
            prevUiMute = uiMute;
        }
    }
}

/**
 * @file AudioSettings.cs
 * @brief In-game audio settings manager for volume sliders and mute controls
 * @author Jack
 * @date January 2026
 * 
 * Usage:
 * 1. Attach this to a GameObject in your settings menu scene
 * 2. Hook up UI sliders/buttons to the public methods
 * 3. Settings are automatically applied to AudioManager
 */

using System.Runtime.CompilerServices;


namespace Engine
{
    /// <summary>
    /// Manages in-game audio settings including volume sliders and mute toggles.
    /// Provides methods to be called from UI elements (sliders, buttons, etc.)
    /// </summary>
    public class AudioSettings 
    {
        // ==========================================
        // Settings State (can be saved/loaded)
        // ==========================================

        private float masterVolume = 1.0f;
        private float bgmVolume = 1.0f;
        private float sfxVolume = 1.0f;
        private float uiVolume = 1.0f;

        private bool masterMuted = false;
        private bool bgmMuted = false;
        private bool sfxMuted = false;
        private bool uiMuted = false;

        // ==========================================
        // Initialization
        // ==========================================

        void OnStart()
        {
            // Load saved settings (if you have a save system)
            LoadSettings();

            // Apply loaded settings to AudioManager
            ApplyAllSettings();

            //Debug.Log("AudioSettings initialized");
        }

        // ==========================================
        // Master Volume Control
        // ==========================================

        /// <summary>
        /// Set master volume (0.0 to 1.0)
        /// Call this from your master volume slider's OnValueChanged event
        /// </summary>
        public void SetMasterVolume(float volume)
        {
            masterVolume = Clamp01(volume);
            AudioManager.SetGroupVolume(AudioType.MASTER, masterVolume);

            //Debug.Log($"Master Volume: {masterVolume:F2}");

            // Optional: Save setting
            SaveSettings();
        }

        /// <summary>
        /// Get current master volume
        /// </summary>
        public float GetMasterVolume()
        {
            return masterVolume;
        }

        /// <summary>
        /// Toggle master mute on/off
        /// Call this from your master mute button's OnClick event
        /// </summary>
        public void ToggleMasterMute()
        {
            masterMuted = !masterMuted;
            AudioManager.SetMuteGroup(AudioType.MASTER, masterMuted);

            //Debug.Log($"Master Muted: {masterMuted}");

            SaveSettings();
        }

        /// <summary>
        /// Set master mute state directly
        /// </summary>
        public void SetMasterMute(bool mute)
        {
            masterMuted = mute;
            AudioManager.SetMuteGroup(AudioType.MASTER, masterMuted);

            SaveSettings();
        }

        /// <summary>
        /// Check if master is muted
        /// </summary>
        public bool IsMasterMuted()
        {
            return masterMuted;
        }

        // ==========================================
        // BGM/Ambience Volume Control
        // ==========================================

        /// <summary>
        /// Set BGM/ambience volume (0.0 to 1.0)
        /// Call this from your BGM volume slider's OnValueChanged event
        /// </summary>
        public void SetBGMVolume(float volume)
        {
            bgmVolume = Clamp01(volume);
            AudioManager.SetGroupVolume(AudioType.BGM, bgmVolume);

           // Debug.Log($"BGM Volume: {bgmVolume:F2}");

            SaveSettings();
        }

        /// <summary>
        /// Get current BGM volume
        /// </summary>
        public float GetBGMVolume()
        {
            return bgmVolume;
        }

        /// <summary>
        /// Toggle BGM mute on/off
        /// Call this from your BGM mute button's OnClick event
        /// </summary>
        public void ToggleBGMMute()
        {
            bgmMuted = !bgmMuted;
            AudioManager.SetMuteGroup(AudioType.BGM, bgmMuted);

           // Debug.Log($"BGM Muted: {bgmMuted}");

            SaveSettings();
        }

        /// <summary>
        /// Set BGM mute state directly
        /// </summary>
        public void SetBGMMute(bool mute)
        {
            bgmMuted = mute;
            AudioManager.SetMuteGroup(AudioType.BGM, bgmMuted);

            SaveSettings();
        }

        /// <summary>
        /// Check if BGM is muted
        /// </summary>
        public bool IsBGMMuted()
        {
            return bgmMuted;
        }

        // ==========================================
        // SFX Volume Control
        // ==========================================

        /// <summary>
        /// Set SFX volume (0.0 to 1.0)
        /// Call this from your SFX volume slider's OnValueChanged event
        /// </summary>
        public void SetSFXVolume(float volume)
        {
            sfxVolume = Clamp01(volume);
            AudioManager.SetGroupVolume(AudioType.SFX, sfxVolume);

           //Debug.Log($"SFX Volume: {sfxVolume:F2}");

            SaveSettings();
        }

        /// <summary>
        /// Get current SFX volume
        /// </summary>
        public float GetSFXVolume()
        {
            return sfxVolume;
        }

        /// <summary>
        /// Toggle SFX mute on/off
        /// Call this from your SFX mute button's OnClick event
        /// </summary>
        public void ToggleSFXMute()
        {
            sfxMuted = !sfxMuted;
            AudioManager.SetMuteGroup(AudioType.SFX, sfxMuted);

           // Debug.Log($"SFX Muted: {sfxMuted}");

            SaveSettings();
        }

        /// <summary>
        /// Set SFX mute state directly
        /// </summary>
        public void SetSFXMute(bool mute)
        {
            sfxMuted = mute;
            AudioManager.SetMuteGroup(AudioType.SFX, sfxMuted);

            SaveSettings();
        }

        /// <summary>
        /// Check if SFX is muted
        /// </summary>
        public bool IsSFXMuted()
        {
            return sfxMuted;
        }

        // ==========================================
        // UI Audio Volume Control
        // ==========================================

        /// <summary>
        /// Set UI audio volume (0.0 to 1.0)
        /// Call this from your UI volume slider's OnValueChanged event
        /// </summary>
        public void SetUIVolume(float volume)
        {
            uiVolume = Clamp01(volume);
            AudioManager.SetGroupVolume(AudioType.UI, uiVolume);

            //Debug.Log($"UI Volume: {uiVolume:F2}");

            SaveSettings();
        }

        /// <summary>
        /// Get current UI volume
        /// </summary>
        public float GetUIVolume()
        {
            return uiVolume;
        }

        /// <summary>
        /// Toggle UI audio mute on/off
        /// Call this from your UI mute button's OnClick event
        /// </summary>
        public void ToggleUIMute()
        {
            uiMuted = !uiMuted;
            AudioManager.SetMuteGroup(AudioType.UI, uiMuted);

            //Debug.Log($"UI Muted: {uiMuted}");

            SaveSettings();
        }

        /// <summary>
        /// Set UI mute state directly
        /// </summary>
        public void SetUIMute(bool mute)
        {
            uiMuted = mute;
            AudioManager.SetMuteGroup(AudioType.UI, uiMuted);

            SaveSettings();
        }

        /// <summary>
        /// Check if UI audio is muted
        /// </summary>
        public bool IsUIMuted()
        {
            return uiMuted;
        }

        // ==========================================
        // Utility Methods
        // ==========================================

        /// <summary>
        /// Reset all audio settings to defaults
        /// </summary>
        public void ResetToDefaults()
        {
            // Set default volumes
            SetMasterVolume(1.0f);
            SetBGMVolume(0.7f);
            SetSFXVolume(0.8f);
            SetUIVolume(0.9f);

            // Unmute all
            SetMasterMute(false);
            SetBGMMute(false);
            SetSFXMute(false);
            SetUIMute(false);

            //Debug.Log("Audio settings reset to defaults");
        }

        /// <summary>
        /// Apply all current settings to AudioManager
        /// Useful after loading settings
        /// </summary>
        private void ApplyAllSettings()
        {
            // Apply volumes
            AudioManager.SetGroupVolume(AudioType.MASTER, masterVolume);
            AudioManager.SetGroupVolume(AudioType.BGM, bgmVolume);
            AudioManager.SetGroupVolume(AudioType.SFX, sfxVolume);
            AudioManager.SetGroupVolume(AudioType.UI, uiVolume);

            // Apply mute states
            AudioManager.SetMuteGroup(AudioType.MASTER, masterMuted);
            AudioManager.SetMuteGroup(AudioType.BGM, bgmMuted);
            AudioManager.SetMuteGroup(AudioType.SFX, sfxMuted);
            AudioManager.SetMuteGroup(AudioType.UI, uiMuted);

           // Debug.Log("Applied all audio settings");
        }

        /// <summary>
        /// Clamp value between 0 and 1
        /// </summary>
        private float Clamp01(float value)
        {
            if (value < 0.0f) return 0.0f;
            if (value > 1.0f) return 1.0f;
            return value;
        }

        // ==========================================
        // Save/Load Settings (Optional)
        // ==========================================

        /// <summary>
        /// Save audio settings to PlayerPrefs or your save system
        /// Modify this to use your engine's save system
        /// </summary>
        private void SaveSettings()
        {
            // TODO: Implement with your engine's save system
            // For now, using PlayerPrefs as an example:

            /*
            PlayerPrefs.SetFloat("Audio_MasterVolume", masterVolume);
            PlayerPrefs.SetFloat("Audio_BGMVolume", bgmVolume);
            PlayerPrefs.SetFloat("Audio_SFXVolume", sfxVolume);
            PlayerPrefs.SetFloat("Audio_UIVolume", uiVolume);
            
            PlayerPrefs.SetInt("Audio_MasterMuted", masterMuted ? 1 : 0);
            PlayerPrefs.SetInt("Audio_BGMMuted", bgmMuted ? 1 : 0);
            PlayerPrefs.SetInt("Audio_SFXMuted", sfxMuted ? 1 : 0);
            PlayerPrefs.SetInt("Audio_UIMuted", uiMuted ? 1 : 0);
            
            PlayerPrefs.Save();
            */
        }

        /// <summary>
        /// Load audio settings from PlayerPrefs or your save system
        /// Modify this to use your engine's save system
        /// </summary>
        private void LoadSettings()
        {
            // TODO: Implement with your engine's save system
            // For now, using PlayerPrefs as an example:

            /*
            masterVolume = PlayerPrefs.GetFloat("Audio_MasterVolume", 1.0f);
            bgmVolume = PlayerPrefs.GetFloat("Audio_BGMVolume", 0.7f);
            sfxVolume = PlayerPrefs.GetFloat("Audio_SFXVolume", 0.8f);
            uiVolume = PlayerPrefs.GetFloat("Audio_UIVolume", 0.9f);
            
            masterMuted = PlayerPrefs.GetInt("Audio_MasterMuted", 0) == 1;
            bgmMuted = PlayerPrefs.GetInt("Audio_BGMMuted", 0) == 1;
            sfxMuted = PlayerPrefs.GetInt("Audio_SFXMuted", 0) == 1;
            uiMuted = PlayerPrefs.GetInt("Audio_UIMuted", 0) == 1;
            */

            // If no save system, use defaults
            masterVolume = 1.0f;
            bgmVolume = 0.7f;
            sfxVolume = 0.8f;
            uiVolume = 0.9f;

            masterMuted = false;
            bgmMuted = false;
            sfxMuted = false;
            uiMuted = false;
        }

        // ==========================================
        // Debug/Testing Methods
        // ==========================================

        /// <summary>
        /// Print all current audio settings to console
        /// Useful for debugging
        /// </summary>
        public void PrintCurrentSettings()
        {
    /*        Debug.Log("=== Audio Settings ===");
            Debug.Log($"Master: Volume={masterVolume:F2}, Muted={masterMuted}");
            Debug.Log($"BGM:    Volume={bgmVolume:F2}, Muted={bgmMuted}");
            Debug.Log($"SFX:    Volume={sfxVolume:F2}, Muted={sfxMuted}");
            Debug.Log($"UI:     Volume={uiVolume:F2}, Muted={uiMuted}");
            Debug.Log("=====================");*/
        }

        /// <summary>
        /// Test method - cycles through different volume levels
        /// Call this to test if AudioManager bindings work
        /// </summary>
        public void TestAudioSettings()
        {
            //Debug.Log("Testing audio settings...");

            // Test master volume
            SetMasterVolume(0.5f);
            //Debug.Log($"Master volume should be 0.5, actual: {AudioManager.GetGroupVolume(AudioType.MASTER)}");

            // Test BGM volume
            SetBGMVolume(0.3f);
            //Debug.Log($"BGM volume should be 0.3, actual: {AudioManager.GetGroupVolume(AudioType.BGM)}");

            // Test SFX mute
            SetSFXMute(true);
           // Debug.Log($"SFX should be muted, actual: {AudioManager.IsGroupMuted(AudioType.SFX)}");
            SetSFXMute(false);

           // Debug.Log("Audio settings test complete!");
        }
    }
}

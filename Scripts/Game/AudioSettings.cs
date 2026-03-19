/**
* @file AudioSettings.cs
* @brief Simple audio settings manager with JSON persistence
* @author Jack
* @date January 2026
*/

using System;
using System.Collections.Generic;
using Engine;

namespace Game
{
    public class AudioSettings : ScriptBehaviour
    {
        private static AudioSettings instance = null;
        public static AudioSettings Instance { get { return instance; } }

        // ===== Audio Settings =====
        private float masterVolume = 1.0f;
        private float bgmVolume = 0.7f;
        private float sfxVolume = 0.8f;
        private float uiVolume = 0.9f;
        private bool masterMuted = false;
        private bool bgmMuted = false;
        private bool sfxMuted = false;
        private bool uiMuted = false;

        // ===== Mouse Sensitivity =====
        private static readonly float[] MOUSE_SENSITIVITY_STEPS = new float[]
        {
            0.0f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.25f, 2.5f
        };
        private const int MOUSE_SENSITIVITY_DEFAULT_INDEX = 3; // 1.0f
        private int mouseSensitivityIndex = MOUSE_SENSITIVITY_DEFAULT_INDEX;

        // ===== Gamma =====
        private static readonly float[] GAMMA_STEPS = new float[]
        {
            1.2f, 1.4f, 1.6f, 1.8f, 2.0f, 2.2f, 2.4f, 2.6f, 2.8f, 3.0f
        };
        private const int GAMMA_DEFAULT_INDEX = 5; // 2.2f
        private int gammaIndex = GAMMA_DEFAULT_INDEX;

        // File paths
        private const string SAVE_FILE = "Resources/Sources/SaveData/UserAudioSettings.json";
        private const string DEFAULT_FILE = "Config/DefaultAudioSettings.json";

        private float logTimer = 0.0f;
        private const float LOG_INTERVAL = 2.0f;

        public override void OnStart()
        {
            if (instance != null && instance != this)
            {
                Logger.LogWarning("[AudioSettings] Instance already exists - reusing existing");
                // Don't destroy - just return, the old instance is still valid
                return;
            }

            instance = this;
            LoadSettings();
            ApplyAllSettings();

            Logger.LogMessage("[AudioSettings] Initialized with persistent data");
        }


        public override void OnUpdate(float deltaTime)
        {
            logTimer += deltaTime;
            if (logTimer >= LOG_INTERVAL)
            {
                logTimer = 0.0f;
                Logger.LogMessage("[AudioSettings] Volumes - Master: " + masterVolume.ToString("F2") +
                    ", BGM: " + bgmVolume.ToString("F2") +
                    ", SFX: " + sfxVolume.ToString("F2") +
                    ", UI: " + uiVolume.ToString("F2") +
                    ", MouseSens: " + GetMouseSensitivity().ToString("F2") +
                    ", Gamma: " + GetGamma().ToString("F1"));
            }

            if (Input.IsKeyPressed(KeyCode.Up)) { SetMasterVolume(masterVolume + 0.1f); Logger.LogMessage("[AudioSettings] UP - Master: " + masterVolume.ToString("F2")); }
            if (Input.IsKeyPressed(KeyCode.Down)) { SetMasterVolume(masterVolume - 0.1f); Logger.LogMessage("[AudioSettings] DOWN - Master: " + masterVolume.ToString("F2")); }
            if (Input.IsKeyPressed(KeyCode.Left)) { SetBGMVolume(bgmVolume - 0.1f); Logger.LogMessage("[AudioSettings] LEFT - BGM: " + bgmVolume.ToString("F2")); }
            if (Input.IsKeyPressed(KeyCode.Right)) { SetBGMVolume(bgmVolume + 0.1f); Logger.LogMessage("[AudioSettings] RIGHT - BGM: " + bgmVolume.ToString("F2")); }
            if (Input.IsKeyPressed(KeyCode.M)) { ToggleMasterMute(); Logger.LogMessage("[AudioSettings] M - Mute: " + masterMuted.ToString()); }
        }

        public override void OnDestroy()
        {
            if (instance == this) instance = null;
        }

        // ===== Volume Controls =====
        public void SetMasterVolume(float volume) { masterVolume = Clamp01(volume); AudioManager.SetGroupVolume(AudioType.MASTER, masterVolume); SaveSettings(); }
        public float GetMasterVolume() { return masterVolume; }

        public void SetBGMVolume(float volume) { bgmVolume = Clamp01(volume); AudioManager.SetGroupVolume(AudioType.BGM, bgmVolume); SaveSettings(); }
        public float GetBGMVolume() { return bgmVolume; }

        public void SetSFXVolume(float volume) { sfxVolume = Clamp01(volume); AudioManager.SetGroupVolume(AudioType.SFX, sfxVolume); SaveSettings(); }
        public float GetSFXVolume() { return sfxVolume; }

        public void SetUIVolume(float volume) { uiVolume = Clamp01(volume); AudioManager.SetGroupVolume(AudioType.UI, uiVolume); SaveSettings(); }
        public float GetUIVolume() { return uiVolume; }

        // ===== Mute Controls =====
        public void ToggleMasterMute() { masterMuted = !masterMuted; AudioManager.SetMuteGroup(AudioType.MASTER, masterMuted); SaveSettings(); }
        public void SetMasterMute(bool mute) { masterMuted = mute; AudioManager.SetMuteGroup(AudioType.MASTER, masterMuted); SaveSettings(); }
        public bool IsMasterMuted() { return masterMuted; }

        public void ToggleBGMMute() { bgmMuted = !bgmMuted; AudioManager.SetMuteGroup(AudioType.BGM, bgmMuted); SaveSettings(); }
        public void SetBGMMute(bool mute) { bgmMuted = mute; AudioManager.SetMuteGroup(AudioType.BGM, bgmMuted); SaveSettings(); }
        public bool IsBGMMuted() { return bgmMuted; }

        public void ToggleSFXMute() { sfxMuted = !sfxMuted; AudioManager.SetMuteGroup(AudioType.SFX, sfxMuted); SaveSettings(); }
        public void SetSFXMute(bool mute) { sfxMuted = mute; AudioManager.SetMuteGroup(AudioType.SFX, sfxMuted); SaveSettings(); }
        public bool IsSFXMuted() { return sfxMuted; }

        public void ToggleUIMute() { uiMuted = !uiMuted; AudioManager.SetMuteGroup(AudioType.UI, uiMuted); SaveSettings(); }
        public void SetUIMute(bool mute) { uiMuted = mute; AudioManager.SetMuteGroup(AudioType.UI, uiMuted); SaveSettings(); }
        public bool IsUIMuted() { return uiMuted; }

        // ===== Mouse Sensitivity Controls =====
        public float GetMouseSensitivity() { return MOUSE_SENSITIVITY_STEPS[mouseSensitivityIndex]; }
        public float GetMouseSensitivityNormalized() { return (float)mouseSensitivityIndex / (float)(MOUSE_SENSITIVITY_STEPS.Length - 1); }

        public void SetMouseSensitivityUp()
        {
            if (mouseSensitivityIndex < MOUSE_SENSITIVITY_STEPS.Length - 1) mouseSensitivityIndex++;
            SaveSettings();
            Logger.LogMessage("[AudioSettings] Mouse Sensitivity: " + GetMouseSensitivity().ToString("F2"));
        }

        public void SetMouseSensitivityDown()
        {
            if (mouseSensitivityIndex > 0) mouseSensitivityIndex--;
            SaveSettings();
            Logger.LogMessage("[AudioSettings] Mouse Sensitivity: " + GetMouseSensitivity().ToString("F2"));
        }

        public void ResetMouseSensitivity()
        {
            mouseSensitivityIndex = MOUSE_SENSITIVITY_DEFAULT_INDEX;
            SaveSettings();
            Logger.LogMessage("[AudioSettings] Mouse Sensitivity reset to default: " + GetMouseSensitivity().ToString("F2"));
        }

        // ===== Gamma Controls =====
        public float GetGamma() { return GAMMA_STEPS[gammaIndex]; }
        public float GetGammaNormalized() { return (float)gammaIndex / (float)(GAMMA_STEPS.Length - 1); }
        public int GetGammaIndex() { return gammaIndex; }
        public int GetGammaStepCount() { return GAMMA_STEPS.Length; }

        public void IncrementGamma()
        {
            if (gammaIndex < GAMMA_STEPS.Length - 1) gammaIndex++;
            RenderSettings.SetGamma(GetGamma());
            SaveSettings();
            Logger.LogMessage("[AudioSettings] Gamma: " + GetGamma().ToString("F1"));
        }

        public void DecrementGamma()
        {
            if (gammaIndex > 0) gammaIndex--;
            RenderSettings.SetGamma(GetGamma());
            SaveSettings();
            Logger.LogMessage("[AudioSettings] Gamma: " + GetGamma().ToString("F1"));
        }

        public void ResetGamma()
        {
            gammaIndex = GAMMA_DEFAULT_INDEX;
            RenderSettings.SetGamma(GetGamma());
            SaveSettings();
            Logger.LogMessage("[AudioSettings] Gamma reset to default: " + GetGamma().ToString("F1"));
        }

        // ===== Save / Load =====
        private void LoadSettings()
        {
            try
            {
                if (FileIO.FileExists(SAVE_FILE)) { Logger.LogMessage("[AudioSettings] Loading user settings from: " + SAVE_FILE); LoadFromJson(SAVE_FILE); return; }
                if (FileIO.FileExists(DEFAULT_FILE)) { Logger.LogMessage("[AudioSettings] Loading default settings from: " + DEFAULT_FILE); LoadFromJson(DEFAULT_FILE); return; }
                Logger.LogWarning("[AudioSettings] No settings file found, using hardcoded defaults");
            }
            catch (Exception e) { Logger.LogError("[AudioSettings] Failed to load settings: " + e.Message); }
        }

        private void LoadFromJson(string filePath)
        {
            try
            {
                string json = FileIO.ReadAllText(filePath);
                var data = ParseSimpleJson(json);

                masterVolume = Clamp01(GetFloatValue(data, "masterVolume", 1.0f));
                bgmVolume = Clamp01(GetFloatValue(data, "bgmVolume", 0.7f));
                sfxVolume = Clamp01(GetFloatValue(data, "sfxVolume", 0.8f));
                uiVolume = Clamp01(GetFloatValue(data, "uiVolume", 0.9f));
                masterMuted = GetBoolValue(data, "masterMuted", false);
                bgmMuted = GetBoolValue(data, "bgmMuted", false);
                sfxMuted = GetBoolValue(data, "sfxMuted", false);
                uiMuted = GetBoolValue(data, "uiMuted", false);

                mouseSensitivityIndex = (int)GetFloatValue(data, "mouseSensitivityIndex", MOUSE_SENSITIVITY_DEFAULT_INDEX);
                if (mouseSensitivityIndex < 0 || mouseSensitivityIndex >= MOUSE_SENSITIVITY_STEPS.Length)
                    mouseSensitivityIndex = MOUSE_SENSITIVITY_DEFAULT_INDEX;

                gammaIndex = (int)GetFloatValue(data, "gammaIndex", GAMMA_DEFAULT_INDEX);
                if (gammaIndex < 0 || gammaIndex >= GAMMA_STEPS.Length)
                    gammaIndex = GAMMA_DEFAULT_INDEX;

                Logger.LogMessage("[AudioSettings] Loaded successfully");
            }
            catch (Exception e) { Logger.LogError("[AudioSettings] Failed to load JSON: " + e.Message); }
        }
        private void SaveSettings()
        {
            try
            {
                string json = "{\n";
                json += "  \"masterVolume\": " + masterVolume + ",\n";
                json += "  \"bgmVolume\": " + bgmVolume + ",\n";
                json += "  \"sfxVolume\": " + sfxVolume + ",\n";
                json += "  \"uiVolume\": " + uiVolume + ",\n";
                json += "  \"masterMuted\": " + masterMuted.ToString().ToLower() + ",\n";
                json += "  \"bgmMuted\": " + bgmMuted.ToString().ToLower() + ",\n";
                json += "  \"sfxMuted\": " + sfxMuted.ToString().ToLower() + ",\n";
                json += "  \"uiMuted\": " + uiMuted.ToString().ToLower() + ",\n";
                json += "  \"mouseSensitivityIndex\": " + mouseSensitivityIndex + ",\n";
                json += "  \"gammaIndex\": " + gammaIndex + "\n";
                json += "}";

                if (FileIO.WriteAllText(SAVE_FILE, json))
                    Logger.LogMessage("[AudioSettings] Saved to: " + SAVE_FILE);
                else
                    Logger.LogError("[AudioSettings] Failed to save file - SaveData folder may not exist");
            }
            catch (Exception e) { Logger.LogError("[AudioSettings] Save error: " + e.Message); }
        }


        private void ApplyAllSettings()
        {
            AudioManager.SetGroupVolume(AudioType.MASTER, masterVolume);
            AudioManager.SetGroupVolume(AudioType.BGM, bgmVolume);
            AudioManager.SetGroupVolume(AudioType.SFX, sfxVolume);
            AudioManager.SetGroupVolume(AudioType.UI, uiVolume);
            AudioManager.SetMuteGroup(AudioType.MASTER, masterMuted);
            AudioManager.SetMuteGroup(AudioType.BGM, bgmMuted);
            AudioManager.SetMuteGroup(AudioType.SFX, sfxMuted);
            AudioManager.SetMuteGroup(AudioType.UI, uiMuted);
            RenderSettings.SetGamma(GetGamma());
        }

        // ===== JSON Parser =====
        private Dictionary<string, string> ParseSimpleJson(string json)
        {
            var result = new Dictionary<string, string>();
            json = json.Trim().TrimStart('{').TrimEnd('}');
            string[] pairs = json.Split(',');
            foreach (string pair in pairs)
            {
                string[] keyValue = pair.Split(':');
                if (keyValue.Length == 2)
                {
                    string key = keyValue[0].Trim().Trim('"');
                    string value = keyValue[1].Trim().Trim('"');
                    result[key] = value;
                }
            }
            return result;
        }

        private float GetFloatValue(Dictionary<string, string> data, string key, float defaultValue)
        {
            if (data.ContainsKey(key) && float.TryParse(data[key], out float value)) return value;
            return defaultValue;
        }

        private bool GetBoolValue(Dictionary<string, string> data, string key, bool defaultValue)
        {
            if (data.ContainsKey(key) && bool.TryParse(data[key], out bool value)) return value;
            return defaultValue;
        }

        private float Clamp01(float value)
        {
            if (value < 0.0f) return 0.0f;
            if (value > 1.0f) return 1.0f;
            return value;
        }
    }
}

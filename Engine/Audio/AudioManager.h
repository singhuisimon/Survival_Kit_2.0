/**
 * @file AudioManager.h
 * @brief Wrapper for accessing FMOD Core API functionality
 * @author Amanda Leow Boon Suan (100%)
 * @date 20/10/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#include "ECS/Components.h"
#include "DSPEffect.h"
#include <fmod.hpp>
#include <fmod_errors.h>
#include <unordered_map>
#include <string>

namespace Engine {

	struct MixerBusSettings {
		float editorCap = 1.0f; // Slider value from 0 to 2 for designers
		float playerVolume = 1.0f; // Slider value from 0 to 1
	};

    /**
	 * @class Audio Manager
	 * @brief Global FMOD Core API manager for handling sound playback, caching, and channelgroups.
	 * @details
	 *	- Handles initialization and shutdown of FMOD system.
	 *  - Manages channel groups for SFX, BGM, and UI audio types.
	 *  - Caches loaded sounds to avoid redundant loading.
	 *	- Provides methods to play sounds, stop sounds by type, and adjust group volumes.
	 *	- Exposes access to the underlying FMOD system and channel groups.
	 *	- Designed for use within the engine's audio system.
     */
    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        bool Init();
		void OnUpdate(float deltaTime);
        void Shutdown();

		void PlaySound(AudioComponent* audio, TransformComponent* transform, RigidbodyComponent* rb);
		void PauseSound(AudioComponent* audio, bool pause);
		void StopSound(AudioComponent* audio);

		void UpdateSound(AudioComponent* audio, TransformComponent* transform, RigidbodyComponent* rb);

		void CheckChannelValid(AudioComponent* audio);

		void PauseGroup(AudioType type, bool pause);
		void PauseAll(bool pause);

        void StopAll();
		void StopByType(AudioType type);

		void GetGroupVolume(AudioType type, float& volume);
		void GetGroupPitch(AudioType type, float& pitch);
		bool IsGroupMuted(AudioType type);
        
		void SetGroupVolume(AudioType type, float volume);
		void SetGroupPitch(AudioType type, float pitch);
		void MuteGroup(AudioType type, bool mute);

		/*void GetMasterVolume(float& volume);
		void GetMasterPitch(float& pitch);
		bool IsMasterMuted();

		void SetMasterVolume(float volume);
		void SetMasterPitch(float pitch);
		void MuteMaster(bool mute);*/

		void SetListenerAttributes(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& velocity);

		FMOD::ChannelGroup* GetGroup(AudioType type);

		FMOD::DSP* CreateDSP(DSPEffectType effect, AudioType group);
		void EnableDSP(AudioType group, DSPEffectType effect, bool enable);
		FMOD::DSP* GetDSP(AudioType group, DSPEffectType effect);
		void SetDSPParameter(AudioType group, DSPEffectType effect, int paramIndex, float value);
		void ReleaseSpecificDSPinGroup(AudioType group, DSPEffectType effect);
		void ReleaseDSPByGroup(AudioType group);
		void ReleaseAllDSPs();

		FMOD::System* GetSystem() const { return coresystem; }
		std::unordered_map<std::string, FMOD::Sound*>& GetSoundCache() { return soundCache; }

		void SetEditorCap(AudioType type, float cap);
		float GetEditorCap(AudioType type) const;

		void SetPlayerVolume(AudioType type, float volume);
		float GetPlayerVolume(AudioType type) const;

		void ApplyBusVolume(AudioType type);
		void ApplyAllBusVolumes();

    private:
        bool CreateChannelGroups();

		FMOD::Sound* LoadSound(const std::string& filepath, bool stream);

		void UnloadSound(const std::string& filepath);

		static bool LogFMODError(FMOD_RESULT result, const char* context);

		void ApplyDirtySettings(AudioComponent* audio);

		float ComputeFinalGroupVolume(float engineDefault, float playerSlider);

		FMOD::System* coresystem = nullptr;

		FMOD::ChannelGroup* mastergroup = nullptr;
		FMOD::ChannelGroup* sfxgroup = nullptr;
		FMOD::ChannelGroup* bgmgroup = nullptr;
		FMOD::ChannelGroup* uigroup = nullptr;
		FMOD::ChannelGroup* vogroup = nullptr;
		FMOD::ChannelGroup* gamesfxgroup = nullptr;

		std::unordered_map<std::string, FMOD::Sound*> soundCache;

		std::unordered_map<DSPEffectType, FMOD::DSP*> m_MasterDSPs;
		std::unordered_map<DSPEffectType, FMOD::DSP*> m_SFXDSPs;
		std::unordered_map<DSPEffectType, FMOD::DSP*> m_BGMDSPs;
		std::unordered_map<DSPEffectType, FMOD::DSP*> m_UIDSPs;

		std::unordered_map<AudioType, MixerBusSettings> m_MixerBusSettings;

		bool initialized = false;

		// Define a mask that covers all rolloff modes
		const FMOD_MODE RolloffMask =
			FMOD_3D_INVERSEROLLOFF |
			FMOD_3D_LINEARROLLOFF |
			FMOD_3D_LINEARSQUAREROLLOFF |
			FMOD_3D_CUSTOMROLLOFF;
    };

} // namespace Engine
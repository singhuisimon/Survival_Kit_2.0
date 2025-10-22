#pragma once
#include "ECS/Components.h"
#include <fmod.hpp>
#include <fmod_errors.h>
#include <unordered_map>
#include <string>

namespace Engine {

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

		void GetMasterVolume(float& volume);
		void GetMasterPitch(float& pitch);
		bool IsMasterMuted();

		void SetMasterVolume(float volume);
		void SetMasterPitch(float pitch);
		void MuteMaster(bool mute);

		void SetListenerAttributes(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& velocity);

		FMOD::ChannelGroup* GetGroup(AudioType type);

		FMOD::System* GetSystem() const { return coresystem; }
		std::unordered_map<std::string, FMOD::Sound*>& GetSoundCache() { return soundCache; }
    private:
        bool CreateChannelGroups();
		FMOD::Sound* LoadSound(const std::string& filepath, bool stream);
		void UnloadSound(const std::string& filepath);
		static bool LogFMODError(FMOD_RESULT result, const char* context);
		std::string GetFullPath(const std::string& filepath);

		FMOD::System* coresystem = nullptr;

		FMOD::ChannelGroup* mastergroup = nullptr;
		FMOD::ChannelGroup* sfxgroup = nullptr;
		FMOD::ChannelGroup* bgmgroup = nullptr;
		FMOD::ChannelGroup* uigroup = nullptr;

		std::unordered_map<std::string, FMOD::Sound*> soundCache;
		bool initialized = false;
    };

} // namespace Engine
#include <fmod.hpp>
#include "Utility/Logger.h"
#include "Utility/AssetPath.h"
#include "AudioManager.h"

namespace Engine {
	
	AudioManager::AudioManager() {}
	AudioManager::~AudioManager() { Shutdown(); }

	bool AudioManager::Init() {

		if(initialized)
			return true;

		FMOD_RESULT result;

		// Create FMOD system
		result = FMOD::System_Create(&coresystem);
		if (!LogFMODError(result, "FMOD::System_Create") || !coresystem) {
			coresystem = nullptr;
			return false;
		}

		result = coresystem->init(512, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, nullptr);
		if (!LogFMODError(result, "FMOD::System::init")) {
			coresystem->release();
			coresystem = nullptr;
			return false;
		}

		if(!CreateChannelGroups()) {
			LOG_ERROR("AudioManager::Init - Failed to create channel groups");
			return false;
		}

		result = coresystem->set3DSettings(1.0f, 1.0f, 1.0f);
		if (!LogFMODError(result, "set3DSettings")) {
			coresystem->release();
			coresystem = nullptr;
			return false;
		}

		initialized = true;
		LOG_INFO("AudioManager initialized successfully");
		return true;
	}

	bool AudioManager::CreateChannelGroups() {
		FMOD_RESULT result;

		// Get master group
		result = coresystem->getMasterChannelGroup(&mastergroup);
		if (!LogFMODError(result, "getMasterChannelGroup")) {
			return false;
		}

		// Create SFX group
		result = coresystem->createChannelGroup("SFX", &sfxgroup);
		if (!LogFMODError(result, "createChannelGroup - SFX")) {
			return false;
		}

		// Create BGM group
		result = coresystem->createChannelGroup("BGM", &bgmgroup);
		if (!LogFMODError(result, "createChannelGroup - BGM")) {
			return false;
		}
		
		// Create UI group
		result = coresystem->createChannelGroup("UI", &uigroup);
		if (!LogFMODError(result, "createChannelGroup - UI")) {
			return false;
		}
		
		// Set up hierarchy
		result = mastergroup->addGroup(sfxgroup);
		if (!LogFMODError(result, "Add Group - SFX")) {
			return false;
		}

		result = mastergroup->addGroup(bgmgroup);
		if (!LogFMODError(result, "Add Group - BGM")) {
			return false;
		}

		result = mastergroup->addGroup(uigroup);
		if (!LogFMODError(result, "Add Group - UI")) {
			return false;
		}

		return true;
	}

	void AudioManager::OnUpdate(float) {
		if (!initialized) {
			return;
		}

		coresystem->update();
	}

	void AudioManager::Shutdown() {
		if (!initialized) {
			return;
		}

		// Stop all sounds
		StopAll();

		ReleaseAllDSPs();

		// Release sounds
		for (auto& pair : soundCache) {
			if (pair.second) {
				pair.second->release();
			}
		}

		soundCache.clear();
		m_MasterDSPs.clear();
		m_BGMDSPs.clear();
		m_SFXDSPs.clear();
		m_UIDSPs.clear();

		// Release channel groups
		if (uigroup) {
			uigroup->release();
			uigroup = nullptr;
		}
		if (bgmgroup) {
			bgmgroup->release();
			bgmgroup = nullptr;
		}
		if (sfxgroup) {
			sfxgroup->release();
			sfxgroup = nullptr;
		}
		if (mastergroup) {
			mastergroup = nullptr;
		}

		if (coresystem) {
			coresystem->release();
			coresystem = nullptr;
		}

		initialized = false;
		LOG_INFO("AudioManager shutdown completed");
	}

	void AudioManager::PlaySound(AudioComponent* audio, TransformComponent* transform, RigidbodyComponent* rb) {
		if (!initialized || !coresystem || !audio) {
			LOG_WARNING("AudioManager::PlaySound - Not initialized or invalid audio component");
			return;
		}

		if(audio->AudioFilePath.empty()) {
			LOG_WARNING("AudioManager::PlaySound - Audio file path is empty");
			return;
		}

		auto it = soundCache.find(audio->AudioFilePath);

		if(it == soundCache.end()) {
			LOG_INFO("AudioManager::PlaySound - Sound not in cache, loading: ", audio->AudioFilePath);

			// Determine if streaming is needed
			bool stream = (audio->Type == AudioType::BGM);

			// Load sound
			FMOD::Sound* sound = LoadSound(audio->AudioFilePath, stream);
			if (!sound) {
				LOG_WARNING("AudioManager::PlaySound - Failed to load sound: ", audio->AudioFilePath);
				return;
			}
			
			it = soundCache.find(audio->AudioFilePath);
		}

		LOG_INFO("AudioManager::PlaySound - Sound is loaded: ", audio->AudioFilePath);

		FMOD::Sound* sound = it->second;
		if (!sound) {
			LOG_CRITICAL("AudioManager::PlaySound - Sound pointer is null for: ", audio->AudioFilePath);
			return;
		}

		if (!audio->PreviousPath.empty() && (audio->PreviousPath != audio->AudioFilePath)) {
			// Stop previous sound if different
			StopSound(audio);
			LOG_INFO("AudioManager::PlaySound - Stopped previous sound: ", audio->PreviousPath);
		}

		FMOD::Channel* channel = nullptr;
		FMOD::ChannelGroup* group = GetGroup(audio->Type);

		FMOD_RESULT result = coresystem->playSound(sound, group, true, &channel);
		if (!LogFMODError(result, "playSound")) {
			LOG_WARNING("AudioManager::PlaySound - Failed to play sound: ", audio->AudioFilePath);
			return;
		}

		audio->Channel = channel;
		audio->PreviousPath = audio->AudioFilePath;

		if (channel) {
			channel->setVolume(audio->Volume);
			channel->setPitch(audio->Pitch);
			channel->setMute(audio->Mute);
			channel->setReverbProperties(0, audio->ReverbProperties);

			FMOD_MODE mode = FMOD_DEFAULT;
			mode |= audio->Is3D ? FMOD_3D : FMOD_2D;
			mode |= audio->Loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
			channel->setMode(mode);
			if (audio->Loop) {
				channel->setLoopCount(-1); // Infinite loop
			}
			else {
				channel->setLoopCount(0); // No loop
			}

			if (audio->Is3D && transform) {
				FMOD_VECTOR pos = { transform->Position.x, transform->Position.y, transform->Position.z };
				FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f }; // Static for now
				if (rb) {
					vel.x = rb->Velocity.x;
					vel.y = rb->Velocity.y;
					vel.z = rb->Velocity.z;
				}
				channel->set3DAttributes(&pos, &vel);
				channel->set3DMinMaxDistance(audio->MinDistance, audio->MaxDistance);
			}
			else {
				FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f };
				channel->set3DAttributes(&pos, nullptr);
			}

			channel->setPaused(false);
			LOG_INFO("AudioManager::PlaySound - Playing sound: ", audio->AudioFilePath);
		}
		
		//if(audio->Channel) {
		//	LOG_INFO("AudioManager::PlaySound - Audio channel exists for sound: ", audio->AudioFilePath);
		//	bool isPlaying = false;
		//	bool isPaused = false;
		//	audio->Channel->isPlaying(&isPlaying);
		//	audio->Channel->getPaused(&isPaused);

		//	if (isPlaying && !isPaused) {
		//		audio->Channel->setVolume(audio->Volume);
		//		audio->Channel->setPitch(audio->Pitch);

		//		bool muted = false;
		//		audio->Channel->getMute(&muted);
		//		if (muted != audio->Mute) {
		//			audio->Channel->setMute(audio->Mute);
		//		}

		//		FMOD_MODE currentmode;
		//		audio->Channel->getMode(&currentmode);

		//		if (audio->Loop && !(currentmode & FMOD_LOOP_NORMAL)) {
		//			audio->Channel->setMode(FMOD_LOOP_NORMAL);
		//			audio->Channel->setLoopCount(-1); // Infinite loop
		//			LOG_INFO("AudioManager::PlaySound - Set sound to loop: ", audio->AudioFilePath);
		//		}
		//		else if (!audio->Loop && (currentmode & FMOD_LOOP_NORMAL)) {
		//			audio->Channel->setMode(FMOD_LOOP_OFF);
		//			audio->Channel->setLoopCount(0); // No loop
		//			LOG_INFO("AudioManager::PlaySound - Set sound to no loop: ", audio->AudioFilePath);
		//		}

		//		audio->Channel->setReverbProperties(0, audio->Reverb ? 1.0f : 0.0f);

		//		bool is3D = (currentmode & FMOD_3D) != 0;
		//		if (audio->Is3D && !is3D) {
		//			audio->Channel->setMode((currentmode & ~FMOD_2D) | FMOD_3D);
		//			LOG_INFO("AudioManager::PlaySound - Updated 3D mode for sound: ", audio->AudioFilePath);
		//		}
		//		else if (!audio->Is3D && is3D) {
		//			audio->Channel->setMode((currentmode & ~FMOD_3D & ~FMOD_3D_LINEARROLLOFF) | FMOD_2D);
		//			LOG_INFO("AudioManager::PlaySound - Updated 2D mode for sound: ", audio->AudioFilePath);
		//		}

		//		if (audio->Is3D) {
		//			if (transform) {
		//				FMOD_VECTOR pos = { transform->Position.x, transform->Position.y, transform->Position.z };
		//				FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f }; // Static for now
		//				audio->Channel->set3DAttributes(&pos, &vel);
		//				LOG_INFO("AudioManager::PlaySound - Setting 3D position for sound: ", audio->AudioFilePath, " Position: (", pos.x, ", ", pos.y, ", ", pos.z, ")");
		//			}

		//			audio->Channel->set3DMinMaxDistance(audio->MinDistance, audio->MaxDistance);
		//		}

		//		LOG_INFO("AudioManager::PlaySound - Sound is already playing: ", audio->AudioFilePath);
		//		return;
		//	}
		//	else if (!isPlaying && !isPaused) {
		//		StopSound(audio);
		//		LOG_INFO("AudioManager::PlaySound - Stop sound: ", audio->AudioFilePath);
		//		return;
		//	}
		//	else if (isPaused) {
		//		PauseSound(audio, false);
		//		LOG_INFO("AudioManager::PlaySound - Resumed paused sound: ", audio->AudioFilePath);
		//		return;
		//	}
		//}
	}

	void AudioManager::PauseSound(AudioComponent* audio, bool pause) {
		if (!initialized || !audio || !audio->Channel) {
			LOG_WARNING("AudioManager::PauseSound - Not initialized or invalid audio component/channel");
			return;
		}

		bool isPaused = false;
		audio->Channel->getPaused(&isPaused);

		if (isPaused == pause) {
			LOG_INFO("AudioManager::PauseSound - Sound already in desired pause state: ", audio->AudioFilePath);
			return;
		}

		FMOD_RESULT result = audio->Channel->setPaused(pause);

		if (!LogFMODError(result, "PauseSound")) {
			LOG_WARNING("AudioManager::PauseSound - Failed to set pause state for ", audio->AudioFilePath);
			return;
		}

		//audio->PreviousState = audio->State;
		//audio->State = pause ? PlayState::PAUSE : PlayState::PLAY;
		LOG_INFO("AudioManager::PauseSound - ", (pause ? "Paused" : "Resumed"), " sound: ", audio->AudioFilePath);
	}

	void AudioManager::StopSound(AudioComponent* audio) {
		if (!initialized || !audio || !audio->Channel) {
			LOG_WARNING("AudioManager::StopSound - Not initialized or invalid audio component/channel");
			return;
		}

		bool isPlaying = false;
		audio->Channel->isPlaying(&isPlaying);
		if (isPlaying) {
			float current_volume;
			audio->Channel->getVolume(&current_volume);

			unsigned long long dspClock;
			audio->Channel->getDSPClock(nullptr, &dspClock);
			audio->Channel->addFadePoint(dspClock, current_volume);
			audio->Channel->addFadePoint(dspClock + 44100, 0.0f); // 1 second fade
			
			audio->Channel->setVolume(0.0f);
			audio->Channel->stop();
			LOG_INFO("AudioManager::StopSound - Stopped sound: ", audio->AudioFilePath);
		}

		audio->Channel = nullptr;
		LOG_INFO("AudioManager::StopSound - Set State to STOP: ", audio->AudioFilePath);
		audio->PreviousPath = "";
	}

	void AudioManager::UpdateSound(AudioComponent* audio, TransformComponent* transform, RigidbodyComponent* rb) {
		if (!initialized || !audio || !audio->Channel) {
			return;
		}

		FMOD::Channel* channel = audio->Channel;
		bool isPlaying = false;
		channel->isPlaying(&isPlaying);
		if (!isPlaying) {
			return;
		}

		//Update playback parameters
		channel->setVolume(audio->Volume);
		channel->setPitch(audio->Pitch);
		channel->setMute(audio->Mute);
		channel->setReverbProperties(0, audio->ReverbProperties);

		//Looping mode
		FMOD_MODE mode;
		channel->getMode(&mode);
		if (audio->Loop && !(mode & FMOD_LOOP_NORMAL)) {
			channel->setMode((mode & ~FMOD_LOOP_OFF) | FMOD_LOOP_NORMAL);
			channel->setLoopCount(-1);
		}
		else if (!audio->Loop && (mode & FMOD_LOOP_NORMAL)) {
			channel->setMode((mode & ~FMOD_LOOP_NORMAL) | FMOD_LOOP_OFF);
			channel->setLoopCount(0);
		}

		//3D mode update
		if (audio->Is3D && !(mode & FMOD_3D)) {
			channel->setMode((mode & ~FMOD_2D) | FMOD_3D);
		}
		else if (!audio->Is3D && (mode & FMOD_3D)) {
			channel->setMode((mode & ~FMOD_3D) | FMOD_2D);
		}

		//update 3d attributes
		if (audio->Is3D && transform) {
			FMOD_VECTOR pos = { transform->Position.x, transform->Position.y, transform->Position.z };
			FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
			if (rb) {
				vel.x = rb->Velocity.x;
				vel.y = rb->Velocity.y;
				vel.z = rb->Velocity.z;
			}
			channel->set3DAttributes(&pos, &vel);
			channel->set3DMinMaxDistance(audio->MinDistance, audio->MaxDistance);
		}

		//For tracing debug purpose.
		//LOG_TRACE("AudioManager::UpdateSound - Update sound properties: ", audio->AudioFilePath);
	}

	void AudioManager::CheckChannelValid(AudioComponent* audio) {
		if (!initialized || !audio) {
			return;
		}

		if (audio->State == PlayState::PLAY && audio->Channel) {
			bool isPlaying = false;
			FMOD_RESULT result = audio->Channel->isPlaying(&isPlaying);

			if (!(LogFMODError(result, "Checking Channel Play Status")) || !isPlaying) {
				audio->Channel = nullptr;
				//audio->PreviousState = audio->State;
				//audio->State = PlayState::STOP;

				LOG_INFO("AudioManager - Auto-Stop: {} finish playing", audio->AudioFilePath);
			}
		}
	}

	void AudioManager::PauseGroup(AudioType type, bool pause) {
		if (!initialized)
			return;

		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			group->setPaused(pause);
		}
	}

	void AudioManager::PauseAll(bool pause) {
		if (!initialized)
			return;

		if(!mastergroup)
			return;

		mastergroup->setPaused(pause);
	}

	void AudioManager::StopAll() {
		if (!initialized)
			return;

		if(!mastergroup)
			return;
		
		mastergroup->stop();
	}

	void AudioManager::StopByType(AudioType type) {
		if (!initialized)
			return;

		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			group->stop();
		}

		ReleaseDSPByGroup(type);
	}

	FMOD::Sound* AudioManager::LoadSound(const std::string& filepath, bool stream) {

		if (!coresystem || !initialized) {
			LOG_ERROR("AudioManager::LoadSound - FMOD system not initialized");
			return nullptr;
		}

		if (soundCache.find(filepath) != soundCache.end()) {
			return soundCache[filepath];
		}


		// Use AssetPath helper
		std::string fullpath = getAssetFilePath("Sources/Audio/" + filepath);

		FMOD::Sound* newSound = nullptr;

		FMOD_MODE mode = FMOD_DEFAULT;
		mode |= FMOD_3D;
		mode |= FMOD_LOOP_OFF;

		if (stream) {
			mode |= FMOD_CREATESTREAM;
		} else {
			mode |= FMOD_CREATESAMPLE;
		}

		FMOD_RESULT result = coresystem->createSound(fullpath.c_str(), mode, nullptr, &newSound);
		if (!LogFMODError(result, ("createSound - " + fullpath).c_str())) {
			return nullptr;
		}

		soundCache[filepath] = newSound;
		LOG_INFO("Loaded sound: ", filepath);
		return newSound;
	}

	void AudioManager::UnloadSound(const std::string& filepath) {
		if (filepath.empty() || !coresystem || !initialized) {
			return;
		}

		auto it = soundCache.find(filepath);
		if (it == soundCache.end()) {
			LOG_INFO("AudioManager::UnloadSound - Audio not found in soundCache");
			return;
		}

		it->second->release();
		soundCache.erase(it);

		LOG_INFO("AudioManager::UnloadSound - released audio: ", filepath.c_str());
	}

	void AudioManager::GetGroupVolume(AudioType type, float& volume) {
		if (!initialized)
			return;
		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			group->getVolume(&volume);
		}
	}

	void AudioManager::GetGroupPitch(AudioType type, float& pitch) {
		if (!initialized)
			return;
		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			group->getPitch(&pitch);
		}
	}

	bool AudioManager::IsGroupMuted(AudioType type) {
		if (!initialized)
			return false;
		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			bool mute = false;
			group->getMute(&mute);
			return mute;
		}
		return false;
	}

	void AudioManager::SetGroupVolume(AudioType type, float volume) {
		if (!initialized)
			return;

		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			group->setVolume(volume);
		}
	}

	void AudioManager::SetGroupPitch(AudioType type, float pitch) {
		if (!initialized)
			return;

		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			group->setPitch(pitch);
		}
	}

	void AudioManager::MuteGroup(AudioType type, bool mute) {
		if (!initialized)
			return;

		FMOD::ChannelGroup* group = GetGroup(type);
		if (group) {
			group->setMute(mute);
		}
	}

	void AudioManager::GetMasterVolume(float& volume) {
		if (!initialized || !mastergroup)
			return;
		mastergroup->getVolume(&volume);
	}

	void AudioManager::GetMasterPitch(float& pitch) {
		if (!initialized || !mastergroup)
			return;
		mastergroup->getPitch(&pitch);
	}

	void AudioManager::MuteMaster(bool mute) {
		if (!initialized || !mastergroup)
			return;
		mastergroup->setMute(mute);
	}

	void AudioManager::SetMasterVolume(float volume) {
		if (!initialized || !mastergroup)
			return;
		mastergroup->setVolume(volume);
	}

	void AudioManager::SetMasterPitch(float pitch) {
		if (!initialized || !mastergroup)
			return;
		mastergroup->setPitch(pitch);
	}

	bool AudioManager::IsMasterMuted() {
		if (!initialized || !mastergroup)
			return false;
		bool mute = false;
		mastergroup->getMute(&mute);
		return mute;
	}

	void AudioManager::SetListenerAttributes(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& velocity) {
		if (!initialized || !coresystem)
			return;
		FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
		FMOD_VECTOR fmodForward = { forward.x, forward.y, forward.z };
		FMOD_VECTOR fmodUp = { up.x, up.y, up.z };
		FMOD_VECTOR fmodVelocity = { velocity.x, velocity.y, velocity.z };
		FMOD_RESULT result = coresystem->set3DListenerAttributes(0, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);
		LogFMODError(result, "set3DListenerAttributes");
	}

	FMOD::ChannelGroup* AudioManager::GetGroup(AudioType type) {
		switch (type) {
		case AudioType::MASTER:
			return mastergroup;
		case AudioType::SFX:
			return sfxgroup;
		case AudioType::BGM:
			return bgmgroup;
		case AudioType::UI:
			return uigroup;
		default:
			return nullptr;
		}
	}

	bool AudioManager::LogFMODError(FMOD_RESULT result, const char* context) {

		if (result != FMOD_OK) {
			LOG_ERROR("FMOD Error in ", context, ": ", FMOD_ErrorString(result));
			return false;
		}

		return true;
	}

	// AudioManager.cpp — NEW HELPER
	void AudioManager::ApplyDirtySettings(AudioComponent* audio) {
		if (!audio || !audio->Channel) return;

		// Volume / Pitch / Mute
		audio->Channel->setVolume(audio->Volume);
		audio->Channel->setPitch(audio->Pitch);
		audio->Channel->setMute(audio->Mute);

		// Loop settings
		FMOD_MODE mode = FMOD_DEFAULT;
		mode |= audio->Is3D ? FMOD_3D : FMOD_2D;
		mode |= audio->Loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		audio->Channel->setMode(mode);
		audio->Channel->setLoopCount(audio->Loop ? -1 : 0);

		// Reverb send (simple toggle)
		audio->Channel->setReverbProperties(0, audio->ReverbProperties);

		audio->IsDirty = false; // synced
	}


	FMOD::DSP* AudioManager::CreateDSP(DSPEffectType effect, AudioType group) {
		if (!initialized || !coresystem) {
			return nullptr;
		}

		auto& dspMap = (group == AudioType::MASTER) ? m_MasterDSPs :
						(group == AudioType::SFX) ? m_SFXDSPs :
						(group == AudioType::BGM) ? m_BGMDSPs : m_UIDSPs;

		//If already created for this group, just reuse it
		if (auto it = dspMap.find(effect); it != dspMap.end() && it->second) {
			return it->second;
		}

		FMOD::DSP* dsp = nullptr;
		FMOD_RESULT result = coresystem->createDSPByType(DSPEffectUtil::ToFMODType(effect), &dsp);
		if (!LogFMODError(result, DSPEffectUtil::ToString(effect).c_str())) {
			return nullptr;
		}

		FMOD::ChannelGroup* targetgroup = GetGroup(group);
		if (!targetgroup) {
			dsp->release();
			return nullptr;
		}

		targetgroup->addDSP(FMOD_CHANNELCONTROL_DSP_TAIL, dsp);
		dsp->setBypass(true); //start disabled

		dspMap[effect] = dsp;

		LOG_INFO("Created DSP: ", DSPEffectUtil::ToString(effect), " for group ", (int)group);
		return dsp;
	}

	void AudioManager::EnableDSP(AudioType group, DSPEffectType effect, bool enable) {
		auto& dspMap = (group == AudioType::MASTER) ? m_MasterDSPs :
					(group == AudioType::SFX) ? m_SFXDSPs :
					(group == AudioType::BGM) ? m_BGMDSPs :
					m_UIDSPs;

		auto it = dspMap.find(effect);
		if (it == dspMap.end() || !it->second) {
			LOG_WARNING("EnableDSP: DSP not found for ", DSPEffectUtil::ToString(effect));
			return;
		}

		it->second->setBypass(!enable);
		LOG_INFO("DSP ", DSPEffectUtil::ToString(effect), (enable ? " enabled" : " disabled"),
			" on group ", (int)group);
	}

	FMOD::DSP* AudioManager::GetDSP(AudioType group, DSPEffectType effect) {
		auto& dspMap = (group == AudioType::MASTER) ? m_MasterDSPs :
			(group == AudioType::SFX) ? m_SFXDSPs :
			(group == AudioType::BGM) ? m_BGMDSPs : m_UIDSPs;
		auto it = dspMap.find(effect);
		return (it != dspMap.end()) ? it->second : nullptr;
	}

	void AudioManager::SetDSPParameter(AudioType group, DSPEffectType effect, int paramIndex, float value) {
		auto& dspMap = (group == AudioType::MASTER) ? m_MasterDSPs :
					(group == AudioType::SFX) ? m_SFXDSPs :
					(group == AudioType::BGM) ? m_BGMDSPs :
					m_UIDSPs;

		auto it = dspMap.find(effect);
		if (it != dspMap.end() && it->second) {
			it->second->setParameterFloat(paramIndex, value);
			LOG_TRACE("Set parameter ", paramIndex, " = ", value,
				" for DSP ", DSPEffectUtil::ToString(effect), " in group ", (int)group);
		}
	}

	void AudioManager::ReleaseDSP(AudioType group, DSPEffectType effect) {
		auto& dspMap = (group == AudioType::MASTER) ? m_MasterDSPs :
					(group == AudioType::SFX) ? m_SFXDSPs :
					(group == AudioType::BGM) ? m_BGMDSPs :
					m_UIDSPs;

		auto it = dspMap.find(effect);
		if (it != dspMap.end() && it->second) {
			FMOD::DSP* dsp = it->second;
			FMOD::ChannelGroup* groupPtr = GetGroup(group);

			if (groupPtr) {
				//Directly remove the DSP from the chain
				groupPtr->removeDSP(dsp);
			}

			//Then release and erase
			dsp->release();
			dspMap.erase(it);

			LOG_INFO("Released DSP: ", DSPEffectUtil::ToString(effect));
		}
	}

	void AudioManager::ReleaseDSPByGroup(AudioType group)
	{
		auto& dspMap = (group == AudioType::MASTER) ? m_MasterDSPs :
			(group == AudioType::SFX) ? m_SFXDSPs :
			(group == AudioType::BGM) ? m_BGMDSPs :
			m_UIDSPs;

		for (auto& [effect, dsp] : dspMap)
		{
			if (dsp)
			{
				FMOD::ChannelGroup* groupPtr = GetGroup(group);
				if (groupPtr)
					groupPtr->removeDSP(dsp);

				dsp->release();
				LOG_INFO("Released DSP: ", DSPEffectUtil::ToString(effect), " from group ", (int)group);
			}
		}
		dspMap.clear();
	}

	void AudioManager::ReleaseAllDSPs() {
		auto releaseMap = [](std::unordered_map<DSPEffectType, FMOD::DSP*>& map) {
			for (auto& [type, dsp] : map) {
				if (dsp) dsp->release();
			}
			map.clear();
		};

		releaseMap(m_MasterDSPs);
		releaseMap(m_SFXDSPs);
		releaseMap(m_BGMDSPs);
		releaseMap(m_UIDSPs);

		LOG_INFO("All DSPs released");
	}


} // namespace Engine
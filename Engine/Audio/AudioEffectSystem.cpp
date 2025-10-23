#include <fmod.hpp>
#include <fmod_common.h>
#include "Utility/Logger.h"
#include "AudioEffectSystem.h"

namespace Engine {

	AudioEffectSystem::AudioEffectSystem(AudioManager* audioManager): m_AudioManager(audioManager), Initialized(false){}

	AudioEffectSystem::~AudioEffectSystem(){

		if (Initialized) {
			OnShutdown(nullptr);
		}

	}

	void AudioEffectSystem::OnInit(Scene* scene) {

		if (!scene || !m_AudioManager) {
			LOG_ERROR("AudioEffectSystem::OnInit failed - Invalid Dependencies");
			return;
		}

		auto& registry = scene->GetRegistry();

		// Cleanup ReverbComponent on destroy
		registry.on_destroy<ReverbZoneComponent>().connect<&AudioEffectSystem::OnReverbComponentRemoved>(*this);

		Initialized = true;
		LOG_INFO("AudioEffectSystem initialized successfully");
	}

	void AudioEffectSystem::OnUpdate(Scene* scene, Timestep ts) {
		(void)ts;

		if (!Initialized || !scene || !m_AudioManager) {
			LOG_WARNING("AudioEffectSystem::OnUpdate fail to update as not init, null scene, etc");
			return;
		}

		UpdateReverbZones(scene);
	}

	void AudioEffectSystem::OnShutdown(Scene* scene) {
		if (!Initialized)
			return;

		if (!m_AudioManager || !m_AudioManager->GetSystem()) {
			LOG_WARNING("AudioEffectSystem::OnShutdown - AudioManager already null or system released!");
		}

		LOG_INFO("AudioEffectSystem shutting down....");

		// Disconnect event
		if (scene) {
			auto& registry = scene->GetRegistry();
			registry.on_destroy<ReverbZoneComponent>().disconnect<&AudioEffectSystem::OnReverbComponentRemoved>(*this);
		}

		DestroyReverbZones();

		Initialized = false;
		LOG_INFO("AudioEffectSystem shutting down....");
	}

	void AudioEffectSystem::UpdateReverbZones(Scene* scene) {
		if (!m_AudioManager || !m_AudioManager->GetSystem()) {
			return;
		}

		auto& registry = scene->GetRegistry();
		auto view = registry.view<ReverbZoneComponent>();

		std::unordered_set<entt::entity> activeEntities;

		for (auto entityHandle : view) {
			Entity entity(entityHandle, &registry);
			auto& reverb = entity.GetComponent<ReverbZoneComponent>();

			TransformComponent* transform = entity.HasComponent<TransformComponent>()
				? &entity.GetComponent<TransformComponent>() : nullptr;

			activeEntities.insert(entityHandle);

			if (!reverb.ReverbZone || !reverbzones.contains(entityHandle)) {
				CreateReverbZone(entity, reverb, transform);
			}
			else if(reverb.IsDirty){
				
				//Update exisiting zone
				UpdateReverbZone(reverb, transform);

				reverb.IsDirty = false;
			}
			else if (transform) {
				FMOD_VECTOR pos = { transform->Position.x, transform->Position.y, transform->Position.z };
				reverb.ReverbZone->set3DAttributes(&pos, reverb.MinDistance, reverb.MaxDistance);
			}
		}

		// Clean up missing entities
		for (auto it = reverbzones.begin(); it != reverbzones.end();) {
			if (!activeEntities.contains(it->first)) {
				LOG_INFO("Cleaning up orphaned reverb zone for entities");
				if (it->second) {
					it->second->setActive(false);
					it->second->release(); 
				}
				it = reverbzones.erase(it);
			}
			else {
				++it;
			}
		}
	}

	void AudioEffectSystem::CreateReverbZone(Entity entity, ReverbZoneComponent& reverb, const TransformComponent* transform) {
		if (!m_AudioManager || !m_AudioManager->GetSystem()) {
			LOG_ERROR("AudioEffectSystem::CreateReverbZone failed - AudioManager not initialized");
			return;
		}

		FMOD::Reverb3D* newReverb = nullptr;
		FMOD_RESULT result = m_AudioManager->GetSystem()->createReverb3D(&newReverb);

		if (result != FMOD_OK || !newReverb) {
			LOG_ERROR("Failed to create FMOD Reverb3D: ", FMOD_ErrorString(result));
			return;
		}

		FMOD_REVERB_PROPERTIES props;
		GetReverbProperties(reverb, props);

		result = newReverb->setProperties(&props);
		if (result != FMOD_OK) {
			LOG_ERROR("Fail to set reverb properties: ", FMOD_ErrorString(result));
			newReverb->release();
			return;
		}

		// Set initial pos
		FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f };
		if (transform) {
			pos = { transform->Position.x, transform->Position.y, transform->Position.z };
		}

		result = newReverb->set3DAttributes(&pos, reverb.MinDistance, reverb.MaxDistance);
		if (result != FMOD_OK) {
			LOG_ERROR("Failed to set reverb 3D attributes: ", FMOD_ErrorString(result));
			newReverb->release();
			return;
		}

		// Activate the zone
		result = newReverb->setActive(true);
		if (result != FMOD_OK) {
			LOG_WARNING("Failed to activate reverb zone: ", FMOD_ErrorString(result));
		}

		reverb.ReverbZone = newReverb;
		reverbzones[(entt::entity)entity] = newReverb;
		reverb.IsDirty = false;

		LOG_INFO("Created FMOD Reverb Zone for entity with preset ", static_cast<int>(reverb.Preset));
	}

	void AudioEffectSystem::UpdateReverbZone(ReverbZoneComponent& reverb,
		const TransformComponent* transform) {
		if (!reverb.ReverbZone) {
			return;
		}

		// Update properties
		FMOD_REVERB_PROPERTIES props;
		GetReverbProperties(reverb, props);
		
		FMOD_RESULT result = reverb.ReverbZone->setProperties(&props);
		if (result != FMOD_OK) {
			LOG_WARNING("Failed to update reverb properties: ", FMOD_ErrorString(result));
		}

		// Update position and distance
		FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f };
		if (transform) {
			pos = { transform->Position.x, transform->Position.y, transform->Position.z };
		}
		
		result = reverb.ReverbZone->set3DAttributes(&pos, reverb.MinDistance, reverb.MaxDistance);
		if (result != FMOD_OK) {
			LOG_WARNING("Failed to update reverb 3D attributes: ", FMOD_ErrorString(result));
		}

		LOG_TRACE("Updated reverb zone - Preset: ", static_cast<int>(reverb.Preset));
	}

	void AudioEffectSystem::GetReverbProperties(const ReverbZoneComponent& reverb,
		FMOD_REVERB_PROPERTIES& props) {
		if (reverb.Preset != ReverbPreset::Custom) {
			// Use FMOD preset directly for non custom
			switch (reverb.Preset) {
			case ReverbPreset::Generic:
				props = FMOD_PRESET_GENERIC;
				break;
			case ReverbPreset::Bathroom:
				props = FMOD_PRESET_BATHROOM;
				break;
			case ReverbPreset::Room:
				props = FMOD_PRESET_ROOM;
				break;
			case ReverbPreset::Cave:
				props = FMOD_PRESET_CAVE;
				break;
			case ReverbPreset::Arena:
				props = FMOD_PRESET_ARENA;
				break;
			case ReverbPreset::Custom:
			default:
				props = FMOD_PRESET_GENERIC; // Start with generic as base
				break;
			}

			LOG_TRACE("Using FMOD preset: ", static_cast<int>(reverb.Preset));
		}
		else {
			// Custom preset - start with Generic as base, then override with custom values
			props = FMOD_PRESET_GENERIC;
			ConvertToFmodReverb(reverb, props);
			LOG_TRACE("Using custom reverb settings");
		}
	}

	void AudioEffectSystem::DestroyReverbZones() {
		if (!m_AudioManager || !m_AudioManager->GetSystem()) {
			LOG_WARNING("Skipping reverb zone destruction - FMOD System alerady release");
			reverbzones.clear();
			return;
		}

		if (reverbzones.empty()) {
			LOG_INFO("No reverb zones to destroy");
			return;
		}

		LOG_INFO("Destroying ", reverbzones.size(), " reverb zones...");

		int successCount = 0, failCount = 0;

		for (auto& [entity, zone] : reverbzones) {
			if (zone) {
				//First deactivate
				FMOD_RESULT activeCheck = zone->setActive(false);
				if (activeCheck != FMOD_OK) {
					LOG_WARNING("Failed to deactivate reverb zone: ", FMOD_ErrorString(activeCheck));
				}

				//Then release
				FMOD_RESULT result = zone->release();
				if (result != FMOD_OK) {
					LOG_WARNING("Failed to release reverb zone for entity ", (uint32_t)entity,
						": ", FMOD_ErrorString(result));
					failCount++;
				}
				else {
					LOG_INFO("[DestroyReverbZones] Released zone for entity ", (uint32_t)entity);
					successCount++;
				}
			}
		}

		reverbzones.clear();
		LOG_INFO("[DestroyReverbZones] Cleanup complete: ", successCount, " released, ", failCount, " failed");
	}

	void AudioEffectSystem::OnReverbComponentRemoved(entt::registry& registry, entt::entity entity) {
		// Find and remove the reverb zone
		auto it = reverbzones.find(entity);
		if (it != reverbzones.end()) {
			LOG_INFO("Reverb component removed from entity ", (uint32_t)entity,
				" - cleaning up zone");

			if (it->second) {
				it->second->setActive(false);
				it->second->release();
			}

			reverbzones.erase(it);
		}

		// Also clear the component's pointer if it still exists
		if (ReverbZoneComponent* reverb = registry.try_get<ReverbZoneComponent>(entity)) {
			reverb->ReverbZone = nullptr;
		}
	}

	void AudioEffectSystem::ConvertToFmodReverb(const ReverbZoneComponent& rv, FMOD_REVERB_PROPERTIES& props) {
		// Override the generic preset with custom values
		// All values are clamped to FMOD's valid ranges

		// DecayTime: milliseconds (100 to 20000)
		props.DecayTime = std::clamp(rv.DecayTime, 100.0f, 20000.0f);

		// HFDecayRatio: percentage (10 to 100)
		props.HFDecayRatio = std::clamp(rv.HfDecayRatio, 10.0f, 100.0f);

		// Diffusion: percentage (0 to 100)
		props.Diffusion = std::clamp(rv.Diffusion, 0.0f, 100.0f);

		// Density: percentage (0 to 100)
		props.Density = std::clamp(rv.Density, 0.0f, 100.0f);

		// WetLevel: dB (-80 to 20)
		props.WetLevel = std::clamp(rv.WetLevel, -80.0f, 20.0f);

		LOG_TRACE("Custom reverb properties applied - "
			"Decay: ", props.DecayTime, "ms, "
			"HFRatio: ", props.HFDecayRatio, "%, "
			"Diffusion: ", props.Diffusion, "%, "
			"Density: ", props.Density, "%, "
			"Wet: ", props.WetLevel, "dB");
	}

} // namespace Engine
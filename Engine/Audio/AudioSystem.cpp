#include <fmod.hpp>
#include "Utility/Logger.h"
#include "AudioSystem.h"

namespace Engine {

	AudioSystem::AudioSystem(AudioManager* audioManager): m_AudioManager(audioManager), Initialized(false){}

	AudioSystem::~AudioSystem(){
		OnShutdown(nullptr);
	}

	void AudioSystem::OnInit(Scene* scene) {

		if (!scene || !m_AudioManager) {
			return;
		}

		auto& registry = scene->GetRegistry();

		// Cleanup AudioComponent on destroy
		registry.on_destroy<AudioComponent>().connect<&AudioSystem::OnAudioComponentRemoved>(*this);

		Initialized = true;
		LOG_INFO("AudioSystem initialized successfully");
	}

	void AudioSystem::OnUpdate(Scene* scene, Timestep ts) {
		(void)ts;

		//LOG_TRACE("AudioSystem::OnUpdate Running");

		if (!Initialized || !scene || !m_AudioManager) {
			LOG_WARNING("AudioSystem::OnUpdate fail to update as not init, null scene, etc");
			return;
		}

		UpdateListenerPosition(scene);

		ProcessAudioEntities(scene);
	}

	void AudioSystem::OnShutdown(Scene* scene) {
		(void)scene;

		if (!Initialized)
			return;

		if (scene) {
			auto& registry = scene->GetRegistry();
			registry.on_destroy<AudioComponent>().disconnect<&AudioSystem::OnAudioComponentRemoved>(*this);
		}

		LOG_INFO("AudioSystem shutting down....");
		Initialized = false;
	}

	void AudioSystem::UpdateListenerPosition(Scene* scene) {
		auto& registry = scene->GetRegistry();
		auto listenerview = registry.view<ListenerComponent>();

		for (auto entityHandle : listenerview) {
			Entity entity(entityHandle, &registry);
			auto& listener = entity.GetComponent<ListenerComponent>();

			if (!listener.Active)
				continue;

			RigidbodyComponent* rb = entity.HasComponent<RigidbodyComponent>()
				? &entity.GetComponent<RigidbodyComponent>() : nullptr;

			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 rotation = glm::vec3(0.0f);
			glm::vec3 velocity = rb ? rb->Velocity : glm::vec3(0.0f);

			if (entity.HasComponent<TransformComponent>())
			{
				auto& transform = entity.GetComponent<TransformComponent>();
				position = transform.Position;
				rotation = { transform.Rotation.x, transform.Rotation.y, transform.Rotation.z };
			}

			glm::vec3 forward(0, 0, -1);
			glm::vec3 up(0, 1, 0);

			//Basic forward direction based on Y rotation
			float yaw = glm::radians(rotation.y);
			forward.x = sin(yaw);
			forward.z = -cos(yaw);

			m_AudioManager->SetListenerAttributes(position, forward, up, velocity);

			//only the first active listener is used
			break;
		}
	}

	void AudioSystem::ProcessAudioEntities(Scene* scene) {
		auto& registry = scene->GetRegistry();
		auto view = registry.view<AudioComponent>();

		for (auto entityHandle : view) {
			Entity entity(entityHandle, &registry);
			auto& audio = entity.GetComponent<AudioComponent>();

			TransformComponent* transform = entity.HasComponent<TransformComponent>() ? &entity.GetComponent<TransformComponent>() : nullptr;
			RigidbodyComponent* rb = entity.HasComponent<RigidbodyComponent>() ? &entity.GetComponent<RigidbodyComponent>() : nullptr;

			UpdateAudioComponentState(entity, audio, transform, rb);

			//check if the audio has already stop playing if so ensure the channel in the audiocomponet
			//becomes a nullptr to prevent dangling.
			m_AudioManager->CheckChannelValid(&audio);

			if (audio.State == PlayState::PLAY && !audio.Channel) {
				audio.State = PlayState::STOP;
				LOG_INFO("AudioSystem - Auto Stop detected for finished sound: ", audio.AudioFilePath);
			}

		}

	}

	void AudioSystem::UpdateAudioComponentState(Entity entity, AudioComponent& audio, TransformComponent* transform, RigidbodyComponent* rb) {
		switch (audio.State) {
		case PlayState::PLAY:
			//haven't play any sound as no channel assign
			if (!audio.Channel) {
				m_AudioManager->PlaySound(&audio, transform, rb);
				audio.IsDirty = false;
				LOG_INFO("Entity playing audio: ", audio.AudioFilePath);
			}
			else {
				//sound is already playing
				//check if the audio is previously pause
				bool isPaused = false;
				audio.Channel->getPaused(&isPaused);
				if (isPaused) {
					//if so resume it
					m_AudioManager->PauseSound(&audio, false);
					LOG_INFO("Entity resume audio: ", audio.AudioFilePath);
				}
				//update the sound as it is already playing
				m_AudioManager->UpdateSound(&audio, transform, rb);
			}
			break;

		case PlayState::PAUSE:
			if (audio.Channel) {
				bool isPaused = false;
				audio.Channel->getPaused(&isPaused);
				if (!isPaused) { //only pause if currently not pause
					m_AudioManager->PauseSound(&audio, true);
					LOG_INFO("Entity pause audio: ", audio.AudioFilePath);
				}
			}
			audio.IsDirty = false;
			break;

		case PlayState::STOP:
			if (audio.Channel) {
				m_AudioManager->StopSound(&audio);
				LOG_INFO("Entity stop audio: ", audio.AudioFilePath);
			}
			audio.IsDirty = false;
			break;

		default: 
			break;
		}
	}

	void AudioSystem::OnAudioComponentRemoved(entt::registry& registry, entt::entity entity) {
		Entity e(entity, &registry);

		if (!e.HasComponent<AudioComponent>()) return;

		auto& audio = e.GetComponent<AudioComponent>();

		if (audio.Channel) {
			LOG_INFO("AudioSystem cleanup - releasing FMOD channel for entity {}", (uint32_t)entity);
			
			if (audio.Type != AudioType::UI) {
				audio.Channel->stop();
			}

			audio.Channel = nullptr;
		}
	}

} // namespace Engine
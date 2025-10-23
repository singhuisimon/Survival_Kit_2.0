#pragma once
#include "ECS/Scene.h"
#include "ECS/System.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "AudioManager.h"
#include <fmod.hpp>
#include <fmod_errors.h>
#include <unordered_map>
#include <string>
#include <mutex>

namespace Engine {

    /**
	 * @class AudioSystem
     * @brief ECS system that updates AudioComponents every frame
	 * @details 
     * - Handles entity-level playback state (Play/Pause/Stop)
     * - Updates 3D attributes for sounds
     * - Delegates actual playback to AudioManager
     */
    class AudioSystem : public System {
    public:
        explicit AudioSystem(AudioManager* audioManager);
        ~AudioSystem() override;

		void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, Timestep ts) override;
        void OnShutdown(Scene* scene) override;

        int GetPriority() const override { return 80; }
		const char* GetName() const override { return "AudioSystem"; }

        //void PlayEditor(const std::string& filepath);
        //void StopEditor(const std::string& filepath);
		//PlayState EditorChannelStatus(const std::string& filepath);

    private:
        AudioManager* m_AudioManager;

        bool Initialized;

        void UpdateListenerPosition(Scene* scene);
        void ProcessAudioEntities(Scene* scene);
        void UpdateAudioComponentState(Entity entity, AudioComponent& audio,
            TransformComponent* transform, RigidbodyComponent* rb);
        void OnAudioComponentRemoved(entt::registry& registry, entt::entity entity);
    };

} // namespace Engine
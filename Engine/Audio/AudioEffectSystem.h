/**
 * @file AudioEffectSystem.h
 * @brief Class for DSP audio effect as well as Reverb Zone management in ECS
 * @author Amanda Leow Boon Suan (100%)
 * @date 22/10/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

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

namespace Engine {

    class AudioManger;
    class Scene;

    /**
	 * @class AudioEffectSystem
     * @brief ECS system that updates ReverbComponent, AudioEffect every frame
	 * @details 
     * - Handles the creation of ReverbZones every Scene.
     * - Handles the DSP attach to channel
     */
    class AudioEffectSystem : public System {
    public:
        explicit AudioEffectSystem(AudioManager* audioManager);
        ~AudioEffectSystem() override;

		void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, Timestep ts) override;
        void OnShutdown(Scene* scene) override;

        int GetPriority() const override { return 85; }
		const char* GetName() const override { return "AudioEffectSystem"; }


    private:
        AudioManager* m_AudioManager;

        bool Initialized;

        void CreateReverbZone(Entity entity, ReverbZoneComponent& reverb, const TransformComponent* transform);
        void UpdateReverbZones(Scene* scene);
        void DestroyReverbZones();

        void UpdateReverbZone(ReverbZoneComponent& reverb, const TransformComponent* transform);
        void OnReverbComponentRemoved(entt::registry& registry, entt::entity entity);
        void GetReverbProperties(const ReverbZoneComponent& reverb, FMOD_REVERB_PROPERTIES& props);
        static void ConvertToFmodReverb(const ReverbZoneComponent& rv, FMOD_REVERB_PROPERTIES& props);

        std::unordered_map<entt::entity, FMOD::Reverb3D*> reverbzones;
    };

} // namespace Engine
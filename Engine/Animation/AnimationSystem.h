/**
 * @file AnimationSystem.h
 * @brief	Declaration of Camera System that manages Camera components in the ECS
 * @details Updates and rebuild component's View and Perpsective transform upon modification
 * @author Chua Wen Bin Kenny
 * @date 20 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"
#include "AnimationStorage.h"


namespace Engine {

	class AnimationSystem : public System {
	public:

		AnimationSystem();

		void OnUpdate(Scene* scene, Timestep ts) override;
		int  GetPriority() const override;
		const char* GetName() const override;

		// Sampling functions for transform tracks
		glm::vec3 SamplePosition(const AnimationClip& clip, float t, const glm::vec3& defaultPosition);
		glm::quat SampleRotation(const AnimationClip& clip, float t, const glm::quat& defaultRotation);
		glm::vec3 SampleScale(const AnimationClip& clip, float t, const glm::vec3& defaultScale);
		std::array<float, 2> SampleUVTrack(const std::vector<Engine::UVKeyframe>& keys,
										   float localTime,
										   const std::array<float, 2>& defaultValue);
		

		// Sample and apply an entire clip to a TransformComponent
		void SampleClipAtTime(const AnimationClip& clip, float t, TransformComponent& transform);

		// Getter
		AnimatorController* GetAnimatorController(u32 handle); 
		AnimationClip* GetAnimationClip(u32 handle); 

	private:
	};

}
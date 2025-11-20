/**
 * @file AnimationSystem.cpp
 * @brief	Definition of Camera System that manages Camera components in the ECS
 * @details Updates and rebuild component's View and Perpsective transform upon modification
 * @author Chua Wen Bin Kenny
 * @date 20 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "Animation/AnimationSystem.h"
#include "../ECS/Scene.h"
#include <GLFW/glfw3.h>

 /*-------------------- Utility Helpers ----------------------*/
namespace {
	// Generic segment finder: returns indices [i0, i1] such that
	// keys[i0].time <= t <= keys[i1].time. If out of range, returns
	// (0,0) or (last,last).
	template<typename KeyT>
	std::pair<int, int> FindSegment(const std::vector<KeyT>& keys, float t)
	{
		const int count = static_cast<int>(keys.size());
		if (count == 0) return { -1, -1 };
		if (count == 1) return { 0, 0 };

		if (t <= keys.front().time)  return { 0, 0 };
		if (t >= keys.back().time)   return { count - 1, count - 1 };

		for (int i = 0; i < count - 1; ++i)
		{
			const float t0 = keys[i].time;
			const float t1 = keys[i + 1].time;
			if (t >= t0 && t <= t1)
			{
				return { i, i + 1 };
			}
		}

		return { count - 1, count - 1 };
	}

	inline glm::vec3 LerpVec3(const glm::vec3& a, const glm::vec3& b, float alpha)
	{
		return glm::mix(a, b, alpha); // linear interpolation
	}

	inline glm::quat SlerpQuat(const glm::quat& a, const glm::quat& b, float alpha)
	{
		return glm::slerp(a, b, alpha);
	}
}
/*-------------------- Utility Helpers ----------------------*/

namespace Engine {

	// Initialize temp storage
	std::unordered_map<u32, AnimatorController> m_AnimatorControllerStorage;
	std::unordered_map<u32, AnimationClip> m_AnimationClipStorage;
	
	AnimationSystem::AnimationSystem() : System() {}

	void AnimationSystem::OnUpdate(Scene* scene, Timestep ts) {

		auto& registry = scene->GetRegistry();
		auto animView = registry.view<AnimatorComponent>();
		for (auto anim : animView) {

			// Get entity's animator component
			Entity entity(anim, &registry);
			auto& animator = entity.GetComponent<AnimatorComponent>();

			// Check if it has transform component
			if (!entity.HasComponent<TransformComponent>()) continue;
			auto& transform = entity.GetComponent<TransformComponent>();

			// Skip inactive animators
			if (!animator.playing) continue;

			// Get controller
			AnimatorController* controller = GetAnimatorController(animator.controller);
			if (!controller) continue;

			// Get current clip; ensure current clip is not out of range
			//std::cout << "Controller name: " << controller->name << std::endl;
			//std::cout << "Controller clip size: " << controller->clips.size() << std::endl;
			//std::cout << "Animator current clip index: " << animator.currentClipIndex << std::endl;
			if (animator.currentClipIndex >= static_cast<u32>(controller->clips.size())) continue;

			AnimationClip* clip = GetAnimationClip(controller->clips[animator.currentClipIndex]);
			if (!clip) continue;

			// Advance time
			float scaledDt = ts.GetSeconds() * animator.playbackSpeed;
			animator.currentTime += scaledDt;
			
			// Override loop 
			bool loop = animator.respectClipLoop ? clip->loop : false;

			if (loop) {
				// Wrap time
				if (clip->duration > 0.0f) {
					while (animator.currentTime >= clip->duration)
						animator.currentTime -= clip->duration;
				}
			}
			else {
				// Clamp and stop at end
				if (animator.currentTime >= clip->duration) {
					animator.currentTime = clip->duration;
					animator.playing = false;
				}
			}
			//std::cout << "Animator current time: " << animator.currentTime << std::endl;

			// Sample full transform (position, rotation, scale)
			SampleClipAtTime(*clip, animator.currentTime, transform);
			//glm::quat rot = SampleRotation(*clip, animator.currentTime);

			//glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(transform.Rotation));
			//std::cout << "Sphere euler: "
			//	<< eulerDeg.x << ", "
			//	<< eulerDeg.y << ", "
			//	<< eulerDeg.z << std::endl;
		}
	}

	int AnimationSystem::GetPriority() const { return 20; }

	const char* AnimationSystem::GetName() const { return "AnimationSystem"; }

	glm::vec3 AnimationSystem::SamplePosition(const AnimationClip& clip, float t,
											  const glm::vec3& defaultPosition) {
		
		const auto& keys = clip.positionKeys;
		if (keys.empty())
			return defaultPosition;

		// Clamp t into [0, duration]
		t = glm::clamp(t, 0.0f, clip.duration);

		auto [i0, i1] = FindSegment(keys, t);
		if (i0 < 0) return defaultPosition;

		const auto& K0 = keys[i0];
		const auto& K1 = keys[i1];

		// Step mode or same key -> hold value
		if (i0 == i1 || clip.positionInterp == InterpMode::Step)
			return K0.position;

		const float dt = K1.time - K0.time;
		const float alpha = (dt > 0.0f) ? (t - K0.time) / dt : 0.0f;

		// Linear interpolation for now
		return LerpVec3(K0.position, K1.position, alpha);
	}

	glm::quat AnimationSystem::SampleRotation(const AnimationClip& clip, float t,
										      const glm::quat& defaultRotation) {
		
		const auto& keys = clip.rotationKeys;
		if (keys.empty())
			return defaultRotation;

		// Clamp t into [0, duration]
		t = glm::clamp(t, 0.0f, clip.duration);

		auto [i0, i1] = FindSegment(keys, t);
		if (i0 < 0) return defaultRotation;

		const auto& K0 = keys[i0];
		const auto& K1 = keys[i1];

		// Step mode or same key -> hold value
		if (i0 == i1 || clip.rotationInterp == InterpMode::Step)
			return K0.rotation;

		const float dt = K1.time - K0.time;
		const float alpha = (dt > 0.0f) ? (t - K0.time) / dt : 0.0f;

		// Slerp for smooth quaternion interpolation
		return SlerpQuat(K0.rotation, K1.rotation, alpha);
	}

	glm::vec3 AnimationSystem::SampleScale(const AnimationClip& clip, float t,
										   const glm::vec3& defaultScale) {
		
		const auto& keys = clip.scaleKeys;
		if (keys.empty())
			return defaultScale;

		// Clamp t into [0, duration]
		t = glm::clamp(t, 0.0f, clip.duration);

		auto [i0, i1] = FindSegment(keys, t);
		if (i0 < 0) return defaultScale;

		const auto& K0 = keys[i0];
		const auto& K1 = keys[i1];

		// Step mode or same key -> hold value
		if (i0 == i1 || clip.scaleInterp == InterpMode::Step)
			return K0.scale;

		const float dt = K1.time - K0.time;
		const float alpha = (dt > 0.0f) ? (t - K0.time) / dt : 0.0f;

		return LerpVec3(K0.scale, K1.scale, alpha);
	}

	void AnimationSystem::SampleClipAtTime(const AnimationClip& clip, float t,
										   TransformComponent& transform) {
		
		// Use current transform as fallback if a track has no keys
		glm::vec3 basePos = transform.Position;
		glm::vec3 baseScale = transform.Scale;

		// If your TransformComponent stores Rotation as glm::quat, keep this.
		// If it stores Euler, you’d convert here instead.
		glm::quat baseRot = transform.Rotation;

		transform.Position = SamplePosition(clip, t, basePos);
		transform.Rotation = SampleRotation(clip, t, baseRot);
		transform.Scale = SampleScale(clip, t, baseScale);

		// Mark transform as dirty if your system uses that flag
		transform.IsDirty = true;
	}

	AnimatorController* AnimationSystem::GetAnimatorController(u32 handle) {
		return &m_AnimatorControllerStorage[handle];
	}

	AnimationClip* AnimationSystem::GetAnimationClip(u32 handle) { 
		return &m_AnimationClipStorage[handle];
	}
}

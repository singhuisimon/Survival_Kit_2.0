/**
 * @file CameraSystem.cpp
 * @brief	Definition of Camera System that manages Camera components in the ECS
 * @details Updates and rebuild component's View and Perpsective transform upon modification
 * @author Chua Wen Bin Kenny
 * @date 20 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "../Graphics/CameraSystem.h"
#include "../ECS/Scene.h"
#include <GLFW/glfw3.h>

namespace Engine {

	CameraSystem::CameraSystem() : System() {}

	void CameraSystem::OnUpdate(Scene* scene, Timestep ts) {

		(void)ts;

		auto& registry = scene->GetRegistry();
		auto view = registry.view<CameraComponent, TransformComponent>();

		for (auto entity : view) {

			auto& camera = view.get<CameraComponent>(entity);
			auto& trans = view.get<TransformComponent>(entity);

			// Camera must be enabled to proceed
			if (!camera.Enabled) { continue; }

			// Update aspect ratio if autoAspect is on 
			if (camera.autoAspect) {
				int vp_w = 0, vp_h = 0;
				glfwGetWindowSize(glfwGetCurrentContext(), &vp_w, &vp_h);

				if (vp_w > 0 && vp_h > 0) {
					float newAspect = static_cast<float>(vp_w) / static_cast<float>(vp_h);

					// Only touch projection if aspect actually changed
					if (newAspect != camera.Aspect) {
						camera.SetAspect(newAspect);   // also RebuildProjection()
					}
				}
			}


			camera.RebuildProjection();
			// Always build view transform
			camera.View = glm::lookAt(trans.Position, camera.Target, { 0.0f, 1.0f, 0.0f }); // Default up is 0.0f, 1.0f, 0.0f
		}
	}

	int CameraSystem::GetPriority() const { return 100; }

	const char* CameraSystem::GetName() const { return "CameraSystem"; }
}
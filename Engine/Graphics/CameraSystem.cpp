#include "../Graphics/CameraSystem.h"
#include "../ECS/Scene.h"
#include <GLFW/glfw3.h>

namespace Engine {

	CameraSystem::CameraSystem() : System() {}

	void CameraSystem::OnUpdate(Scene* scene, Timestep ts) {

		(void)ts;

		auto camView = scene->GetRegistry().view<CameraComponent, TransformComponent>();
		for (auto cam : camView) {

			auto& camera = camView.get<CameraComponent>(cam);
			if (camera.Enabled && camera.isDirty) {

				// Get transform component
				auto& trans = camView.get<TransformComponent>(cam);

				// Update aspect ratio if autoAspect is on (This should be updated somewhere in script/elsewhere using the setter; will be here for now)
				if (camera.autoAspect) {
					int vp_w, vp_h;
					glfwGetWindowSize(glfwGetCurrentContext(), &vp_w, &vp_h);
					camera.Aspect = static_cast<float>(vp_w) / static_cast<float>(vp_h);
				}

				// Build transforms
				camera.View = glm::lookAt(trans.Position, camera.Target, { 0.0f, 1.0f, 0.0f }); // Default up is 0.0f, 1.0f, 0.0f
				camera.Persp = (glm::perspective(
					glm::radians(camera.FOV),
					camera.Aspect,
					camera.NearPlane,
					camera.FarPlane));

				camera.isDirty = false;
			}
		}
	}

	int CameraSystem::GetPriority() const { return 100; }

	const char* CameraSystem::GetName() const { return "CameraSystem"; }
}
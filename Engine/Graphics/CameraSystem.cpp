#include "../Graphics/CameraSystem.h"
#include "../ECS/Scene.h"

namespace Engine {

	CameraSystem::CameraSystem() : System() { }

	void CameraSystem::OnUpdate(Scene* scene, Timestep ts) {

		(void)ts;

		auto camView = scene->GetRegistry().view<CameraComponent, TransformComponent>();
		for (auto cam : camView) {

			auto& camera = camView.get<CameraComponent>(cam);
			if (camera.Enabled && camera.isDirty) {

				// Get transform component
				auto& trans = camView.get<TransformComponent>(cam);

				// Update aspect ratio if autoAspect is on (Need to get window width + height somewhere)
				//if (camera.autoAspect) {
				//	camera.Aspect = /*width / height*/;
				//}
				
				// Build transforms
				camera.View = glm::lookAt(trans.Position, camera.Target, {0.0f, 1.0f, 0.0f}); // Default up is 0.0f, 1.0f, 0.0f
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
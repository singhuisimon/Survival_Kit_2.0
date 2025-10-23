#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"

namespace Engine {

	class CameraSystem : public System {
	public:
		CameraSystem();

		void OnUpdate(Scene* scene, Timestep ts) override;
		int  GetPriority() const override;
		const char* GetName() const override;

	private:
	};

}
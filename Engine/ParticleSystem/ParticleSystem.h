#pragma once

#include  "../ECS/Components.h"
#include  "../ECS/System.h"

namespace Engine{

	class ParticleSystem : public System {
	public:
		void OnUpdate(Scene* scene, Timestep ts) override;
		int GetPriority() const override;
		const char* GetName() const override;
	};

}
#pragma once

#include  "../ECS/Components.h"
#include  "../ECS/System.h"

namespace Engine{

	class ParticleSystem : public System {
	public:
		void OnUpdate(Scene* scene, Timestep ts) override;
		int GetPriority() const override;
		const char* GetName() const override;

	private:
		std::mt19937 m_RandomEngine;
		std::uniform_real_distribution<float> m_Distribution{ 0.0f, 1.0f };

		float Random();
		float Random(float min, float max);
		glm::vec3 RandomVec3(float min, float max);
		glm::vec3 RandomInCone(glm::vec3 const& direction, float angle);

		glm::quat RandomRotation();
		glm::quat RandomRotationAxis(const glm::vec3& axis, float minAngle, float maxAngle);
	};

}
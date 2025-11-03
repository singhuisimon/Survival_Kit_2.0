#include "../ParticleSystem/ParticleSystem.h"
#include "../ECS/Scene.h"
#include "../Component/ParticleComponent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Engine {

	void ParticleSystem::OnUpdate(Scene* scene, Timestep ts) {

		auto view = scene->GetRegistry().view<ParticleComponent, TransformComponent>();

		for (auto entity : view) {

			// Get the particle component -> the particle component is actually the particle emitter
			auto& emitter = view.get<ParticleComponent>(entity);

			// If this emitter is not currently set to active skip it
			if (!emitter.Active)
				continue;

			// Get the emitter position, NOT the particle position
			auto& transform = view.get<TransformComponent>(entity);

			// Emit new particles based on the emission rate
			if (emitter.Loop || emitter.Particles.size() < emitter.MaxParticles) {

				emitter.EmissionAccumulator += emitter.EmissionRate * ts.GetSeconds();

				while (emitter.EmissionAccumulator >= 1.0f && 
					emitter.Particles.size() < emitter.MaxParticles) {

					ParticleData* particleToUse = nullptr;

					for (auto& particle : emitter.Particles) {
						if (!particle.Alive) {
							particleToUse = &particle;
							break; // Only recycle ONE per emission tick
						}
					}

					// If no dead particle found, create a new one
					if (particleToUse == nullptr) {
						if (emitter.Particles.size() < emitter.MaxParticles) {
							emitter.Particles.emplace_back();
							particleToUse = &emitter.Particles.back();
						}else {
							break;
						}
					}

					// Initialize the particles
					particleToUse->Position = transform.Position;
					particleToUse->PreviousPosition = transform.Position;

					// Random velocity in a cone around InitialVelocity
					float speed = Random(std::max(0.1f, emitter.MinSpeed), emitter.MaxSpeed); // Random speed multiplier
					glm::vec3 randomDir = RandomInCone(glm::normalize(emitter.InitialVelocity), emitter.SpreadAngle);
					particleToUse->Velocity = randomDir * glm::length(emitter.InitialVelocity) * speed;

					// Random lifetime variation
					particleToUse->Lifetime = emitter.ParticleLifetime * Random(0.8f, 1.2f);

					// Random color variation (blue-ish particles)
					particleToUse->Color = glm::vec4(
						Random(emitter.ColorMin.r, emitter.ColorMax.r),
						Random(emitter.ColorMin.g, emitter.ColorMax.g),
						Random(emitter.ColorMin.b, emitter.ColorMax.b),
						Random(emitter.ColorMin.a, emitter.ColorMax.a)
					);

					// In particle initialization:
					if (emitter.RandomizeRotation) {
						switch (emitter.ParticleType) {
						case 0: // Cube - full random rotation
							particleToUse->Rotation = RandomRotation();
							break;

						case 1: // Plane - rotate around forward axis only (billboard-like)
							particleToUse->Rotation = RandomRotationAxis(glm::vec3(0, 0, 1), 0.0f, 360.0f);
							break;

						case 2: // Sphere - doesn't matter, but can still randomize for variety
							particleToUse->Rotation = RandomRotation();
							break;

						default:
							particleToUse->Rotation = RandomRotation();
							break;
						}
					}
					else {
						particleToUse->Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity (no rotation)
					}

					particleToUse->Alive = true;
					emitter.EmissionAccumulator -= 1.0f;
				}

				// Update existing particles
				for (auto& particle : emitter.Particles) {

					if (!particle.Alive) 
						continue;

					particle.PreviousPosition = particle.Position;
					particle.Position += particle.Velocity * ts.GetSeconds();
					particle.Lifetime -= ts.GetSeconds();

					// Calculate particle transform matrix
					glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(emitter.ParticleSize)); // Uniform scale for particles
					glm::mat4 rot = glm::toMat4(particle.Rotation); // No rotation for
					glm::mat4 trans = glm::translate(glm::mat4(1.0f), particle.Position);

					particle.Transform = trans * rot * scale;

					if (particle.Lifetime <= 0.0f) {
						particle.Alive = false;
					}
				}
			}
		}
	}

	int ParticleSystem::GetPriority() const { return 101; }

	const char* ParticleSystem::GetName() const { return "ParticleSystem"; }

	// Helper functions for common random operations
	float ParticleSystem::Random() { return m_Distribution(m_RandomEngine); }

	float ParticleSystem::Random(float min, float max) { return min + (max - min) * Random(); }

	glm::vec3 ParticleSystem::RandomVec3(float min, float max) {
		return glm::vec3(Random(min, max), Random(min, max), Random(min, max));
	}

	glm::vec3 ParticleSystem::RandomInCone(const glm::vec3& direction, float angle) {
		// Random direction within a cone
		float cosAngle = glm::cos(glm::radians(angle));
		float z = Random(cosAngle, 1.0f);
		float phi = Random(0.0f, 2.0f * glm::pi<float>());

		float x = glm::sqrt(1.0f - z * z) * glm::cos(phi);
		float y = glm::sqrt(1.0f - z * z) * glm::sin(phi);

		glm::vec3 randomDir = glm::vec3(x, y, z);

		// Align with the given direction
		glm::vec3 up = glm::abs(direction.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
		glm::vec3 right = glm::normalize(glm::cross(up, direction));
		up = glm::cross(direction, right);

		return randomDir.x * right + randomDir.y * up + randomDir.z * direction;
	}

	// Random quaternion rotation
	glm::quat ParticleSystem::RandomRotation() {
		// Random rotation around all axes
		float pitch = Random(0.0f, glm::two_pi<float>());
		float yaw = Random(0.0f, glm::two_pi<float>());
		float roll = Random(0.0f, glm::two_pi<float>());
		return glm::quat(glm::vec3(pitch, yaw, roll));
	}

	// Random rotation around a specific axis
	glm::quat ParticleSystem::RandomRotationAxis(const glm::vec3& axis, float minAngle, float maxAngle) {
		float angle = Random(minAngle, maxAngle);
		return glm::angleAxis(glm::radians(angle), glm::normalize(axis));
	}

}

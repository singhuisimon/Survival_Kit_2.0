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

			emitter.DelayAccumualator += ts.GetSeconds();

			// If this emitter is not currently set to active skip it
			if (!emitter.Active || emitter.DelayAccumualator < emitter.PlayDelay)
				continue;

			// Get the emitter position, NOT the particle position
			auto& transform = view.get<TransformComponent>(entity);
			
			if (emitter.BurstMode) {

				// BURST MODE: Emit ALL particles in one frame
				size_t particlesToEmit = emitter.MaxParticles - emitter.Particles.size();

				for (size_t i = 0; i < particlesToEmit; ++i) {
					emitter.Particles.emplace_back();
					ParticleData& particle = emitter.Particles.back();

					// Recalculate random offset for EACH particle
					glm::vec3 particleOffset = glm::vec3(0.f);
					if (emitter.Shape == EmitterShape::BOX) {
						particleOffset = glm::vec3(
							Random(-emitter.EmissionBoxSize.x * 0.5f, emitter.EmissionBoxSize.x * 0.5f),
							Random(-emitter.EmissionBoxSize.y * 0.5f, emitter.EmissionBoxSize.y * 0.5f),
							Random(-emitter.EmissionBoxSize.z * 0.5f, emitter.EmissionBoxSize.z * 0.5f)
						);
					}
					else if (emitter.Shape == EmitterShape::SPHERE) {
						glm::vec3 RandomDirection = glm::normalize(glm::vec3(Random(-1.f, 1.f), Random(-1.f, 1.f), Random(-1.f, 1.f)));
						float RandomRadius = Random(0.f, emitter.EmissionSphereRadius);
						particleOffset = RandomDirection * RandomRadius;
					}

					// Initialize particle (same logic as before)
					particle.Position = emitter.WorldSpace ? transform.Position + particleOffset : particleOffset;
					particle.PreviousPosition = transform.Position;
					particle.Size = emitter.StartSize;

					float speed = Random(std::max(0.1f, emitter.MinSpeed), emitter.MaxSpeed);
					glm::vec3 randomDir = RandomInCone(glm::normalize(emitter.InitialVelocity), emitter.SpreadAngle);
					particle.Velocity = randomDir * glm::length(emitter.InitialVelocity) * speed;

					particle.Lifetime = emitter.ParticleLifetime * Random(0.8f, 1.2f);
					particle.Color = emitter.ColorMin;

					if (emitter.RandomizeRotation) {
						switch (emitter.ParticleType) {
						case 0: particle.Rotation = RandomRotation(); break;
						case 1: particle.Rotation = RandomRotationAxis(glm::vec3(0, 0, 1), 0.0f, 360.0f); break;
						case 2: particle.Rotation = RandomRotation(); break;
						default: particle.Rotation = RandomRotation(); break;
						}
					}
					else {
						particle.Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
					}

					particle.Alive = true;
				}

			}
			// Emit new particles based on the emission rate
			else if (emitter.Loop || emitter.Particles.size() < emitter.MaxParticles) {

				emitter.EmissionAccumulator += emitter.EmissionRate * ts.GetSeconds();

				glm::vec3 RandomOffset = glm::vec3(0.f);
				if (emitter.Shape == EmitterShape::POINT) {
					RandomOffset = glm::vec3(0.f);
				}
				else if (emitter.Shape == EmitterShape::BOX) {
					RandomOffset = glm::vec3(
						Random(-emitter.EmissionBoxSize.x * 0.5f, emitter.EmissionBoxSize.x * 0.5f),
						Random(-emitter.EmissionBoxSize.y * 0.5f, emitter.EmissionBoxSize.y * 0.5f),
						Random(-emitter.EmissionBoxSize.z * 0.5f, emitter.EmissionBoxSize.z * 0.5f)
					);
				}
				else if (emitter.Shape == EmitterShape::SPHERE) {
					glm::vec3 RandomDirection = glm::normalize(glm::vec3(Random(-1.f, 1.f), Random(-1.f, 1.f), Random(-1.f, 1.f)));
					glm::vec3 RandomRadius = glm::vec3(Random(0.f, emitter.EmissionSphereRadius * 1.f));
					RandomOffset = RandomDirection * RandomRadius;
				}

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
						// Create new particles if we have not hit the cap yet
						if (emitter.Particles.size() < emitter.MaxParticles) {
							emitter.Particles.emplace_back();
							particleToUse = &emitter.Particles.back();
						}else {
							// End the entire loop
							break;
						}
					}

					// Initialize the particles
					particleToUse->Position = emitter.WorldSpace ? transform.Position + RandomOffset : RandomOffset;
					particleToUse->PreviousPosition = transform.Position;
					particleToUse->Size = emitter.StartSize;

					// Random velocity in a cone around InitialVelocity
					float speed = Random(std::max(0.1f, emitter.MinSpeed), emitter.MaxSpeed); // Random speed multiplier
					glm::vec3 randomDir = RandomInCone(glm::normalize(emitter.InitialVelocity), emitter.SpreadAngle);
					particleToUse->Velocity = randomDir * glm::length(emitter.InitialVelocity) * speed;

					// Random lifetime variation
					particleToUse->Lifetime = emitter.ParticleLifetime * Random(0.8f, 1.2f);
					particleToUse->Color = emitter.ColorMin;

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
			}

			// Update existing particles
			for (auto& particle : emitter.Particles) {

				if (!particle.Alive)
					continue;

				particle.PreviousPosition = particle.Position;
				particle.Position += particle.Velocity * ts.GetSeconds();
				particle.Age += ts.GetSeconds();

				float normalizedLife = particle.Age / particle.Lifetime;

				if (normalizedLife < emitter.GrowPhaseEnd) {
					// Growing phase: start -> default
					float t = normalizedLife / emitter.GrowPhaseEnd; // 0 to 1 within grow phase
					particle.Size = glm::mix(emitter.StartSize, emitter.DefaultSize, t);
					particle.Color = glm::mix(emitter.ColorMin, emitter.ColorMax, t);
				}
				else if (normalizedLife < emitter.ShrinkPhaseStart) {
					// Stable phase: stay at default
					particle.Size  = emitter.DefaultSize;
					particle.TransitionColor = particle.Color;
				}
				else {
					// Shrinking phase: default -> end
					float t = (normalizedLife - emitter.ShrinkPhaseStart) / (1.0f - emitter.ShrinkPhaseStart);
					particle.Size = glm::mix(emitter.DefaultSize, emitter.EndSize, t);
					particle.Color = glm::mix(particle.TransitionColor, emitter.ColorMax, t);
				}


				// Calculate particle transform matrix
				glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(particle.Size));
				glm::mat4 rot = glm::toMat4(particle.Rotation); // No rotation for
				glm::mat4 trans = glm::translate(glm::mat4(1.0f), particle.Position);

				if (emitter.WorldSpace) {
					particle.Transform = trans * rot * scale;
				}
				else {
					particle.Transform = transform.WorldTransform * trans * rot * scale;
				}

				if ((particle.Lifetime - particle.Age) <= 0.00001f) {

					// Reset particle lifetime
					particle.Lifetime = 0.f;
					particle.Age = 0.f;
					particle.Alive = false;
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

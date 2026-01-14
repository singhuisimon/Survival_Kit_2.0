#pragma once
#include  <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../Utility/Types.h"
#include "../Serialization/ComponentRegistry.h"

namespace Engine {

	struct ParticleData {

		glm::mat4 Transform;
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec3 PreviousPosition;
		glm::vec4 Color;
		float	  Lifetime;
		glm::quat Rotation;
		bool	  Alive;
	};

	struct ParticleComponent {
		static constexpr ComponentTypeID TypeID = ComponentTypeID::ParticleSystem;
		static constexpr const char* TypeName = "ParticleComponent";

		xresource::instance_guid ComponentGUID;
		std::vector<ParticleData> Particles;

		glm::vec3 InitialVelocity = glm::vec3(0.f, 1.f, 0.f);
		glm::vec4 ColorMin = glm::vec4(0.f, 0.f, 0.7f, 1.f);
		glm::vec4 ColorMax = glm::vec4(0.3f, 0.3f, 1.f, 1.f);

		u32       MaxParticles        = 1000;
		u32       ParticleType        = 0; // Cube, Plane or Sphere

		float     EmissionRate        = 5.0f; // Particles per second
		float     ParticleLifetime    = 2.0f; // Lifetime of each particle in seconds
		float     EmissionAccumulator = 0.0f; // Accumulator for emission timing
		float     ParticleSize		  = 0.2f; // Size of each particle

		// Randomization parameters
		float     VelocityRandomness = 0.5f;  // 0-1, how much velocity varies
		float     LifetimeRandomness = 0.2f;  // 0-1, how much lifetime varies
		float     SpreadAngle = 15.0f; // Cone angle in degrees
		float     MinSpeed	  = 0.5f;     // Minimum speed multiplie
		float     MaxSpeed    = 1.5f; // Speed multiplier
		float     RotationSpeed = 0.0f;   // Angular velocity (degrees/sec), 0 = no spin

		bool      RandomizeRotation = true;   // Enable/disable random rotation
		bool      Loop = false;
		bool      Active = true;
	};

}
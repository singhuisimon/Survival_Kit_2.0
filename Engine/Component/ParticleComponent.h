#pragma once
#include  <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../Utility/Types.h"
#include "../Serialization/ComponentRegistry.h"

namespace Engine {

	enum class EmitterShape : u32 {
		POINT = 0,
		BOX,
		SPHERE
	};

	struct ParticleData {

		glm::mat4 Transform;
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec3 PreviousPosition;
		glm::vec3 Size;
		glm::vec4 Color;
		glm::vec4 TransitionColor;
		float	  Lifetime;
		float     Age;
		glm::quat Rotation;
		bool	  Alive;
	};

	struct ParticleComponent {
		static constexpr ComponentTypeID TypeID = ComponentTypeID::ParticleSystem;
		static constexpr const char* TypeName = "ParticleComponent";

		xresource::instance_guid	  ComponentGUID;

		// Advanced features
		xresource::instance_guid	  ParticleTypeAdvanced = 0;
		xresource::instance_guid	  MaterialType = 0;

		std::vector<ParticleData> Particles;

		glm::vec3 InitialVelocity = glm::vec3(0.f, 1.f, 0.f);

		glm::vec3 StartSize = glm::vec3(0.f, 0.f, 0.f);
		glm::vec3 DefaultSize = glm::vec3(0.5f, 0.5f, 0.5f);
		glm::vec3 EndSize = glm::vec3(0.f, 0.f, 0.f);

		glm::vec3 EmissionBoxSize = glm::vec3(1.f, 1.f, 1.f);

		glm::vec4 ColorMin = glm::vec4(0.f, 0.f, 0.7f, 1.f);
		glm::vec4 ColorMax = glm::vec4(0.3f, 0.3f, 1.f, 1.f);

		EmitterShape Shape = EmitterShape::POINT;

		u32       MaxParticles        = 1000;
		u32       ParticleType        = 0; // Cube, Plane or Sphere

		float     EmissionRate        = 5.0f; // Particles per second
		float     ParticleLifetime    = 2.0f; // Lifetime of each particle in seconds
		float     EmissionAccumulator = 0.0f; // Accumulator for emission timing
		float     ParticleSize		  = 0.2f; // Size of each particle
		float	  GrowPhaseEnd		  = 0.3f;
		float	  ShrinkPhaseStart	  = 0.7f;

		float	  EmissionSphereRadius = 1.f;

		// Randomization parameters
		float     VelocityRandomness = 0.5f;  // 0-1, how much velocity varies
		float     LifetimeRandomness = 0.2f;  // 0-1, how much lifetime varies
		float     SpreadAngle = 15.0f; // Cone angle in degrees
		float     MinSpeed	  = 0.5f;     // Minimum speed multiplie
		float     MaxSpeed    = 1.5f; // Speed multiplier
		float     RotationSpeed = 0.0f;   // Angular velocity (degrees/sec), 0 = no spin

		float     PlayDelay = 0.0f;
		float     DelayAccumualator = 0.0f; // In Seconds

		bool      RandomizeRotation = true;   // Enable/disable random rotation
		bool      Loop = false; // Non loop is basically "Burst Mode" -> Will just set it in editor.
		bool      Active = true;

		bool      WorldSpace = true; // Boolean to check whether it's set to world space or local space
		bool      BurstMode  = false;
	};

}
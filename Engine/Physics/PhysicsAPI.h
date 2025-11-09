#pragma once
/*****************************************************************************/
/*!
\file       PhysicsAPI.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/11/09
\brief      Jolt Physics high-level façade:
			- Entity-addressed body queries and activation
			- Force/impulse/torque and velocity nudges
			- Per-body properties (damping, gravity factor, CCD)
			- Lightweight per-frame collision events buffer and hooks

			Units: meters, kilograms, seconds.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include <vector>

#include <glm/glm.hpp>

#include "ECS/Entity.h"
#include "Physics/PhysicsBridge.h"
#include <Jolt/Physics/Body/BodyID.h>

namespace Engine
{
	/**************************************************************************
	 * @brief
	 * Minimal per-contact record: the two entities that touched this frame.
	 **************************************************************************/
	struct ContactEvent
	{
		EntityID entA{ entt::null };
		EntityID entB{ entt::null };
	};

	/**************************************************************************
	 * @brief
	 * Static physics helper API for entity-addressed body ops and collisions.
	 *
	 * All methods that take an Entity perform a best-effort BodyID lookup.
	 * If the entity has no registered body, the call becomes a no-op.
	 **************************************************************************/
	class PhysicsAPI
	{
	public:
		// Singleton easy access
		static PhysicsAPI &GetInstance()
		{
			static PhysicsAPI s_instance;   // Meyers singleton (thread-safe)
			return s_instance;
		}
		// =========================
		//  Body query / activation
		// =========================

		/**************************************************************************
		 * @brief
		 * Checks whether an entity has a physics body registered.
		 * @param e
		 * The entity handle.
		 * @return
		 * True if a body exists for the given entity, else false.
		 **************************************************************************/
		static bool HasBody(const Entity &e);

		/**************************************************************************
		 * @brief
		 * Wakes the body so it participates in simulation next step.
		 * @param e
		 * The entity handle.
		 **************************************************************************/
		static void Activate(const Entity &e);

		// =========================
		//  Forces / impulses
		// =========================

		/**************************************************************************
		 * @brief
		 * Applies a world-space force at the body’s center of mass.
		 * @param e
		 * The entity handle.
		 * @param force
		 * Force vector (Newtons), integrated over the next step.
		 **************************************************************************/
		static void AddForce(const Entity &e, const glm::vec3 &force);

		/**************************************************************************
		 * @brief
		 * Applies a world-space linear impulse at the center of mass.
		 * @param e
		 * The entity handle.
		 * @param impulse
		 * Impulse vector (N·s).
		 **************************************************************************/
		static void AddImpulse(const Entity &e, const glm::vec3 &impulse);

		/**************************************************************************
		 * @brief
		 * Applies a world-space torque.
		 * @param e
		 * The entity handle.
		 * @param torque
		 * Torque vector (N·m).
		 **************************************************************************/
		static void AddTorque(const Entity &e, const glm::vec3 &torque);

		/**************************************************************************
		 * @brief
		 * Applies an angular impulse.
		 * @param e
		 * The entity handle.
		 * @param angularImpulse
		 * Angular impulse (N·m·s).
		 **************************************************************************/
		static void AddAngularImpulse(const Entity &e, const glm::vec3 &angularImpulse);

		// =========================
		//  Velocity nudges
		// =========================

		/**************************************************************************
		 * @brief
		 * Adds a delta to the body’s linear velocity.
		 * @param e
		 * The entity handle.
		 * @param deltaVelocity
		 * Linear velocity change (m/s).
		 **************************************************************************/
		static void AddLinearVelocity(const Entity &e, const glm::vec3 &deltaVelocity);

		/**************************************************************************
		 * @brief
		 * Adds a delta to the body’s angular velocity.
		 * @param e
		 * The entity handle.
		 * @param deltaOmega
		 * Angular velocity change (rad/s).
		 **************************************************************************/
		static void AddAngularVelocity(const Entity &e, const glm::vec3 &deltaOmega);

		// =========================
		//  Body properties
		// =========================

		/**************************************************************************
		 * @brief
		 * Sets linear damping (drag-like slowdown) on the body.
		 * @param e
		 * The entity handle.
		 * @param damping
		 * Linear damping coefficient (unitless, >= 0).
		 **************************************************************************/
		static void SetLinearDamping(const Entity &e, float damping);

		/**************************************************************************
		 * @brief
		 * Sets angular damping on the body.
		 * @param e
		 * The entity handle.
		 * @param damping
		 * Angular damping coefficient (unitless, >= 0).
		 **************************************************************************/
		static void SetAngularDamping(const Entity &e, float damping);

		/**************************************************************************
		 * @brief
		 * Scales world gravity for this body only.
		 * @param e
		 * The entity handle.
		 * @param factor
		 * Multiplier for global gravity (1 = default).
		 **************************************************************************/
		static void SetGravityFactor(const Entity &e, float factor);

		/**************************************************************************
		 * @brief
		 * Enables/disables continuous collision detection (CCD).
		 * @param e
		 * The entity handle.
		 * @param enabled
		 * True to enable CCD, false to disable.
		 **************************************************************************/
		static void SetContinuousDetection(const Entity &e, bool enabled);

		// =========================
		//  Collision events
		// =========================

		/**************************************************************************
		 * @brief
		 * Installs the engine’s contact listener and routes events to a buffer.
		 *
		 * Idempotent: safe to call more than once; only one listener is active.
		 **************************************************************************/
		static void EnableCollisionEvents();

		/**************************************************************************
		 * @brief
		 * Starts a new collision frame by clearing the internal event buffer.
		 **************************************************************************/
		static void BeginCollisionFrame();

		/**************************************************************************
		 * @brief
		 * Returns the current frame’s contact events without draining them.
		 * @return
		 * Const reference to the per-frame contact event buffer.
		 **************************************************************************/
		const std::vector<ContactEvent> &GetCollisionEvents();

		private:
			PhysicsAPI() = default;
			~PhysicsAPI() = default;
			PhysicsAPI(const PhysicsAPI &) = delete;
			PhysicsAPI &operator=(const PhysicsAPI &) = delete;
			PhysicsAPI(PhysicsAPI &&) = delete;
			PhysicsAPI &operator=(PhysicsAPI &&) = delete;
	};
} // namespace Engine

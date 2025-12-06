/**************************************************************************
 * @file
 * PhysicsAPI.cpp
 * @author
 * Low Yue Jun, yuejun.low, yuejun.low@digipen.edu
 * @date
 * 2025/11/09 (YYYY/MM/DD)
 * @brief
 * Implementation of a Jolt-backed, entity-addressed physics façade:
 * - Body lookup and activation
 * - Force/impulse/torque and velocity adjustments
 * - Damping, gravity factor, and CCD configuration
 * - Per-frame collision events with pair de-duplication
 *
 * NOTE:
 * Persistent properties that are also stored on the ECS (e.g. damping)
 * are updated via RigidbodyComponent. Jolt is updated from ECS in
 * PhysicsSystem, so ECS remains the source of truth on refresh.
 *
 * Transient operations (forces, impulses, direct velocity nudges) still
 * talk to Jolt directly; PhysicsSystem pulls velocities back into ECS.
 *
 * @copyright
 * Copyright (C) 2025 DigiPen Institute of Technology.
 **************************************************************************/

#include "Physics/PhysicsAPI.h"

#include <cstdint>
#include <unordered_set>
#include <mutex>            // <-- for thread-safety of contact buffers

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/MotionProperties.h>

 // ECS components (for RigidbodyComponent)
#include "ECS/Components.h"

namespace Engine
{
	/**************************************************************************
	 * @brief
	 * Internal helper: resolves BodyID for an entity.
	 * @param e
	 * Entity handle.
	 * @param out
	 * Output BodyID if found.
	 * @return
	 * True if resolution succeeds, false otherwise.
	 **************************************************************************/
	static inline bool GetBodyID(const Entity &e, JPH::BodyID &out)
	{
		auto it = mBodyOf.find(static_cast<entt::entity>(e));
		if (it == mBodyOf.end()) return false;
		out = it->second;
		return true;
	}

	static inline EntityID BodyIDToEntityID(JPH::BodyID id)
	{
		// Use the same mPhysics you already use in SetGravityFactor()
		JPH::BodyLockRead lock{ mPhysics.GetBodyLockInterface(), id };
		if (!lock.Succeeded())
			return entt::null;

		const JPH::Body &body = lock.GetBody();
		JPH::uint64 raw = body.GetUserData();

		// If you keep 0 as "no entity", guard it
		if (raw == 0)
			return entt::null;

		// Stored as uint32_t(EntityID) in mUserData
		return static_cast<EntityID>(static_cast<std::uint32_t>(raw));
	}

	/**************************************************************************
	 * @brief
	 * Internal helper: fetches the RigidbodyComponent for an entity, if any.
	 * NOTE:
	 * PhysicsAPI uses const Entity& in its public API, but we need to mutate
	 * the ECS component here; const_cast is used locally for that purpose.
	 * @param e
	 * Entity handle (const in the public API).
	 * @return
	 * Pointer to RigidbodyComponent or nullptr if missing.
	 **************************************************************************/
	static inline RigidbodyComponent *GetRigidbody(const Entity &e)
	{
		// Strip const because we intentionally want to modify the component
		Entity &nc = const_cast<Entity &>(e);
		if (!nc.HasComponent<RigidbodyComponent>()) return nullptr;
		return &nc.GetComponent<RigidbodyComponent>();
	}

	/**************************************************************************
	 * @brief
	 * Returns true if the given entity has a registered physics body.
	 * @param e
	 * Entity handle.
	 * @return
	 * True if a body exists, false otherwise.
	 **************************************************************************/
	bool PhysicsAPI::HasBody(const Entity &e)
	{
		return mBodyOf.find(static_cast<entt::entity>(e)) != mBodyOf.end();
	}

	/**************************************************************************
	 * @brief
	 * Wakes the body so it participates in the next simulation step.
	 * @param e
	 * Entity handle.
	 **************************************************************************/
	void PhysicsAPI::Activate(const Entity &e)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Applies a world-space force at the center of mass.
	 * @param e
	 * Entity handle.
	 * @param force
	 * Force in Newtons.
	 **************************************************************************/
	void PhysicsAPI::AddForce(const Entity &e, const glm::vec3 &force)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		mBodyInterface->AddForce(id, ToJPHVec3(force));
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Applies a world-space linear impulse at the center of mass.
	 * @param e
	 * Entity handle.
	 * @param impulse
	 * Impulse in N·s.
	 **************************************************************************/
	void PhysicsAPI::AddImpulse(const Entity &e, const glm::vec3 &impulse)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		mBodyInterface->AddImpulse(id, ToJPHVec3(impulse));
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Applies a world-space torque.
	 * @param e
	 * Entity handle.
	 * @param torque
	 * Torque in N·m.
	 **************************************************************************/
	void PhysicsAPI::AddTorque(const Entity &e, const glm::vec3 &torque)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		mBodyInterface->AddTorque(id, ToJPHVec3(torque));
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Applies an angular impulse.
	 * @param e
	 * Entity handle.
	 * @param angularImpulse
	 * Angular impulse in N·m·s.
	 **************************************************************************/
	void PhysicsAPI::AddAngularImpulse(const Entity &e, const glm::vec3 &angularImpulse)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		mBodyInterface->AddAngularImpulse(id, ToJPHVec3(angularImpulse));
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Adds a delta to the body’s linear velocity.
	 * @param e
	 * Entity handle.
	 * @param deltaVelocity
	 * Linear velocity change in m/s.
	 **************************************************************************/
	void PhysicsAPI::AddLinearVelocity(const Entity &e, const glm::vec3 &deltaVelocity)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		JPH::Vec3 cur{ mBodyInterface->GetLinearVelocity(id) };
		JPH::Vec3 nxt{ cur + ToJPHVec3(deltaVelocity) };
		mBodyInterface->SetLinearVelocity(id, nxt);
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Adds a delta to the body’s angular velocity.
	 * @param e
	 * Entity handle.
	 * @param deltaOmega
	 * Angular velocity change in rad/s.
	 **************************************************************************/
	void PhysicsAPI::AddAngularVelocity(const Entity &e, const glm::vec3 &deltaOmega)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		JPH::Vec3 cur{ mBodyInterface->GetAngularVelocity(id) };
		JPH::Vec3 nxt{ cur + ToJPHVec3(deltaOmega) };
		mBodyInterface->SetAngularVelocity(id, nxt);
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Sets linear damping via ECS (RigidbodyComponent).
	 * PhysicsSystem will propagate this value into Jolt each frame.
	 * @param e
	 * Entity handle.
	 * @param damping
	 * Coefficient (>= 0).
	 **************************************************************************/
	void PhysicsAPI::SetLinearDamping(const Entity &e, float damping)
	{
		if (RigidbodyComponent *rb = GetRigidbody(e))
		{
			if (damping < 0.0f) damping = 0.0f;
			rb->LinearDamping = damping;
		}
	}

	/**************************************************************************
	 * @brief
	 * Sets angular damping via ECS (RigidbodyComponent).
	 * PhysicsSystem will propagate this value into Jolt each frame.
	 * @param e
	 * Entity handle.
	 * @param damping
	 * Coefficient (>= 0).
	 **************************************************************************/
	void PhysicsAPI::SetAngularDamping(const Entity &e, float damping)
	{
		if (RigidbodyComponent *rb = GetRigidbody(e))
		{
			if (damping < 0.0f) damping = 0.0f;
			rb->AngularDamping = damping;
		}
	}

	/**************************************************************************
	 * @brief
	 * Scales world gravity for this body.
	 * NOTE: currently still applied directly to Jolt; ECS only stores
	 * a UseGravity flag, which PhysicsSystem maps to 0/1 gravity factor.
	 * @param e
	 * Entity handle.
	 * @param factor
	 * Gravity multiplier (1 = default).
	 **************************************************************************/
	void PhysicsAPI::SetGravityFactor(const Entity &e, float factor)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		JPH::BodyLockWrite lock{ mPhysics.GetBodyLockInterface(), id };
		if (!lock.Succeeded()) return;
		lock.GetBody().GetMotionProperties()->SetGravityFactor(factor);
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
	 * @brief
	 * Enables or disables continuous collision detection (CCD).
	 * Currently applied directly to Jolt. If you add a CCD flag to
	 * RigidbodyComponent, this can be rerouted through ECS as well.
	 * @param e
	 * Entity handle.
	 * @param enabled
	 * True to enable, false to disable.
	 **************************************************************************/
	void PhysicsAPI::SetContinuousDetection(const Entity &e, bool enabled)
	{
		JPH::BodyID id{};
		if (!GetBodyID(e, id)) return;
		mBodyInterface->SetMotionQuality(
			id,
			enabled ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete
		);
		mBodyInterface->ActivateBody(id);
	}

	/**************************************************************************
 * @brief
 * Per-frame collision event buffer (deduped unordered pairs).
 **************************************************************************/
	static std::vector<ContactEvent>         s_frame_contacts{};
	static std::unordered_set<std::uint64_t> s_dedupe{};
	static std::mutex                        s_contact_mutex{};

	/**************************************************************************
	 * @brief
	 * Stable, commutative 64-bit key for an entity pair.
	 * @param a
	 * First entity.
	 * @param b
	 * Second entity.
	 * @return
	 * 64-bit key with smaller ID in high bits.
	 **************************************************************************/
	static inline std::uint64_t PairKey(EntityID a, EntityID b)
	{
		auto aa{ static_cast<std::uint32_t>(a) };
		auto bb{ static_cast<std::uint32_t>(b) };
		if (bb < aa) std::swap(aa, bb);
		return (static_cast<std::uint64_t>(aa) << 32) | static_cast<std::uint64_t>(bb);
	}

	/**************************************************************************
	 * @brief
	 * Local contact listener that records added/persisted contacts.
	 *
	 * NOTE:
	 * We rely on Jolt Body::GetUserData() storing the ECS EntityID.
	 * Make sure body creation sets user data appropriately.
	 **************************************************************************/
	class LocalContactListener final : public JPH::ContactListener
	{
	public:
		/**************************************************************************
		 * @brief
		 * Accepts all contacts for this body pair.
		 **************************************************************************/
		JPH::ValidateResult OnContactValidate(
			const JPH::Body &,
			const JPH::Body &,
			JPH::RVec3Arg,
			const JPH::CollideShapeResult &
		) override
		{
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		/**************************************************************************
		 * @brief
		 * Called when a contact is added; records the pair.
		 **************************************************************************/
		void OnContactAdded(
			const JPH::Body &a,
			const JPH::Body &b,
			const JPH::ContactManifold &,
			JPH::ContactSettings &
		) override
		{
			Record(a, b);
		}

		/**************************************************************************
		 * @brief
		 * Called when a contact persists; records the pair.
		 **************************************************************************/
		void OnContactPersisted(
			const JPH::Body &a,
			const JPH::Body &b,
			const JPH::ContactManifold &,
			JPH::ContactSettings &
		) override
		{
			Record(a, b);
		}

	private:
		/**************************************************************************
		 * @brief
		 * Records a pair into the frame buffer with de-duplication.
		 * @param a
		 * Body A.
		 * @param b
		 * Body B.
		 **************************************************************************/
		static void Record(const JPH::Body &a, const JPH::Body &b)
		{
			// Pull ECS entity IDs from Jolt user data
			const EntityID ea = static_cast<EntityID>(a.GetUserData());
			const EntityID eb = static_cast<EntityID>(b.GetUserData());

			// Skip bodies that are not associated with ECS entities
			if (ea == entt::null || eb == entt::null)
				return;

			const std::uint64_t key{ PairKey(ea, eb) };

			// Protect shared contact buffers from concurrent access
			std::lock_guard<std::mutex> lock(s_contact_mutex);
			if (s_dedupe.insert(key).second)
			{
				s_frame_contacts.push_back(ContactEvent{ ea, eb });

				// inside Record(...)
				LOG_INFO("[Physics] Contact recorded as a: ", static_cast<unsigned int>(ea), " b: ", static_cast<unsigned int>(eb));
			}
		}
	};

	/**************************************************************************
	 * @brief
	 * Singleton contact listener instance.
	 **************************************************************************/
	static LocalContactListener g_contact_listener{};

	/**************************************************************************
	 * @brief
	 * Hooks a contact listener to the physics system (idempotent).
	 **************************************************************************/
	void PhysicsAPI::EnableCollisionEvents()
	{
		mPhysics.SetContactListener(&g_contact_listener);
	}

	/**************************************************************************
	 * @brief
	 * Clears the per-frame collision buffer and de-duplication set.
	 **************************************************************************/
	void PhysicsAPI::BeginCollisionFrame()
	{
		std::lock_guard<std::mutex> lock(s_contact_mutex);
		s_frame_contacts.clear();
		s_dedupe.clear();
	}

	/**************************************************************************
	 * @brief
	 * Returns the current frame’s contact events without draining.
	 * @return
	 * Const reference to the per-frame events buffer.
	 **************************************************************************/
	const std::vector<ContactEvent> &PhysicsAPI::GetCollisionEvents()
	{
		return s_frame_contacts;
	}
} // namespace Engine

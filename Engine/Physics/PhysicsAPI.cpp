#include "Physics/PhysicsAPI.h"

#include <Jolt/Physics/Body/Body.h>

namespace Engine
{
    static inline bool GetBodyID(const Entity &e, JPH::BodyID &out)
    {
        auto it = mBodyOf.find(static_cast<entt::entity>(e));
        if (it == mBodyOf.end()) return false;
        out = it->second;
        return true;
    }

    bool PhysicsAPI::HasBody(const Entity &e)
    {
        return mBodyOf.find(static_cast<entt::entity>(e)) != mBodyOf.end();
    }

    void PhysicsAPI::Activate(const Entity &e)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::AddForce(const Entity &e, const glm::vec3 &force)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        mBodyInterface->AddForce(id, ToJPHVec3(force));
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::AddImpulse(const Entity &e, const glm::vec3 &impulse)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        mBodyInterface->AddImpulse(id, ToJPHVec3(impulse));
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::AddTorque(const Entity &e, const glm::vec3 &torque)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        mBodyInterface->AddTorque(id, ToJPHVec3(torque));
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::AddAngularImpulse(const Entity &e, const glm::vec3 &angularImpulse)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        mBodyInterface->AddAngularImpulse(id, ToJPHVec3(angularImpulse));
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::AddLinearVelocity(const Entity &e, const glm::vec3 &deltaVelocity)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        JPH::Vec3 cur = mBodyInterface->GetLinearVelocity(id);
        JPH::Vec3 nxt = cur + ToJPHVec3(deltaVelocity);
        mBodyInterface->SetLinearVelocity(id, nxt);
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::AddAngularVelocity(const Entity &e, const glm::vec3 &deltaOmega)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        JPH::Vec3 cur = mBodyInterface->GetAngularVelocity(id);
        JPH::Vec3 nxt = cur + ToJPHVec3(deltaOmega);
        mBodyInterface->SetAngularVelocity(id, nxt);
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::SetLinearDamping(const Entity &e, float damping)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
        if (!lock.Succeeded()) return;
        lock.GetBody().GetMotionProperties()->SetLinearDamping(damping);
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::SetAngularDamping(const Entity &e, float damping)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
        if (!lock.Succeeded()) return;
        lock.GetBody().GetMotionProperties()->SetAngularDamping(damping);
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::SetGravityFactor(const Entity &e, float factor)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
        if (!lock.Succeeded()) return;
        lock.GetBody().GetMotionProperties()->SetGravityFactor(factor);
        mBodyInterface->ActivateBody(id);
    }

    void PhysicsAPI::SetContinuousDetection(const Entity &e, bool enabled)
    {
        JPH::BodyID id{};
        if (!GetBodyID(e, id)) return;
        mBodyInterface->SetMotionQuality(id, enabled ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete);
        mBodyInterface->ActivateBody(id);
    }

	static std::vector<ContactEvent> s_frame_contacts;
	static std::unordered_set<std::uint64_t> s_dedupe;

	static inline EntityID BodyIDToEntityID(JPH::BodyID id)
	{
		for (const auto &kv : mBodyOf)
			if (kv.second == id) return kv.first;
		return entt::null;
	}

	static inline std::uint64_t PairKey(EntityID a, EntityID b)
	{
		auto aa = static_cast<std::uint32_t>(a);
		auto bb = static_cast<std::uint32_t>(b);
		if (bb < aa) std::swap(aa, bb);
		return (static_cast<std::uint64_t>(aa) << 32) | static_cast<std::uint64_t>(bb);
	}

	class LocalContactListener final : public JPH::ContactListener
	{
	public:
		JPH::ValidateResult OnContactValidate(const JPH::Body &, const JPH::Body &,
			JPH::RVec3Arg,
			const JPH::CollideShapeResult &) override
		{
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		void OnContactAdded(const JPH::Body &a, const JPH::Body &b,
			const JPH::ContactManifold &,
			JPH::ContactSettings &) override
		{
			Record(a.GetID(), b.GetID());
		}

		void OnContactPersisted(const JPH::Body &a, const JPH::Body &b,
			const JPH::ContactManifold &,
			JPH::ContactSettings &) override
		{
			Record(a.GetID(), b.GetID());
		}

	private:
		static void Record(JPH::BodyID ba, JPH::BodyID bb)
		{
			const EntityID ea = BodyIDToEntityID(ba);
			const EntityID eb = BodyIDToEntityID(bb);
			if (ea == entt::null || eb == entt::null) return;

			const std::uint64_t key = PairKey(ea, eb);
			if (s_dedupe.insert(key).second)
				s_frame_contacts.push_back(ContactEvent{ ea, eb });
		}
	};

	static LocalContactListener g_contact_listener;

	void PhysicsAPI::EnableCollisionEvents()
	{
		mPhysics.SetContactListener(&g_contact_listener);
	}

	void PhysicsAPI::BeginCollisionFrame()
	{
		s_frame_contacts.clear();
		s_dedupe.clear();
	}

	const std::vector<ContactEvent> &PhysicsAPI::GetCollisionEvents()
	{
		return s_frame_contacts;
	}
}

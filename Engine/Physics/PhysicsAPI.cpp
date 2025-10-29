#include "Physics/PhysicsAPI.h"

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
}

#pragma once
#include <glm/glm.hpp>
#include "ECS/Entity.h"
#include "Physics/PhysicsBridge.h"

namespace Engine
{
    class PhysicsAPI
    {
    public:
        static bool HasBody(const Entity &e);
        static void Activate(const Entity &e);

        static void AddForce(const Entity &e, const glm::vec3 &force);
        static void AddImpulse(const Entity &e, const glm::vec3 &impulse);
        static void AddTorque(const Entity &e, const glm::vec3 &torque);
        static void AddAngularImpulse(const Entity &e, const glm::vec3 &angularImpulse);

        static void AddLinearVelocity(const Entity &e, const glm::vec3 &deltaVelocity);
        static void AddAngularVelocity(const Entity &e, const glm::vec3 &deltaOmega);

        static void SetLinearDamping(const Entity &e, float damping);
        static void SetAngularDamping(const Entity &e, float damping);
        static void SetGravityFactor(const Entity &e, float factor);
        static void SetContinuousDetection(const Entity &e, bool enabled);
    };
}

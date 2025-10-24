#pragma once
#include "MathTypes.hxx"

namespace ScriptAPI
{
    public value struct RigidbodyComponent
    {
    public:
        RigidbodyComponent(int id) : entityId(id) {}

        void   AddImpulse(float3 impulse);
        void   SetLinearVelocity(float3 v);
        float3 GetLinearVelocity();

    private:
        int entityId;
    };
}

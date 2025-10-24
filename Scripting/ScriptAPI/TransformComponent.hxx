#pragma once
#include "MathTypes.hxx"

namespace ScriptAPI
{
    public value struct TransformComponent
    {
    public:
        TransformComponent(int id) : entityId(id) {}

        property float3 Position{ float3 get(); void set(float3 v); }
        property float3 Rotation{ float3 get(); void set(float3 v); }
        property float3 Scale{ float3 get(); void set(float3 v); }

    private:
        int entityId;
    };
}

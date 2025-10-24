#pragma once
namespace ScriptAPI
{
    public value struct float3
    {
        float X; float Y; float Z;
        float3(float x, float y, float z) : X(x), Y(y), Z(z) {}
        static float3 FromXYZ(float x, float y, float z) { return float3(x, y, z); }
    };
}

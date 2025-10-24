#pragma once
#include <cstdint>
#include <functional>

namespace Core
{
    struct Float3 { float x, y, z; };

    // Transform function table (bind these once at startup)
    struct TransformAccess
    {
        std::function<Float3(int /*entityId*/)>              GetPosition;
        std::function<void(int /*entityId*/, Float3)>        SetPosition;

        std::function<Float3(int /*entityId*/)>              GetRotationEuler;
        std::function<void(int /*entityId*/, Float3)>        SetRotationEuler;

        std::function<Float3(int /*entityId*/)>              GetScale;
        std::function<void(int /*entityId*/, Float3)>        SetScale;
    };

    // Physics function table (bind these once at startup)
    struct PhysicsAccess
    {
        std::function<void(int /*entityId*/, Float3 /*impulse*/)> AddImpulse;
        std::function<void(int /*entityId*/, Float3 /*vel*/)>     SetLinearVelocity;
        std::function<Float3(int /*entityId*/)>                   GetLinearVelocity;
    };

    // Thin, header-only bridge surface for ScriptAPI facades.
    class ScriptingBridge
    {
    public:
        // Optional: set capacity used to pre-size managed storage (default 8192)
        static void SetEntityCapacity(int capacity);
        static int  GetMaxEntities();

        // Bind engine callbacks once your systems are ready
        static void BindTransformAccessors(const TransformAccess &access);
        static void BindPhysicsAccessors(const PhysicsAccess &access);

        // Transform (no-ops if not bound)
        static Float3 GetPosition(int entityId);
        static void   SetPosition(int entityId, const Float3 &v);
        static Float3 GetRotationEuler(int entityId);
        static void   SetRotationEuler(int entityId, const Float3 &v);
        static Float3 GetScale(int entityId);
        static void   SetScale(int entityId, const Float3 &v);

        // Physics (no-ops / zeros if not bound)
        static void   Physics_AddImpulse(int entityId, const Float3 &impulse);
        static void   Physics_SetLinearVelocity(int entityId, const Float3 &v);
        static Float3 Physics_GetLinearVelocity(int entityId);

    private:
        static int            sCapacity;
        static TransformAccess sXform;
        static PhysicsAccess   sPhys;
    };
}

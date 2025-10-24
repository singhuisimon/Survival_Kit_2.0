#include "ScriptingBridge.h"

namespace Core
{
    int ScriptingBridge::sCapacity = 8192;

    void ScriptingBridge::SetEntityCapacity(int capacity) { sCapacity = capacity > 0 ? capacity : 8192; }
    int  ScriptingBridge::GetMaxEntities() { return sCapacity; }

    void ScriptingBridge::BindTransformAccessors(const TransformAccess &access) { sXform = access; }
    void ScriptingBridge::BindPhysicsAccessors(const PhysicsAccess &access) { sPhys = access; }

    // -------- Transform pass-throughs --------

    void ScriptingBridge::SetPosition(int id, const Float3 &v)
    {
        if (sXform.SetPosition) sXform.SetPosition(id, v);
    }

    void ScriptingBridge::SetRotationEuler(int id, const Float3 &v)
    {
        if (sXform.SetRotationEuler) sXform.SetRotationEuler(id, v);
    }

    void ScriptingBridge::SetScale(int id, const Float3 &v)
    {
        if (sXform.SetScale) sXform.SetScale(id, v);
    }

    // -------- Physics pass-throughs --------
    void ScriptingBridge::Physics_AddImpulse(int id, const Float3 &i)
    {
        if (sPhys.AddImpulse) sPhys.AddImpulse(id, i);
    }

    void ScriptingBridge::Physics_SetLinearVelocity(int id, const Float3 &v)
    {
        if (sPhys.SetLinearVelocity) sPhys.SetLinearVelocity(id, v);
    }
}

#include "PhysicsBridge.h"

namespace Engine
{
    BPLayerInterfaceImpl::BPLayerInterfaceImpl()
        : mObjectToBroadPhase{ JPH::BroadPhaseLayer{ 0 }, JPH::BroadPhaseLayer{ 1 } }, mNumBroadPhaseLayers{ 2u }
    {
    }
    JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
    {
        return mNumBroadPhaseLayers;
    }
    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer layer) const
    {
        return mObjectToBroadPhase[layer];
    }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char *BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const
    {
        switch (layer.GetValue())
        {
        case 0: return "NON_MOVING"; case 1: return "MOVING"; default: return "UNKNOWN";
        }
    }
#endif

    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const
    {
        if (a == Layers::NON_MOVING && b == Layers::NON_MOVING) return false;
        return true;
    }

    bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broad) const
    {
        JPH::uint b = broad.GetValue();
        switch (layer)
        {
        case Layers::NON_MOVING: return b == 1u;
        case Layers::MOVING:     return (b == 0u) || (b == 1u);
        default:                 return false;
        }
    }

    JPH::TempAllocator *mTempAllocator = nullptr;
    JPH::JobSystemThreadPool *mJobSystem = nullptr;
    JPH::PhysicsSystem        mPhysics{};
    JPH::BodyInterface *mBodyInterface = nullptr;

    BPLayerInterfaceImpl              mBPLayers{};
    ObjectVsBroadPhaseLayerFilterImpl mObjVsBPLayerFilter{};
    ObjectLayerPairFilterImpl         mObjPairFilter{};

    std::unordered_map<EntityID, JPH::BodyID> mBodyOf{};
    std::unordered_map<CacheKey, JPH::Ref<JPH::Shape>, CacheKeyHash> mShapeCache{};
    std::unordered_map<EntityID, AppliedProps> mApplied{};

    MakeEntityShapeFn mMakeEntityShape{};
    FetchMeshInfoFn   mFetchMeshInfo{};
}

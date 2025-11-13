/*****************************************************************************/
/*!
\file       PhysicsBridge.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/11/09
\brief      Jolt Physics bridge glue:
            - BroadPhase layer interface (object to broadphase mapping)
            - Collision filters (object-layer pairs and object vs broadphase)
            - Global Jolt systems (temp allocator, job system, physics system)
            - Body/shape caches and engine hooks

            Units: meters, kilograms, seconds.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include "PhysicsBridge.h"

namespace Engine
{
    /*****************************************************************************/
    /*!
    \brief      Constructs the BroadPhase layer interface and sets up the mapping.
    */
    /*****************************************************************************/
    BPLayerInterfaceImpl::BPLayerInterfaceImpl()
        : mObjectToBroadPhase{ JPH::BroadPhaseLayer{ 0 }, JPH::BroadPhaseLayer{ 1 } }
        , mNumBroadPhaseLayers{ 2u }
    {
    }

    /*****************************************************************************/
    /*!
    \brief      Returns the number of BroadPhase layers used by the world.
    \return     Count of BroadPhase layers.
    */
    /*****************************************************************************/
    JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
    {
        return mNumBroadPhaseLayers;
    }

    /*****************************************************************************/
    /*!
    \brief      Maps an object layer to its BroadPhase layer.
    \param      layer   Object layer to map.
    \return     Corresponding BroadPhase layer.
    */
    /*****************************************************************************/
    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer layer) const
    {
        return mObjectToBroadPhase[layer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    /*****************************************************************************/
    /*!
    \brief      Returns a debug-friendly name for a BroadPhase layer.
    \param      layer   BroadPhase layer to name.
    \return     Null-terminated C string name.
    */
    /*****************************************************************************/
    const char *BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const
    {
        switch (layer.GetValue())
        {
        case 0: return "NON_MOVING";
        case 1: return "MOVING";
        default: return "UNKNOWN";
        }
    }
#endif

    /*****************************************************************************/
    /*!
    \brief      Object-layer pair filter: disallows NON_MOVING vs NON_MOVING.
    \param      a   First object layer.
    \param      b   Second object layer.
    \return     True if the pair should collide, false otherwise.
    */
    /*****************************************************************************/
    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const
    {
        if (a == Layers::NON_MOVING && b == Layers::NON_MOVING) return false;
        return true;
    }

    /*****************************************************************************/
    /*!
    \brief      Object-layer vs BroadPhase-layer collision filter.
    \param      layer   Object layer.
    \param      broad   BroadPhase layer.
    \return     True if this (object, broadphase) combination should collide.
    */
    /*****************************************************************************/
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

    /*****************************************************************************/
    /*!
    \brief      Global Jolt systems and engine-shared state.
    */
    /*****************************************************************************/
    JPH::TempAllocator *mTempAllocator = nullptr;
    JPH::JobSystemThreadPool *mJobSystem = nullptr;
    JPH::PhysicsSystem        mPhysics = {};
    JPH::BodyInterface *mBodyInterface = nullptr;

    BPLayerInterfaceImpl              mBPLayers = {};
    ObjectVsBroadPhaseLayerFilterImpl mObjVsBPLayerFilter = {};
    ObjectLayerPairFilterImpl         mObjPairFilter = {};

    std::unordered_map<EntityID, JPH::BodyID>                        mBodyOf = {};
    std::unordered_map<CacheKey, JPH::Ref<JPH::Shape>, CacheKeyHash> mShapeCache = {};
    std::unordered_map<EntityID, AppliedProps>                       mApplied = {};

    MakeEntityShapeFn  mMakeEntityShape = {};
    FetchMeshInfoFn    mFetchMeshInfo = {};
} // namespace Engine

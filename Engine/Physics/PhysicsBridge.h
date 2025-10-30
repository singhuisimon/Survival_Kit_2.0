#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/Scene.h"

namespace Engine
{
    using EntityID = entt::entity;

    namespace Layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING{ 0 };
        static constexpr JPH::ObjectLayer MOVING{ 1 };
        static constexpr JPH::ObjectLayer NUM_LAYERS{ 2 };
    }

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl();
        JPH::uint GetNumBroadPhaseLayers() const override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override;
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override;
#endif
    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[2];
        JPH::uint mNumBroadPhaseLayers{};
    };

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override;
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broad) const override;
    };

    inline JPH::Vec3  ToJPHVec3(const glm::vec3 &v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }
    inline JPH::RVec3 ToJPHRVec3(const glm::vec3 &v)
    {
        return JPH::RVec3(v.x, v.y, v.z);
    }
    inline JPH::Quat  ToJPHQuat(const glm::quat &q)
    {
        return JPH::Quat(q.x, q.y, q.z, q.w);
    }
    inline glm::vec3  ToGLM(const JPH::Vec3 &v)
    {
        return glm::vec3{ v.GetX(), v.GetY(), v.GetZ() };
    }
    inline glm::quat  ToGLM(const JPH::Quat &q)
    {
        return glm::quat{ q.GetW(), q.GetX(), q.GetY(), q.GetZ() };
    }

    inline glm::quat EulerDegToQuat(const glm::vec3 &eulerDeg)
    {
        return glm::quat(glm::radians(eulerDeg));
    }
    inline glm::vec3 QuatToEulerDeg(const glm::quat &qIn)
    {
        double w = qIn.w, x = qIn.x, y = qIn.y, z = qIn.z;
        double sr = 2.0 * (w * x + y * z), cr = 1.0 - 2.0 * (x * x + y * y);
        double roll = std::atan2(sr, cr);
        double sp = 2.0 * (w * y - z * x);
        double pitch = (std::abs(sp) >= 1.0) ? std::copysign(glm::half_pi<double>(), sp) : std::asin(sp);
        double sy = 2.0 * (w * z + x * y), cy = 1.0 - 2.0 * (y * y + z * z);
        double yaw = std::atan2(sy, cy);
        return glm::degrees(glm::vec3{ (float)roll, (float)pitch, (float)yaw });
    }

    using MakeEntityShapeFn = std::function<JPH::Ref<JPH::Shape>(Scene *, entt::entity, const TransformComponent &, const RigidbodyComponent &)>;

    struct MeshBuildInfo
    {
        std::vector<glm::vec3>     vertices;
        std::vector<std::uint32_t> indices;
        glm::vec3                  scale{ 1, 1, 1 };
        bool                       doubleSided{};
        bool                       preferConvex{};
        std::uint64_t              key{};
    };

    using FetchMeshInfoFn = std::function<bool(Scene *, entt::entity, MeshBuildInfo &)>;

    extern JPH::TempAllocator *mTempAllocator;
    extern JPH::JobSystemThreadPool *mJobSystem;
    extern JPH::PhysicsSystem        mPhysics;
    extern JPH::BodyInterface *mBodyInterface;

    extern BPLayerInterfaceImpl              mBPLayers;
    extern ObjectVsBroadPhaseLayerFilterImpl mObjVsBPLayerFilter;
    extern ObjectLayerPairFilterImpl         mObjPairFilter;

    extern std::unordered_map<EntityID, JPH::BodyID> mBodyOf;

    struct CacheKey
    {
        std::uint64_t key{};
        std::uint8_t  kind{};
        std::uint8_t  ds{};
        bool operator==(const CacheKey &o) const
        {
            return key == o.key && kind == o.kind && ds == o.ds;
        }
    };

    struct CacheKeyHash
    {
        std::size_t operator()(const CacheKey &k) const
        {
            std::size_t h = std::hash<std::uint64_t>{}(k.key);
            h ^= (std::size_t)k.kind + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            h ^= (std::size_t)k.ds + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            return h;
        }
    };

    extern std::unordered_map<CacheKey, JPH::Ref<JPH::Shape>, CacheKeyHash> mShapeCache;

    struct AppliedProps
    {
        bool isKinematic{};
        bool useGravity{};
        float mass{};
        std::uint64_t meshKey{};
        std::uint8_t shapeKind{};
        std::uint8_t shapeDS{};
        glm::vec3 shapeScale{ 1, 1, 1 };
        glm::vec3 lastPos{};
        glm::quat lastRot{};
    };

    extern std::unordered_map<EntityID, AppliedProps> mApplied;

    extern MakeEntityShapeFn mMakeEntityShape;
    extern FetchMeshInfoFn   mFetchMeshInfo;
}

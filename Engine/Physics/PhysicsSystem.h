/*****************************************************************************/
/*!
\file       PhysicsSystem.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/25
\brief      Jolt Physics system interface and utilities.

            Provides:
            - Broadphase/object layer definitions and filters
            - GLM <-> Jolt math conversion helpers (+ Euler-deg support)
            - Mesh-driven collider construction contract (callbacks + DTO)
            - PhysicsSystem ECS bridge: world bootstrap, body mirroring,
              shape caching, kinematic/dynamic sync, and lifecycle control

            Units: meters, kilograms, seconds. Coordinate system must match
            TransformComponent usage engine-wide.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
#pragma once

// --- STL (alphabetical) ---
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// --- glm (alphabetical) ---
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

// --- Jolt (alphabetical) ---
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

// --- Engine (alphabetical) ---
#include "../ECS/Components.h"
#include "../ECS/Scene.h"
#include "../ECS/System.h"

namespace Engine
{
    /**************************************************************************
     * @brief
     * Object layer aliases used for narrowphase permissions and broadphase
     * binning. Keep values stable to match serialization and filter logic.
     **************************************************************************/
    namespace Layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING{ 0 };  //!< Static / Kinematic
        static constexpr JPH::ObjectLayer MOVING{ 1 };      //!< Dynamic
        static constexpr JPH::ObjectLayer NUM_LAYERS{ 2 };
    }

    /**************************************************************************
     * @brief
     * BroadPhase layer interface mapping object layers to broadphase bins.
     *
     * Mapping:
     *  - NON_MOVING -> BP 0
     *  - MOVING     -> BP 1
     *
     * Rationale:
     *  Avoid static-static pairs at BP; permit dynamic against both.
     **************************************************************************/
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
            : mObjectToBroadPhase{ JPH::BroadPhaseLayer{ 0 }, JPH::BroadPhaseLayer{ 1 } },
            mNumBroadPhaseLayers{ 2u }
        {}
        JPH::uint GetNumBroadPhaseLayers() const override { return mNumBroadPhaseLayers; }
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override { return mObjectToBroadPhase[layer]; }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
        {
            switch (layer.GetValue()) { case 0: return "NON_MOVING"; case 1: return "MOVING"; default: return "UNKNOWN"; }
        }
#endif
    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[2];
        JPH::uint            mNumBroadPhaseLayers{};
    };

    /**************************************************************************
     * @brief
     * Object-layer pair filter for narrowphase. Disables static-static.
     **************************************************************************/
    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
        {
            if (a == Layers::NON_MOVING && b == Layers::NON_MOVING) return false;
            return true;
        }
    };

    /**************************************************************************
     * @brief
     * Object-vs-broadphase filter. Controls which BP bins a layer tests.
     *
     * Policy:
     *  - NON_MOVING vs BP: only MOVING bin (1)
     *  - MOVING     vs BP: both bins (0 and 1)
     **************************************************************************/
    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broad) const override
        {
            JPH::uint b = broad.GetValue();
            switch (layer)
            {
            case Layers::NON_MOVING: return b == 1u;
            case Layers::MOVING:     return (b == 0u) || (b == 1u);
            default:                 return false;
            }
        }
    };

    /**************************************************************************
     * @brief
     * Convert GLM/Jolt math types (position/rotation helpers).
     *
     * Notes:
     *  - ToJPHQuat expects a glm::quat (w last); ToGLM(JPH::Quat) returns
     *    glm::quat(w,x,y,z). See also Euler helpers below.
     **************************************************************************/
    inline JPH::Vec3  ToJPHVec3(glm::vec3 const &v) { return JPH::Vec3(v.x, v.y, v.z); }
    inline JPH::RVec3 ToJPHRVec3(glm::vec3 const &v) { return JPH::RVec3(v.x, v.y, v.z); }
    inline JPH::Quat  ToJPHQuat(glm::quat const &q) { return JPH::Quat(q.x, q.y, q.z, q.w); }
    inline glm::vec3  ToGLM(JPH::Vec3 const &v) { return glm::vec3{ v.GetX(), v.GetY(), v.GetZ() }; }
    inline glm::quat  ToGLM(JPH::Quat const &q) { return glm::quat{ q.GetW(), q.GetX(), q.GetY(), q.GetZ() }; }

    /**************************************************************************
     * @brief
     * Euler (degrees) <-> quaternion conversion helpers.
     *
     * Conventions:
     *  - Euler input/output in degrees (XYZ).
     *  - Quaternion is glm::quat (w,x,y,z).
     **************************************************************************/
    inline glm::quat EulerDegToQuat(glm::vec3 const &eulerDeg) { return glm::quat(glm::radians(eulerDeg)); }
    inline glm::vec3 QuatToEulerDeg(glm::quat const &qIn)
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

    /**************************************************************************
     * @brief
     * User-provided shape override callback.
     *
     * @param Scene*
     * @param entt::entity
     * @param TransformComponent const&
     * @param RigidbodyComponent const&
     *
     * @return
     * JPH shape to use (may be null to continue default building).
     **************************************************************************/
    using MakeEntityShapeFn = std::function<JPH::Ref<JPH::Shape>(Scene *, entt::entity, TransformComponent const &, RigidbodyComponent const &)>;

    /**************************************************************************
     * @brief
     * Mesh descriptor for collider construction.
     *
     * Fields:
     *  - vertices: model-space vertex positions
     *  - indices:  triangle index buffer (3*i .. 3*i+2)
     *  - scale:    non-uniform scaling to apply (via ScaledShape)
     *  - doubleSided: if true, triangle mesh is treated as double-sided
     *  - preferConvex: hint to build a convex hull (over triangle mesh)
     *  - key:      stable key for shape-cache lookup (hash of mesh asset)
     **************************************************************************/
    struct MeshBuildInfo
    {
        std::vector<glm::vec3>     vertices;
        std::vector<std::uint32_t> indices;
        glm::vec3                  scale{ 1.0f, 1.0f, 1.0f };
        bool                       doubleSided{};
        bool                       preferConvex{};
        std::uint64_t              key{};
    };

    /**************************************************************************
     * @brief
     * Callback to fetch mesh data for an entity. Return true if filled.
     **************************************************************************/
    using FetchMeshInfoFn = std::function<bool(Scene *, entt::entity, MeshBuildInfo &)>;

    /**************************************************************************
     * @brief
     * Physics system bridging ECS and Jolt.
     *
     * Responsibilities:
     *  - Bootstraps Jolt world (allocators, job system, filters, gravity)
     *  - Mirrors (Transform, Rigidbody) entities to Jolt bodies
     *  - Supports mesh/convex colliders via callbacks and a shape cache
     *  - Push/pull loop for kinematic poses and dynamic velocities
     **************************************************************************/
    class PhysicsSystem final : public System
    {
    public:
        /**********************************************************************
         * @brief
         * System name for diagnostics.
         **********************************************************************/
        char const *GetName() const override { return "PhysicsSystem"; }

        /**********************************************************************
         * @brief
         * Execution priority relative to other systems.
         **********************************************************************/
        int GetPriority() const override { return 10; }

        /**********************************************************************
         * @brief
         * Initialize the physics world and build bodies for current entities.
         *
         * @param scene
         * Scene handle used to access the ECS registry.
         **********************************************************************/
        void OnInit(Scene *scene) override;

        /**********************************************************************
         * @brief
         * Step simulation and synchronize ECS transforms and velocities.
         *
         * @param scene
         * Scene whose registry is mirrored to Jolt bodies.
         * @param dt
         * Delta time wrapper.
         **********************************************************************/
        void OnUpdate(Scene *scene, Timestep dt) override;

        /**********************************************************************
         * @brief
         * Shutdown physics and release resources.
         *
         * @param scene
         * Unused (kept for symmetry).
         **********************************************************************/
        void OnShutdown(Scene *scene) override;

        /**********************************************************************
         * @brief
         * Install a shape override callback. If it returns non-null, that
         * shape is used; otherwise mesh/fallback building is attempted.
         *
         * @param fn
         * Callback to set (moved in).
         **********************************************************************/
        void SetMakeEntityShapeCallback(MakeEntityShapeFn fn) { mMakeEntityShape = std::move(fn); }

        /**********************************************************************
         * @brief
         * Install a mesh fetch callback to supply geometry for colliders.
         *
         * @param fn
         * Callback to set (moved in).
         **********************************************************************/
        void SetFetchMeshInfoCallback(FetchMeshInfoFn fn) { mFetchMeshInfo = std::move(fn); }

    private:
        using EntityID = entt::entity;

        // --- Jolt world state ---
        JPH::TempAllocator *mTempAllocator{};          //!< Temp allocator per-step
        JPH::JobSystemThreadPool *mJobSystem{};        //!< Worker threads
        JPH::PhysicsSystem        mPhysics;            //!< Physics world
        JPH::BodyInterface *mBodyInterface{};          //!< Cached interface

        // --- Layering / filters (broadphase + narrowphase) ---
        BPLayerInterfaceImpl              mBPLayers;
        ObjectVsBroadPhaseLayerFilterImpl mObjVsBPLayerFilter;
        ObjectLayerPairFilterImpl         mObjPairFilter;

        // --- ECS <-> Jolt mapping ---
        std::unordered_map<EntityID, JPH::BodyID> mBodyOf;

        /**********************************************************************
         * @brief
         * Key for shape cache (mesh key + build flags).
         **********************************************************************/
        struct CacheKey
        {
            std::uint64_t key{};
            std::uint8_t  kind{};  //!< 0 = tri-mesh, 1 = convex
            std::uint8_t  ds{};    //!< 0 = single-sided, 1 = double-sided
            bool operator==(CacheKey const &o) const { return key == o.key && kind == o.kind && ds == o.ds; }
        };

        /**********************************************************************
         * @brief
         * Hasher for CacheKey (mixes fields; stable across runs).
         **********************************************************************/
        struct CacheKeyHash
        {
            std::size_t operator()(CacheKey const &k) const
            {
                std::size_t h = std::hash<std::uint64_t>{}(k.key);
                h ^= (std::size_t)k.kind + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
                h ^= (std::size_t)k.ds + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
                return h;
            }
        };

        // --- Shape cache: avoids re-building identical hull/mesh shapes ---
        std::unordered_map<CacheKey, JPH::Ref<JPH::Shape>, CacheKeyHash> mShapeCache;

        // --- Extensibility hooks ---
        MakeEntityShapeFn mMakeEntityShape;  //!< Optional custom shape provider
        FetchMeshInfoFn   mFetchMeshInfo;    //!< Optional mesh fetch provider

        /**********************************************************************
         * @brief
         * Translate Rigidbody flags to Jolt motion type.
         *
         * @param rb
         * Rigidbody component.
         * @return
         * JPH motion type (Kinematic or Dynamic).
         **********************************************************************/
        static JPH::EMotionType ToMotionType(RigidbodyComponent const &rb) { return rb.IsKinematic ? JPH::EMotionType::Kinematic : JPH::EMotionType::Dynamic; }

        /**********************************************************************
         * @brief
         * Translate Rigidbody flags to object layer.
         *
         * @param rb
         * Rigidbody component.
         * @return
         * NON_MOVING for kinematic, MOVING for dynamic.
         **********************************************************************/
        static JPH::ObjectLayer ToObjectLayer(RigidbodyComponent const &rb) { return rb.IsKinematic ? Layers::NON_MOVING : Layers::MOVING; }

        /**********************************************************************
         * @brief
         * Ensure Jolt bodies exist precisely for eligible entities.
         *
         * Creates new bodies for (Transform,Rigidbody) entities without one
         * and destroys bodies for entities that are no longer eligible.
         *
         * @param scene
         * Scene to scan.
         **********************************************************************/
        void BuildOrRefreshBodies(Scene *scene);

        /**********************************************************************
         * @brief
         * Create and register a Jolt body for an entity.
         *
         * @param scene
         * Scene handle (for callbacks).
         * @param e
         * Entity identifier.
         **********************************************************************/
        void CreateBodyFor(Scene *scene, EntityID e);

        /**********************************************************************
         * @brief
         * Remove and destroy the Jolt body associated with an entity.
         *
         * @param e
         * Entity identifier.
         **********************************************************************/
        void DestroyBodyFor(EntityID e);

        /**********************************************************************
         * @brief
         * Construct or retrieve a cached collider shape for an entity.
         *
         * Resolution order:
         *  1) mMakeEntityShape callback
         *  2) Mesh-driven (convex hull or triangle mesh) via mFetchMeshInfo
         *     + optional ScaledShape wrapper
         *  3) Fallback unit BoxShape
         *
         * @param scene
         * Scene handle (for callbacks).
         * @param e
         * Entity identifier.
         * @param tc
         * Transform (read-only).
         * @param rb
         * Rigidbody (read-only).
         * @return
         * Ref-counted shape (never null; falls back to box).
         **********************************************************************/
        JPH::Ref<JPH::Shape> MakeShapeForEntity(Scene *scene, EntityID e, TransformComponent const &tc, RigidbodyComponent const &rb);
    };
} // namespace Engine

/*****************************************************************************/
/*!
\file       PhysicsSystem.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/23
\brief      Jolt physics system with broadphase layers/filters and mesh collider
            support (triangle mesh / convex hull), shape cache, and mesh fetch
            callback.

            Provides:
            - Initialize/Shutdown Jolt world
            - Create static triangle-mesh colliders and dynamic convex-hull bodies
            - Broadphase layer interface and object layer filters
            - Step simulation and sync transforms to ECS
            - Gravity / material control
            - Shape caching per MeshType (+ scale)

            Usage notes:
            - Provide a MeshFetchCallback that fills vertices/indices for a
              given MeshType handle. This allows PhysicsSystem to build Jolt
              MeshShape or ConvexHullShape.
            - Call Step(registry, dt) each frame to advance simulation and
              write back positions/rotations to TransformComponent.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this i've comfile or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
#pragma once

// --- STL (alphabetical) ---
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

// --- entt / glm (alphabetical) ---
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// --- Jolt (alphabetical) ---
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilter.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/PhysicsSystem.h>

// --- Your components ---
#include "../ECS/Components.h"                        // MeshRendererComponent
#include "../Component/TransformComponent.h"      // Engine::TransformComponent

namespace Engine
{
    /**************************************************************************
     * @brief
     * Physics system using Jolt. Owns the world, shapes, bodies, and filters.
     **************************************************************************/
    class PhysicsSystem final
    {
    public:
        // --- Types ----------------------------------------------------------

        using u32 = std::uint32_t;

        /**************************************************************************
         * @brief
         * Mesh data container for building shapes.
         * @param Positions
         * Vertex positions in local/model space.
         * @param Indices
         * Triangle indices (triples).
         **************************************************************************/
        struct MeshData
        {
            std::vector<glm::vec3> Positions;
            std::vector<u32>       Indices;    // 3 per triangle
        };

        /**************************************************************************
         * @brief
         * Callback signature to fetch mesh data given MeshType handle.
         * @param meshType
         * Mesh handle from MeshRendererComponent::MeshType.
         * @param out
         * Filled with vertices/indices on success.
         * @return
         * True on success, false if the mesh cannot be provided.
         **************************************************************************/
        using MeshFetchCallback = std::function<bool(u32 meshType, MeshData &out)>;

        /**************************************************************************
         * @brief
         * Basic physics material values for created bodies.
         **************************************************************************/
        struct Material
        {
            float Friction;      // [0..inf), typical 0.2..1.0
            float Restitution;   // [0..1], bounciness
        };

        /**************************************************************************
         * @brief
         * Body creation options for dynamic bodies built from a convex hull.
         **************************************************************************/
        struct DynamicBodyDesc
        {
            float Mass;                // > 0
            bool  StartActive;         // wake on create
            bool  ContinuousDetection; // CCD
            float LinearDamping;
            float AngularDamping;
            Material Mat;
        };

        /**************************************************************************
         * @brief
         * Global settings used to initialize Jolt.
         **************************************************************************/
        struct Settings
        {
            // Limits: tune to your project scale
            u32   MaxBodies = 65536;
            u32   NumBodyMutexes = 0;       // 0 -> Jolt picks
            u32   MaxBodyPairs = 65536;
            u32   MaxContactConstraints = 65536;

            // Threads: 0 -> use hardware concurrency - 1
            u32   WorkerThreadCount = 0;

            // Gravity
            glm::vec3 Gravity = glm::vec3{ 0.0f, -9.81f, 0.0f };

            // Default material for created bodies
            Material DefaultMaterial = Material{ 0.6f, 0.0f };

            // Mesh provider
            MeshFetchCallback MeshFetch = {};
        };

        // --- Ctors / Dtors ---------------------------------------------------

        PhysicsSystem() = default;
        ~PhysicsSystem();

        PhysicsSystem(PhysicsSystem const &) = delete;
        PhysicsSystem &operator=(PhysicsSystem const &) = delete;

        // --- Init / Shutdown -------------------------------------------------

        /**************************************************************************
         * @brief
         * Initializes Jolt, broadphase layers, filters, and world.
         * @param cfg
         * Settings including gravity and mesh callback.
         **************************************************************************/
        void Initialize(Settings const &cfg);

        /**************************************************************************
         * @brief
         * Destroys all bodies and shuts down Jolt world and allocators.
         **************************************************************************/
        void Shutdown();

        // --- World control ---------------------------------------------------

        /**************************************************************************
         * @brief
         * Steps physics and syncs transforms back to ECS for dynamic bodies.
         * @param registry
         * entt registry to write positions/rotations into TransformComponent.
         * @param dt
         * Delta time in seconds.
         * @param collisionSteps
         * Number of collision substeps (typ. 1).
         * @param integrationSubSteps
         * Number of integration substeps (typ. 1..4).
         **************************************************************************/
        void Step(entt::registry &registry, float dt,
            int collisionSteps = 1, int integrationSubSteps = 1);

        /**************************************************************************
         * @brief
         * Adjust gravity at runtime.
         **************************************************************************/
        void SetGravity(glm::vec3 const &g);

        // --- Body creation / destruction -------------------------------------

        /**************************************************************************
         * @brief
         * Creates a STATIC triangle-mesh body from MeshType.
         * @param e
         * Entity id (used for bookkeeping and transform sync during rebuild).
         * @param tr
         * Transform (position/rotation/scale).
         * @param mr
         * MeshRendererComponent providing MeshType handle.
         * @return
         * Jolt BodyID of the created body (invalid if failed).
         **************************************************************************/
        JPH::BodyID CreateStaticMesh(entt::entity e,
            TransformComponent const &tr,
            MeshRendererComponent const &mr);

        /**************************************************************************
         * @brief
         * Creates a DYNAMIC body by building a convex hull from MeshType.
         * @param e
         * Entity id to track.
         * @param tr
         * Transform (position/rotation/scale).
         * @param mr
         * MeshRendererComponent providing MeshType handle.
         * @param desc
         * Dynamic creation parameters (mass, damping, material).
         * @return
         * Jolt BodyID of the created body (invalid if failed).
         **************************************************************************/
        JPH::BodyID CreateDynamicConvex(entt::entity e,
            TransformComponent const &tr,
            MeshRendererComponent const &mr,
            DynamicBodyDesc const &desc);

        /**************************************************************************
         * @brief
         * Destroys a body associated with entity if present.
         **************************************************************************/
        void DestroyBody(entt::entity e);

        // --- Queries ---------------------------------------------------------

        /**************************************************************************
         * @brief
         * Returns true if an entity has a body registered in the physics world.
         **************************************************************************/
        bool HasBody(entt::entity e) const;

        /**************************************************************************
         * @brief
         * Fetch current body transform (COM) into glm types.
         * @return
         * True on success.
         **************************************************************************/
        bool GetBodyTransform(entt::entity e, glm::vec3 &outPos, glm::quat &outRot) const;

    private:
        // --- Broadphase / layers --------------------------------------------

        enum class O

/*****************************************************************************/
/*!
\file       PhysicsSystem.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/23
\brief      Jolt physics system with broadphase layers/filters and mesh collider
			support (triangle mesh / convex hull), shape cache, and mesh fetch
			callback.

			Implementation notes:
			- Broadphase layers:
			  NON_MOVING collides with MOVING; MOVING collides with all;
			  SENSOR collides with MOVING only.
			- Static meshes use JPH::MeshShape; scaling via JPH::ScaledShape.
			- Dynamic bodies use convex hull (JPH::ConvexHullShape).
			- Transform sync: Jolt -> TransformComponent each step for dynamics.
			- Shapes cached per MeshType and (MeshType,Scale) key.
*/
/*****************************************************************************/

#include "PhysicsSystem.h"

#include <cassert>
#include <cstring>
#include <limits>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace JPH;

namespace
{
	/**************************************************************************
	 * @brief
	 * Optional trace sink for Jolt. No-op by default to avoid extra logging.
	 *
	 * @param fmt
	 * printf-style format string.
	 * @param ...
	 * Variable arguments matching the format string.
	 **************************************************************************/
	void TraceImpl(char const *fmt, ...)
	{
		(void)fmt;
	}

	/**************************************************************************
	 * @brief
	 * Assertion hook for Jolt. Returns true to trigger a debugger break when
	 * assertions are enabled (JPH_ENABLE_ASSERTS).
	 *
	 * @param expr
	 * Expression text that failed.
	 * @param msg
	 * Optional message supplied by the assertion.
	 * @param file
	 * Source file where the assertion occurred.
	 * @param line
	 * Line number where the assertion occurred.
	 *
	 * @return
	 * true to request a break, false to continue execution.
	 **************************************************************************/
	bool AssertFailedImpl(char const *expr, char const *msg, char const *file, unsigned line)
	{
		(void)expr; (void)msg; (void)file; (void)line;
		return true;
	}

	/**************************************************************************
	 * @brief
	 * Packs a glm::vec3 into a 32-bit value suitable for hashing caches where
	 * bit-stable float identity is desired.
	 *
	 * @param v
	 * Vector to pack.
	 *
	 * @return
	 * 32-bit hash-friendly key derived from the raw float bits.
	 **************************************************************************/
	inline uint32_t PackVec3ToKey(glm::vec3 const &v)
	{
		union U { float f; uint32_t u; };
		U x{ v.x }, y{ v.y }, z{ v.z };

		uint32_t h = x.u * 0x9E3779B1u;
		h ^= y.u + 0x85EBCA77u + (h << 6) + (h >> 2);
		h ^= z.u + 0xC2B2AE3Du + (h << 6) + (h >> 2);
		return h;
	}
}

namespace Engine
{
	// -------------------------------------------------------------------------
	// Broadphase / layer filters
	// -------------------------------------------------------------------------

	/**************************************************************************
	 * @brief
	 * Constructs the object->broadphase layer mapping table.
	 **************************************************************************/
	PhysicsSystem::BPLayerInterfaceImpl::BPLayerInterfaceImpl()
	{
		mMap[static_cast<int>(ObjectLayer::NON_MOVING)] = static_cast<BroadPhaseLayer>(BPLayer::NON_MOVING);
		mMap[static_cast<int>(ObjectLayer::MOVING)] = static_cast<BroadPhaseLayer>(BPLayer::MOVING);
		mMap[static_cast<int>(ObjectLayer::SENSOR)] = static_cast<BroadPhaseLayer>(BPLayer::SENSOR);
	}

	/**************************************************************************
	 * @brief
	 * Returns number of broadphase layers used.
	 *
	 * @return
	 * Count of distinct broadphase layers.
	 **************************************************************************/
	uint PhysicsSystem::BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return static_cast<uint>(BPLayer::COUNT);
	}

	/**************************************************************************
	 * @brief
	 * Maps an object layer to its broadphase layer.
	 *
	 * @param layer
	 * Object layer to translate.
	 *
	 * @return
	 * Corresponding broadphase layer.
	 **************************************************************************/
	BroadPhaseLayer PhysicsSystem::BPLayerInterfaceImpl::GetBroadPhaseLayer(ObjectLayer layer) const
	{
		return mMap[static_cast<int>(layer)];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	/**************************************************************************
	 * @brief
	 * Human-readable names for broadphase layers (profiling builds).
	 *
	 * @param layer
	 * Broadphase layer enum.
	 *
	 * @return
	 * C-string layer name.
	 **************************************************************************/
	char const *PhysicsSystem::BPLayerInterfaceImpl::GetBroadPhaseLayerName(BroadPhaseLayer layer) const
	{
		switch (static_cast<BPLayer>(layer))
		{
		case BPLayer::NON_MOVING: return "NON_MOVING";
		case BPLayer::MOVING:     return "MOVING";
		case BPLayer::SENSOR:     return "SENSOR";
		default:                  return "UNKNOWN";
		}
	}
#endif

	/**************************************************************************
	 * @brief
	 * Object-vs-broadphase filter. Encodes which object layers traverse which
	 * broadphase bins to find potential pairs.
	 *
	 * @param layer
	 * Object layer of the body.
	 * @param broadphase
	 * Broadphase bin being considered.
	 *
	 * @return
	 * True if broadphase bin should be tested for this object layer.
	 **************************************************************************/
	bool PhysicsSystem::ObjectVsBroadPhaseFilterImpl::ShouldCollide(ObjectLayer layer, BroadPhaseLayer broadphase) const
	{
		BPLayer const bl = static_cast<BPLayer>(broadphase);
		switch (static_cast<ObjectLayer>(layer))
		{
		case ObjectLayer::NON_MOVING: return (bl == BPLayer::MOVING);
		case ObjectLayer::MOVING:     return (bl == BPLayer::MOVING) || (bl == BPLayer::NON_MOVING) || (bl == BPLayer::SENSOR);
		case ObjectLayer::SENSOR:     return (bl == BPLayer::MOVING);
		default:                      return false;
		}
	}

	/**************************************************************************
	 * @brief
	 * Object pair filter (narrowphase). Encodes which object layer pairs are
	 * allowed to collide.
	 *
	 * @param a
	 * First object layer.
	 * @param b
	 * Second object layer.
	 *
	 * @return
	 * True if the two object layers should collide.
	 **************************************************************************/
	bool PhysicsSystem::ObjectLayerPairFilterImpl::ShouldCollide(ObjectLayer a, ObjectLayer b) const
	{
		// NM vs M: yes; NM vs S: no; M vs S: yes; M vs M: yes; NM vs NM: no; S vs S: no.
		if ((a == ObjectLayer::NON_MOVING && b == ObjectLayer::MOVING) ||
			(b == ObjectLayer::NON_MOVING && a == ObjectLayer::MOVING)) return true;
		if (a == ObjectLayer::MOVING && b == ObjectLayer::MOVING) return true;
		if ((a == ObjectLayer::MOVING && b == ObjectLayer::SENSOR) ||
			(b == ObjectLayer::MOVING && a == ObjectLayer::SENSOR)) return true;
		return false;
	}

	// -------------------------------------------------------------------------
	// Conversions
	// -------------------------------------------------------------------------

	/**************************************************************************
	 * @brief Converts glm::vec3 to JPH::Vec3.
	 **************************************************************************/
	JPH::Vec3 PhysicsSystem::ToJPH(glm::vec3 const &v)
	{
		return JPH::Vec3(v.x, v.y, v.z);
	}

	/**************************************************************************
	 * @brief Converts glm::vec3 to JPH::RVec3 (positions).
	 **************************************************************************/
	JPH::RVec3 PhysicsSystem::ToJPH_R(glm::vec3 const &v)
	{
		return JPH::RVec3(v.x, v.y, v.z);
	}

	/**************************************************************************
	 * @brief Converts glm::quat (w,x,y,z) to JPH::Quat (x,y,z,w).
	 **************************************************************************/
	JPH::Quat PhysicsSystem::ToJPH(glm::quat const &q)
	{
		return JPH::Quat(q.x, q.y, q.z, q.w);
	}

	/**************************************************************************
	 * @brief Converts JPH::RVec3 to glm::vec3.
	 **************************************************************************/
	glm::vec3 PhysicsSystem::ToGLM(JPH::RVec3Arg v)
	{
		return glm::vec3(
			static_cast<float>(v.GetX()),
			static_cast<float>(v.GetY()),
			static_cast<float>(v.GetZ()));
	}

	/**************************************************************************
	 * @brief Converts JPH::Quat (x,y,z,w) to glm::quat (w,x,y,z).
	 **************************************************************************/
	glm::quat PhysicsSystem::ToGLM(JPH::QuatArg q)
	{
		return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
	}

	// -------------------------------------------------------------------------
	// Utilities
	// -------------------------------------------------------------------------

	/**************************************************************************
	 * @brief
	 * Hash helper for the scaled-shape cache combining mesh type and scale.
	 *
	 * @param meshType
	 * Engine-defined mesh identifier.
	 * @param scale
	 * Non-uniform instance scale.
	 *
	 * @return
	 * Combined hash value.
	 **************************************************************************/
	std::size_t PhysicsSystem::HashScaledKey(u32 meshType, glm::vec3 const &scale)
	{
		std::size_t h = static_cast<std::size_t>(meshType) * 1469598103934665603ull;
		h ^= static_cast<std::size_t>(PackVec3ToKey(scale)) + 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
		return h;
	}

	/**************************************************************************
	 * @brief
	 * Shortcut to the world's body interface.
	 *
	 * @return
	 * Reference to JPH body interface.
	 **************************************************************************/
	JPH::BodyInterface &PhysicsSystem::BodyIF() const
	{
		return mWorld->GetBodyInterface();
	}

	// -------------------------------------------------------------------------
	// Lifecycle
	// -------------------------------------------------------------------------

	/**************************************************************************
	 * @brief
	 * Destructor. Calls Shutdown to release resources if still initialized.
	 **************************************************************************/
	PhysicsSystem::~PhysicsSystem()
	{
		Shutdown();
	}

	/**************************************************************************
	 * @brief
	 * Initializes Jolt systems, allocators, job system, physics world, gravity,
	 * and default material. Idempotent: repeated calls do nothing after init.
	 *
	 * @param cfg
	 * Settings used to configure the physics system on startup.
	 **************************************************************************/
	void PhysicsSystem::Initialize(Settings const &cfg)
	{
		if (mInitialized) return;

		mCfg = cfg;

		RegisterDefaultAllocator();
		Trace = TraceImpl;
		JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

			if (Factory::sInstance == nullptr)
				Factory::sInstance = new Factory();

		RegisterTypes();

		u32 const hw = std::max(1u, (cfg.WorkerThreadCount == 0)
			? (std::max(1u, std::thread::hardware_concurrency()) - 1u)
			: cfg.WorkerThreadCount);

		mTempAllocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
		mJobSystem = std::make_unique<JobSystemThreadPool>(hw);

		mWorld = std::make_unique<JPH::PhysicsSystem>();
		mWorld->Init(
			cfg.MaxBodies,
			cfg.NumBodyMutexes,
			cfg.MaxBodyPairs,
			cfg.MaxContactConstraints,
			mBPLayerIF,
			mObjVsBPFilter,
			mObjPairFilter
		);

		BodyIF().SetGravity(ToJPH(cfg.Gravity));
		mDefaultMaterial = new PhysicsMaterial("Default");

		mInitialized = true;
	}

	/**************************************************************************
	 * @brief
	 * Shuts down the physics system: destroys bodies, clears caches, releases
	 * world and infrastructure, and unregisters Jolt types/factory if owned.
	 **************************************************************************/
	void PhysicsSystem::Shutdown()
	{
		if (!mInitialized) return;

		for (auto const &kv : mEntityToBody)
		{
			BodyID const id = kv.second;
			BodyIF().RemoveBody(id);
			BodyIF().DestroyBody(id);
		}
		mEntityToBody.clear();
		mBodyToEntity.clear();

		mScaledCache.clear();
		mTriBaseCache.clear();
		mConvexBaseCache.clear();
		mDefaultMaterial = nullptr;

		mWorld.reset();
		mJobSystem.reset();
		mTempAllocator.reset();

		if (Factory::sInstance)
		{
			UnregisterTypes();
			delete Factory::sInstance;
			Factory::sInstance = nullptr;
		}

		mInitialized = false;
	}

	// -------------------------------------------------------------------------
	// World control
	// -------------------------------------------------------------------------

	/**************************************************************************
	 * @brief
	 * Steps the simulation and writes dynamic body transforms back into the ECS.
	 *
	 * @param registry
	 * entt registry containing TransformComponent, etc.
	 * @param dt
	 * Delta time in seconds; non-positive values are ignored.
	 * @param collisionSteps
	 * Sub-steps for collision detection.
	 * @param integrationSubSteps
	 * Sub-steps for integration.
	 **************************************************************************/
	void PhysicsSystem::Step(entt::registry &registry, float dt, int collisionSteps, int integrationSubSteps)
	{
		if (!mInitialized || dt <= 0.0f) return;

		mWorld->Update(dt, collisionSteps, integrationSubSteps, mTempAllocator.get(), mJobSystem.get());

		for (auto const &kv : mEntityToBody)
		{
			entt::entity e = kv.first;
			BodyID const id = kv.second;

			BodyLockRead lock(mWorld->GetBodyLockInterface(), id);
			if (!lock.Succeeded()) continue;

			Body const &body = lock.GetBody();
			if (!body.IsDynamic()) continue;

			if (registry.all_of<TransformComponent>(e))
			{
				auto &tr = registry.get<TransformComponent>(e);
				tr.Position = ToGLM(body.GetPosition());
				tr.Rotation = ToGLM(body.GetRotation());
				tr.IsDirty = true;
			}
		}
	}

	/**************************************************************************
	 * @brief
	 * Sets gravity. If called before Initialize, stores the value in settings.
	 *
	 * @param g
	 * Gravity vector in world units per second squared.
	 **************************************************************************/
	void PhysicsSystem::SetGravity(glm::vec3 const &g)
	{
		if (!mInitialized) { mCfg.Gravity = g; return; }
		BodyIF().SetGravity(ToJPH(g));
	}

	// -------------------------------------------------------------------------
	// Shape builders / cache
	// -------------------------------------------------------------------------

	/**************************************************************************
	 * @brief
	 * Builds or retrieves a triangle mesh shape (optionally scaled) for a mesh
	 * type. Uses a base unscaled mesh in a cache plus a scaled wrapper cache.
	 *
	 * @param meshType
	 * Engine-defined mesh identifier.
	 * @param scale
	 * Non-uniform instance scale.
	 *
	 * @return
	 * RefConst<Shape> to the cooked mesh or nullptr on failure.
	 **************************************************************************/
	RefConst<Shape> PhysicsSystem::BuildOrGetTriMeshShape(u32 meshType, glm::vec3 const &scale)
	{
		RefConst<Shape> base;
		{
			auto it = mTriBaseCache.find(meshType);
			if (it != mTriBaseCache.end())
			{
				base = it->second;
			}
			else
			{
				if (!mCfg.MeshFetch) return nullptr;

				MeshData md;
				if (!mCfg.MeshFetch(meshType, md) || md.Positions.empty() || md.Indices.size() < 3)
					return nullptr;

				Array<Float3> verts; verts.reserve(md.Positions.size());
				for (glm::vec3 const &p : md.Positions) verts.push_back(Float3(p.x, p.y, p.z));

				Array<IndexedTriangle> tris; tris.reserve(md.Indices.size() / 3);
				for (size_t i = 0; i + 2 < md.Indices.size(); i += 3)
					tris.push_back(IndexedTriangle(md.Indices[i + 0], md.Indices[i + 1], md.Indices[i + 2]));

				MeshShapeSettings ms(verts, tris);
				ms.mMaterial = mDefaultMaterial;

				auto result = ms.Create();
				if (!result.IsValid()) return nullptr;

				base = result.Get();
				mTriBaseCache.emplace(meshType, base);
			}
		}

		std::size_t const key = HashScaledKey(meshType, scale);
		if (auto it = mScaledCache.find(key); it != mScaledCache.end()) return it->second;

		if (glm::all(glm::epsilonEqual(scale, glm::vec3(1.0f), 0.0f)))
		{
			mScaledCache.emplace(key, base);
			return base;
		}

		ScaledShapeSettings sss(base, Vec3(scale.x, scale.y, scale.z));
		auto sres = sss.Create();
		if (!sres.IsValid()) return nullptr;

		RefConst<Shape> scaled = sres.Get();
		mScaledCache.emplace(key, scaled);
		return scaled;
	}

	/**************************************************************************
	 * @brief
	 * Builds or retrieves a convex hull shape (optionally scaled) for a mesh
	 * type. Uses an unscaled convex base cache plus a scaled wrapper cache.
	 *
	 * @param meshType
	 * Engine-defined mesh identifier.
	 * @param scale
	 * Non-uniform instance scale.
	 *
	 * @return
	 * RefConst<Shape> to the convex shape or nullptr on failure.
	 **************************************************************************/
	RefConst<Shape> PhysicsSystem::BuildOrGetConvexShape(u32 meshType, glm::vec3 const &scale)
	{
		RefConst<Shape> base;
		{
			auto it = mConvexBaseCache.find(meshType);
			if (it != mConvexBaseCache.end())
			{
				base = it->second;
			}
			else
			{
				if (!mCfg.MeshFetch) return nullptr;

				MeshData md;
				if (!mCfg.MeshFetch(meshType, md) || md.Positions.size() < 4) return nullptr;

				Array<Vec3> points; points.reserve(md.Positions.size());
				for (glm::vec3 const &p : md.Positions) points.push_back(Vec3(p.x, p.y, p.z));

				ConvexHullShapeSettings cs(points);
				cs.mMaterial = mDefaultMaterial;

				auto result = cs.Create();
				if (!result.IsValid()) return nullptr;

				base = result.Get();
				mConvexBaseCache.emplace(meshType, base);
			}
		}

		std::size_t const key = HashScaledKey(meshType ^ 0xC0FFEEu, scale);
		if (auto it = mScaledCache.find(key); it != mScaledCache.end()) return it->second;

		if (glm::all(glm::epsilonEqual(scale, glm::vec3(1.0f), 0.0f)))
		{
			mScaledCache.emplace(key, base);
			return base;
		}

		ScaledShapeSettings sss(base, Vec3(scale.x, scale.y, scale.z));
		auto sres = sss.Create();
		if (!sres.IsValid()) return nullptr;

		RefConst<Shape> scaled = sres.Get();
		mScaledCache.emplace(key, scaled);
		return scaled;
	}

	// -------------------------------------------------------------------------
	// Body creation API
	// -------------------------------------------------------------------------

	/**************************************************************************
	 * @brief
	 * Creates a static triangle-mesh body for the given entity.
	 *
	 * @param e
	 * Entity to associate with the body.
	 * @param tr
	 * Transform component providing position, rotation, and scale.
	 * @param mr
	 * Mesh renderer component providing mesh type.
	 *
	 * @return
	 * BodyID of the created body or an empty ID on failure.
	 **************************************************************************/
	BodyID PhysicsSystem::CreateStaticMesh(entt::entity e, TransformComponent const &tr, MeshRendererComponent const &mr)
	{
		if (!mInitialized) return BodyID();

		RefConst<Shape> shape = BuildOrGetTriMeshShape(mr.MeshType, tr.Scale);
		if (shape == nullptr) return BodyID();

		BodyCreationSettings bcs;
		bcs.mShape = shape;
		bcs.mObjectLayer = static_cast<JPH::ObjectLayer>(ObjectLayer::NON_MOVING);
		bcs.mMotionType = EMotionType::Static;
		bcs.mPosition = ToJPH_R(tr.Position);
		bcs.mRotation = ToJPH(tr.Rotation);
		bcs.mFriction = mCfg.DefaultMaterial.Friction;
		bcs.mRestitution = mCfg.DefaultMaterial.Restitution;
		bcs.mAllowSleeping = true;

		Body *body = BodyIF().CreateBody(bcs);
		if (!body) return BodyID();

		BodyID id = body->GetID();
		BodyIF().AddBody(id, EActivation::DontActivate);

		mEntityToBody[e] = id;
		mBodyToEntity[id] = e;

		return id;
	}

	/**************************************************************************
	 * @brief
	 * Creates a dynamic convex body for the given entity.
	 *
	 * @param e
	 * Entity to associate with the body.
	 * @param tr
	 * Transform component providing position, rotation, and scale.
	 * @param mr
	 * Mesh renderer component providing mesh type.
	 * @param desc
	 * Dynamic body descriptor (mass, damping, material, etc.).
	 *
	 * @return
	 * BodyID of the created body or an empty ID on failure.
	 **************************************************************************/
	BodyID PhysicsSystem::CreateDynamicConvex(entt::entity e, TransformComponent const &tr, MeshRendererComponent const &mr, DynamicBodyDesc const &desc)
	{
		if (!mInitialized || desc.Mass <= 0.0f) return BodyID();

		RefConst<Shape> shape = BuildOrGetConvexShape(mr.MeshType, tr.Scale);
		if (shape == nullptr) return BodyID();

		BodyCreationSettings bcs;
		bcs.mShape = shape;
		bcs.mObjectLayer = static_cast<JPH::ObjectLayer>(ObjectLayer::MOVING);
		bcs.mMotionType = EMotionType::Dynamic;
		bcs.mPosition = ToJPH_R(tr.Position);
		bcs.mRotation = ToJPH(tr.Rotation);
		bcs.mFriction = desc.Mat.Friction;
		bcs.mRestitution = desc.Mat.Restitution;
		bcs.mLinearDamping = desc.LinearDamping;
		bcs.mAngularDamping = desc.AngularDamping;
		bcs.mAllowSleeping = true;

		bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		bcs.mMassPropertiesOverride.mMass = desc.Mass;

		Body *body = BodyIF().CreateBody(bcs);
		if (!body) return BodyID();

		BodyID id = body->GetID();
		BodyIF().AddBody(id, desc.StartActive ? EActivation::Activate : EActivation::DontActivate);

		if (desc.ContinuousDetection)
			BodyIF().SetMotionQuality(id, EMotionQuality::LinearCast);

		mEntityToBody[e] = id;
		mBodyToEntity[id] = e;

		return id;
	}

	/**************************************************************************
	 * @brief
	 * Destroys the physics body associated with an entity (if any).
	 *
	 * @param e
	 * Entity whose body should be removed and destroyed.
	 **************************************************************************/
	void PhysicsSystem::DestroyBody(entt::entity e)
	{
		auto it = mEntityToBody.find(e);
		if (it == mEntityToBody.end()) return;

		BodyID const id = it->second;
		BodyIF().RemoveBody(id);
		BodyIF().DestroyBody(id);

		mBodyToEntity.erase(id);
		mEntityToBody.erase(it);
	}

	/**************************************************************************
	 * @brief
	 * Checks if an entity currently has an associated physics body.
	 *
	 * @param e
	 * Entity to query.
	 *
	 * @return
	 * True if a body exists for the entity, false otherwise.
	 **************************************************************************/
	bool PhysicsSystem::HasBody(entt::entity e) const
	{
		return mEntityToBody.find(e) != mEntityToBody.end();
	}

	/**************************************************************************
	 * @brief
	 * Reads the body's transform for an entity (if present).
	 *
	 * @param e
	 * Entity to query.
	 * @param outPos
	 * Output world position.
	 * @param outRot
	 * Output world rotation (glm::quat).
	 *
	 * @return
	 * True on success, false if no body or lock failed.
	 **************************************************************************/
	bool PhysicsSystem::GetBodyTransform(entt::entity e, glm::vec3 &outPos, glm::quat &outRot) const
	{
		auto it = mEntityToBody.find(e);
		if (it == mEntityToBody.end() || !mInitialized) return false;

		BodyID const id = it->second;

		BodyLockRead lock(mWorld->GetBodyLockInterface(), id);
		if (!lock.Succeeded()) return false;

		Body const &body = lock.GetBody();
		outPos = ToGLM(body.GetPosition());
		outRot = ToGLM(body.GetRotation());
		return true;
	}

} // namespace Engine

/*****************************************************************************/
/*!
\file       PhysicsSystem.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/25
\brief      Jolt Physics runtime integration:
			- World bootstrap (Factory, allocators, job system)
			- Broadphase layer/filter hookup (configured in header)
			- Body lifecycle mirroring ECS (create/refresh/destroy)
			- Kinematic pose push & dynamic velocity push/pull
			- Mesh collider support (triangle mesh / convex hull) with
			  scaled-shape wrapping and shape caching
			- Rotation helpers supporting glm::quat and Euler-deg vec3

			Units: meters, kilograms, seconds.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include <algorithm>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <limits>

#include "PhysicsSystem.h"
#include "PhysicsAPI.h"

namespace Engine
{
	static constexpr float DEFAULT_HALF_EXT = 0.5f;

	/*****************************************************************************/
	/*!
	\brief      Converts a rotation to Jolt quaternion.
	\tparam     R   Either glm::vec3 (Euler degrees) or glm::quat.
	\param      r   Rotation in Euler degrees (glm::vec3) or quaternion (glm::quat).
	\return     JPH::Quat constructed from the input.
	*/
	/*****************************************************************************/
	template<typename R>
	static inline JPH::Quat ToJPHRotation(R const &r)
	{
		if constexpr (std::is_same_v<std::decay_t<R>, glm::vec3>)
			return ToJPHQuat(EulerDegToQuat(r));
		else
			return ToJPHQuat(r);
	}

	/*****************************************************************************/
	/*!
	\brief      Converts a Jolt quaternion back to the requested rotation type.
	\tparam     R   Either glm::vec3 (Euler degrees) or glm::quat.
	\param      q       Jolt quaternion.
	\param      out     Output rotation in the requested type.
	*/
	/*****************************************************************************/
	template<typename R>
	static inline void FromJPHRotation(JPH::Quat const &q, R &out)
	{
		if constexpr (std::is_same_v<std::decay_t<R>, glm::vec3>)
			out = QuatToEulerDeg(ToGLM(q));
		else
			out = ToGLM(q);
	}

	/*****************************************************************************/
	/*!
	\brief      Returns a glm::quat from either Euler degrees or an existing quat.
	\tparam     R   Either glm::vec3 (Euler degrees) or glm::quat.
	\param      r   Input rotation.
	\return     glm::quat representation.
	*/
	/*****************************************************************************/
	template<typename R>
	static inline glm::quat AsQuat(R const &r)
	{
		if constexpr (std::is_same_v<std::decay_t<R>, glm::vec3>)
			return EulerDegToQuat(r);
		else
			return r;
	}

	/*****************************************************************************/
	/*!
	\brief      Float comparison with tolerance.
	\param      a       First value.
	\param      b       Second value.
	\param      eps     Tolerance.
	\return     True if |a - b| <= eps.
	*/
	/*****************************************************************************/
	static inline bool NearlyEqual(float a, float b, float eps = 1e-5f)
	{
		return std::fabs(a - b) <= eps;
	}

	/*****************************************************************************/
	/*!
	\brief      3D vector comparison with component-wise tolerance.
	\param      a       First vector.
	\param      b       Second vector.
	\param      eps     Tolerance per component.
	\return     True if all components are within tolerance.
	*/
	/*****************************************************************************/
	static inline bool NearlyEqualVec3(glm::vec3 const &a, glm::vec3 const &b, float eps = 1e-4f)
	{
		return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps) && NearlyEqual(a.z, b.z, eps);
	}

	/*****************************************************************************/
	/*!
	\brief      Initializes Jolt world components and mirrors existing ECS bodies.
	\param      scene   Scene pointer used to enumerate entities and components.
	*/
	/*****************************************************************************/
	void PhysicsSystem::OnInit(Scene *scene)
	{
		(void)scene;

		JPH::RegisterDefaultAllocator();
		if (JPH::Factory::sInstance == nullptr) JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		JPH::Trace = [](const char *fmt, ...)
			{
				char buf[1024];
				va_list args;
				va_start(args, fmt);
				vsnprintf(buf, sizeof(buf), fmt, args);
				va_end(args);
				std::fputs(buf, stderr);
			};
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = [](char const *, char const *, char const *, JPH::uint)
			{
				return false;
			};)

#ifdef _DEBUG
			mTempAllocator = new JPH::TempAllocatorMalloc();
#else
			mTempAllocator = new JPH::TempAllocatorImpl(64u * 1024u * 1024u);
#endif

		unsigned const hw = std::max(1u, std::thread::hardware_concurrency());
		mJobSystem = new JPH::JobSystemThreadPool(2048, 8, int(hw > 1u ? hw - 1u : 1u));

		uint32_t const cMaxBodies = 8192u;
		uint32_t const cNumBodyMutexes = 0u;
		uint32_t const cMaxBodyPairs = 32768u;
		uint32_t const cMaxContactConstraints = 16384u;

		mPhysics.Init(
			cMaxBodies,
			cNumBodyMutexes,
			cMaxBodyPairs,
			cMaxContactConstraints,
			mBPLayers,
			mObjVsBPLayerFilter,
			mObjPairFilter
		);

		// Engine-level gravity configured here (per-body gravity factor handled per rigidbody)
		mPhysics.SetGravity(JPH::Vec3(0.0f, 0.0f, 0.0f));
		mBodyInterface = &mPhysics.GetBodyInterface();

		BuildOrRefreshBodies(scene);
		Engine::PhysicsAPI::EnableCollisionEvents();
	}

	/*****************************************************************************/
	/*!
	\brief      Shuts down Jolt world and releases owned resources.
	\param      scene   Unused; present for signature symmetry.
	*/
	/*****************************************************************************/
	void PhysicsSystem::OnShutdown(Scene * /*scene*/)
	{
		for (auto const &kv : mBodyOf)
		{
			JPH::BodyID const id = kv.second;
			mBodyInterface->RemoveBody(id);
			mBodyInterface->DestroyBody(id);
		}
		mBodyOf.clear();

		mShapeCache.clear();
		mApplied.clear();

		delete mJobSystem;     mJobSystem = nullptr;
		delete mTempAllocator; mTempAllocator = nullptr;

		JPH::UnregisterTypes();
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	/*****************************************************************************/
	/*!
	\brief      Simulation step: sync ECS -> physics (kinematic/props), simulate,
				then sync physics -> ECS (dynamic pose/velocity). Also manages
				collision frame boundaries.
	\param      scene   Scene pointer.
	\param      dt      Time step.
	*/
	/*****************************************************************************/
	void PhysicsSystem::OnUpdate(Scene *scene, Timestep dt)
	{
		if (!IsEnabled()) return;

		BuildOrRefreshBodies(scene);

		auto &reg = scene->GetRegistry();

		// ---------------------------------------------------------------------
		// ECS -> Jolt: sync kinematics, mass/shape changes, damping/rest,
		// poses and velocities.
		// ---------------------------------------------------------------------
		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &tc, RigidbodyComponent &rb)
			{
				auto it = mBodyOf.find(e);
				if (it == mBodyOf.end()) return;
				JPH::BodyID const id = it->second;

				AppliedProps &ap = mApplied[e];

				// Collider type change -> full rebuild
				std::uint8_t const currentShapeKind = static_cast<std::uint8_t>(rb.Shape);
				if (currentShapeKind != ap.shapeKind)
				{
					DestroyBodyFor(e);
					CreateBodyFor(scene, e);
					return;
				}

				// Kinematic flag -> motion type & layer
				if (rb.IsKinematic != ap.isKinematic)
				{
					mBodyInterface->SetMotionType(id, ToMotionType(rb), JPH::EActivation::Activate);
					mBodyInterface->SetObjectLayer(id, ToObjectLayer(rb));
					ap.isKinematic = rb.IsKinematic;
				}

				// Gravity toggle -> gravity factor
				if (rb.UseGravity != ap.useGravity)
				{
					JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
					if (lock.Succeeded())
					{
						lock.GetBody().GetMotionProperties()->SetGravityFactor(rb.UseGravity ? 1.0f : 0.0f);
					}
					ap.useGravity = rb.UseGravity;
				}

				// Mass change -> rebuild body (keeps inertia in sync)
				if (!NearlyEqual(rb.Mass, ap.mass))
				{
					DestroyBodyFor(e);
					CreateBodyFor(scene, e);
					return;
				}

				// Mesh / shape changes for mesh-backed colliders
				if (mFetchMeshInfo &&
					(rb.Shape == ColliderType::AABB ||
						rb.Shape == ColliderType::SPHERE ||
						rb.Shape == ColliderType::MESH))
				{
					MeshBuildInfo info;
					if (mFetchMeshInfo(scene, e, info))
					{
						std::uint8_t ds = info.doubleSided ? 1u : 0u;
						bool         scaleDiff = !NearlyEqualVec3(info.scale, ap.shapeScale);

						if (info.key != ap.meshKey ||
							ds != ap.shapeDS ||
							scaleDiff)
						{
							DestroyBodyFor(e);
							CreateBodyFor(scene, e);
							return;
						}
					}
				}

				// Damping + restitution + trigger bindings
				{
					JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
					if (lock.Succeeded())
					{
						JPH::Body &body = lock.GetBody();

						if (JPH::MotionProperties *mp = body.GetMotionProperties())
						{
							if (!NearlyEqual(mp->GetLinearDamping(), rb.LinearDamping))
								mp->SetLinearDamping(rb.LinearDamping);

							if (!NearlyEqual(mp->GetAngularDamping(), rb.AngularDamping))
								mp->SetAngularDamping(rb.AngularDamping);
						}

						float const targetRest = std::clamp(rb.Restitution, 0.0f, 1.0f);
						if (!NearlyEqual(body.GetRestitution(), targetRest))
							body.SetRestitution(targetRest);

						// Trigger flag -> Jolt sensor state (no collision response, still contacts)
						bool const isSensor = body.IsSensor();
						if (rb.IsTrigger != isSensor)
						{
							body.SetIsSensor(rb.IsTrigger);
						}
					}
				}

				// Pose push for kinematic bodies (or if transform changed)
				/*glm::vec3 curPos = tc.Position;
				glm::quat curRot = AsQuat(tc.Rotation);
				float     dotq = std::abs(glm::dot(curRot, ap.lastRot));
				bool      rotChanged = (1.0f - dotq) > 1e-4f;
				bool      posChanged = !NearlyEqualVec3(curPos, ap.lastPos);*/
				glm::vec3 worldPos = glm::vec3(tc.WorldTransform[3]);  // FIXED: Get from WorldTransform
				glm::mat3 worldRotMat = glm::mat3(tc.WorldTransform);
				glm::quat worldRot = glm::quat_cast(worldRotMat);      // FIXED: World rotation

				float dotq = std::abs(glm::dot(worldRot, ap.lastRot));
				bool rotChanged = (1.0f - dotq) > 1e-4f;
				bool posChanged = !NearlyEqualVec3(worldPos, ap.lastPos);

				if (posChanged || rotChanged)
				{
					mBodyInterface->SetPositionAndRotation(
						id,
						ToJPHRVec3(worldPos),
						ToJPHQuat(worldRot),
						JPH::EActivation::Activate
					);
					ap.lastPos = worldPos;
					ap.lastRot = worldRot;
				}

				// Velocity push for dynamics
				if (!rb.IsKinematic)
				{
					mBodyInterface->SetLinearVelocity(id, ToJPHVec3(rb.Velocity));
					mBodyInterface->SetAngularVelocity(id, ToJPHVec3(rb.AngularVelocity));
				}
			}
		);

		// ---------------------------------------------------------------------
		// Step the physics world
		// ---------------------------------------------------------------------
		PhysicsAPI::BeginCollisionFrame();
		mPhysics.Update(dt.GetSeconds(), 1, mTempAllocator, mJobSystem);

		// ---------------------------------------------------------------------
		// Jolt -> ECS: sync back dynamic poses and velocities
		// ---------------------------------------------------------------------
		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &tc, RigidbodyComponent &rb)
			{
				auto it = mBodyOf.find(e);
				if (it == mBodyOf.end()) return;
				JPH::BodyID const id = it->second;

				JPH::RVec3 p{}; JPH::Quat q{};
				mBodyInterface->GetPositionAndRotation(id, p, q);

				/*tc.Position = glm::vec3(
					static_cast<float>(p.GetX()),
					static_cast<float>(p.GetY()),
					static_cast<float>(p.GetZ())
				);
				FromJPHRotation(q, tc.Rotation);
				tc.IsDirty = true;

				AppliedProps &ap = mApplied[e];
				ap.lastPos = tc.Position;
				ap.lastRot = AsQuat(tc.Rotation);*/
				glm::vec3 physicsWorldPos = glm::vec3(
					static_cast<float>(p.GetX()),
					static_cast<float>(p.GetY()),
					static_cast<float>(p.GetZ())
				);
				glm::quat physicsWorldRot = ToGLM(q);

				if (tc.Parent == u32_max) {
					// Root entity: world = local
					tc.Position = physicsWorldPos;
					FromJPHRotation(q, tc.Rotation);
				}
				else {
					// Child entity: convert world to local
					// Get parent's world transform
					auto& parent_tc = reg.get<TransformComponent>(static_cast<entt::entity>(tc.Parent));

					// Convert world position to local: local = parent_inverse × world
					glm::mat4 parent_inverse = glm::inverse(parent_tc.WorldTransform);
					glm::vec4 localPos = parent_inverse * glm::vec4(physicsWorldPos, 1.0f);
					tc.Position = glm::vec3(localPos);

					// Convert world rotation to local
					glm::mat3 parentWorldRotMat = glm::mat3(parent_tc.WorldTransform);
					glm::quat parentWorldRot = glm::quat_cast(parentWorldRotMat);
					tc.Rotation = glm::inverse(parentWorldRot) * physicsWorldRot;
				}

				tc.IsDirty = true;

				AppliedProps& ap = mApplied[e];
				ap.lastPos = physicsWorldPos;  // Store world position
				ap.lastRot = physicsWorldRot;  // Store world rotation


				if (!rb.IsKinematic)
				{
					JPH::Vec3 v = mBodyInterface->GetLinearVelocity(id);
					rb.Velocity = glm::vec3(v.GetX(), v.GetY(), v.GetZ());

					JPH::Vec3 w = mBodyInterface->GetAngularVelocity(id);
					rb.AngularVelocity = glm::vec3(w.GetX(), w.GetY(), w.GetZ());
				}
			}
		);
	}

	/*****************************************************************************/
	/*!
	\brief      Mirrors ECS entities with RigidbodyComponent to Jolt bodies.
				Creates missing bodies and destroys bodies for removed entities.
	\param      scene   Scene pointer.
	*/
	/*****************************************************************************/
	void PhysicsSystem::BuildOrRefreshBodies(Scene *scene)
	{
		auto &reg = scene->GetRegistry();

		std::unordered_set<EntityID> seen;
		seen.reserve(mBodyOf.size() + 128u);

		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &, RigidbodyComponent &)
			{
				seen.insert(e);
				if (mBodyOf.find(e) == mBodyOf.end()) CreateBodyFor(scene, e);
			}
		);

		std::vector<EntityID> to_remove;
		to_remove.reserve(mBodyOf.size());
		for (auto const &kv : mBodyOf)
			if (seen.find(kv.first) == seen.end())
				to_remove.push_back(kv.first);

		for (EntityID e : to_remove)
		{
			DestroyBodyFor(e);
			mBodyOf.erase(e);
		}
	}

	/*****************************************************************************/
	/*!
	\brief      Produces a shape for an entity, using hooks and mesh cache when
				possible. Falls back to a unit box when no mesh data is available.
	\param      scene   Scene pointer.
	\param      e       Entity id.
	\param      tc      Transform component.
	\param      rb      Rigidbody component.
	\return     Ref-counted Jolt Shape to be used for body creation.
	*/
	/*****************************************************************************/
	JPH::Ref<JPH::Shape> PhysicsSystem::MakeShapeForEntity(
		Scene *scene,
		EntityID e,
		TransformComponent const &tc,
		RigidbodyComponent const &rb
	)
	{
		// 1) User-provided hook can override everything
		if (mMakeEntityShape)
		{
			if (auto s = mMakeEntityShape(scene, e, tc, rb))
				return s;
		}

		// 2) Try to fetch mesh info once
		MeshBuildInfo info{};
		bool hasMesh = false;
		if (mFetchMeshInfo)
		{
			if (mFetchMeshInfo(scene, e, info) && !info.vertices.empty())
				hasMesh = true;
		}

		switch (rb.Shape)
		{
		case ColliderType::AABB:
			if (hasMesh)
			{
				glm::vec3 minv(
					std::numeric_limits<float>::max(),
					std::numeric_limits<float>::max(),
					std::numeric_limits<float>::max());
				glm::vec3 maxv(
					std::numeric_limits<float>::lowest(),
					std::numeric_limits<float>::lowest(),
					std::numeric_limits<float>::lowest());

				for (auto const &v : info.vertices)
				{
					minv.x = std::min(minv.x, v.x);
					minv.y = std::min(minv.y, v.y);
					minv.z = std::min(minv.z, v.z);

					maxv.x = std::max(maxv.x, v.x);
					maxv.y = std::max(maxv.y, v.y);
					maxv.z = std::max(maxv.z, v.z);
				}

				glm::vec3 he = (maxv - minv) * 0.5f;
				if (he.x <= 0.0f || he.y <= 0.0f || he.z <= 0.0f)
					he = glm::vec3(DEFAULT_HALF_EXT);

				return JPH::Ref<JPH::Shape>(
					new JPH::BoxShape(JPH::Vec3(he.x, he.y, he.z)));
			}
			// No mesh? fall through to BOX behaviour using BoxHalfExtents.
			[[fallthrough]];

		case ColliderType::BOX:
		{
			glm::vec3 he = rb.BoxHalfExtents;
			if (he.x <= 0.0f || he.y <= 0.0f || he.z <= 0.0f)
				he = glm::vec3(DEFAULT_HALF_EXT);

			return JPH::Ref<JPH::Shape>(
				new JPH::BoxShape(JPH::Vec3(he.x, he.y, he.z)));
		}

		case ColliderType::SPHERE:
		{
			float radius = rb.SphereRadius;

			if (radius <= 0.0f && hasMesh)
			{
				// auto-fit from mesh if designer didn't set a positive radius
				float maxLen2 = 0.0f;
				for (auto const &v : info.vertices)
				{
					float len2 = v.x * v.x + v.y * v.y + v.z * v.z;
					if (len2 > maxLen2) maxLen2 = len2;
				}
				radius = maxLen2 > 0.0f ? std::sqrt(maxLen2) : DEFAULT_HALF_EXT;
			}
			else if (radius <= 0.0f)
			{
				radius = DEFAULT_HALF_EXT;
			}

			return JPH::Ref<JPH::Shape>(new JPH::SphereShape(radius));
		}

		case ColliderType::MESH:
			if (hasMesh && info.indices.size() >= 3)
			{
				// Cache key uses collider type, mesh key, and double-sided flag
				std::uint8_t kind = static_cast<std::uint8_t>(rb.Shape);
				std::uint8_t ds = info.doubleSided ? 1u : 0u;
				CacheKey     key{ info.key, kind, ds };

				if (auto it = mShapeCache.find(key); it != mShapeCache.end())
				{
					JPH::Ref<JPH::Shape> base = it->second;
					if (info.scale != glm::vec3(1.0f))
						return JPH::Ref<JPH::Shape>(
							new JPH::ScaledShape(base, ToJPHVec3(info.scale)));
					return base;
				}

				JPH::Array<JPH::Float3> verts;
				verts.resize(info.vertices.size());
				for (size_t i = 0; i < info.vertices.size(); ++i)
				{
					auto const &v = info.vertices[i];
					verts[i] = JPH::Float3(v.x, v.y, v.z);
				}

				JPH::Array<JPH::IndexedTriangle> tris;
				tris.reserve(info.indices.size() / 3);
				for (size_t i = 0; i + 2 < info.indices.size(); i += 3)
				{
					tris.push_back(JPH::IndexedTriangle(
						(JPH::uint32)info.indices[i + 0],
						(JPH::uint32)info.indices[i + 1],
						(JPH::uint32)info.indices[i + 2]));
				}

				JPH::MeshShapeSettings mss(verts, tris);
				auto                   res = mss.Create();
				if (!res.HasError())
				{
					JPH::Ref<JPH::Shape> base = res.Get();
					mShapeCache.emplace(key, base);

					if (info.scale != glm::vec3(1.0f))
						return JPH::Ref<JPH::Shape>(
							new JPH::ScaledShape(base, ToJPHVec3(info.scale)));
					return base;
				}
			}
			break;
		}

		// 3) Fallback: simple unit box if everything else fails
		return JPH::Ref<JPH::Shape>(
			new JPH::BoxShape(JPH::Vec3::sReplicate(DEFAULT_HALF_EXT)));
	}

	/*****************************************************************************/
	/*!
	\brief      Creates and registers a Jolt body for the given entity.
	\param      scene   Scene pointer.
	\param      e       Entity id.
	*/
	/*****************************************************************************/
	void PhysicsSystem::CreateBodyFor(Scene *scene, EntityID e)
	{
		auto &reg = scene->GetRegistry();
		auto &tc = reg.get<TransformComponent>(e);
		auto &rb = reg.get<RigidbodyComponent>(e);

		JPH::Ref<JPH::Shape> shape = MakeShapeForEntity(scene, e, tc, rb);
		glm::vec3 worldPos = glm::vec3(tc.WorldTransform[3]);
		glm::mat3 worldRotMat = glm::mat3(tc.WorldTransform);
		glm::quat worldRot = glm::quat_cast(worldRotMat);
		JPH::BodyCreationSettings settings(
			shape,
			ToJPHRVec3(worldPos),
			ToJPHRotation(worldRot),
			ToMotionType(rb),
			ToObjectLayer(rb)
		);

		settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		settings.mMassPropertiesOverride.mMass = std::max(0.0001f, rb.Mass);
		settings.mFriction = 0.6f;
		settings.mRestitution = std::clamp(rb.Restitution, 0.0f, 1.0f);
		settings.mLinearDamping = rb.LinearDamping;
		settings.mAngularDamping = rb.AngularDamping;

		settings.mIsSensor = rb.IsTrigger;

		settings.mUserData = static_cast<JPH::uint64>(static_cast<std::uint32_t>(e));

		JPH::BodyID const id =
			mBodyInterface->CreateAndAddBody(settings, JPH::EActivation::Activate);

		if (!rb.IsKinematic)
		{
			mBodyInterface->SetLinearVelocity(id, ToJPHVec3(rb.Velocity));
			mBodyInterface->SetAngularVelocity(id, ToJPHVec3(rb.AngularVelocity));
		}

		if (!rb.UseGravity)
		{
			JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
			if (lock.Succeeded())
			{
				lock.GetBody().GetMotionProperties()->SetGravityFactor(0.0f);
			}
		}

		mBodyOf.emplace(e, id);

		AppliedProps ap{};
		ap.isKinematic = rb.IsKinematic;
		ap.useGravity = rb.UseGravity;
		ap.mass = rb.Mass;
		ap.lastPos = tc.Position;
		ap.lastRot = AsQuat(tc.Rotation);
		ap.shapeKind = static_cast<std::uint8_t>(rb.Shape);

		if (mFetchMeshInfo)
		{
			MeshBuildInfo info;
			if (mFetchMeshInfo(scene, e, info))
			{
				ap.meshKey = info.key;
				ap.shapeDS = info.doubleSided ? 1u : 0u;
				ap.shapeScale = info.scale;
			}
		}

		mApplied[e] = ap;
	}

	/*****************************************************************************/
	/*!
	\brief      Destroys the Jolt body and removes cached state for the entity.
	\param      e   Entity id.
	*/
	/*****************************************************************************/
	void PhysicsSystem::DestroyBodyFor(EntityID e)
	{
		auto it = mBodyOf.find(e);
		if (it == mBodyOf.end()) return;

		JPH::BodyID const id = it->second;
		mBodyInterface->RemoveBody(id);
		mBodyInterface->DestroyBody(id);
		mBodyOf.erase(it);
		mApplied.erase(e);
	}
}

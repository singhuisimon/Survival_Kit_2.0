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
			- Internal accumulator-based fixed timestep stepping

			Units: meters, kilograms, seconds.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include <algorithm>
#include <cassert>
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

// Runtime mesh extraction for physics colliders
#include "Asset/ResourceData.h"
#include "Asset/ResourceManager.h"
#include "Asset/ResourceTypes.h"

namespace Engine
{
	static constexpr float DEFAULT_HALF_EXT = 0.5f;
	static constexpr float WORLD_SCALE_EPS = 1e-6f;

	static constexpr double DEFAULT_FIXED_STEP_SECONDS = 1.0 / 60.0;
	static constexpr float  MAX_FRAME_DT_SECONDS = 0.25f;
	static constexpr double ACCUMULATOR_EPS = 1e-9;
	static constexpr std::uint32_t DEFAULT_MAX_CATCH_UP_STEPS = 8u;
	static constexpr int   DEFAULT_COLLISION_STEPS = 2;
	static constexpr float SMALL_BODY_CCD_EXTENT = 0.25f;
	static constexpr float FAST_BODY_CCD_SPEED = 12.0f;

	/*****************************************************************************/
	/*!
	\brief      Extracts world-space scale from a 4x4 transform matrix.
	\param      m   World transform matrix.
	\return     Per-axis scale (absolute), with near-zero axes clamped to 1.
	*/
	/*****************************************************************************/
	static inline glm::vec3 ExtractWorldScale(glm::mat4 const &m)
	{
		glm::vec3 sx = glm::vec3(m[0]);
		glm::vec3 sy = glm::vec3(m[1]);
		glm::vec3 sz = glm::vec3(m[2]);
		float lx = glm::length(sx);
		float ly = glm::length(sy);
		float lz = glm::length(sz);
		if (lx < WORLD_SCALE_EPS) lx = 1.0f;
		if (ly < WORLD_SCALE_EPS) ly = 1.0f;
		if (lz < WORLD_SCALE_EPS) lz = 1.0f;
		return glm::vec3(std::fabs(lx), std::fabs(ly), std::fabs(lz));
	}

	/*****************************************************************************/
	/*!
	\brief      Extracts a pure rotation quaternion from a world transform matrix.
	\details    Normalizes the basis vectors to remove scale before casting.
	\param      m   World transform matrix.
	\return     Rotation quaternion.
	*/
	/*****************************************************************************/
	static inline glm::quat ExtractWorldRotation(glm::mat4 const &m)
	{
		glm::vec3 x = glm::vec3(m[0]);
		glm::vec3 y = glm::vec3(m[1]);
		glm::vec3 z = glm::vec3(m[2]);

		float lx = glm::length(x);
		float ly = glm::length(y);
		float lz = glm::length(z);
		if (lx >= WORLD_SCALE_EPS) x /= lx;
		if (ly >= WORLD_SCALE_EPS) y /= ly;
		if (lz >= WORLD_SCALE_EPS) z /= lz;

		glm::mat3 r(1.0f);
		r[0] = x;
		r[1] = y;
		r[2] = z;
		return glm::quat_cast(r);
	}

	/*****************************************************************************/
	/*!
	\brief      Extracts world translation from a 4x4 transform matrix.
	\param      m   World transform matrix.
	\return     Translation component.
	*/
	/*****************************************************************************/
	static inline glm::vec3 ExtractWorldPosition(glm::mat4 const &m)
	{
		return glm::vec3(m[3]);
	}

	/*****************************************************************************/
	/*!
	\brief      Builds a stable cache key for a mesh instance + submesh.
	\param      meshGuid     xresource instance GUID.
	\param      submeshIndex Optional submesh index.
	\return     64-bit key suitable for PhysicsBridge::CacheKey.
	*/
	/*****************************************************************************/
	static inline std::uint64_t MakeMeshKey(xresource::instance_guid const &meshGuid, std::uint32_t submeshIndex)
	{
		std::uint64_t h = static_cast<std::uint64_t>(meshGuid.m_Value);
		h ^= (static_cast<std::uint64_t>(submeshIndex) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
		return h;
	}

	/*****************************************************************************/
	/*!
	\brief      Populates MeshBuildInfo from engine resources for an entity.
	\details    When needGeometry is false, only key/scale/doubleSided are filled.
	\param      scene        Scene pointer.
	\param      e            Entity id.
	\param      out          Output mesh build info.
	\param      needGeometry True to also populate vertices/indices.
	\return     True if mesh reference (and geometry, if requested) was found.
	*/
	/*****************************************************************************/
	static bool TryBuildMeshInfoFromEntity(Scene *scene, EntityID e, MeshBuildInfo &out, bool needGeometry)
	{
		assert(scene != nullptr);

		auto &reg = scene->GetRegistry();
		if (!reg.valid(e))
			return false;

		if (!reg.all_of<MeshRendererComponent, TransformComponent>(e))
			return false;

		auto const &mrc = reg.get<MeshRendererComponent>(e);
		auto const &tc = reg.get<TransformComponent>(e);

		if (mrc.MeshGuid.m_Value == 0)
			return false;

		out.scale = ExtractWorldScale(tc.WorldTransform);
		out.doubleSided = (mrc.CastType == ShadowCastType::TwoSided);
		out.key = MakeMeshKey(mrc.MeshGuid, mrc.SubmeshIndex);

		if (!needGeometry)
			return true;

		xresource::full_guid meshFull{ mrc.MeshGuid, Engine::ResourceGUID::mesh_type_guid_v };
		MeshResource *mesh = RM.loadResource<MeshResource>(meshFull);
		if (mesh == nullptr)
			return false;

		constexpr std::size_t STRIDE = 11; // pos3 + nrm3 + col3 + uv2
		if (mesh->vertices.size() < STRIDE || (mesh->vertices.size() % STRIDE) != 0)
		{
			assert(false && "Mesh vertex data has invalid stride");
			xresource::full_guid tmp = meshFull;
			RM.releaseResource<MeshResource>(tmp);
			return false;
		}

		std::size_t vcount = mesh->vertices.size() / STRIDE;
		out.vertices.clear();
		out.vertices.reserve(vcount);

		for (std::size_t i = 0; i < vcount; ++i)
		{
			std::size_t off = i * STRIDE;
			out.vertices.emplace_back(
				mesh->vertices[off + 0],
				mesh->vertices[off + 1],
				mesh->vertices[off + 2]
			);
		}

		out.indices.clear();
		if (!mesh->subMeshes.empty() && mrc.SubmeshIndex < mesh->subMeshes.size())
		{
			SubMeshDescriptor const &sm = mesh->subMeshes[mrc.SubmeshIndex];
			std::uint32_t start = sm.startIndex;
			std::uint32_t count = sm.indexCount;

			if (count >= 3 &&
				start < mesh->indices.size() &&
				(static_cast<std::size_t>(start) + count) <= mesh->indices.size())
			{
				out.indices.assign(mesh->indices.begin() + start, mesh->indices.begin() + start + count);
			}
			else
			{
				assert(false && "Submesh descriptor invalid, falling back to full index buffer");
				out.indices = mesh->indices;
			}
		}
		else
		{
			out.indices = mesh->indices;
		}

		for (std::uint32_t idx : out.indices)
		{
			if (idx >= out.vertices.size())
			{
				assert(false && "Mesh index out of range");
				xresource::full_guid tmp = meshFull;
				RM.releaseResource<MeshResource>(tmp);
				return false;
			}
		}

		xresource::full_guid tmp = meshFull;
		RM.releaseResource<MeshResource>(tmp);
		return !out.vertices.empty() && out.indices.size() >= 3;
	}

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
		return NearlyEqual(a.x, b.x, eps) &&
			NearlyEqual(a.y, b.y, eps) &&
			NearlyEqual(a.z, b.z, eps);
	}

	/*****************************************************************************/
	/*!
	\brief      Computes the smallest authored extent of a body for CCD heuristics.
	\param      rb   Rigidbody component.
	\return     Smallest positive extent/radius, or negative if unavailable.
	*/
	/*****************************************************************************/
	static inline float ComputeSmallestBodyExtent(RigidbodyComponent const &rb)
	{
		switch (rb.Shape)
		{
		case ColliderType::BOX:
		case ColliderType::AABB:
		{
			glm::vec3 he = rb.BoxHalfExtents;
			if (he.x > 0.0f && he.y > 0.0f && he.z > 0.0f)
				return std::min(he.x, std::min(he.y, he.z));
			break;
		}

		case ColliderType::SPHERE:
			if (rb.SphereRadius > 0.0f)
				return rb.SphereRadius;
			break;

		default:
			break;
		}

		return -1.0f;
	}

	/*****************************************************************************/
	/*!
	\brief      Chooses motion quality for a body using size/speed heuristics.
	\details    Small, fast dynamic bodies use LinearCast to reduce tunneling.
	\param      rb   Rigidbody component.
	\return     Jolt motion quality for the body.
	*/
	/*****************************************************************************/
	static inline JPH::EMotionQuality ChooseMotionQuality(RigidbodyComponent const &rb)
	{
		// Jolt sensors / triggers are discrete-only.
		if (rb.Mass <= 0.0f || rb.IsKinematic || rb.IsTrigger)
			return JPH::EMotionQuality::Discrete;

		float const smallestExtent = ComputeSmallestBodyExtent(rb);
		float const linearSpeed = glm::length(rb.Velocity);

		bool const smallBody = (smallestExtent > 0.0f && smallestExtent <= SMALL_BODY_CCD_EXTENT);
		bool const fastBody = (linearSpeed >= FAST_BODY_CCD_SPEED);
		bool const canCrossOwnThicknessInStep =
			(smallestExtent > 0.0f) &&
			(linearSpeed * static_cast<float>(DEFAULT_FIXED_STEP_SECONDS) >= (smallestExtent * 2.0f));

		return (smallBody || fastBody || canCrossOwnThicknessInStep)
			? JPH::EMotionQuality::LinearCast
			: JPH::EMotionQuality::Discrete;
	}

	/*****************************************************************************/
	/*!
	\brief      Initializes Jolt world components and mirrors existing ECS bodies.
	\param      scene   Scene pointer used to enumerate entities and components.
	*/
	/*****************************************************************************/
	void PhysicsSystem::OnInit(Scene *scene)
	{
		assert(scene != nullptr);
		(void)scene;

		mAccumulatorSeconds = 0.0;
		mFixedStepSeconds = DEFAULT_FIXED_STEP_SECONDS;
		mMaxCatchUpSteps = DEFAULT_MAX_CATCH_UP_STEPS;

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

		JPH_IF_ENABLE_ASSERTS(
			JPH::AssertFailed = [](char const *, char const *, char const *, JPH::uint)
			{
				return false;
			};
		)

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

		mAccumulatorSeconds = 0.0;
		mFixedStepSeconds = DEFAULT_FIXED_STEP_SECONDS;
		mMaxCatchUpSteps = DEFAULT_MAX_CATCH_UP_STEPS;
	}

	/*****************************************************************************/
	/*!
	\brief      Simulation step: sync ECS -> physics, advance using a fixed
				internal timestep accumulator, then sync physics -> ECS.
	\param      scene   Scene pointer.
	\param      dt      Variable frame time step.
	*/
	/*****************************************************************************/
	void PhysicsSystem::OnUpdate(Scene *scene, Timestep dt)
	{
		assert(scene != nullptr);
		if (!IsEnabled()) return;

		BuildOrRefreshBodies(scene);
		SyncECSBodiesToPhysics(scene);

		float frameDt = dt.GetSeconds();
		if (frameDt < 0.0f)
			frameDt = 0.0f;
		if (frameDt > MAX_FRAME_DT_SECONDS)
			frameDt = MAX_FRAME_DT_SECONDS;

		mAccumulatorSeconds += static_cast<double>(frameDt);

		// High-FPS fix:
		// Do NOT clear the collision frame unless we are actually going to step
		// physics this update. Otherwise, render frames that take 0 fixed steps
		// will wipe all contact data and scripts will observe "no collision".
		if ((mAccumulatorSeconds + ACCUMULATOR_EPS) < mFixedStepSeconds)
			return;

		// Clear once per actual physics tick batch, then accumulate/dedupe contacts
		// across all fixed substeps performed in this update.
		PhysicsAPI::BeginCollisionFrame();

		std::uint32_t stepsTaken = 0u;
		while ((mAccumulatorSeconds + ACCUMULATOR_EPS) >= mFixedStepSeconds &&
			stepsTaken < mMaxCatchUpSteps)
		{
			mPhysics.Update(
				static_cast<float>(mFixedStepSeconds),
				DEFAULT_COLLISION_STEPS,
				mTempAllocator,
				mJobSystem
			);

			mAccumulatorSeconds -= mFixedStepSeconds;
			++stepsTaken;
		}

		// Prevent unbounded backlog growth under severe hitching.
		if (stepsTaken == mMaxCatchUpSteps && mAccumulatorSeconds > mFixedStepSeconds)
			mAccumulatorSeconds = mFixedStepSeconds;

		if (mAccumulatorSeconds < 0.0)
			mAccumulatorSeconds = 0.0;

		// We know at least one fixed step ran, so pull physics state back to ECS.
		SyncPhysicsBodiesToECS(scene);
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
		assert(scene != nullptr);

		auto &reg = scene->GetRegistry();

		std::unordered_set<EntityID> seen;
		seen.reserve(mBodyOf.size() + 128u);

		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &, RigidbodyComponent &)
			{
				seen.insert(e);
				if (mBodyOf.find(e) == mBodyOf.end())
					CreateBodyFor(scene, e);
			}
		);

		std::vector<EntityID> to_remove;
		to_remove.reserve(mBodyOf.size());
		for (auto const &kv : mBodyOf)
		{
			if (seen.find(kv.first) == seen.end())
				to_remove.push_back(kv.first);
		}

		for (EntityID e : to_remove)
		{
			DestroyBodyFor(e);
		}
	}

	/*****************************************************************************/
	/*!
	\brief      Pushes ECS-authored transform/property/velocity changes into
				Jolt bodies before fixed-step simulation.
	\param      scene   Scene pointer.
	*/
	/*****************************************************************************/
	void PhysicsSystem::SyncECSBodiesToPhysics(Scene *scene)
	{
		assert(scene != nullptr);

		auto &reg = scene->GetRegistry();

		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &tc, RigidbodyComponent &rb)
			{
				auto it = mBodyOf.find(e);
				if (it == mBodyOf.end()) return;
				JPH::BodyID const id = it->second;

				AppliedProps &ap = mApplied[e];

				std::uint8_t const currentShapeKind = static_cast<std::uint8_t>(rb.Shape);
				if (currentShapeKind != ap.shapeKind)
				{
					DestroyBodyFor(e);
					CreateBodyFor(scene, e);
					return;
				}

				bool const immovable = (rb.Mass <= 0.0f);
				if (immovable)
				{
					mBodyInterface->SetMotionType(id, JPH::EMotionType::Static, JPH::EActivation::DontActivate);
					mBodyInterface->SetObjectLayer(id, Layers::NON_MOVING);
					ap.isKinematic = rb.IsKinematic;
				}
				else
				{
					if (rb.IsKinematic != ap.isKinematic)
					{
						mBodyInterface->SetMotionType(id, ToMotionType(rb), JPH::EActivation::Activate);
						mBodyInterface->SetObjectLayer(id, ToObjectLayer(rb));
						ap.isKinematic = rb.IsKinematic;
					}
				}

				if (rb.UseGravity != ap.useGravity)
				{
					JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
					if (lock.Succeeded())
					{
						if (JPH::MotionProperties *mp = lock.GetBody().GetMotionProperties())
							mp->SetGravityFactor(rb.UseGravity ? 1.0f : 0.0f);
					}
					ap.useGravity = rb.UseGravity;
				}

				if (!NearlyEqual(rb.Mass, ap.mass))
				{
					DestroyBodyFor(e);
					CreateBodyFor(scene, e);
					return;
				}

				if (rb.Shape == ColliderType::AABB ||
					rb.Shape == ColliderType::SPHERE ||
					rb.Shape == ColliderType::MESH)
				{
					MeshBuildInfo info;
					bool ok = false;
					if (mFetchMeshInfo)
						ok = mFetchMeshInfo(scene, e, info);
					else
						ok = TryBuildMeshInfoFromEntity(scene, e, info, false);

					if (ok)
					{
						std::uint8_t ds = info.doubleSided ? 1u : 0u;
						bool scaleDiff = !NearlyEqualVec3(info.scale, ap.shapeScale);

						if (info.key != ap.meshKey || ds != ap.shapeDS || scaleDiff)
						{
							DestroyBodyFor(e);
							CreateBodyFor(scene, e);
							return;
						}
					}
				}

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

						bool const isSensor = body.IsSensor();
						if (rb.IsTrigger != isSensor)
							body.SetIsSensor(rb.IsTrigger);
					}
				}

				if (!immovable)
				{
					JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
					if (lock.Succeeded())
					{
						lock.GetBody().SetEnhancedInternalEdgeRemoval(true);
					}
				}

				if (!immovable)
				{
					JPH::EMotionQuality const targetMotionQuality = ChooseMotionQuality(rb);
					mBodyInterface->SetMotionQuality(id, targetMotionQuality);
				}

				glm::vec3 curPos = ExtractWorldPosition(tc.WorldTransform);
				glm::quat curRot = ExtractWorldRotation(tc.WorldTransform);
				float dotq = std::abs(glm::dot(curRot, ap.lastRot));
				bool rotChanged = (1.0f - dotq) > 1e-4f;
				bool posChanged = !NearlyEqualVec3(curPos, ap.lastPos);

				if (!immovable && (posChanged || rotChanged))
				{
					mBodyInterface->SetPositionAndRotation(
						id,
						ToJPHRVec3(curPos),
						ToJPHQuat(curRot),
						JPH::EActivation::Activate
					);
					ap.lastPos = curPos;
					ap.lastRot = curRot;
				}

				if (!immovable && !rb.IsKinematic)
				{
					mBodyInterface->SetLinearVelocity(id, ToJPHVec3(rb.Velocity));
					mBodyInterface->SetAngularVelocity(id, ToJPHVec3(rb.AngularVelocity));
				}
			}
		);
	}

	/*****************************************************************************/
	/*!
	\brief      Pulls latest transform and velocity state from Jolt into ECS
				after fixed-step simulation.
	\param      scene   Scene pointer.
	*/
	/*****************************************************************************/
	void PhysicsSystem::SyncPhysicsBodiesToECS(Scene *scene)
	{
		assert(scene != nullptr);

		auto &reg = scene->GetRegistry();

		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &tc, RigidbodyComponent &rb)
			{
				auto it = mBodyOf.find(e);
				if (it == mBodyOf.end()) return;
				JPH::BodyID const id = it->second;

				JPH::RVec3 p{};
				JPH::Quat q{};
				mBodyInterface->GetPositionAndRotation(id, p, q);

				tc.Position = glm::vec3(
					static_cast<float>(p.GetX()),
					static_cast<float>(p.GetY()),
					static_cast<float>(p.GetZ())
				);
				FromJPHRotation(q, tc.Rotation);
				tc.IsDirty = true;

				AppliedProps &ap = mApplied[e];
				ap.lastPos = glm::vec3(
					static_cast<float>(p.GetX()),
					static_cast<float>(p.GetY()),
					static_cast<float>(p.GetZ())
				);
				ap.lastRot = ToGLM(q);

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
		if (mMakeEntityShape)
		{
			if (auto s = mMakeEntityShape(scene, e, tc, rb))
				return s;
		}

		MeshBuildInfo info{};
		bool hasMesh = false;
		if (mFetchMeshInfo)
		{
			hasMesh = mFetchMeshInfo(scene, e, info) && !info.vertices.empty();
		}
		else
		{
			bool wantGeom = (rb.Shape == ColliderType::AABB ||
				rb.Shape == ColliderType::SPHERE ||
				rb.Shape == ColliderType::MESH);
			hasMesh = wantGeom && TryBuildMeshInfoFromEntity(scene, e, info, true);
		}

		bool const immovable = (rb.Mass <= 0.0f);
		ColliderType effectiveShape = rb.Shape;

		// Dynamic mesh colliders are poor for player/controller response.
		// Force movable MESH bodies onto BOX unless explicitly authored otherwise.
		if (!immovable && effectiveShape == ColliderType::MESH)
			effectiveShape = ColliderType::BOX;

		switch (effectiveShape)
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

				JPH::Ref<JPH::Shape> base(new JPH::BoxShape(JPH::Vec3(he.x, he.y, he.z)));
				if (!NearlyEqualVec3(info.scale, glm::vec3(1.0f)))
					return JPH::Ref<JPH::Shape>(new JPH::ScaledShape(base, ToJPHVec3(info.scale)));
				return base;
			}
			[[fallthrough]];

		case ColliderType::BOX:
		{
			glm::vec3 he = rb.BoxHalfExtents;
			if (he.x <= 0.0f || he.y <= 0.0f || he.z <= 0.0f)
			{
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

					he = (maxv - minv) * 0.5f;
					if (he.x <= 0.0f || he.y <= 0.0f || he.z <= 0.0f)
						he = glm::vec3(DEFAULT_HALF_EXT);
				}
				else
				{
					he = glm::vec3(DEFAULT_HALF_EXT);
				}
			}

			return JPH::Ref<JPH::Shape>(
				new JPH::BoxShape(JPH::Vec3(he.x, he.y, he.z)));
		}

		case ColliderType::SPHERE:
		{
			float radius = rb.SphereRadius;

			if (radius <= 0.0f && hasMesh)
			{
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
				std::uint8_t kind = static_cast<std::uint8_t>(effectiveShape);
				std::uint8_t ds = info.doubleSided ? 1u : 0u;
				CacheKey key{ info.key, kind, ds };

				if (auto it = mShapeCache.find(key); it != mShapeCache.end())
				{
					JPH::Ref<JPH::Shape> base = it->second;
					if (!NearlyEqualVec3(info.scale, glm::vec3(1.0f)))
						return JPH::Ref<JPH::Shape>(new JPH::ScaledShape(base, ToJPHVec3(info.scale)));
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
				tris.reserve(info.indices.size() / 3 * (info.doubleSided ? 2 : 1));
				for (size_t i = 0; i + 2 < info.indices.size(); i += 3)
				{
					JPH::uint32 i0 = (JPH::uint32)info.indices[i + 0];
					JPH::uint32 i1 = (JPH::uint32)info.indices[i + 1];
					JPH::uint32 i2 = (JPH::uint32)info.indices[i + 2];
					tris.push_back(JPH::IndexedTriangle(i0, i1, i2));
					if (info.doubleSided)
						tris.push_back(JPH::IndexedTriangle(i0, i2, i1));
				}

				JPH::MeshShapeSettings mss(verts, tris);
				auto res = mss.Create();
				if (!res.HasError())
				{
					JPH::Ref<JPH::Shape> base = res.Get();
					mShapeCache.emplace(key, base);

					if (!NearlyEqualVec3(info.scale, glm::vec3(1.0f)))
						return JPH::Ref<JPH::Shape>(new JPH::ScaledShape(base, ToJPHVec3(info.scale)));
					return base;
				}

				assert(false && "Failed to create mesh collider");
			}
			break;
		}

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
		assert(scene != nullptr);

		auto &reg = scene->GetRegistry();
		assert(reg.valid(e));
		assert((reg.all_of<TransformComponent, RigidbodyComponent>(e)));

		auto &tc = reg.get<TransformComponent>(e);
		auto &rb = reg.get<RigidbodyComponent>(e);

		bool const immovable = (rb.Mass <= 0.0f);

		JPH::Ref<JPH::Shape> shape = MakeShapeForEntity(scene, e, tc, rb);
		glm::vec3 worldPos = ExtractWorldPosition(tc.WorldTransform);
		glm::quat worldRot = ExtractWorldRotation(tc.WorldTransform);

		JPH::BodyCreationSettings settings(
			shape,
			ToJPHRVec3(worldPos),
			ToJPHQuat(worldRot),
			ToMotionType(rb),
			ToObjectLayer(rb)
		);

		if (!immovable && !rb.IsKinematic)
		{
			settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
			settings.mMassPropertiesOverride.mMass = std::max(0.0001f, rb.Mass);
		}

		settings.mFriction = 0.6f;
		settings.mMotionQuality = ChooseMotionQuality(rb);
		settings.mRestitution = std::clamp(rb.Restitution, 0.0f, 1.0f);
		settings.mLinearDamping = rb.LinearDamping;
		settings.mAngularDamping = rb.AngularDamping;
		settings.mIsSensor = rb.IsTrigger;
		settings.mUserData = static_cast<JPH::uint64>(static_cast<std::uint32_t>(e));

		JPH::BodyID const id =
			mBodyInterface->CreateAndAddBody(settings, JPH::EActivation::Activate);

		if (!immovable && !rb.IsKinematic)
		{
			mBodyInterface->SetLinearVelocity(id, ToJPHVec3(rb.Velocity));
			mBodyInterface->SetAngularVelocity(id, ToJPHVec3(rb.AngularVelocity));
		}

		{
			JPH::BodyLockWrite lock(mPhysics.GetBodyLockInterface(), id);
			if (lock.Succeeded())
			{
				JPH::Body &body = lock.GetBody();
				if (!immovable)
					body.SetEnhancedInternalEdgeRemoval(true);

				if (!rb.UseGravity)
				{
					if (JPH::MotionProperties *mp = body.GetMotionProperties())
						mp->SetGravityFactor(0.0f);
				}
			}
		}

		mBodyOf.emplace(e, id);

		AppliedProps ap{};
		ap.isKinematic = rb.IsKinematic;
		ap.useGravity = rb.UseGravity;
		ap.mass = rb.Mass;
		ap.lastPos = worldPos;
		ap.lastRot = worldRot;
		ap.shapeKind = static_cast<std::uint8_t>(rb.Shape);

		{
			MeshBuildInfo info;
			bool ok = false;
			if (mFetchMeshInfo)
				ok = mFetchMeshInfo(scene, e, info);
			else
				ok = TryBuildMeshInfoFromEntity(scene, e, info, false);

			if (ok)
			{
				ap.meshKey = info.key;
				ap.shapeDS = info.doubleSided ? 1u : 0u;
				ap.shapeScale = info.scale;
			}
			else
			{
				ap.meshKey = 0u;
				ap.shapeDS = 0u;
				ap.shapeScale = glm::vec3(1.0f);
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
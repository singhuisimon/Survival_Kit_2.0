#include <algorithm>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "PhysicsSystem.h"

namespace Engine
{
	static constexpr float DEFAULT_HALF_EXT = 0.5f;

	template<typename R>
	static inline JPH::Quat ToJPHRotation(R const &r)
	{
		if constexpr (std::is_same_v<std::decay_t<R>, glm::vec3>)
			return ToJPHQuat(EulerDegToQuat(r));
		else
			return ToJPHQuat(r);
	}

	template<typename R>
	static inline void FromJPHRotation(JPH::Quat const &q, R &out)
	{
		if constexpr (std::is_same_v<std::decay_t<R>, glm::vec3>)
			out = QuatToEulerDeg(ToGLM(q));
		else
			out = ToGLM(q);
	}

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
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = [](char const *, char const *, char const *, JPH::uint) { return false; };)

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

		mPhysics.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
		mBodyInterface = &mPhysics.GetBodyInterface();

		BuildOrRefreshBodies(scene);
	}

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

		delete mJobSystem;     mJobSystem = nullptr;
		delete mTempAllocator; mTempAllocator = nullptr;

		JPH::UnregisterTypes();
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	void PhysicsSystem::OnUpdate(Scene *scene, Timestep dt)
	{
		if (!IsEnabled()) return;

		BuildOrRefreshBodies(scene);

		auto &reg = scene->GetRegistry();

		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &tc, RigidbodyComponent &rb)
			{
				auto it = mBodyOf.find(e);
				if (it == mBodyOf.end()) return;
				JPH::BodyID const id = it->second;

				if (rb.IsKinematic)
				{
					mBodyInterface->SetPositionAndRotation(
						id,
						ToJPHRVec3(tc.Position),
						ToJPHRotation(tc.Rotation),
						JPH::EActivation::DontActivate
					);
				}
				else
				{
					mBodyInterface->SetLinearVelocity(id, ToJPHVec3(rb.Velocity));
				}
			}
		);

		mPhysics.Update(dt.GetSeconds(), 1, mTempAllocator, mJobSystem);

		reg.view<TransformComponent, RigidbodyComponent>().each(
			[&](EntityID e, TransformComponent &tc, RigidbodyComponent &rb)
			{
				auto it = mBodyOf.find(e);
				if (it == mBodyOf.end()) return;
				JPH::BodyID const id = it->second;

				JPH::RVec3 p{}; JPH::Quat q{};
				mBodyInterface->GetPositionAndRotation(id, p, q);

				tc.Position = glm::vec3(
					static_cast<float>(p.GetX()),
					static_cast<float>(p.GetY()),
					static_cast<float>(p.GetZ())
				);
				FromJPHRotation(q, tc.Rotation);

				if (!rb.IsKinematic)
				{
					JPH::Vec3 v = mBodyInterface->GetLinearVelocity(id);
					rb.Velocity = glm::vec3(v.GetX(), v.GetY(), v.GetZ());
				}
			}
		);
	}

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

	JPH::Ref<JPH::Shape> PhysicsSystem::MakeShapeForEntity(Scene *scene, EntityID e, TransformComponent const &tc, RigidbodyComponent const &rb)
	{
		if (mMakeEntityShape)
		{
			if (auto s = mMakeEntityShape(scene, e, tc, rb)) return s;
		}

		if (mFetchMeshInfo)
		{
			MeshBuildInfo info;
			if (mFetchMeshInfo(scene, e, info) && !info.vertices.empty() && info.indices.size() >= 3)
			{
				bool useConvex = info.preferConvex || !rb.IsKinematic;
				std::uint8_t kind = useConvex ? 1u : 0u;
				std::uint8_t ds = info.doubleSided ? 1u : 0u;

				CacheKey key{ info.key, kind, ds };
				if (auto it = mShapeCache.find(key); it != mShapeCache.end())
				{
					JPH::Ref<JPH::Shape> base = it->second;
					if (info.scale != glm::vec3(1.0f))
						return JPH::Ref<JPH::Shape>(new JPH::ScaledShape(base, ToJPHVec3(info.scale)));
					return base;
				}

				JPH::Ref<JPH::Shape> base;

				if (useConvex)
				{
					JPH::Array<JPH::Vec3> pts; pts.resize(info.vertices.size());
					for (size_t i = 0; i < info.vertices.size(); ++i)
					{
						auto const &v = info.vertices[i];
						pts[i] = JPH::Vec3(v.x, v.y, v.z);
					}
					JPH::ConvexHullShapeSettings hull(pts);
					hull.mMaxConvexRadius = 0.0f;
					auto res = hull.Create();
					if (!res.HasError()) base = res.Get();
				}
				else
				{
					JPH::Array<JPH::Float3> verts; verts.resize(info.vertices.size());
					for (size_t i = 0; i < info.vertices.size(); ++i)
					{
						auto const &v = info.vertices[i];
						verts[i] = JPH::Float3(v.x, v.y, v.z);
					}
					JPH::Array<JPH::IndexedTriangle> tris; tris.reserve(info.indices.size() / 3);
					for (size_t i = 0; i + 2 < info.indices.size(); i += 3)
					{
						tris.push_back(JPH::IndexedTriangle(
							(JPH::uint32)info.indices[i + 0],
							(JPH::uint32)info.indices[i + 1],
							(JPH::uint32)info.indices[i + 2]
						));
					}
					JPH::MeshShapeSettings mss(verts, tris);
					auto res = mss.Create();
					if (!res.HasError()) base = res.Get();
				}

				if (base)
				{
					mShapeCache.emplace(key, base);
					if (info.scale != glm::vec3(1.0f))
						return JPH::Ref<JPH::Shape>(new JPH::ScaledShape(base, ToJPHVec3(info.scale)));
					return base;
				}
			}
		}

		return JPH::Ref<JPH::Shape>(new JPH::BoxShape(JPH::Vec3::sReplicate(DEFAULT_HALF_EXT)));
	}

	void PhysicsSystem::CreateBodyFor(Scene *scene, EntityID e)
	{
		auto &reg = scene->GetRegistry();
		auto &tc = reg.get<TransformComponent>(e);
		auto &rb = reg.get<RigidbodyComponent>(e);

		JPH::Ref<JPH::Shape> shape = MakeShapeForEntity(scene, e, tc, rb);

		JPH::BodyCreationSettings settings(
			shape,
			ToJPHRVec3(tc.Position),
			ToJPHRotation(tc.Rotation),
			ToMotionType(rb),
			ToObjectLayer(rb)
		);

		settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		settings.mMassPropertiesOverride.mMass = std::max(0.0001f, rb.Mass);
		settings.mFriction = 0.6f;
		settings.mRestitution = 0.1f;

		JPH::BodyID const id = mBodyInterface->CreateAndAddBody(settings, JPH::EActivation::Activate);

		if (!rb.IsKinematic)
		{
			mBodyInterface->SetLinearVelocity(id, ToJPHVec3(rb.Velocity));
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
	}

	void PhysicsSystem::DestroyBodyFor(EntityID e)
	{
		auto it = mBodyOf.find(e);
		if (it == mBodyOf.end()) return;

		JPH::BodyID const id = it->second;
		mBodyInterface->RemoveBody(id);
		mBodyInterface->DestroyBody(id);
	}
}

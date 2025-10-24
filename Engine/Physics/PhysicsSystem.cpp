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

	void PhysicsSystem::CreateBodyFor(Scene *scene, EntityID e)
	{
		auto &reg = scene->GetRegistry();
		auto &tc = reg.get<TransformComponent>(e);
		auto &rb = reg.get<RigidbodyComponent>(e);

		JPH::Ref<JPH::Shape> shape;
		if (mMakeEntityShape)
		{
			if (auto s = mMakeEntityShape(scene, e, tc, rb)) shape = s;
		}
		if (!shape)
		{
			shape = JPH::Ref<JPH::Shape>(new JPH::BoxShape(JPH::Vec3::sReplicate(DEFAULT_HALF_EXT)));
		}

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

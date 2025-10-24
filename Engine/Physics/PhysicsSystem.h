#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
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
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include "../ECS/Components.h"
#include "../ECS/Scene.h"
#include "../ECS/System.h"

namespace Engine
{
	namespace Layers
	{
		static constexpr JPH::ObjectLayer NON_MOVING{ 0 };
		static constexpr JPH::ObjectLayer MOVING{ 1 };
		static constexpr JPH::ObjectLayer NUM_LAYERS{ 2 };
	}

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

	class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
		{
			if (a == Layers::NON_MOVING && b == Layers::NON_MOVING) return false;
			return true;
		}
	};

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

	inline JPH::Vec3  ToJPHVec3(glm::vec3 const &v) { return JPH::Vec3(v.x, v.y, v.z); }
	inline JPH::RVec3 ToJPHRVec3(glm::vec3 const &v) { return JPH::RVec3(v.x, v.y, v.z); }
	inline JPH::Quat  ToJPHQuat(glm::quat const &q) { return JPH::Quat(q.x, q.y, q.z, q.w); }
	inline glm::vec3  ToGLM(JPH::Vec3 const &v) { return glm::vec3{ v.GetX(), v.GetY(), v.GetZ() }; }
	inline glm::quat  ToGLM(JPH::Quat const &q) { return glm::quat{ q.GetW(), q.GetX(), q.GetY(), q.GetZ() }; }

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

	using MakeEntityShapeFn = std::function<JPH::Ref<JPH::Shape>(Scene *, entt::entity, TransformComponent const &, RigidbodyComponent const &)>;

	class PhysicsSystem final : public System
	{
	public:
		char const *GetName() const override { return "PhysicsSystem"; }
		int GetPriority() const override { return 10; }

		void OnInit(Scene *scene) override;
		void OnUpdate(Scene *scene, Timestep dt) override;
		void OnShutdown(Scene *scene) override;

		void SetMakeEntityShapeCallback(MakeEntityShapeFn fn) { mMakeEntityShape = std::move(fn); }

	private:
		using EntityID = entt::entity;

		JPH::TempAllocator *mTempAllocator{};
		JPH::JobSystemThreadPool *mJobSystem{};
		JPH::PhysicsSystem        mPhysics;
		JPH::BodyInterface *mBodyInterface{};

		BPLayerInterfaceImpl              mBPLayers;
		ObjectVsBroadPhaseLayerFilterImpl mObjVsBPLayerFilter;
		ObjectLayerPairFilterImpl         mObjPairFilter;

		std::unordered_map<EntityID, JPH::BodyID> mBodyOf;
		MakeEntityShapeFn mMakeEntityShape;

		static JPH::EMotionType ToMotionType(RigidbodyComponent const &rb) { return rb.IsKinematic ? JPH::EMotionType::Kinematic : JPH::EMotionType::Dynamic; }
		static JPH::ObjectLayer ToObjectLayer(RigidbodyComponent const &rb) { return rb.IsKinematic ? Layers::NON_MOVING : Layers::MOVING; }

		void BuildOrRefreshBodies(Scene *scene);
		void CreateBodyFor(Scene *scene, EntityID e);
		void DestroyBodyFor(EntityID e);
	};
}

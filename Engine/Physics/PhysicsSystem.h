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
			- Internal fixed-step accumulator for deterministic simulation stepping

			Units: meters, kilograms, seconds.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
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

#include "ECS/Components.h"
#include "ECS/Scene.h"
#include "ECS/System.h"
#include "PhysicsBridge.h"

namespace Engine
{
	class PhysicsSystem : public System
	{
	public:
		char const *GetName() const override
		{
			return "PhysicsSystem";
		}

		int GetPriority() const override
		{
			return 55;
		}

		void OnInit(Scene *scene) override;
		void OnUpdate(Scene *scene, Timestep dt) override;
		void OnShutdown(Scene *scene) override;

		void SetMakeEntityShapeCallback(MakeEntityShapeFn fn)
		{
			mMakeEntityShape = std::move(fn);
		}

		void SetFetchMeshInfoCallback(FetchMeshInfoFn fn)
		{
			mFetchMeshInfo = std::move(fn);
		}

	private:
		static JPH::EMotionType ToMotionType(RigidbodyComponent const &rb)
		{
			if (rb.IsKinematic)
				return JPH::EMotionType::Kinematic;

			if (rb.Mass <= 0.0f)
				return JPH::EMotionType::Static;

			return JPH::EMotionType::Dynamic;
		}

		static JPH::ObjectLayer ToObjectLayer(RigidbodyComponent const &rb)
		{
			if (rb.IsKinematic)
				return Layers::MOVING;

			if (rb.Mass <= 0.0f)
				return Layers::NON_MOVING;

			return Layers::MOVING;
		}

		void BuildOrRefreshBodies(Scene *scene);
		void SyncECSBodiesToPhysics(Scene *scene);
		void ApplyKinematicTargetsForStep(Scene *scene, float fixedStep);
		void SyncPhysicsBodiesToECS(Scene *scene);
		void CreateBodyFor(Scene *scene, EntityID e);
		void DestroyBodyFor(EntityID e);

		JPH::Ref<JPH::Shape> MakeShapeForEntity(
			Scene *scene,
			EntityID e,
			TransformComponent const &tc,
			RigidbodyComponent const &rb
		);

	private:
		double        mAccumulatorSeconds{ 0.0 };
		double        mFixedStepSeconds{ 1.0 / 60.0 };
		std::uint32_t mMaxCatchUpSteps{ 8u };
	};
} // namespace Engine
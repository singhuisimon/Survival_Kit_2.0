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
	/*****************************************************************************/
	/*!
	\brief      ECS system that integrates Jolt Physics with the engine world.
	*/
	/*****************************************************************************/
	class PhysicsSystem : public System
	{
	public:
		/*****************************************************************************/
		/*!
		\brief      Returns the system name.
		\return     Null-terminated C string identifier.
		*/
		/*****************************************************************************/
		char const *GetName() const override
		{
			return "PhysicsSystem";
		}

		/*****************************************************************************/
		/*!
		\brief      Execution priority among systems (higher runs earlier).
		\return     Integer priority value.
		*/
		/*****************************************************************************/
		int GetPriority() const override
		{
			return 55;
		}

		/*****************************************************************************/
		/*!
		\brief      Initializes Jolt world and mirrors existing ECS bodies.
		\param      scene   Scene pointer used to enumerate entities and components.
		*/
		/*****************************************************************************/
		void OnInit(Scene *scene) override;

		/*****************************************************************************/
		/*!
		\brief      Simulation step and ECS sync.
					1) Push ECS-authored changes to physics.
					2) Advance Jolt using a fixed internal timestep.
					3) Pull latest dynamic poses/velocities back into ECS.
		\param      scene   Scene pointer.
		\param      dt      Variable frame delta supplied by the engine.
		*/
		/*****************************************************************************/
		void OnUpdate(Scene *scene, Timestep dt) override;

		/*****************************************************************************/
		/*!
		\brief      Tears down the Jolt world and releases owned resources.
		\param      scene   Scene pointer (unused).
		*/
		/*****************************************************************************/
		void OnShutdown(Scene *scene) override;

		/*****************************************************************************/
		/*!
		\brief      Registers a callback to construct a Jolt shape for an entity.
		\param      fn  Function taking (Scene*, entt::entity, TransformComponent&,
						 RigidbodyComponent&) and returning a Jolt Shape ref.
		*/
		/*****************************************************************************/
		void SetMakeEntityShapeCallback(MakeEntityShapeFn fn)
		{
			mMakeEntityShape = std::move(fn);
		}

		/*****************************************************************************/
		/*!
		\brief      Registers a callback to fetch mesh data for collider building.
		\param      fn  Function taking (Scene*, entt::entity, MeshBuildInfo&) and
						 returning true on success.
		*/
		/*****************************************************************************/
		void SetFetchMeshInfoCallback(FetchMeshInfoFn fn)
		{
			mFetchMeshInfo = std::move(fn);
		}

	private:
		/*****************************************************************************/
		/*!
		\brief      Maps rigidbody kinematic flag to Jolt motion type.
		\param      rb  Rigidbody component.
		\return     JPH motion type (Static, Kinematic, or Dynamic).
		*/
		/*****************************************************************************/
		static JPH::EMotionType ToMotionType(RigidbodyComponent const &rb)
		{
			// Kinematic wins even when mass is zero so that authored
			// "immovable but moving" objects still participate correctly.
			if (rb.IsKinematic)
				return JPH::EMotionType::Kinematic;

			if (rb.Mass <= 0.0f)
				return JPH::EMotionType::Static;

			return JPH::EMotionType::Dynamic;
		}

		/*****************************************************************************/
		/*!
		\brief      Maps rigidbody kinematic flag to engine object layer.
		\param      rb  Rigidbody component.
		\return     Object layer used for broadphase and pair filtering.
		*/
		/*****************************************************************************/
		static JPH::ObjectLayer ToObjectLayer(RigidbodyComponent const &rb)
		{
			// Kinematic bodies can move every fixed step, so they belong in MOVING.
			if (rb.IsKinematic)
				return Layers::MOVING;

			if (rb.Mass <= 0.0f)
				return Layers::NON_MOVING;

			return Layers::MOVING;
		}

		/*****************************************************************************/
		/*!
		\brief      Ensures every ECS rigidbody has a matching Jolt body and that
					orphaned bodies are removed.
		\param      scene   Scene pointer.
		*/
		/*****************************************************************************/
		void BuildOrRefreshBodies(Scene *scene);

		/*****************************************************************************/
		/*!
		\brief      Pushes ECS-authored transform/property/velocity changes into
					existing Jolt bodies before stepping.
		\param      scene   Scene pointer.
		*/
		/*****************************************************************************/
		void SyncECSBodiesToPhysics(Scene *scene);

		/*****************************************************************************/
		/*!
		\brief      Applies authored kinematic velocity targets for a single fixed
					physics step.
		\param      scene       Scene pointer.
		\param      fixedStep   Fixed physics step duration in seconds.
		*/
		/*****************************************************************************/
		void ApplyKinematicTargetsForStep(Scene *scene, float fixedStep);

		/*****************************************************************************/
		/*!
		\brief      Pulls latest transform and velocity state from Jolt into ECS
					after one or more fixed simulation steps.
		\param      scene   Scene pointer.
		*/
		/*****************************************************************************/
		void SyncPhysicsBodiesToECS(Scene *scene);

		/*****************************************************************************/
		/*!
		\brief      Creates and registers a Jolt body for the given entity.
		\param      scene   Scene pointer.
		\param      e       Entity id.
		*/
		/*****************************************************************************/
		void CreateBodyFor(Scene *scene, EntityID e);

		/*****************************************************************************/
		/*!
		\brief      Destroys the Jolt body and clears cached state for the entity.
		\param      e   Entity id.
		*/
		/*****************************************************************************/
		void DestroyBodyFor(EntityID e);

		/*****************************************************************************/
		/*!
		\brief      Produces a shape for an entity, using hooks and mesh cache when
					possible. Falls back to a default box if no mesh/shape is provided.
		\param      scene   Scene pointer.
		\param      e       Entity id.
		\param      tc      Transform component.
		\param      rb      Rigidbody component.
		\return     Ref-counted Jolt Shape used for body creation.
		*/
		/*****************************************************************************/
		JPH::Ref<JPH::Shape> MakeShapeForEntity(Scene *scene, EntityID e, TransformComponent const &tc, RigidbodyComponent const &rb);

	private:
		// Fixed-step state
		double        mAccumulatorSeconds{ 0.0 };
		double        mFixedStepSeconds{ 1.0 / 60.0 };
		std::uint32_t mMaxCatchUpSteps{ 8u };
	};
} // namespace Engine
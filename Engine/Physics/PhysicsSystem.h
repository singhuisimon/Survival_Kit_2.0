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
					1) Push kinematic poses and property changes to physics.
					2) Step the physics world.
					3) Pull dynamic poses/velocities back into ECS.
		\param      scene   Scene pointer.
		\param      dt      Fixed or variable time step.
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
		\return     JPH motion type (Kinematic or Dynamic).
		*/
		/*****************************************************************************/
		static JPH::EMotionType ToMotionType(RigidbodyComponent const &rb)
		{
			// Mass <= 0 means immovable/static regardless of other flags.
			if (rb.Mass <= 0.0f)
				return JPH::EMotionType::Static;

			return rb.IsKinematic ? JPH::EMotionType::Kinematic : JPH::EMotionType::Dynamic;
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
			// Keep static bodies in the NON_MOVING layer.
			if (rb.Mass <= 0.0f)
				return Layers::NON_MOVING;

			return rb.IsKinematic ? Layers::NON_MOVING : Layers::MOVING;
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
	};
} // namespace Engine

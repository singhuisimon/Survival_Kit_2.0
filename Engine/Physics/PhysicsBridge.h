#pragma once
/*****************************************************************************/
/*!
\file       PhysicsBridge.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/11/09
\brief      Jolt Physics bridge declarations:
			- BroadPhase layer interface and collision filters
			- Global Jolt systems and engine-level state
			- Helper conversions between GLM and Jolt math types
			- Mesh/shape cache keys and mesh info fetch hook

			Units: meters, kilograms, seconds.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include <cstdint>
#include <functional>
#include <unordered_map>
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
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/Scene.h"

namespace Engine
{
	using EntityID = entt::entity;

	/*****************************************************************************/
	/*!
	\brief      Object-layer constants used by the world and filters.
	*/
	/*****************************************************************************/
	namespace Layers
	{
		static constexpr JPH::ObjectLayer NON_MOVING{ 0 };
		static constexpr JPH::ObjectLayer MOVING{ 1 };
		static constexpr JPH::ObjectLayer NUM_LAYERS{ 2 };
	}

	/*****************************************************************************/
	/*!
	\brief      BroadPhase layer interface mapping object layers to broadphase.
	*/
	/*****************************************************************************/
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		/*****************************************************************************/
		/*!
		\brief      Constructs the interface and prepares the mapping table.
		*/
		/*****************************************************************************/
		BPLayerInterfaceImpl();

		/*****************************************************************************/
		/*!
		\brief      Returns the number of BroadPhase layers.
		\return     BroadPhase layer count.
		*/
		/*****************************************************************************/
		JPH::uint GetNumBroadPhaseLayers() const override;

		/*****************************************************************************/
		/*!
		\brief      Maps an object layer to its BroadPhase layer.
		\param      layer   Object layer value.
		\return     BroadPhase layer corresponding to the object layer.
		*/
		/*****************************************************************************/
		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override;

		/*****************************************************************************/
		/*!
		\brief      Debug name for a BroadPhase layer.
		\param      layer   BroadPhase layer.
		\return     Null-terminated C string.
		*/
		/*****************************************************************************/
		const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override;

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[2];
		JPH::uint            mNumBroadPhaseLayers{};
	};

	/*****************************************************************************/
	/*!
	\brief      Object-layer pair collision filter.
	*/
	/*****************************************************************************/
	class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
	{
	public:
		/*****************************************************************************/
		/*!
		\brief      Returns whether two object layers should collide.
		\param      a   First object layer.
		\param      b   Second object layer.
		\return     True if the pair should collide, false otherwise.
		*/
		/*****************************************************************************/
		bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override;
	};

	/*****************************************************************************/
	/*!
	\brief      Object layer vs BroadPhase layer collision filter.
	*/
	/*****************************************************************************/
	class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		/*****************************************************************************/
		/*!
		\brief      Returns whether an object layer collides with a BroadPhase layer.
		\param      layer   Object layer value.
		\param      broad   BroadPhase layer value.
		\return     True if the combination should collide.
		*/
		/*****************************************************************************/
		bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broad) const override;
	};

	/*****************************************************************************/
	/*!
	\brief      Convert GLM vectors and quaternions to Jolt types.
	*/
	/*****************************************************************************/
	inline JPH::Vec3 ToJPHVec3(const glm::vec3 &v)
	{
		return JPH::Vec3{ v.x, v.y, v.z };
	}
	inline JPH::RVec3 ToJPHRVec3(const glm::vec3 &v)
	{
		return JPH::RVec3{ v.x, v.y, v.z };
	}
	inline JPH::Quat ToJPHQuat(const glm::quat &q)
	{
		return JPH::Quat{ q.x, q.y, q.z, q.w };
	}

	/*****************************************************************************/
	/*!
	\brief      Convert Jolt vectors and quaternions to GLM types.
	*/
	/*****************************************************************************/
	inline glm::vec3 ToGLM(const JPH::Vec3 &v)
	{
		return glm::vec3{ v.GetX(), v.GetY(), v.GetZ() };
	}
	inline glm::quat ToGLM(const JPH::Quat &q)
	{
		return glm::quat{ q.GetW(), q.GetX(), q.GetY(), q.GetZ() };
	}

	/*****************************************************************************/
	/*!
	\brief      Helpers for Euler degrees <-> quaternion conversions (GLM).
	*/
	/*****************************************************************************/
	inline glm::quat EulerDegToQuat(const glm::vec3 &eulerDeg)
	{
		return glm::quat{ glm::radians(eulerDeg) };
	}
	inline glm::vec3 QuatToEulerDeg(const glm::quat &qIn)
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

	/*****************************************************************************/
	/*!
	\brief      Hook to construct a Jolt shape for an entity.
	\param      Scene*                Scene pointer.
	\param      entt::entity          Entity id.
	\param      TransformComponent&   Transform component.
	\param      RigidbodyComponent&   Rigidbody component.
	\return     Jolt shape reference.
	*/
	/*****************************************************************************/
	using MakeEntityShapeFn = std::function<
		JPH::Ref<JPH::Shape>(Scene *, entt::entity, const TransformComponent &, const RigidbodyComponent &)
	>;

	/*****************************************************************************/
	/*!
	\brief      Mesh data for building triangle or convex shapes.
	*/
	/*****************************************************************************/
	struct MeshBuildInfo
	{
		std::vector<glm::vec3>     vertices;
		std::vector<std::uint32_t> indices;
		glm::vec3                  scale{ 1, 1, 1 };
		bool                       doubleSided{};
		bool                       preferConvex{};
		std::uint64_t              key{};
	};

	/*****************************************************************************/
	/*!
	\brief      Hook to fetch mesh data for an entity. Returns true on success.
	*/
	/*****************************************************************************/
	using FetchMeshInfoFn = std::function<bool(Scene *, entt::entity, MeshBuildInfo &)>;

	// Global Jolt systems and interfaces (owned/managed elsewhere)
	extern JPH::TempAllocator *mTempAllocator;
	extern JPH::JobSystemThreadPool *mJobSystem;
	extern JPH::PhysicsSystem        mPhysics;
	extern JPH::BodyInterface *mBodyInterface;

	// Layer interfaces and filters
	extern BPLayerInterfaceImpl              mBPLayers;
	extern ObjectVsBroadPhaseLayerFilterImpl mObjVsBPLayerFilter;
	extern ObjectLayerPairFilterImpl         mObjPairFilter;

	// Entity -> BodyID map
	extern std::unordered_map<EntityID, JPH::BodyID> mBodyOf;

	/*****************************************************************************/
	/*!
	\brief      Cache key for reusable shapes (by source mesh and options).
	*/
	/*****************************************************************************/
	struct CacheKey
	{
		std::uint64_t key{};
		std::uint8_t  kind{};
		std::uint8_t  ds{};
		bool operator==(const CacheKey &o) const
		{
			return key == o.key && kind == o.kind && ds == o.ds;
		}
	};

	/*****************************************************************************/
	/*!
	\brief      Hash function for CacheKey.
	*/
	/*****************************************************************************/
	struct CacheKeyHash
	{
		std::size_t operator()(const CacheKey &k) const
		{
			std::size_t h = std::hash<std::uint64_t>{}(k.key);
			h ^= (std::size_t)k.kind + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
			h ^= (std::size_t)k.ds + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
			return h;
		}
	};

	// Shape cache shared by the bridge
	extern std::unordered_map<CacheKey, JPH::Ref<JPH::Shape>, CacheKeyHash> mShapeCache;

	/*****************************************************************************/
	/*!
	\brief      Snapshot of properties applied to a body for change tracking.
	*/
	/*****************************************************************************/
	struct AppliedProps
	{
		bool        isKinematic{};
		bool        useGravity{};
		float       mass{};
		std::uint64_t meshKey{};
		std::uint8_t  shapeKind{};
		std::uint8_t  shapeDS{};
		glm::vec3   shapeScale{ 1, 1, 1 };
		glm::vec3   lastPos{};
		glm::quat   lastRot{};
	};

	// Per-entity applied properties
	extern std::unordered_map<EntityID, AppliedProps> mApplied;

	// Hooks registered by the engine
	extern MakeEntityShapeFn mMakeEntityShape;
	extern FetchMeshInfoFn   mFetchMeshInfo;
} // namespace Engine

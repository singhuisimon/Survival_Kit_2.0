/*****************************************************************************/
/*!
\file       PhysicsBridge.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/11/09
\brief      Jolt Physics bridge glue:
			- BroadPhase layer interface (object to broadphase mapping)
			- Collision filters (object-layer pairs and object vs broadphase)
			- Global Jolt systems (temp allocator, job system, physics system)
			- Body/shape caches and engine hooks

			Units: meters, kilograms, seconds.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include "PhysicsBridge.h"

// Broadphase query helpers
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

// For safe read access to bodies during queries
#include <Jolt/Physics/Body/BodyLock.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <limits>

namespace Engine
{
	/*****************************************************************************/
	/*!
	\brief      Constructs the BroadPhase layer interface and sets up the mapping.
	*/
	/*****************************************************************************/
	BPLayerInterfaceImpl::BPLayerInterfaceImpl()
		: mObjectToBroadPhase{ JPH::BroadPhaseLayer{ 0 }, JPH::BroadPhaseLayer{ 1 } }
		, mNumBroadPhaseLayers{ 2u }
	{}

	/*****************************************************************************/
	/*!
	\brief      Returns the number of BroadPhase layers used by the world.
	\return     Count of BroadPhase layers.
	*/
	/*****************************************************************************/
	JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return mNumBroadPhaseLayers;
	}

	/*****************************************************************************/
	/*!
	\brief      Maps an object layer to its BroadPhase layer.
	\param      layer   Object layer to map.
	\return     Corresponding BroadPhase layer.
	*/
	/*****************************************************************************/
	JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer layer) const
	{
		return mObjectToBroadPhase[layer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	/*****************************************************************************/
	/*!
	\brief      Returns a debug-friendly name for a BroadPhase layer.
	\param      layer   BroadPhase layer to name.
	\return     Null-terminated C string name.
	*/
	/*****************************************************************************/
	const char *BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const
	{
		switch (layer.GetValue())
		{
		case 0: return "NON_MOVING";
		case 1: return "MOVING";
		default: return "UNKNOWN";
		}
	}
#endif

	bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const
	{
		if (a == Layers::NON_MOVING && b == Layers::NON_MOVING) return false;
		return true;
	}

	bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broad) const
	{
		JPH::uint b = broad.GetValue();
		switch (layer)
		{
		case Layers::NON_MOVING: return b == 1u;
		case Layers::MOVING:     return (b == 0u) || (b == 1u);
		default:                 return false;
		}
	}

	/*****************************************************************************/
	/*!
	\brief      Global Jolt systems and engine-shared state.
	*/
	/*****************************************************************************/
	JPH::TempAllocator *mTempAllocator = nullptr;
	JPH::JobSystemThreadPool *mJobSystem = nullptr;
	JPH::PhysicsSystem        mPhysics = {};
	JPH::BodyInterface *mBodyInterface = nullptr;

	BPLayerInterfaceImpl              mBPLayers = {};
	ObjectVsBroadPhaseLayerFilterImpl mObjVsBPLayerFilter = {};
	ObjectLayerPairFilterImpl         mObjPairFilter = {};

	std::unordered_map<EntityID, JPH::BodyID>                        mBodyOf = {};
	std::unordered_map<CacheKey, JPH::Ref<JPH::Shape>, CacheKeyHash> mShapeCache = {};
	std::unordered_map<EntityID, AppliedProps>                       mApplied = {};

	MakeEntityShapeFn  mMakeEntityShape = {};
	FetchMeshInfoFn    mFetchMeshInfo = {};

	/*****************************************************************************/
	/*!
	\brief      Local helpers for frustum queries.
	*/
	/*****************************************************************************/
	static inline void NormalizePlane(glm::vec4 &p)
	{
		const float lenSq = p.x * p.x + p.y * p.y + p.z * p.z;
		if (lenSq <= 1e-12f) return;
		const float invLen = 1.0f / std::sqrt(lenSq);
		p *= invLen;
	}

	// GLM is column-major: m[col][row]. We build rows explicitly.
	static inline void ExtractFrustumPlanes_OpenGL(const glm::mat4 &m, glm::vec4 outPlanes[6])
	{
		const glm::vec4 row0{ m[0][0], m[1][0], m[2][0], m[3][0] };
		const glm::vec4 row1{ m[0][1], m[1][1], m[2][1], m[3][1] };
		const glm::vec4 row2{ m[0][2], m[1][2], m[2][2], m[3][2] };
		const glm::vec4 row3{ m[0][3], m[1][3], m[2][3], m[3][3] };

		// Plane equations in world space: ax + by + cz + d >= 0 is inside.
		outPlanes[0] = row3 + row0; // Left
		outPlanes[1] = row3 - row0; // Right
		outPlanes[2] = row3 + row1; // Bottom
		outPlanes[3] = row3 - row1; // Top
		outPlanes[4] = row3 + row2; // Near  (OpenGL NDC z in [-w, w])
		outPlanes[5] = row3 - row2; // Far

		for (int i = 0; i < 6; ++i)
			NormalizePlane(outPlanes[i]);
	}

	static inline bool AABBInsideFrustum(const glm::vec3 &bmin, const glm::vec3 &bmax, const glm::vec4 planes[6])
	{
		for (int i = 0; i < 6; ++i)
		{
			const glm::vec3 n{ planes[i].x, planes[i].y, planes[i].z };

			// Select the vertex most in the direction of the plane normal (positive vertex).
			const glm::vec3 p{
				(n.x >= 0.0f) ? bmax.x : bmin.x,
				(n.y >= 0.0f) ? bmax.y : bmin.y,
				(n.z >= 0.0f) ? bmax.z : bmin.z
			};

			// If positive vertex is outside, whole AABB is outside.
			if (glm::dot(n, p) + planes[i].w < 0.0f)
				return false;
		}
		return true;
	}

	static inline bool ComputeFrustumEnclosingAABB_OpenGL(const glm::mat4 &viewProjWS, glm::vec3 &outMin, glm::vec3 &outMax)
	{
		const float det = glm::determinant(viewProjWS);
		if (std::abs(det) < 1e-12f)
			return false;

		const glm::mat4 inv = glm::inverse(viewProjWS);

		outMin = glm::vec3{ std::numeric_limits<float>::infinity() };
		outMax = glm::vec3{ -std::numeric_limits<float>::infinity() };

		// OpenGL NDC cube corners: x,y,z in [-1, 1]
		const float xs[2] = { -1.0f, 1.0f };
		const float ys[2] = { -1.0f, 1.0f };
		const float zs[2] = { -1.0f, 1.0f };

		for (int ix = 0; ix < 2; ++ix)
			for (int iy = 0; iy < 2; ++iy)
				for (int iz = 0; iz < 2; ++iz)
				{
					glm::vec4 corner = inv * glm::vec4(xs[ix], ys[iy], zs[iz], 1.0f);
					if (std::abs(corner.w) < 1e-12f) continue;
					corner /= corner.w;

					const glm::vec3 p{ corner.x, corner.y, corner.z };
					outMin = glm::min(outMin, p);
					outMax = glm::max(outMax, p);
				}

		return std::isfinite(outMin.x) && std::isfinite(outMax.x);
	}

	static inline EntityID EntityFromUserData(std::uint64_t user)
	{
		return static_cast<EntityID>(static_cast<std::uint32_t>(user));
	}

	/*****************************************************************************/
	/*!
	\brief      Query the broadphase for bodies whose AABB overlaps a world-space AABox.
	*/
	/*****************************************************************************/
	void QueryEntitiesInAABB(const glm::vec3 &minWS, const glm::vec3 &maxWS, std::vector<EntityID> &outEntities)
	{
		outEntities.clear();
		if (mBodyInterface == nullptr)
			return;

		const JPH::BroadPhaseQuery &bpq = mPhysics.GetBroadPhaseQuery();
		const JPH::AABox box(ToJPHVec3(minWS), ToJPHVec3(maxWS));

		JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> collector;
		bpq.CollideAABox(box, collector);

		outEntities.reserve(collector.mHits.size());
		for (const JPH::BodyID &bodyID : collector.mHits)
		{
			const std::uint64_t user = mBodyInterface->GetUserData(bodyID);
			if (user == 0)
				continue;
			outEntities.push_back(EntityFromUserData(user));
		}
	}

	/*****************************************************************************/
	/*!
	\brief      Query the broadphase for bodies whose AABB overlaps a world-space sphere.
	*/
	/*****************************************************************************/
	void QueryEntitiesInSphere(const glm::vec3 &centerWS, float radius, std::vector<EntityID> &outEntities)
	{
		outEntities.clear();
		if (mBodyInterface == nullptr)
			return;

		const JPH::BroadPhaseQuery &bpq = mPhysics.GetBroadPhaseQuery();
		JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> collector;
		bpq.CollideSphere(ToJPHVec3(centerWS), radius, collector);

		outEntities.reserve(collector.mHits.size());
		for (const JPH::BodyID &bodyID : collector.mHits)
		{
			const std::uint64_t user = mBodyInterface->GetUserData(bodyID);
			if (user == 0)
				continue;
			outEntities.push_back(EntityFromUserData(user));
		}
	}

	/*****************************************************************************/
	/*!
	\brief      Query the broadphase for bodies visible within a camera frustum.
	\param      viewProjWS   World->Clip matrix (proj * view).
	\param      outEntities  Visible entities (cleared by this call).
	\details    Broadphase returns candidates based on an enclosing AABox; then we
				test each candidate's Body::GetWorldSpaceBounds() AABox against
				the frustum planes. :contentReference[oaicite:1]{index=1}
	*/
	/*****************************************************************************/
	void QueryEntitiesInFrustum(const glm::mat4 &viewProjWS, std::vector<EntityID> &outEntities)
	{
		outEntities.clear();
		if (mBodyInterface == nullptr)
			return;

		glm::vec3 frMin{}, frMax{};
		if (!ComputeFrustumEnclosingAABB_OpenGL(viewProjWS, frMin, frMax))
			return;

		glm::vec4 planes[6];
		ExtractFrustumPlanes_OpenGL(viewProjWS, planes);

		const JPH::BroadPhaseQuery &bpq = mPhysics.GetBroadPhaseQuery();
		const JPH::AABox frBox(ToJPHVec3(frMin), ToJPHVec3(frMax));

		JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> collector;
		bpq.CollideAABox(frBox, collector);

		outEntities.reserve(collector.mHits.size());

		// Lock interface is the safe way to read body data; BodyLockRead may fail if body was removed.
		const JPH::BodyLockInterfaceLocking &lockInterface = mPhysics.GetBodyLockInterface();

		for (const JPH::BodyID &bodyID : collector.mHits)
		{
			JPH::BodyLockRead lock(lockInterface, bodyID);
			if (!lock.SucceededAndIsInBroadPhase())
				continue;

			const JPH::Body &body = lock.GetBody();
			const std::uint64_t user = body.GetUserData();
			if (user == 0)
				continue;

			const JPH::AABox &b = body.GetWorldSpaceBounds();
			const glm::vec3 bmin{ b.mMin.GetX(), b.mMin.GetY(), b.mMin.GetZ() };
			const glm::vec3 bmax{ b.mMax.GetX(), b.mMax.GetY(), b.mMax.GetZ() };

			if (!AABBInsideFrustum(bmin, bmax, planes))
				continue;

			outEntities.push_back(EntityFromUserData(user));
		}
	}

} // namespace Engine

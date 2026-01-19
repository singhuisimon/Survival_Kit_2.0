/**
 * @file CollisionSystem2D.cpp
 * @brief Defines the system that handles 2D collision detection for UI interaction
 * @details Performs collision detection between mouse input and 2D
 *          UI elements. The system does not perform collision resolution directly.
 *          Instead, it notifies other systems via callbacks and events. While
 *          primarily designed for input hit-testing, the underlying collision
 *          detection can be used for general 2D object-to-object queries.
 * @author Tan Jun Rui
 * @date 18 January 2026
 * Copyright (C) 2026 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "../Physics/CollisionSystem2D.h"
#include "../Physics/Collision2D.h"

#include "../ECS/Scene.h"

#include "../Utility/Logger.h"

namespace Engine 
{
	
	/**
	 * @brief	  Constructor for collision system
	 * @param[in] input_ref
	 *			  Provides a reference for the mouse input to be exposed
	 *			  to the collision system.
	 * param[in]  meshdata2d_ref
	 *			  Provides a reference to the positional data of the 2D UI
	 *			  elements
	 * param[in]  viewport_ref
	 *			  Provides a reference to the viewport data
	 * param[in]  ortho_proj_ref
	 *			  Provides a reference to the orthographic projection data
	 */
	CollisionSystem2D::CollisionSystem2D( std::vector<MeshData2D> const& meshdata2d_ref,
										  glm::vec4 const& viewport_ref,
										  glm::mat4 const& ortho_proj_ref) :
		meshdata2d_(meshdata2d_ref), viewport_(viewport_ref), ortho_proj_(ortho_proj_ref), active_scene_(nullptr) { }

	/**
	 * @brief     Continuously updates the active scene to point to the
	 *			  correct scene.
	 * @param[in] scene
	 *			  Pointer to the scene that contains data related to 2D UI
	 *			  elements.
	 * @param[in] ts
	 *			  Time step to sync with other systems.
	 */
	void CollisionSystem2D::OnUpdate(Scene* scene, Timestep ts) {
		(void)ts;
		active_scene_ = scene; 
	}

	/**
	 * @brief     Gets the priority in which this system is updated in the main
	 *			  update loop.
	 * @return    int representing the system's update priority in the main
	 *			  update loop.
	 */
	int CollisionSystem2D::GetPriority() const { return 21; }

	/**
	 * @brief     Gets the system name.
	 * @return    c-string representing the system's name
	 */
	const char* CollisionSystem2D::GetName() const { return "CollisionSystem2D"; }

	/**
	 * @brief     Checks if a point lies within a game object.
	 * @param[in] entityId
	 *			  Entity ID of the game object to check.
	 * @param[in] point
	 *			  The point to check.
	 * @return    bool true if the point lies inside the game object
	 *			  else false.
	 */
	bool CollisionSystem2D::IsPointInEntity(entt::entity const entityId, glm::vec2 const& point) {
		 
		if (!active_scene_) return false;

		auto view = active_scene_->GetRegistry().view<TransformComponent, SpriteRendererComponent>();

		auto& transform = view.get<TransformComponent>(entityId);
		auto& sprite = view.get<SpriteRendererComponent>(entityId);

		AABB2D aabb = ComputeAABB(meshdata2d_[sprite.GetMeshIndex()].positions,
								  ortho_proj_,
								  transform.WorldTransform,
								  glm::vec2(viewport_.z, viewport_.w));

		return Mouse2DCollision(aabb.min, aabb.max, point);
	}
}
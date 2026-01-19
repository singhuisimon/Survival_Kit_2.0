/**
 * @file CollisionSystem2D.h
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
#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"
#include "../Graphics/MeshData.h" // Dependency injection, for UI element bounds collision

namespace Engine 
{
	/**
	 * @brief System responsible for detecting collision detection between 2D objects.
	 * @details Performs AABB checks between 2D objects and mouse input.
	 */
	class CollisionSystem2D : public System {
	public:

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
		CollisionSystem2D(std::vector<MeshData2D>& meshdata2d_ref, glm::vec2& viewport_ref, glm::mat4& ortho_proj_ref);
		
		/**
		 * @brief     Continuously updates the active scene to point to the
		 *			  correct scene.
		 * @param[in] scene
		 *			  Pointer to the scene that contains data related to 2D UI
		 *			  elements.
		 * @param[in] ts
		 *			  Time step to sync with other systems.
		 */
		void OnUpdate(Scene* scene, Timestep ts) override;

		/**
		 * @brief     Gets the priority in which this system is updated in the main
		 *			  update loop.
		 * @return    int representing the system's update priority in the main 
		 *			  update loop.
		 */
		int GetPriority() const override;

		/**
		 * @brief     Gets the system name.
		 * @return    c-string representing the system's name
		 */
		const char* GetName() const override;

		/**
		 * @brief     Checks if a point lies within a game object.
		 * @param[in] entityId
		 *			  Entity ID of the game object to check.
		 * @param[in] point
		 *			  The point to check.
		 * @return    bool true if the point lies inside the game object
		 *			  else false.
		 */
		bool IsPointInEntity(entt::entity const entityId, glm::vec2 const& point);

	private:
		
		// Collision system only uses these data it doesn't own them.
		std::vector<MeshData2D>& meshdata2d_;
		glm::vec2&			     viewport_;
		glm::mat4&				 ortho_proj_;
		Scene*			         active_scene_;
	};
}
/**
 * @file Collision2D.h
 * @brief Contains functions to resolve collision detection problems in 2D.
 * @details Contains non member functions that are to be used in resolving 2D 
 *			collision problems.
 * @author Tan Jun Rui
 * @date 12 January 2026
 * Copyright (C) 2026 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#include <glm/glm.hpp>
#include "../Component/TransformComponent.h"

namespace Engine {

	/**
	 * @brief Represents a 2D bounding box.
	 */
	struct AABB2D { glm::vec2 min, max; };

	/**
	 * @brief	  Performs a simple AABB collision test in 2D between the mouse position
	 *			  and a 2D object of interest. The function expects the parameters to be
	 *			  in screen space coordinates.
	 * @param[in] min
	 *			  The minimum point of the object of interest.
	 * @param[in] max
	 *			  The maximum point of the object of interest.
	 * @param[in] mouse_pos
	 *			  The current position of the mouse.
	 * @return	  bool 
	 *			  value on whether a mouse is intersecting a 2D object.
	 */
	bool Mouse2DCollision(glm::vec2 const& min, glm::vec2 const& max, glm::vec2 const& mouse_pos);

	/**
	 * @brief	  Computes the AABB of a 2D object.
	 * @param[in] positions
	 *			  The positions of the 2D object.
	 * @param[in] projection
	 *			  The orthographic projection matrix.
	 * @param[in] model_to_world
	 *			  The model to world transformation matrix of the 2D
	 *			  object.
	 * @param[in] viewport
	 *			  The viewport dimensions.
	 * @return	  AABB2D
	 *			  A 2D bounding box of the object.
	 */
	AABB2D ComputeAABB(std::vector<glm::vec2> const& positions, 
						 glm::mat4 const& projection, 
						 glm::mat4 const& model_to_world, 
						 glm::vec2 const& viewport);
}
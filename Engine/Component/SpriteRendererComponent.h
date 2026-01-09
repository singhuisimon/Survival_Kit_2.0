/**
 * @file SpriteRendererComponent.h
 * @brief   Component that stores sprite-related properties.
 * @details The data stored will be sent down to the graphics pipeline to render 2D sprites.
 * @authors Tan Jun Rui
 * @date 31 December 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#include <glm/glm.hpp>
#include "../../External/xresource_guid/include/xresource_guid.h"
#include "Utility/Types.h"

namespace Engine {

	/**
	 * @brief Sprite renderer component for rendering 2D sprites, used in a simplistic back to front rendering order.
	 */
	struct SpriteRendererComponent {

		xresource::instance_guid TextureGuid; // Guid for the texture resource
		glm::vec4                Color;		  // RGBA
		u32						 Quad;		  // Index to the quad geometry data in storage
		u32                      SpriteLayer; // Layer for rendering order
		bool                     IsActive;    // Enables interactivity
		bool					 IsVisible;   // Enables rendering

		// Default constructor
		SpriteRendererComponent()
			: TextureGuid(0),
			  Color(1.0f, 1.0f, 1.0f, 1.0f),
			  Quad(3),
			  SpriteLayer(0),
		      IsActive(true),
		      IsVisible(true) { }
	};

}
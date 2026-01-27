/**
 * @file DrawItem.h
 * @brief Drawable object representation for rendering
 * @details Defines a lightweight structure representing a single drawable
 *          entity in the scene. Contains indices to shared resources (mesh,
 *          texture, material) and per-instance transformation data. Used to
 *          submit geometry to the renderer efficiently.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include "../Utility/Types.h"
#include "../Asset/ResourceTypes.h"
#include <glm/glm.hpp>
#include "../Component/TextComponent.h"

#include "Asset/ResourceData.h"

namespace Engine{
	
	/**
	* @brief Tells the renderer what type of draw item a specific entity represents
	* 
	*/
	enum class DrawItemType : u32 
	{
		MESH3D,
		SPRITE2D,
		Particle,
		TEXT
	};

	/**
	 * @brief Represents a single drawable object instance in the scene
	 * @details Compact structure containing resource handles and transform data.
	 *          Multiple DrawItems can reference the same mesh/texture/material,
	 *          enabling efficient instanced rendering. The renderer uses these
	 *          to batch and execute draw calls.
	 */
	struct DrawItem
	{
		// Transformation matrix that describes rendered object from it's own local coordinates to world coordinates
		glm::mat4				 m_model_to_world_transform; // No default, most be provided

		// The draw item the entity represents
		DrawItemType			 m_drawitem_type;				 // No default, most be provided

		// Unique entity identifier
		u32						 m_entity_id                 = 0;

		// Submesh index for multi-mesh objects
		u32						 m_submesh_index             = 0;

		// Fallback resource handles
		u32						 m_default_mesh_handle       = 0;
		u32						 m_default_material_handle   = 0;
		u32						 m_default_u32texture_handle = 0;

		u32						 m_render_layer			     = 0;

		// Color if the rendered object has self defined colors
		glm::vec4				 m_color = { 0.f, 0.f, 0.f, 1.f };

		// Resource GUIDs
		xresource::instance_guid m_mesh_guid = 0;
		xresource::instance_guid m_material_guid = 0;
		xresource::instance_guid m_texture_guid = 0;

		// Shadow settings
		bool     m_render_main_pass = true;		// false for CastType::ShadowsOnly
		bool     m_receive_shadows = false;		// MeshRendererComponent::ShadowReceive
		u32		 m_cast_shadow_type = 0u;		// MeshRendererComponent::CastType: 0 = Off,1 = On,2 = TwoSided,3 = ShadowsOnly

		// text data
		std::string m_text = ""; 
		float m_fontSize = 24.0f; 
		TextAlignment m_textAlignment; // 0=left, 1=Center, 2=Right, 3=Justified
		float m_lineSpacing = 1.0f; 
		float m_letterSpacing = 0.0f; 
		float m_maxWidth = 0.0f;
	};

}


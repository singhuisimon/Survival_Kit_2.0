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

#include "Asset/ResourceData.h"

namespace Engine{

	/**
	 * @brief Represents a single drawable object instance in the scene
	 * @details Compact structure containing resource handles and transform data.
	 *          Multiple DrawItems can reference the same mesh/texture/material,
	 *          enabling efficient instanced rendering. The renderer uses these
	 *          to batch and execute draw calls.
	 */
	struct DrawItem
	{
		glm::mat4 m_model_to_world_transform;

		// Unique entity identifier
		u32		  m_entity_id; 

		// Submesh index for multi-mesh objects
		u32		  m_submesh_index;

		// Fallback resource handles
		u32       m_default_mesh_handle;
		u32       m_default_material_handle;
		u32       m_default_u32texture_handle;

		// Resource GUIDs
		xresource::instance_guid m_mesh_guid;
		xresource::instance_guid m_material_guid;
		xresource::instance_guid m_texture_guid;
	};

}


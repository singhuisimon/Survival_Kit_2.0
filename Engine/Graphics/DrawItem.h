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
#include <glm/glm.hpp>

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
		u16       m_mesh_handle;
		u16       m_texture_handle;
		u16       m_material_handle;

		glm::mat4 m_model_to_world_transform;
	};

}


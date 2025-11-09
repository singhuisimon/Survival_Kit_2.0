/**
 * @file GraphicsLoader.h
 * @brief	Temporary space and loader for graphics-related assets and data
 * @details Store assets and data for use in the graphics pipeline
 * @author Chua Wen Bin Kenny
 * @date 14 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

// For graphics related defines and functionality
#include "Graphics/Light.h"
#include "Graphics/Camera.h"
#include "Graphics/DrawItem.h"
#include "Graphics/Primitives.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture.h"
#include "Graphics/Material.h"

// For ImGui type
#include "imgui.h"


namespace Engine {

	struct EditorViewport {
		ImVec2 tl;	// Top Left
		ImVec2 size;
	};

	struct GraphicsLoader {
		std::vector<MeshGL>                      m_mesh_storage;
		std::vector<MeshData>                    m_mesh_data_storage;
		std::vector<ShaderProgram>               m_shader_storage;
		std::vector<Texture>                     m_textures;

		std::vector<Texture>                     t_testing_textures;
		std::vector<Material>                    t_testing_material;
	};
}
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

namespace Engine {

	struct GraphicsLoader {
		std::vector<MeshGL>                      m_mesh_storage;
		std::vector<MeshData>                    m_mesh_data_storage;
		std::vector<ShaderProgram>               m_shader_storage;
		std::vector<Texture>                     m_textures;

		std::vector<Texture>                     t_testing_textures;
		std::vector<Material>                    t_testing_material;
	};
}
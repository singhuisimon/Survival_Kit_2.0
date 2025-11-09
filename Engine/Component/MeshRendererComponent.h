/**
 * @file MeshRendererComponent.h
 * @brief   Component that stores Mesh-related properties
 * @details The data stored will be used by the graphics pipeline to
 *          render 3D meshes in accordance to the settings
 * @authors Chua Wen Bin Kenny (50%), Tan Jun Rui (50%)
 * @date 14 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "Utility/Types.h"
#include "../Asset/ResourceTypes.h"
#include "Asset/ResourceData.h"

namespace Engine {

    /**
     * @brief Mesh renderer component (for future rendering system)
     */
    struct MeshRendererComponent {

		// Guids for resources
        xresource::instance_guid MeshGuid;
		xresource::instance_guid MaterialGuid;
		xresource::instance_guid TextureGuid;

		// Lighting, shadows and visibility
        bool Visible;               // Determine if sent to draw call
        bool ShadowReceive;         // For future expansion (WIP)
        bool ShadowCast;            // For future expansion (WIP)
        bool GlobalIlluminate;      // Require further expansion; for now true means it receives light from a light object

        // Defaults
        u32 MeshType;          // Fallback to default mesh if custom mesh not found
        u32 Material;          // Fallback to default material if custom material not found
        u32 Texture;           // Fallback to default texture if no texture found (0 means no texture, actual textures start from 1)

        // Submesh
		u32 SubmeshIndex;           // Submesh index for multi mesh objects, assumes all custom meshes imported as a group of submeshes, even if its 1 mesh.
									// If a single mesh is imported, it's treated as a group of 1 submesh.

        // Default constructor
        MeshRendererComponent()
            : MeshGuid(0),
			MaterialGuid(0),
			TextureGuid(0),
    		Visible(true),
            ShadowReceive(false),
            ShadowCast(false),
            GlobalIlluminate(true),
            MeshType(0),
            Material(0),
            Texture(0),
            SubmeshIndex(0) { }
    };

} // namespace Engine
/**
 * @file MeshRendererComponent.h
 * @brief Mesh renderer component - rendering properties for 3D meshes
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "Utility/Types.h"
#include "../Asset/ResourceTypes.h"

namespace Engine {

    /**
     * @brief Mesh renderer component (for future rendering system)
     */
    struct MeshRendererComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        bool Visible;           // Determine if sent to draw call
        bool ShadowReceive;     // For future expansion (WIP)
        bool ShadowCast;        // For future expansion (WIP)
        bool GlobalIlluminate;  // Require further expansion; for now true means it receives light from a light object
        u32 MeshType;           // Mesh that the object uses (primitive/custom)
        u32 Material;           // Material handle
        u32 Texture;            // Texture handle (0 means no texture, actual textures start from 1)

        // Default constructor
        MeshRendererComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()),
            Visible(true),
            ShadowReceive(false),
            ShadowCast(false),
            GlobalIlluminate(true),
            MeshType(0),
            Material(0),
            Texture(0) {
        }
    };

} // namespace Engine
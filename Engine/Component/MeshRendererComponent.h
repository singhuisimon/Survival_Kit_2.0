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

#include "../Asset/ResourceTypes.h"
#include "../Utility/Types.h"

namespace Engine {

    /**
     * @brief Mesh renderer component - controls rendering of 3D meshes
     */
    struct MeshRendererComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        /// Whether this mesh should be rendered
        bool Visible;

        /// Mesh type/handle
        u32 MeshType;

        /// Material handle
        u32 Material;

        /// Texture handle
        u32 Texture;

        /**
         * @brief Default constructor - creates visible mesh renderer
         */
        MeshRendererComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Visible(true)
            , MeshType(0)
            , Material(0)
            , Texture(0) {
        }

        /**
         * @brief Constructor with visibility setting
         * @param visible Initial visibility state
         */
        explicit MeshRendererComponent(bool visible)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Visible(visible)
            , MeshType(0)
            , Material(0)
            , Texture(0) {
        }

        /**
         * @brief Set visibility
         * @param visible Whether the mesh should be visible
         */
        void SetVisible(bool visible) {
            Visible = visible;
        }

        /**
         * @brief Check if mesh is visible
         * @return True if mesh should be rendered
         */
        bool IsVisible() const {
            return Visible;
        }

        /**
         * @brief Show the mesh
         */
        void Show() {
            Visible = true;
        }

        /**
         * @brief Hide the mesh
         */
        void Hide() {
            Visible = false;
        }

        /**
         * @brief Toggle visibility
         */
        void ToggleVisibility() {
            Visible = !Visible;
        }
    };

} // namespace Engine
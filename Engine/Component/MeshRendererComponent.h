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

namespace Engine {

    /**
     * @brief Mesh renderer component - controls rendering of 3D meshes
     * @details Contains properties that control how an entity's mesh is rendered,
     *          including visibility flags and future rendering settings.
     *          This component will be expanded as the rendering system is developed.
     */
    struct MeshRendererComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        /// Whether this mesh should be rendered
        bool Visible;

        // Future fields (planned):
        // xresource::instance_guid MeshGUID;      // Reference to mesh asset
        // xresource::instance_guid MaterialGUID;  // Reference to material asset
        // bool CastsShadows;                      // Shadow casting flag
        // bool ReceivesShadows;                   // Shadow receiving flag
        // int RenderLayer;                        // Sorting/culling layer

        /**
         * @brief Default constructor - creates visible mesh renderer
         */
        MeshRendererComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Visible(true) {
        }

        /**
         * @brief Constructor with visibility setting
         * @param visible Initial visibility state
         */
        explicit MeshRendererComponent(bool visible)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Visible(visible) {
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
/**
 * @file CameraComponent.h
 * @brief Camera component - camera properties and projection
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "../Asset/ResourceTypes.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

    /**
     * @brief Camera component - defines camera properties and projection
     * @details Contains all settings needed to render a scene from a specific
     *          viewpoint, including field of view, clipping planes, and whether
     *          this is the primary rendering camera.
     */
    struct CameraComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        /// Field of view in degrees (vertical FOV)
        float FOV;

        /// Near clipping plane distance
        float NearClip;

        /// Far clipping plane distance
        float FarClip;

        /// Whether this is the primary/active camera
        bool Primary;

        /**
         * @brief Default constructor - creates camera with standard settings
         */
        CameraComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , FOV(45.0f)
            , NearClip(0.1f)
            , FarClip(1000.0f)
            , Primary(true) {
        }

        /**
         * @brief Constructor with custom FOV
         * @param fov Field of view in degrees
         */
        explicit CameraComponent(float fov)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , FOV(fov)
            , NearClip(0.1f)
            , FarClip(1000.0f)
            , Primary(true) {
        }

        /**
         * @brief Get the projection matrix for this camera
         * @param aspectRatio Viewport aspect ratio (width/height)
         * @return 4x4 perspective projection matrix
         */
        glm::mat4 GetProjection(float aspectRatio) const {
            return glm::perspective(glm::radians(FOV), aspectRatio, NearClip, FarClip);
        }

        /**
         * @brief Get the orthographic projection matrix
         * @param left Left boundary
         * @param right Right boundary
         * @param bottom Bottom boundary
         * @param top Top boundary
         * @return 4x4 orthographic projection matrix
         */
        glm::mat4 GetOrthographicProjection(float left, float right, float bottom, float top) const {
            return glm::ortho(left, right, bottom, top, NearClip, FarClip);
        }

        /**
         * @brief Set the field of view
         * @param fov New FOV in degrees
         */
        void SetFOV(float fov) {
            FOV = fov;
        }

        /**
         * @brief Set clipping planes
         * @param nearClip Near plane distance
         * @param farClip Far plane distance
         */
        void SetClippingPlanes(float nearClip, float farClip) {
            NearClip = nearClip;
            FarClip = farClip;
        }

        /**
         * @brief Set as primary camera
         * @param isPrimary Whether this should be the primary camera
         */
        void SetPrimary(bool isPrimary) {
            Primary = isPrimary;
        }

        /**
         * @brief Check if this is the primary camera
         * @return True if this is marked as primary
         */
        bool IsPrimary() const {
            return Primary;
        }
    };

} // namespace Engine
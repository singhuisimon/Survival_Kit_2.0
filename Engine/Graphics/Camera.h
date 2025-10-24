/**
 * @file Camera.h
 * @brief Declaration of the camera class to manipulate the viewing angle of the scene.
 * @details Manages the camera in the editor and game engine.
 * @author Chua Wen Bin Kenny
 * @date 10 September 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __CAMERA_H__
#define __CAMERA_H__

 // Includes
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>                  // Core types: vec, mat
#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective, translate/scale/rotate
#include <glm/common.hpp>               // glm::clamp
#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr

// Math Utility
#include "../Utility/MathUtils.h"

// For logging information
#include "../Utility/Logger.h"

// Support setting of camera uniforms
#include "../Graphics/ShaderProgram.h"

namespace Engine {

    // Types of camera usage
    enum CameraType {
        ORBITING,
        WALKING
    };

    class Camera3D {

    private:

        // Data members of a camera
        CameraType camType; // Specify the current type of camera
        glm::vec3 pos;      // 3D position of the camera
        glm::vec3 target;   // target the camera is looking at
        float FOV;          // Field of view of camera in degrees
        float nearPlane;         // Near clipping plane
        float farPlane;          // Far clipping plane

    public:

        // Default constructor for a default 3D camera
        Camera3D() :    camType{ CameraType::WALKING },
                        pos{ 0.0f, 5.0f, 5.0f },
                        target{ 0.0f, 0.0f, 0.0f },
                        FOV{ 45.0f },
                        nearPlane{ 0.5f },
                        farPlane{ 100.0f }
        {}

        // Constructor for a 3D camera with custom values
        Camera3D(CameraType camType,
                 glm::vec3 pos      ,
                 glm::vec3 target   ,
                 float FOV          ,
                 float near         ,
                 float far          ) :
                 
                 camType{ camType },
                 pos{ pos },
                 target{ target },
                 FOV{ FOV },
                 nearPlane{ near },
                 farPlane{ far } 
        {}
        
        // Compute the view matrix (V) for the camera (Default up is 0.0f, 1.0f, 0.0f)
        glm::mat4 getLookAt(glm::vec3 up = { 0.0f, 1.0f, 0.0f }) const
        {
            return glm::lookAt(pos, target, up);
        }

        // Compute the perspective projection matrix (P) based on the field of view and aspect ratio (Default aspect ratio is 1)
        glm::mat4 getPerspective(float aspect = 1.0f) const
        {
            return glm::perspective(glm::radians(FOV), aspect, nearPlane, farPlane);
        }

        // Handles cursor movement to adjust camera orientation
        void cameraOnCursor(double xoffset, double yoffset/*, ShaderProgram* shader*/)
        {

            if (camType == CameraType::ORBITING)
            {
                // Calculate spherical coordinates for orbiting movement
                const float r = glm::sqrt(pos.x * pos.x +
                    pos.y * pos.y + pos.z * pos.z);
                float alpha = glm::asin(pos.y / r); // Vertical angle
                float betta = std::atan2f(pos.x, pos.z); // Horizontal angle

                // Adjust angles based on cursor offset
                if (yoffset < 0.0)
                    alpha += -0.02f;
                else if (yoffset > 0.0)
                    alpha += 0.02f;

                if (xoffset < 0.0)
                    betta += 0.05f;
                else if (xoffset > 0.0)
                    betta += -0.05f;

                // Clamp vertical angle
                alpha = glm::clamp(alpha, -MathUtils::HALF_PI + 0.01f, MathUtils::HALF_PI - 0.01f);

                // Update position based on spherical coordinates
                pos.x = r * glm::cos(alpha) * glm::sin(betta);
                pos.y = r * glm::sin(alpha);
                pos.z = r * glm::cos(alpha) * glm::cos(betta);
            }
            else if (camType == CameraType::WALKING)
            {
                // Calculate spherical coordinates for walking movement
                const float r = glm::sqrt(
                    (target.x - pos.x) * (target.x - pos.x) +
                    (target.y - pos.y) * (target.y - pos.y) +
                    (target.z - pos.z) * (target.z - pos.z));
                float alpha = glm::asin((target.y - pos.y) / r);
                float betta = std::atan2f((target.x - pos.x), (target.z - pos.z));

                // Adjust angles based on cursor offset
                if (yoffset < 0.0)
                    alpha += -0.02f;
                else if (yoffset > 0.0)
                    alpha += 0.02f;

                if (xoffset < 0.0)
                    betta += -0.05f;
                else if (xoffset > 0.0)
                    betta += 0.05f;

                // Clamp vertical angle
                alpha = glm::clamp(alpha, -MathUtils::HALF_PI + 0.01f, MathUtils::HALF_PI - 0.01f);

                // Update target based on spherical coordinates
                target.x = pos.x + r * glm::cos(alpha) * glm::sin(betta);
                target.y = pos.y + r * glm::sin(alpha);
                target.z = pos.z + r * glm::cos(alpha) * glm::cos(betta);
            }

            //// Update shader program with the new camera settings
            //shader->programUse();
            //shader->setUniform("camera.position", pos);
            //shader->programFree();
            // Unreferenced Parameter
            //shader;
        }

        // Handles scroll input to adjust zoom or camera position
        void cameraOnScroll(double yoffset/*, ShaderProgram* shader*/)
        {
            if (camType == CameraType::ORBITING)
            {
                // Calculate the distance from the origin and adjust it based on scroll input
                float r = glm::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
                const float alpha = glm::asin(pos.y / r);
                const float betta = std::atan2f(pos.x, pos.z);

                r += yoffset > 0.0f ? -1.0f : 1.0f; // Zoom in or out
                if (r < 1.0f) r = 1.0f; // Clamp minimum distance

                // Update position based on new distance
                pos.x = r * glm::cos(alpha) * glm::sin(betta);
                pos.y = r * glm::sin(alpha);
                pos.z = r * glm::cos(alpha) * glm::cos(betta);
            }
            else if (camType == CameraType::WALKING)
            {
                // Adjust position and target based on scroll input for walking
                const glm::vec3 velocity = glm::normalize(yoffset > 0.0f ? target - pos : pos - target);
                pos += velocity * glm::vec3(1.0f, 0.0f, 1.0f);
                target += velocity * glm::vec3(1.0f, 0.0f, 1.0f);
            }

            //// Update shader program with the new camera settings
            //shader->programUse();
            //shader->setUniform("camera.position", pos);
            //shader->programFree();
            
            // Unreferenced Parameter
            //shader;
        }

        //// Getters for camera data
        //CameraType getCamType() { return camType; }
        //glm::vec3& getCamPos() { return pos; }
        //glm::vec3& getCamTarget() { return target; }
        //float& getCamFOV() { return FOV; }
        //float& getCamNear() { return nearPlane; }
        //float& getCamFar() { return farPlane; }

        //// Setters for camera data
        //void setCamType(CameraType newType) {
        //    camType = newType;

        //    // Set default target if type changed to orbiting
        //    if (camType == ORBITING) {
        //        target = { 0.0f, 0.0f, 0.0f };
        //    }
        //}
        //void setCamPos(glm::vec3 newPos) { pos = newPos; }
        //void setCamTarget(glm::vec3 newTarget) { target = newTarget; }
        //void setCamFOV(float newFOV) { FOV = newFOV; }
        //void setCamNear(float newNear) { nearPlane = newNear; }
        //void setCamFar(float newFar) { farPlane = newFar; }
    };

} // end of namespace gam300
#endif // __CAMERA_H__

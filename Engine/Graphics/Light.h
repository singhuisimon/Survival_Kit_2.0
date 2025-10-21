/**
 * @file Light.h
 * @brief Definitions of the light class that influences the lighting in the game.
 * @details Manages the lighting in the editor and game engine.
 * @author Chua Wen Bin Kenny
 * @date 10 September 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __LIGHT_H__
#define __LIGHT_H__

 // Includes
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <glm/glm.hpp>                  // Core types: vec, mat
#include <glm/common.hpp>               // glm::clamp

// Mathematical operations
#include "../Utility/MathUtils.h"

// For logging information
#include "../Utility/Logger.h"

// Support setting of light uniforms
#include "../Graphics/ShaderProgram.h"


namespace Engine {

    class Light {

    private:

        // Data members of a light
        glm::vec3 pos;              // Position of the light source
        glm::vec3 light_ambient;    // Intensity for the ambient component
        glm::vec3 light_diffuse;    // Intensity for the diffuse component
        glm::vec3 light_specular;   // Intensity for the specular component

    public:

        // Default constructor for a default light
        Light() :   pos{ 0.0f, 2.0f, 0.0f},         // From the top
                    light_ambient{ 0.4f, 0.4f, 0.4f },
                    light_diffuse{ 1.0f, 1.0f, 1.0f },
                    light_specular{ 1.0f, 1.0f, 1.0f }
        {}

        // Constructor for a light with custom values
        Light(  glm::vec3 pos,
                glm::vec3 ambient,
                glm::vec3 diffuse,
                glm::vec3 specular ) : 
            
                pos{ pos },        
                light_ambient{ ambient },
                light_diffuse{ diffuse },
                light_specular{ specular }
        {}

        // Reference to light data
        glm::vec3& getLightPos() { return pos; }
        glm::vec3& getLightAmbient() { return light_ambient; }
        glm::vec3& getLightDiffuse() { return light_diffuse; }
        glm::vec3& getLightSpecular() { return light_specular; }

        // Handles cursor movement events to adjust the light's position.
        void lightOnCursor(double xoffset, double yoffset, ShaderProgram* shader/*, int i = 0*/)
        {
            const float r = glm::sqrt(pos.x * pos.x +
                pos.y * pos.y + pos.z * pos.z); // Calculate radial distance
            float alpha = glm::asin(pos.y / r); // Calculate elevation angle
            float betta = std::atan2f(pos.x, pos.z); // Calculate azimuth angle

            // Adjust angles based on cursor offsets
            alpha += yoffset > 0.0 ? -0.05f : 0.05f;
            betta += xoffset < 0.0 ? -0.05f : 0.05f;

            // Clamp alpha to avoid flipping at the poles
            alpha = glm::clamp(alpha, -MathUtils::HALF_PI + 0.01f, MathUtils::HALF_PI - 0.01f);

            // Recalculate position based on new angles
            pos.x = r * glm::cos(alpha) * glm::sin(betta);
            pos.y = r * glm::sin(alpha);
            pos.z = r * glm::cos(alpha) * glm::cos(betta);

            // Set uniform to shader after update light values
            if (shader) {
                shader->setUniform("light.position", pos);      // Position
                //shader->setUniform("light.La", light_ambient);  // Ambient
                //shader->setUniform("light.Ld", light_diffuse);  // Diffuse
                //shader->setUniform("light.Ls", light_specular); // Specular
            }
        }
    };

} // end of namespace gam300
#endif // __LIGHT_H__

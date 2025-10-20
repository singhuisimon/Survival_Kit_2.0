/**
 * @file ShaderProgram.h
 * @brief Declaration of shader program class used in the game engine for rendering scenes.
 * @details Manages the creation and usage of shader programs.
 * @author Chua Wen Bin Kenny
 * @date 10 September 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __SHADERPROGRAM_H__
#define __SHADERPROGRAM_H__

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

namespace Engine {

    class ShaderProgram {

    private:

        GLuint program_handle = 0;
        GLboolean link_status = GL_FALSE; 

    public:

        /**
        * @brief Compile the shaders, link the shader objects to create an executable,
                 and ensure the program can work in the current OpenGL state.
        * @param shader_files The data which contains the shader type and its filepath.
        * @param shader The shader program that will be created, compiled, and link.
        * @return True if shader program compile and link successfully, false otherwise.
        */
        GLboolean compileShader(std::vector<std::pair<GLenum, std::string>> shader_files);

        // Reading shader files
        bool readShaderFile(const std::string& file_path, std::string& shader_source);

        /**
         * @brief Start the shader program
         *.@param shader The shader program that is to be started
         */
        void programUse();

        /**
         * @brief Free the shader program
         */
        void programFree();

        /**
         * @brief Return the shader program handle
         * @return The program handle
         */
        GLuint getShaderProgramHandle() const;

        /**
         * @brief Return the shader program link status
         * @return The program link status
         */
        GLuint getShaderProgramLinkStatus() const;

        // Bind object's VAO handle

        /******************** Functions for setting uniforms ********************/

        /**
         * @brief Set uniform for type vec2 (float x, y)
         * @param name The name of the uniform
         * @param x The X value in float
         * @param y The Y value in float
         */
        void setUniform(const std::string& name, float x, float y);

        /**
         * @brief Set uniform for type vec3 (float x, y, z)
         * @param name The name of the uniform
         * @param x The X value in float
         * @param y The Y value in float
         * @param z The Z value in float
         */
        void setUniform(const std::string& name, float x, float y, float z);

        /**
         * @brief Set uniform for type vec4 (float x, y, z, w)
         * @param name The name of the uniform
         * @param x The X value in float
         * @param y The Y value in float
         * @param z The Z value in float
         * @param w The W value in float
         */
        void setUniform(const std::string& name, float x, float y, float z, float w);

        /**
         * @brief Set uniform for type vec2 (glm::vec2)
         * @param name The name of the uniform
         * @param v The vector in glm::vec2
         * @param cnt The count of glm::vec2 uniforms to set
         */
        void setUniform(const std::string& name, glm::vec2 v, GLsizei cnt = 1);

        /**
         * @brief Set uniform for type vec3 (glm::vec3)
         * @param name The name of the uniform
         * @param v The vector in glm::vec3
         * @param cnt The count of glm::vec3 uniforms to set
         */
        void setUniform(const std::string& name, glm::vec3 v, GLsizei cnt = 1);

        /**
         * @brief Set uniform for type vec3 (glm::vec4)
         * @param name The name of the uniform
         * @param v The vector in glm::vec4
         * @param cnt The count of glm::vec4 uniforms to set
         */
        void setUniform(const std::string& name, glm::vec4 v, GLsizei cnt = 1);

        /**
         * @brief Set uniform for type mat3 (glm::mat3)
         * @param name The name of the uniform
         * @param mat The matrix in glm::mat3
         * @param cnt The count of glm::mat3 uniforms to set
         */
        void setUniform(const std::string& name, glm::mat3 mat, GLsizei cnt = 1);

        /**
         * @brief Set uniform for type mat4 (glm::mat4)
         * @param name The name of the uniform
         * @param mat The matrix in glm::mat4
         * @param cnt The count of glm::mat4 uniforms to set
         */
        void setUniform(const std::string& name, glm::mat4 mat, GLsizei cnt = 1);

        /**
         * @brief Set uniform for float
         * @param name The name of the uniform
         * @param val The value to set
         */
        void setUniform(const std::string& name, float val);

        /**
         * @brief Set uniform for integer
         * @param name The name of the uniform
         * @param val The value to set
         */
        void setUniform(const std::string& name, int val);

        /**
         * @brief Set uniform for unsigned int
         * @param name The name of the uniform
         * @param val The value to set
         */
        void setUniform(const std::string& name, GLuint val);

        /**
         * @brief Set uniform for pointer to unsigned int
         * @param name The name of the uniform
         * @param val The value to set
         * @param cnt The count of GLuint* to set
         */
        // P.S. -> Set variable name as such: "uIndices[0]" instead of "uIndices"
        void setUniform(const std::string& name, const GLuint* val, GLsizei cnt = 1);

        /**
         * @brief Set uniform for boolean
         * @param name The name of the uniform
         * @param val The value to set
         */
        void setUniform(const std::string& name, GLboolean val);

    };

} // end of namespace gam300
#endif // __SHADERPROGRAM_H__

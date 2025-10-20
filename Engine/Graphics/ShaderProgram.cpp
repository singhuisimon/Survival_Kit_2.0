/**
 * @file ShaderProgram.cpp
 * @brief Implementation of shader program class used in the game engine for rendering scenes.
 * @details Handles the creation and usage of shader programs.
 * @author Chua Wen Bin Kenny
 * @date 10 September 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "ShaderProgram.h"

namespace Engine {

    // Create and compile shader files to shader program
    GLboolean ShaderProgram::compileShader(std::vector<std::pair<GLenum, std::string>> shader_files) {
        // Read each shader file details such as shader type and file path
        for (auto& file : shader_files) {
            // Check if file's state is good for reading
            std::string shader_source;
            if (!readShaderFile(file.second, shader_source)) {
                //LM.writeLog("ShaderProgram::compileShader: File %s has error.", file.second.c_str());
                return GL_FALSE;
            }
            //LM.writeLog("ShaderProgram::compileShader: File %s is good for reading.", file.second.c_str());

            // Create shader program
            if (program_handle <= 0) {
                program_handle = glCreateProgram();
                if (program_handle == 0) {
                    //LM.writeLog("ShaderProgram::compileShader: Cannot create program handle");
                    return GL_FALSE;
                }
                //LM.writeLog("ShaderProgram::compileShader: Program handle %u created", program_handle);
            }

            // Create shader object and load shader code with it
            GLuint shader_obj = 0;
            if (file.first == GL_VERTEX_SHADER) {
                shader_obj = glCreateShader(GL_VERTEX_SHADER);
            }
            else if (file.first == GL_FRAGMENT_SHADER) {
                shader_obj = glCreateShader(GL_FRAGMENT_SHADER);
            }
            else {
                //LM.writeLog("ShaderProgram::compileShader: Invalid shader type.");
                return GL_FALSE;
            }
            const GLchar* shader_code[] = { shader_source.c_str() };
            glShaderSource(shader_obj, 1, shader_code, NULL);

            // Compile and check if successful
            glCompileShader(shader_obj);
            GLint compile_status;
            glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &compile_status);
            if (compile_status == GL_FALSE) {
                //LM.writeLog("ShaderProgram::compileShader: Shader from file %s compilation fail.", file.second.c_str());

                return GL_FALSE;
            }
            else {
                glAttachShader(program_handle, shader_obj);
                //LM.writeLog("ShaderProgram::compileShader: Shader from file %s compilation successful.", file.second.c_str());
            }
        }

        // Check if compiled shaders in shader program is linked
        if (link_status == GL_FALSE) {
            glLinkProgram(program_handle);
            GLint status;
            glGetProgramiv(program_handle, GL_LINK_STATUS, &status); 
            if (status == GL_FALSE) {
                //LM.writeLog("ShaderProgram::compileShader: Compiled shaders failed to link.");
                return GL_FALSE;
            }
            link_status = GL_TRUE;
            //LM.writeLog("ShaderProgram::compileShader: Compiled shaders are linked successfully.");
        }

        // Check if the program created can be executed in current OpenGL state
        glValidateProgram(program_handle);
        GLint validate_status;
        glGetProgramiv(program_handle, GL_VALIDATE_STATUS, &validate_status);
        if (validate_status == GL_FALSE) {
            //LM.writeLog("ShaderProgram::compileShader: Shader program is invalid in current OpenGL state.");
            return GL_FALSE;
        }
        //LM.writeLog("ShaderProgram::compileShader: Shader program is validated and ready to execute in current OpenGL state.");
        return GL_TRUE;

    }

    // Read shader from the given filepath
    bool ShaderProgram::readShaderFile(const std::string& file_path, std::string& shader_source) {
        // Check if file's state is good for reading
        std::ifstream input_file(file_path); 
        if (input_file.good() == GL_FALSE) {
            //LM.writeLog("ShaderProgram::readShaderFile: File %s has error.", file_path.c_str());
            return false;
        }

        // Read code from shader file
        std::stringstream ss;
        ss << input_file.rdbuf();
        input_file.close();

        shader_source = ss.str();
        //LM.writeLog("ShaderProgram::readShaderFile: Successfully read shader file %s", file_path.c_str());
        return true;
    }

    // Use program with given program handle
    void ShaderProgram::programUse() {
        if (program_handle > 0) {
            glUseProgram(program_handle);
        }
    }

    // Free current program
    void ShaderProgram::programFree() { glUseProgram(0); }

    // Getter for shader program handle
    GLuint ShaderProgram::getShaderProgramHandle() const { return program_handle; }

    // Getter for shader program link status
    GLuint ShaderProgram::getShaderProgramLinkStatus() const { return link_status; }

    // Set uniform for type vec2 (float x, y)
    void ShaderProgram::setUniform(const std::string& name, float x, float y) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform2f(location, x, y);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): vec2 (float x, y) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for type vec3 (float x, y, z)
    void ShaderProgram::setUniform(const std::string& name, float x, float y, float z) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform3f(location, x, y, z);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): vec3 (float x, y, z) uniform variable doesn't exist.");
           //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for type vec4 (float x, y, z, w)
    void ShaderProgram::setUniform(const std::string& name, float x, float y, float z, float w) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform4f(location, x, y, z, w);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): vec4 (float x, y, z, w) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for type vec2 (glm::vec2)
    void ShaderProgram::setUniform(const std::string& name, glm::vec2 v, GLsizei cnt) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform2fv(location, cnt, glm::value_ptr(v));
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): vec2 (glm::vec2) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for type vec3 (glm::vec3)
    void ShaderProgram::setUniform(const std::string& name, glm::vec3 v, GLsizei cnt) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform3fv(location, cnt, glm::value_ptr(v));
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): vec3 (glm::vec3) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for type vec4 (glm::vec4)
    void ShaderProgram::setUniform(const std::string& name, glm::vec4 v, GLsizei cnt) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform4fv(location, cnt, glm::value_ptr(v));
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): vec4 (glm::vec4) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for type mat3 (glm::mat3)
    void ShaderProgram::setUniform(const std::string& name, glm::mat3 mat, GLsizei cnt) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniformMatrix3fv(location, cnt, GL_FALSE, glm::value_ptr(mat));
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): mat3 (glm::mat3) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for type mat4 (glm::mat4)
    void ShaderProgram::setUniform(const std::string& name, glm::mat4 mat, GLsizei cnt) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniformMatrix4fv(location, cnt, GL_FALSE, glm::value_ptr(mat));
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): mat4 (glm::mat4) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for float
    void ShaderProgram::setUniform(const std::string& name, float val) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform1f(location, val);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): float uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for integer
    void ShaderProgram::setUniform(const std::string& name, int val) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform1i(location, val);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): integer uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for unsigned integer
    void ShaderProgram::setUniform(const std::string& name, GLuint val) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform1ui(location, val);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): unsigned integer uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // P.S. -> Set variable name as such: "uIndices[0]" instead of "uIndices"
    void ShaderProgram::setUniform(const std::string& name, const GLuint* val, GLsizei cnt) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform1uiv(location, cnt, val);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): unsigned integer (array) uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }

    // Set uniform for boolean
    void ShaderProgram::setUniform(const std::string& name, GLboolean val) {
        GLint location = glGetUniformLocation(program_handle, name.c_str());
        if (location >= 0) {
            glUniform1i(location, val);
        }
        else {
            //LM.writeLog("ShaderProgram::setUniform(): boolean uniform variable doesn't exist.");
            //std::exit(EXIT_FAILURE);
        }
    }


} // end of namespace gam300
/**
 * @file survival_kit_obj.vert
 * @brief Declaration of the objects vertex shader for the game engine.
 * @details Manages per-vertex data of the object 
 * @author Chua Wen Bin Kenny
 * @date 25 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#version 420 core

layout(location = 0) in vec3 aPos;     // match your mesh layout
layout(location = 1) in vec3 aNormal;  // unused
layout(location = 2) in vec2 aUV;      // unused

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
    gl_Position = P * V * M * vec4(aPos, 1.0);
}

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

layout(location=0) in vec3 VertexPosition;

uniform mat4 u_World;          // Model to world Matrix
uniform mat4 u_ViewProjection; // View * Projection Matrix (This is 2 matrices concaneted)

void main()
{
    // Transform positions and normals into world position for lighting calculation
    vec3 Position = vec3(u_World * vec4(VertexPosition, 1.0));
    gl_Position = u_ViewProjection * vec4(Position, 1.0);
}

/**
 * @file survival_kit_obj.vert
 * @brief Declaration of the objects vertex shader for the game engine.
 * @details Manages per-vertex data of the object 
 * @author Chua Wen Bin Kenny
 * @date 10 September 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#version 460 core

// in/out
layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;
layout(location=2) in vec3 VertexColor;
layout(location=3) in vec2 VertexTexCoords;

out vec3 Position;
out vec3 Normal;
out vec3 Color;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 Bitangent;

// Uniforms
uniform mat4 u_World;          // Model to world Matrix
uniform mat4 u_ViewProjection; // View * Projection Matrix (This is 2 matrices concaneted)
uniform mat4 u_NormalMatrix;

void main()
{
    // Transform positions and normals into world position for lighting calculation
    Position = vec3(u_World * vec4(VertexPosition, 1.0));
    Normal   = vec3(u_NormalMatrix * vec4(VertexNormal, 1.0));
    Color    = VertexColor;
    TexCoord = VertexTexCoords;

    // Generate tangent space basis per-vertex
    vec3 N = Normal;
    
    // Choose an arbitrary up vector that's not parallel to the normal
    vec3 up = abs(N.y) > 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    
    // Generate tangent perpendicular to normal
    Tangent = normalize(cross(up, N));
    
    // Generate bitangent perpendicular to both
    Bitangent = cross(N, Tangent);

    gl_Position = u_ViewProjection * vec4(Position, 1.0);
}
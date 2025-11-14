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

uniform mat4 M; // Model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

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
    mat4 MV = V * M; // Model-View transform matrix

    mat3 N = mat3(vec3(MV[0]), vec3(MV[1]), vec3(MV[2])); // Normal transform matrix
    //Normal = normalize(N * VertexNormal);

    // Transform positions and normals into world position for lighting calculation
    Position = vec3(u_World * vec4(VertexPosition, 1.0));
    Normal   = vec3(u_NormalMatrix * vec4(VertexNormal, 1.0));
    Color    = VertexColor;
    TexCoord = VertexTexCoords;

    // Simple tangent calculation: pick a perpendicular vector
    vec3 c1 = cross(VertexNormal, vec3(0.0, 0.0, 1.0));
    vec3 c2 = cross(VertexNormal, vec3(0.0, 1.0, 0.0));
    
    // Choose the cross product that isn't near-zero
    vec3 tangent = length(c1) > length(c2) ? c1 : c2;
    tangent = normalize(tangent);
    
    vec3 bitangent = normalize(cross(VertexNormal, tangent));
    
    // Transform to view space
    Tangent = normalize(N * tangent);
    Bitangent = normalize(N * bitangent);

    vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0f);
    Position = VertexPositionInView.xyz;
    gl_Position = P * VertexPositionInView;

    // NEW PBR
    //gl_Position = u_ViewProjection * vec4(Position, 1.0);
}
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
#version 420 core

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


void main()
{

    mat4 MV = V * M; // Model-View transform matrix

    mat3 N = mat3(vec3(MV[0]), vec3(MV[1]), vec3(MV[2])); // Normal transform matrix
    Normal = normalize(N * VertexNormal);
    TexCoord = VertexTexCoords;

    vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0f);
    Position = VertexPositionInView.xyz;
    gl_Position = P * VertexPositionInView;

}
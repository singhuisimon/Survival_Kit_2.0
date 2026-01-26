#version 450 core

///////////////////////////////////////////////////////////////////////////////////////
///
/// \file     font.vert
///
/// \brief    Vertex shader for font rendering.
///
/// \authors  Tan Jun Rui [100%]
///
/// Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////////////

layout (location = 0) in vec4 vertex; 
out vec2 TexCoords;

uniform mat3 uCameraMatrix;

void main(){
    
    gl_Position = vec4(vec2(uCameraMatrix * vec3(vertex.xy, 1.f)), 0.0, 1.0);
    TexCoords = vertex.zw;
}

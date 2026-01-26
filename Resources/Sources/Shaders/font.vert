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

uniform mat4 u_Projection;

void main(){
    
    gl_Position = vec4(vec2(u_Projection * vec4(vertex.x, vertex.y, 0.f, 1.f)), 0.0, 1.0);
    TexCoords = vertex.zw;
}

#version 420 core

layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec3 VertexNormal;
layout (location=2) in vec3 VertexColor;

out vec3 Position;
out vec3 Normal;
out vec3 Color;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main(){

    mat4 MV = V * M; 
    vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0);
    gl_Position = P * VertexPositionInView;
}
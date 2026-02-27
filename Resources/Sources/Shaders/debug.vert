#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor; // normalized from RGBA8

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform vec4 u_ModelColor;

out vec4 vColor;

void main()
{
    vColor = aColor * u_ModelColor;
    gl_Position = u_ViewProjection * u_Model * vec4(aPos, 1.0);
}

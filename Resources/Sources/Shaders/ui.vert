#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 3) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 u_World2D;
uniform mat4 u_Ortho;

void main()
{
    gl_Position = u_Ortho * u_World2D * vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}
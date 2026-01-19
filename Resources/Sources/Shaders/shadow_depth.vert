#version 460 core
layout(location = 0) in vec3 aPos;

uniform mat4 u_World;
uniform mat4 u_LightViewProj;

void main()
{
    gl_Position = u_LightViewProj * u_World * vec4(aPos, 1.0);
}

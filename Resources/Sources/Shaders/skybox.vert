#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 u_SkyboxViewProjection; // The translation component is removed

void main(){

    TexCoords = aPos;
    vec4 pos = u_SkyboxViewProjection * vec4(aPos, 1.0);

    // This trick allows the skybox to always be rendered at the far plane since, perspective divide z/w gives us the depth value
    // if we set w/w it always be 1.0 therefore the skybox will always have a depth value of 1.0
    gl_Position = pos.xyww; 
}
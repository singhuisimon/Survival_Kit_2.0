#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

layout (binding = 2) uniform samplerCube skybox;

void main(){
    FragColor = texture(skybox, TexCoords) * 0.3;
}
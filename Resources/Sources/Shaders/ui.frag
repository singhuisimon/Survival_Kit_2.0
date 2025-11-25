#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout (binding = 5) uniform sampler2D Texture2D;

uniform vec4 uColor;

void main()
{
    vec4 baseColor; 

    baseColor = texture(Texture2D, TexCoords) * uColor;
    FragColor = baseColor;
}

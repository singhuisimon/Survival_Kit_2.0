#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout (binding = 4) uniform sampler2D hdrBuffer;

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    FragColor = vec4(hdrColor, 1.0);
}
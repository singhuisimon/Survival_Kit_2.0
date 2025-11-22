#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout (binding = 4) uniform sampler2D hdrBuffer;

const float gamma = 2.2;

uniform float exposure;

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
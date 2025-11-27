#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

// HDR scene buffer
layout (binding = 4) uniform sampler2D hdrTexture;

// Final bloom texture (top mip A')
layout (binding = 5) uniform sampler2D bloomTexture;

const float gamma = 2.2;

uniform float exposure;
uniform float bloomStrength;  // e.g. 0.04
uniform bool  useBloom;

void main()
{
    vec3 hdr = texture(hdrTexture,   TexCoords).rgb;
    vec3 blm = texture(bloomTexture, TexCoords).rgb;

    // Physically-based mix (pre-tonemap)
    vec3 color = hdr;

    // Apply bloom only if turned on
    if (useBloom) {
        color = mix(hdr, blm, bloomStrength);
        //color = blm; // Just render the bloom texture
    }

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * exposure);

    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
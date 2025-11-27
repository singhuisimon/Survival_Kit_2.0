#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

// Source bloom texture (smaller mip)
layout (binding = 4) uniform sampler2D srcTexture;

// Radius in texture coordinates (e.g. ~0.005)
uniform float filterRadius;

void main()
{
    float x = filterRadius;
    float y = filterRadius;

    // 3x3 tent filter:
    // 1 | 1 2 1 |
    // --*| 2 4 2 |
    //16 | 1 2 1 |

    vec3 a = texture(srcTexture, TexCoords + vec2(-x,  y)).rgb;
    vec3 b = texture(srcTexture, TexCoords + vec2( 0,  y)).rgb;
    vec3 c = texture(srcTexture, TexCoords + vec2( x,  y)).rgb;

    vec3 d = texture(srcTexture, TexCoords + vec2(-x,  0)).rgb;
    vec3 e = texture(srcTexture, TexCoords + vec2( 0,  0)).rgb;
    vec3 f = texture(srcTexture, TexCoords + vec2( x,  0)).rgb;

    vec3 g = texture(srcTexture, TexCoords + vec2(-x, -y)).rgb;
    vec3 h = texture(srcTexture, TexCoords + vec2( 0, -y)).rgb;
    vec3 i = texture(srcTexture, TexCoords + vec2( x, -y)).rgb;

    vec3 upsample = e * 4.0;
    upsample     += (b + d + f + h) * 2.0;
    upsample     += (a + c + g + i);
    upsample     *= 1.0 / 16.0;

    FragColor = vec4(upsample, 1.0);
}

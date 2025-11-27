#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

// HDR source texture (scene for first mip, previous mip for later)
layout (binding = 4) uniform sampler2D srcTexture;

// Resolution of the source texture (in pixels)
uniform vec2 srcResolution;

void main()
{
    vec2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel (COD 2014 pattern):
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i

    vec3 a = texture(srcTexture, TexCoords + vec2(-2.0 * x,  2.0 * y)).rgb;
    vec3 b = texture(srcTexture, TexCoords + vec2( 0.0 * x,  2.0 * y)).rgb;
    vec3 c = texture(srcTexture, TexCoords + vec2( 2.0 * x,  2.0 * y)).rgb;

    vec3 d = texture(srcTexture, TexCoords + vec2(-2.0 * x,  0.0 * y)).rgb;
    vec3 e = texture(srcTexture, TexCoords + vec2( 0.0 * x,  0.0 * y)).rgb;
    vec3 f = texture(srcTexture, TexCoords + vec2( 2.0 * x,  0.0 * y)).rgb;

    vec3 g = texture(srcTexture, TexCoords + vec2(-2.0 * x, -2.0 * y)).rgb;
    vec3 h = texture(srcTexture, TexCoords + vec2( 0.0 * x, -2.0 * y)).rgb;
    vec3 i = texture(srcTexture, TexCoords + vec2( 2.0 * x, -2.0 * y)).rgb;

    vec3 j = texture(srcTexture, TexCoords + vec2(-1.0 * x,  1.0 * y)).rgb;
    vec3 k = texture(srcTexture, TexCoords + vec2( 1.0 * x,  1.0 * y)).rgb;
    vec3 l = texture(srcTexture, TexCoords + vec2(-1.0 * x, -1.0 * y)).rgb;
    vec3 m = texture(srcTexture, TexCoords + vec2( 1.0 * x, -1.0 * y)).rgb;

    // Weighted distribution (energy preserving):
    // See article for derivation.
    vec3 downsample = e * 0.125;
    downsample     += (a + c + g + i) * 0.03125;
    downsample     += (b + d + f + h) * 0.0625;
    downsample     += (j + k + l + m) * 0.125;

    // Avoid exact zeros to prevent black-box artifacts (Bonus 0)
    downsample = max(downsample, vec3(0.0001));

    FragColor = vec4(downsample, 1.0);
}

#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout (binding = 6) uniform sampler2D Texture2D;

uniform float gamma; 
uniform vec4 uColor;
uniform bool uHasTexture;

void main()
{
    vec4 baseColor; 

    if(uHasTexture){
        baseColor = texture(Texture2D, TexCoords) * uColor;
    }
    else{
        baseColor = uColor;
    }

    // gamma correction 
    baseColor.rgb = pow(baseColor.rgb, vec3(1.0 / gamma));

    FragColor = baseColor;
}

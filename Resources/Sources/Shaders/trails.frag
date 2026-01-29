#version 460 core

in vec2 v_UV;
in float v_Age;

out vec4 FragColor;

uniform vec4 u_StartColor;
uniform vec4 u_EndColor;
uniform sampler2D u_TrailTexture;  // Optional gradient texture

void main() {
    // Interpolate color based on age
    vec4 color = mix(u_StartColor, u_EndColor, v_Age);
    
    // Optional: Sample gradient texture
    // vec4 texColor = texture(u_TrailTexture, v_UV);
    // color *= texColor;
    
    // Fade edges (optional soft particle effect)
    float edgeFade = smoothstep(0.0, 0.1, v_UV.y) * smoothstep(1.0, 0.9, v_UV.y);
    color.a *= edgeFade;
    
    FragColor = color;
}
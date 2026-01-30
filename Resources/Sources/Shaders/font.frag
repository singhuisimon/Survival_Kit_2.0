#version 450 core

in vec2 TexCoords;
in vec4 VertexColor;  // Added: receive color from vertex shader

out vec4 color;

uniform sampler2D u_FontAtlas;  // Renamed for clarity

void main() {   
    // Read SDF from ALPHA channel (not red!)
    float distance = texture(u_FontAtlas, TexCoords).a;
    
    // Apply smoothstep threshold
    float smoothing = fwidth(distance) * 0.5;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    
    // Use vertex color
    color = vec4(VertexColor.rgb, VertexColor.a * alpha);
}
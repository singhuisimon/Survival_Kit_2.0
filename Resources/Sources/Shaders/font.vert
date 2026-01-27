#version 450 core

// Match the C++ vertex attributes
layout (location = 0) in vec2 position;    // 2 floats
layout (location = 1) in vec2 texCoords;   // 2 floats  
layout (location = 2) in vec4 color;       // 4 floats

out vec2 TexCoords;
out vec4 VertexColor;  // Pass color to fragment shader

uniform mat4 u_Projection;  // Changed from mat3 to mat4

void main() {
    gl_Position = u_Projection * vec4(position, 0.0, 1.0);
    TexCoords = texCoords;
    VertexColor = color;
}
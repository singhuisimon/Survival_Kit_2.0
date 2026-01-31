#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Tangent;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in float a_Width;
layout(location = 4) in float a_Age;

out vec2 v_UV;
out float v_Age;

uniform mat4 u_ViewProjection;
uniform vec3 u_CameraPos;

void main() {
    // Compute billboard orientation
    vec3 toCamera = normalize(u_CameraPos - a_Position);
    vec3 right = normalize(cross(a_Tangent, toCamera));
    
    // Offset position based on UV.y (0 = left, 1 = right)
    float sideOffset = (a_UV.y - 0.5) * 2.0;  // [-1, 1]
    vec3 offsetPosition = a_Position + right * sideOffset * a_Width;
    
    gl_Position = u_ViewProjection * vec4(offsetPosition, 1.0);
    
    v_UV = a_UV;
    v_Age = a_Age;
}
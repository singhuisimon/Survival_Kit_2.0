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

// Material UV transform
uniform vec2 u_UVTiling  = vec2(1.0, 1.0);
uniform vec2 u_UVOffset  = vec2(0.0, 0.0);

// Beam UV scroll (zero for trails)
uniform float u_UVScrollOffset = 0.0;

void main()
{
    vec3 toCamera    = normalize(u_CameraPos - a_Position);
    vec3 right       = normalize(cross(a_Tangent, toCamera));

    float sideOffset    = (a_UV.y - 0.5) * 2.0;
    vec3 offsetPosition = a_Position + right * sideOffset * a_Width;

    gl_Position = u_ViewProjection * vec4(offsetPosition, 1.0);

    // Apply scroll on U, then tiling and offset
    vec2 scrolledUV = vec2(a_UV.x + u_UVScrollOffset, a_UV.y);
    v_UV  = scrolledUV * u_UVTiling + u_UVOffset;
    v_Age = a_Age;
}
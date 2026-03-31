#version 460 core

in vec2 v_UV;
in float v_Age;

out vec4 FragColor;

// Age color gradient (trail/beam tint)
uniform vec4 u_StartColor;
uniform vec4 u_EndColor;

// Base map
layout(binding = 15) uniform sampler2D u_BaseMap;
uniform bool      u_HasBaseMap  = false;
uniform vec3      u_BaseColor   = vec3(1.0);

// Emission
layout(binding = 16) uniform sampler2D u_EmissionMap;
uniform bool      u_HasEmissionMap    = false;
uniform vec3      u_EmissionColor     = vec3(0.0);
uniform float     u_EmissionStrength  = 0.0;
uniform bool      u_EnableEmission    = false;

// Opacity
uniform float u_Opacity = 1.0;

void main()
{
    // --- Age-based color lerp (trail fade / beam color ramp) ---
    vec4 ageColor = mix(u_StartColor, u_EndColor, v_Age);

    // --- Base color ---
    vec4 baseColor = vec4(u_BaseColor, 1.0);
    if (u_HasBaseMap)
        baseColor *= texture(u_BaseMap, v_UV);

    // Modulate base by age color
    vec4 color = baseColor * ageColor;

    // --- Emission ---
    if (u_EnableEmission || u_HasEmissionMap)
    {
        vec3 emission;
        if (u_HasEmissionMap)
            // Map drives the color, EmissionColor tints it, strength scales it
            emission = texture(u_EmissionMap, v_UV).rgb * u_EmissionColor * u_EmissionStrength;
        else
            // No map, use flat color * strength
            emission = u_EmissionColor * u_EmissionStrength;

        color.rgb += emission;
    }

    // --- Opacity ---
    color.a *= u_Opacity;

    // --- Edge fade (soft sides) ---
    float edgeFade = smoothstep(0.0, 0.1, v_UV.y) * smoothstep(1.0, 0.9, v_UV.y);
    color.a *= edgeFade;

    FragColor = color;
}
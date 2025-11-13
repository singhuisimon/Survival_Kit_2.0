/**
 * @file survival_kit_obj.frag
 * @brief Declaration of the objects fragment shader for the game engine.
 * @details Manages per-fragment data of the object 
 * @author Chua Wen Bin Kenny
 * @date 10 September 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#version 460 core

in vec3 Position;   // view-space position
in vec3 Normal;     // view-space normal
in vec3 Color;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;

layout(location=0) out vec4 FragColor;

// ===== Material UBO (std140 block, binding = 1) =====
struct Material {
    vec3 Ka; float _pad0;
    vec3 Kd; float _pad1;
    vec3 Ks; float shininess;
};
layout(std140, binding = 1) uniform MaterialBlock {
    Material material;
};

// ===== Lights UBO (binding = 0), matches renderer =====
const uint LIGHT_DIRECTIONAL = 0u;
const uint LIGHT_POINT       = 1u;
const uint LIGHT_SPOT        = 2u;

struct LightGPU {
    vec4 color_intensity; // rgb + intensity
    vec4 position_range;  // xyz (world) + range
    vec4 direction_type;  // xyz (world dir) + type (float)
    vec4 spot_cos_misc;   // x=cosInner, y=cosOuter, z=indirect, w=unused
};
layout(std140, binding = 0) uniform LightsBlock {
    vec4  ambient_indirect; // rgb ambient, a = global indirect multiplier
    uvec4 count;             // x = lightCount, yzw unused
    LightGPU lights[64];
};

// ===== Engine uniforms kept =====
uniform mat4 V;               
uniform bool isPBR;
uniform bool isGamma;
uniform bool isTexture;
uniform bool useNormalMap;
layout(binding = 0) uniform sampler2D Texture2D;
layout(binding = 1) uniform sampler2D NormalMap;

// ===== PBR constants =====
const float PI = 3.14159265358979323846;
const float ROUGH = 0.3f;
const float METAL = 0.0f;

// ===== Helpers =====
float saturate(float x){ return clamp(x,0.0,1.0); }

// Softer attenuation so point/spot reach further but still fade out at range
float AttenuationSoft(float dist, float range) {
    float r = max(range, 1e-4);        // never 0
    float x = clamp(dist / r, 0.0, 1.0);  // 0..1 within range
    // Soft rolloff that reaches 0 at d == range, with no 1/d^2 term
    float smoothing = (1.0 - x);          // linear
    return smoothing * smoothing;            // quadratic falloff
}

float SpotFade(vec3 Ldir_view, vec3 spotDir_view, float cosInner, float cosOuter)
{
    // Ldir_view is surface->light; compare opposite with axis (light->surface)
    float cd = dot(-normalize(Ldir_view), normalize(spotDir_view));
    return saturate(smoothstep(cosOuter, cosInner, cd));
}

// ===== GGX helpers =====
float ggxDistribution(float nDotH)
{
    float a2 = ROUGH * ROUGH * ROUGH * ROUGH;
    float d  = (nDotH * nDotH) * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * d * d);
}
float geomSmith(float nDotX)
{
    float k = (ROUGH + 1.0f) * (ROUGH + 1.0f) / 8.0f;
    float denom = nDotX * (1.0f - k) + k;
    return 1.0f / denom;
}
vec3 schlickFresnel(float lDotH)
{
    vec3 f0 = vec3(0.04f);
    if (METAL == 1.0f) f0 = material.Kd;
    return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5.0f);
}

// ---- Single-light BRDFs in VIEW space  ----
vec3 BRDF_BlinnPhong_View(vec3 N, vec3 L, vec3 Vview, vec3 albedo, float shininess, float specStrength)
{
    
    // Compute half vector of vector to light and view vector
    vec3  H   = normalize(Vview + L);

    // Compute radiant energy
    float ndl = max(dot(N, L), 0.0);

    // Compute specular
    float ndh = max(dot(N, H), 0.0);
    float spec = pow(ndh, shininess) * specStrength;

    return albedo * ndl + vec3(spec);
}

vec3 BRDF_Microfacet_View(vec3 N, vec3 Ldir, vec3 Vview, vec3 albedo)
{
    vec3 H    = normalize(Vview + Ldir);
    float nDotH = max(dot(N, H), 0.0);
    float lDotH = max(dot(Ldir, H), 0.0);
    float nDotL = max(dot(N, Ldir), 0.0);
    float nDotV = max(dot(N, Vview), 0.0);

    vec3  spec = 0.25f * ggxDistribution(nDotH) * schlickFresnel(lDotH)
                           * geomSmith(nDotL) * geomSmith(nDotV);
    return (albedo + PI * spec) * nDotL;
}

void main()
{
    vec3 N;
    if (useNormalMap) {
        // Construct TBN matrix in fragment shader
        mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(Normal));
        
        // Sample and remap normal map
        vec3 normalSample = texture(NormalMap, TexCoord).rgb;
        normalSample = normalSample * 2.0 - 1.0;
        
        // Transform to view space
        N = normalize(TBN * normalSample);
    } else {
        N = normalize(Normal);
    }

    vec3 Vview  = normalize(-Position);   // camera at origin in view space

    // Albedo (linear)
    vec3 albedo = material.Kd;
    if (isTexture) {
        vec3 tex = texture(Texture2D, TexCoord).rgb;
        if (isGamma) tex = pow(tex, vec3(2.2));   // sRGB -> linear
        albedo = mix(material.Kd, tex, 0.85); // Material - Texture (0.0 - 1.0)
    }
    float shininess    = material.shininess;
    float specStrength = max(max(material.Ks.r, material.Ks.g), material.Ks.b);

    // Ambient computation
    vec3 accum = ambient_indirect.rgb * ambient_indirect.a;

    // Light loop
    uint lightCount = count.x;

    // Include all lights in calculation
    for (uint i = 0u; i < lightCount; ++i)
    {
        LightGPU Lg = lights[i];

        vec3  color     = Lg.color_intensity.rgb;
        float intensity = Lg.color_intensity.a;

        // World -> View (Get light direction and position in view space)
        vec3 dir_view = normalize(mat3(V) * Lg.direction_type.xyz); 
        vec3 pos_view = (V * vec4(Lg.position_range.xyz, 1.0)).xyz;

        // Get light type
        uint type = uint(Lg.direction_type.w + 0.5);

        // Light attributes
        vec3 Ldir;              // Light direction
        float attenuation = 1.0;
        float angular     = 1.0;

        // Set up lights based on type
        if (type == LIGHT_DIRECTIONAL)
        {
            // CPU stored light->scene direction; we need surface->light
            Ldir = normalize(-dir_view);
        } else {

            // Surface -> light (view space)
            vec3 toLight = pos_view - Position;      
            float d = length(toLight);
            if (d >= Lg.position_range.w) continue;  // Cull by light range
            Ldir = toLight / max(d, 1e-4);

            // Soft attenuation to create nicer falloff
            attenuation = AttenuationSoft(d, Lg.position_range.w);

            // Calculation of cone for spot light
            if (type == LIGHT_SPOT)
            {
                float cosIn  = Lg.spot_cos_misc.x;
                float cosOut = Lg.spot_cos_misc.y;
                angular = SpotFade(Ldir, dir_view, cosIn, cosOut);
                if (angular <= 0.0) continue;
            }
        }

        // Compute radiance for current light
        vec3 radiance = color * intensity;

        // Determine accumulated BRDF with (1) radiance, (2) attenuation, (3) angular
        if (isPBR) {
            // PBR
            accum += BRDF_Microfacet_View(N, Ldir, Vview, albedo) * radiance * attenuation * angular;
        } else {
            // Blinn-Phong
            accum += BRDF_BlinnPhong_View(N, Ldir, Vview, albedo, shininess, specStrength) * radiance * attenuation * angular;
        }
    }

    // === Gamma out ===
    vec3 outColor = accum;
    if (isGamma) {
        outColor = pow(outColor, vec3(1.0/2.2));
    }

    FragColor = vec4(outColor, 1.0);
}

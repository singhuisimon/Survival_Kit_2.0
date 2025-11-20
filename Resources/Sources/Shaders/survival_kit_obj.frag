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

// ==== In/Outs ====
in vec3 Position;   // World-space position
in vec3 Normal;     // World-space normal
in vec3 Color;      // Unused
in vec2 TexCoord;   
in vec3 Tangent;    // Currently unused
in vec3 Bitangent;  // Currenty unused

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

struct Material_
{
    vec3  albedo;
    vec3  emissionColor;
    float emissionStrength;
    float metallic;
    float roughness;
    float ao;
    float opacity;
};

uniform Material_ material_;


// ===== Engine uniforms kept =====      
uniform vec3 CamPos;       
uniform bool isGamma;
uniform bool isTexture;
uniform bool useNormalMap;

layout(binding = 0) uniform sampler2D Texture2D;
layout(binding = 1) uniform sampler2D NormalMap;

// ===== PBR constants =====
const float PI = 3.14159265358979323846;

// ===== Helpers =====
float saturate(float x){ return clamp(x,0.0,1.0); }

// Softer attenuation so point/spot reach further but still fade out at range
float AttenuationSoft(float dist, float range) {
    float r = max(range, 1e-4);
    float x = clamp(dist / r, 0.0, 1.0);
    float smoothing = (1.0 - x);
    return smoothing * smoothing;
}

float SpotFade(vec3 LDir, vec3 SpotDir, float cosInner, float cosOuter)
{
    float cd = dot(-normalize(LDir), normalize(SpotDir));
    return saturate(smoothstep(cosOuter, cosInner, cd));
}

////////////////////////////////////////////////////////////////////
// Distribution (GGX/Trowbridge-Reitz)
////////////////////////////////////////////////////////////////////
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

////////////////////////////////////////////////////////////////////
// Geometry (Smith's Schlick-GGX)
////////////////////////////////////////////////////////////////////
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

////////////////////////////////////////////////////////////////////
// Fresnel (Schlick approximation)
////////////////////////////////////////////////////////////////////
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // ===== Normal computation =====
    vec3 N;
    if (useNormalMap) {
        mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(Normal));
        vec3 normalSample = texture(NormalMap, TexCoord).rgb;
        normalSample = normalSample * 2.0 - 1.0;
        N = normalize(TBN * normalSample);
    } else {
        N = normalize(Normal);
    }

    // ===== View direction =====
    vec3 V = normalize(CamPos - Position);

    // ===== Material properties =====
    vec3 albedo = material_.albedo;
    if (isTexture) {
        vec3 tex = texture(Texture2D, TexCoord).rgb;
        if (isGamma) {
            tex = pow(tex, vec3(2.2)); // sRGB -> linear
        }
        albedo *= tex;
    }
    
    float roughness = material_.roughness;
    float metallic = material_.metallic;
    float ao = material_.ao;

    // ===== F0 for Fresnel =====
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // ===== Reflectance equation =====
    vec3 Lo = vec3(0.0);
    
    uint lightCount = count.x;
    for (uint i = 0u; i < lightCount; ++i)
    {
        LightGPU Lg = lights[i];

        vec3  color     = Lg.color_intensity.rgb;
        float intensity = Lg.color_intensity.a;
        vec3 dir = normalize(Lg.direction_type.xyz); 
        vec3 pos = Lg.position_range.xyz;
        uint type = uint(Lg.direction_type.w + 0.5);

        // ===== Light direction setup =====
        vec3 L;
        float attenuation = 1.0;
        float angular = 1.0;

        if (type == LIGHT_DIRECTIONAL)
        {
            L = normalize(-dir);
        } 
        else 
        {
            vec3 toLight = pos - Position;      
            float distance = length(toLight);
            
            if (distance >= Lg.position_range.w) continue;
            
            L = toLight / max(distance, 1e-4);
            attenuation = AttenuationSoft(distance, Lg.position_range.w);

            if (type == LIGHT_SPOT)
            {
                float cosIn  = Lg.spot_cos_misc.x;
                float cosOut = Lg.spot_cos_misc.y;
                angular = SpotFade(L, dir, cosIn, cosOut);
                if (angular <= 0.0) continue;
            }
        }

        // ===== Cook-Torrance BRDF =====
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        
        // Skip if light is below surface
        if (NdotL <= 0.0) continue;
        
        float NdotV = max(dot(N, V), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        // Calculate radiance
        vec3 radiance = color * intensity * attenuation * angular;

        // Cook-Torrance BRDF components
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(HdotV, F0);

        // Energy conservation
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        // Specular term
        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        // Add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ===== Ambient lighting =====
    vec3 ambient = ambient_indirect.rgb * albedo * ao * ambient_indirect.a;
    vec3 color = ambient + Lo + (material_.emissionColor * material_.emissionStrength);

    // ===== Tone mapping (Reinhard) =====
    color = color / (color + vec3(1.0));

    // ===== Gamma correction =====
    if (isGamma) {
        color = pow(color, vec3(1.0/2.2));
    }

    FragColor = vec4(color, material_.opacity);
}

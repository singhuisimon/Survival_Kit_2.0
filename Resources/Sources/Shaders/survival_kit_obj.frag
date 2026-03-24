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

const float FOG_MINDIST = 5.0f;
const float FOG_MAXDIST = 5000.0f;
const vec3 FOG_COLOR = vec3(0,0,0);


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

// ===== Shadows UBO (binding = 2), matches renderer =====
layout(std140, binding = 2) uniform ShadowsBlock
{
    mat4 lightViewProj[4];
    vec4 cascadeSplits;     // future use
    vec4 shadowParams;      // x=strength, y=bias, z=texelSize(1/res), w=shadowType(0/1/2)
    vec4 lightDirEnabled;   // xyz=lightDir(world), w=enabled
};

uniform sampler2DShadow u_ShadowMap;
uniform int u_ReceiveShadows;

struct Material_
{
    vec3  albedo;
    vec3  emissionColor;
    float emissionStrength;
    float metallic;
    float roughness;
    float ao;
    float opacity;
    float textureOffsetX;
    float textureOffsetY;
    float textureTileX;
    float textureTileY;
};

uniform Material_ material_;

uniform vec4 uColor;
uniform bool uParticle;
uniform bool uEmissive;
uniform bool uBlacksAsTransparent;

// ===== Engine uniforms kept =====      
uniform vec3 CamPos;       
uniform bool isTexture;
uniform bool useNormalMap;
uniform bool hasMetallicMap;
uniform bool hasRoughnessMap;
uniform bool hasEmissionMap;
uniform bool hasOcclusionMap;

layout(binding = 0) uniform sampler2D Texture2D;
layout(binding = 1) uniform sampler2D NormalMap;
layout(binding = 11) uniform sampler2D metallicMap;
layout(binding = 12) uniform sampler2D roughnessMap;
layout(binding = 13) uniform sampler2D emissionMap;
layout(binding = 14) uniform sampler2D occlusionMap;

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

////////////////////////////////////////////////////////////////////
// Shadow sampling helper (Hard + Soft PCF)
////////////////////////////////////////////////////////////////////
float ComputeShadowFactor(vec3 worldPos, vec3 normalWS, vec3 lightDirWS)
{
    if (lightDirEnabled.w < 0.5) return 0.0;
    if (u_ReceiveShadows == 0)   return 0.0;

    vec4 ls = lightViewProj[0] * vec4(worldPos, 1.0);
    vec3 proj = ls.xyz / ls.w;
    proj = proj * 0.5 + 0.5;

    // outside shadow map => no shadow
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0)
        return 0.0;

    float strength = shadowParams.x;
    float baseBias = shadowParams.y;
    float texel    = shadowParams.z;
    float type     = shadowParams.w; // 1=Hard, 2=Soft

    // slope-scaled receiver bias (reduces acne without huge peter-panning)
    float ndotl = max(dot(normalWS, lightDirWS), 0.0);
    float bias  = max(baseBias * (1.0 - ndotl), baseBias * 0.25);

    float visibility = 1.0;

    if (type < 1.5) {
        // Hard
        visibility = texture(u_ShadowMap, vec3(proj.xy, proj.z - bias));
    } else {
        // Soft 3x3 PCF
        float sum = 0.0;
        for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
        {
            vec2 offset = vec2(x, y) * texel;
            sum += texture(u_ShadowMap, vec3(proj.xy + offset, proj.z - bias));
        }
        visibility = sum / 9.0;
    }

    float shadow = (1.0 - visibility) * strength;
    return shadow;
}

void main()
{
    // ===== Normal computation =====
    vec3 N;
    vec2 offset = vec2(material_.textureOffsetX, material_.textureOffsetY);
    vec2 tiling = vec2(material_.textureTileX, material_.textureTileY);
    if (useNormalMap) {
        mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(Normal));
        vec3 normalSample = texture(NormalMap, TexCoord * tiling + offset).rgb;
        normalSample = normalSample * 2.0 - 1.0;
        N = normalize(TBN * normalSample);
    } else {
        N = normalize(Normal);
    }

    // ===== View direction =====
    vec3 V = normalize(CamPos - Position);

    // ===== Material properties =====
    vec3 albedo = material_.albedo;
    float texAlpha = 1.0;
    if (isTexture) {
        vec4 texSample = texture(Texture2D, TexCoord * tiling + offset);
        albedo *= texSample.rgb;

        if(uBlacksAsTransparent){
            // Convert luminance to alpha - black becomes transparent
            float luminance = dot(texSample.rgb, vec3(0.299, 0.587, 0.114));
            texAlpha = luminance;  // Black (0,0,0) = 0.0 alpha, White (1,1,1) = 1.0 alpha
        }
        else{
            texAlpha = texSample.a;
        }

    }

    if (uParticle) {
        albedo *= uColor.rgb;        // Tint the albedo
        texAlpha *= uColor.a;        // Modulate texture alpha
    }
    
    float roughness = material_.roughness;
    if(hasRoughnessMap){
        roughness = texture(roughnessMap, TexCoord * tiling + offset).r;
    }
    float metallic = material_.metallic;
    if(hasMetallicMap){
        metallic = texture(metallicMap, TexCoord * tiling + offset).r;
    }

    float ao = material_.ao;
    if(hasOcclusionMap){
        ao = texture(occlusionMap, TexCoord * tiling + offset).r;
    }
    
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
        vec3 pos = Lg.position_range.xyz; // Light position, xyz
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

//        // Add to outgoing radiance Lo
//        Lo += (kD * albedo / PI + specular) * radiance * NdotL;

        // Shadowing: apply only to the shadow-casting directional light for now
        float shadow = 0.0;
        if (type == LIGHT_DIRECTIONAL)
        {
            // ComputeShadowFactor returns [0..strength]
            shadow = ComputeShadowFactor(Position, N, L);
        }

        vec3 direct = (kD * albedo / PI + specular) * radiance * NdotL;
        direct *= (1.0 - shadow);
        Lo += direct;
    }

    // ===== Ambient lighting =====
    vec3 ambient = ambient_indirect.rgb * albedo * ao * ambient_indirect.a;

    vec3 color = ambient + Lo;
    if(uEmissive){
        if(hasEmissionMap){
            color += (texture(emissionMap, TexCoord * tiling + offset).rgb) * material_.emissionStrength;
        }else{
            if (uParticle) {
                color += (uColor.rgb * material_.emissionStrength);
            } else {
                color += (material_.emissionColor * material_.emissionStrength);
            }
        }
    }

    //vec3 color = ambient + Lo + (material_.emissionColor * material_.emissionStrength);

    float finalAlpha = material_.opacity * texAlpha;

    // Discard fully transparent fragments
    if (finalAlpha <= 0.01) {
        discard;
    }

    // Compute fog factor and mix it into the background
    float dist = length(Position - CamPos);
    float fogFactor = clamp((dist - FOG_MINDIST) / (FOG_MAXDIST - FOG_MINDIST), 0.0f, 1.0f);
    color = mix(color , FOG_COLOR, fogFactor);

    FragColor = vec4(color, finalAlpha);

}

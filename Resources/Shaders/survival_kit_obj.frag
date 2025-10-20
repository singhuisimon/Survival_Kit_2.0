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

#version 420 core
//
struct Material 
{
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess factor
};

// For PBR (Temporary hardcoded values, will add into materials subsequently)
const float ROUGH = 0.3f;                              // Roughness
const float METAL = 0.0f;                              // Metallic (1.0f) or dielectric (0.0f)
const vec3 PBR_COLOR = vec3(1.0f, 0.0f, 0.0f);         // Diffuse color for metal (Now replaced by material kd)
//float effect;                                  // Additional effect (discard, cartoon, etc)

struct Light 
{
    vec3 position;      // Position of the light source in the world space
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

in vec3 Position;       // In view space
in vec3 Normal;         // In view space
in vec3 Color;         
in vec2 TexCoord;


////////// Uniforms and Constants //////////
uniform Light light;
uniform Material material;
uniform mat4 V;         // View transform matrix
const float PI = 3.14159265358979323846;
uniform bool isPBR;
uniform bool isGamma;

// For handling textures
uniform bool isTexture;
layout(binding=0) uniform sampler2D Texture2D;

layout(location=0) out vec4 FragColor;


/////////////////////////////////// Functions /////////////////////////////////////////

// The Microgeometry Normal Distribution Function, based on GGX/Trowbrodge-Reitz, 
// that describes the relative concentration of microfacet normals 
// in the direction H. It has an effect on the size and shape 
// of the specular highlight.
//
// Parameter is cosine of the angle between the normal and H which is the halfway vector of 
// both the light direction and the view direction
//
float ggxDistribution(float nDotH) 
{
    float alpha2 = ROUGH * ROUGH * ROUGH * ROUGH;
    float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (PI * d * d);
}

//
// The Smith Masking-Shadowing Function describes the probability that microfacets with 
// a given normal are visible from both the light direction and the view direction.
//
// Parameter is cosine of the angle between the normal vector and the view direction.
//
float geomSmith(float nDotL) 
{
    float k = (ROUGH + 1.0f) * (ROUGH + 1.0f) / 8.0f;
    float denom = nDotL * (1.0f - k) + k;
    return 1.0f / denom;
}

//
// Schlick approximation for Fresnel reflection that defines the probability of light
// reflected from an optically flat surface.
//
// Parameter is cosine of the angle between the light direction vector and 
// the halfway vector of both the light direction and the view direction
//
vec3 schlickFresnel(float lDotH) 
{
    vec3 f0 = vec3(0.04f); // Dielectrics
    if (METAL == 1.0f)
        f0 = material.Kd;
    return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5);
}

//
// Bidirectional Reflectance Distribution Function.
// This is the common way to model reflectance based on microfacet theory. 
// This theory was developed to describe reflection from general, non-optically flat surfaces. 
// It models the surface as consisting of small facets that are optically flat (mirrors) and 
// are oriented in various directions. Only those that are oriented correctly to reflect toward 
// the viewer can contribute.
//
// Parameters are the position of a fragment and the surface normal in the view space.
//
vec3 microfacetModel(vec3 position, vec3 n) 
{  
    vec3 diffuseBrdf = material.Kd;

    vec3 lightI = light.La;
    vec3 lightPositionInView = (V * vec4(light.position, 1.0f)).xyz;

    vec3 l = lightPositionInView - position;
    float dist = length(l);
    l = normalize(l);
    lightI *= 100 / (dist * dist); // Intensity is normalized, so scale up by 100 first

    vec3 v = normalize(-position);
    vec3 h = normalize(v + l);
    float nDotH = dot(n, h);
    float lDotH = dot(l, h);
    float nDotL = max(dot(n, l), 0.0f);
    float nDotV = dot(n, v);
    vec3 specBrdf = 0.25f * ggxDistribution(nDotH) * schlickFresnel(lDotH) 
                            * geomSmith(nDotL) * geomSmith(nDotV);

    return (diffuseBrdf + PI * specBrdf) * lightI * nDotL;
}

vec3 BlinnPhong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
{

    // Transform light position from world to view space
    vec3 LightInViewSpace = vec3(view * vec4(light.position, 1.0));

    // Return 0 if light in view space is same as position
    if (LightInViewSpace == position) {
        return vec3(0.0);
    }

    // Compute vector to light normal
    vec3 vectorL = normalize(LightInViewSpace - position); 
    
    // Compute view vector (camera is at vec3(0,0,0) if position is in view space
    vec3 vectorV = normalize(-position);

    // Compute half vector of vector to light and view vector
    vec3 vectorH = normalize(vectorV + vectorL);
    
    // Compute ambient light contribution
    vec3 ambient = light.La * material.Ka;

    // Compute diffuse light contribution
    float radiantEnergy = max(dot(normal, vectorL), 0.0);
    vec3 diffuse = light.Ld * material.Kd * radiantEnergy;

    // Compute specular light contribution
    vec3 specular = vec3(0.0);
    if (0.0 < radiantEnergy) {
        specular = light.Ls * material.Ks * pow(max(dot(normal, vectorH), 0.0), material.shininess);  
    }

    // Compute final illumination
    vec3 illumination = ambient + diffuse + specular; 
    
    return illumination;
}

/////////////////////////////////// Functions /////////////////////////////////////////

void main() 
{
    // Set default to BlinnPhong
    vec3 shadeColor = BlinnPhong(Position, Normal, light, material, V); 
    
    // Set PBR if toggled
    if(isPBR) {
        shadeColor = microfacetModel(Position, normalize(Normal));
    }

    if(isTexture){
        vec4 texColor = texture(Texture2D, TexCoord);
        float texture_factor = 0.7f; // Texture 1.0f - 0.0f shadeColor
        shadeColor = mix(shadeColor, texColor.xyz, texture_factor);
    }

    // Gamma correction
    if(isGamma) {
        // RGB to sRGB
        FragColor = vec4(pow(shadeColor, vec3(1.0/2.2)), 1.0);
    } else {
        FragColor = vec4(shadeColor, 1.0);
    }

}
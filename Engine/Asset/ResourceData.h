/**
* @file ResourceData.h
* @brief Resource properties definitions
*	Defines property structure for different resource types (descriptor files data)
* @author Wai Lwin Thit
* @date September 11 2025
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/

#pragma once
#ifndef __RESOURCE_DATA_H__
#define __RESOURCE_DATA_H__

#include "ResourceTypes.h"
#include "../include/xresource_mgr.h"
#include <string>
#include <array>
#include <vector>

namespace Engine {

    // Forward declaration
    class ResourceManager;

    // ========== RUNTIME RESOURCE DATA STRUCTURES ==========

    /**
     * @brief Runtime texture resource data.
     */
    struct TextureResource {
        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned int textureID = 0;  // OpenGL texture ID
        std::string format;
    };


    /**
	 * @brief Sub-mesh descriptor for mesh resources.
     */
    struct SubMeshDescriptor {
        uint32_t startIndex;
        uint32_t indexCount;
        uint32_t materialId;
        char name[64];
        uint32_t reserved[4];
    };

    /**
     * @brief Runtime mesh resource data.
     */
    struct MeshResource {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
		std::vector<SubMeshDescriptor> subMeshes;

        unsigned int VAO = 0;  // Vertex Array Object
        unsigned int VBO = 0;  // Vertex Buffer Object
        unsigned int EBO = 0;  // Element Buffer Object

    };

    /**
     * @brief Runtime material resource data.
     */
    struct MaterialResource {

        std::string shaderName; // Tells the material what shader to use

        // Texture Maps
		xresource::instance_guid baseMap;           // Albedo
		xresource::instance_guid normalMap;         // For normal mapping
		xresource::instance_guid metallicMap;       
        xresource::instance_guid roughnessMap;     
        xresource::instance_guid emissionMap;       // If emission is enabled
		xresource::instance_guid occlusionMap;      // Ambient Occlusion map

		// Color properties
        std::array<float, 3> baseColor    =  { 1.0f, 1.0f, 1.0f };        // White, fully opaque
        std::array<float, 3> emissionColor = { 0.0f, 0.0f, 0.0f };        // Black (no emission)

		float                    metallic = 0.f;
        float                    roughness = 0.f;
        float                    emissionStrength = 1.f;  
		float                    alphaThreshold   = 0.5f;	  // For alpha testing
        float                    opacity          = 1.0f;
		float                    ambientOcclusion = 0.5f;      // Ambient occlusion intensity

        // UV transforms
		std::array<float, 2>     tiling = {1.f, 1.f};        // UV scale
		std::array<float, 2>     offset = {0.f, 0.f};        // UV offset

        bool enableEmission = false;
        bool alphaTest = false;

        // Render flags
        bool doubleSided    = false;
        bool receiveShadows = true;
        bool castShadows    = true;
    };

    /**
     * @brief Runtime audio resource data.
     */
    struct AudioResource {
        std::vector<char> audioData;
        int sampleRate = 44100;
        int channels = 2;
        int bitDepth = 16;
        unsigned int bufferID = 0;  // OpenAL buffer ID

    };

    /**
     * @brief Runtime shader resource data.
     */
    struct ShaderResource {
        unsigned int programID = 0;  // OpenGL shader program ID
        std::string vertexSource;
        std::string fragmentSource;
        std::string geometrySource;

    };

} // namespace Engine

// ========== XRESOURCE_MGR LOADER SPECIALIZATIONS ==========
/**
 * @brief Texture loader specialization.
 */
template<>
struct xresource::loader<Engine::ResourceGUID::texture_type_guid_v> {
    constexpr static inline auto type_name_v = L"Texture";
    using data_type = Engine::TextureResource;
    constexpr static inline auto use_death_march_v = false;

    static data_type* Load(xresource::mgr& mgr, const full_guid& guid);
    static void Destroy(xresource::mgr& mgr, data_type&& data, const full_guid& guid);
};

/**
 * @brief Mesh loader specialization.
 */
template<>
struct xresource::loader<Engine::ResourceGUID::mesh_type_guid_v> {
    constexpr static inline auto type_name_v = L"Mesh";
    using data_type = Engine::MeshResource;
    constexpr static inline auto use_death_march_v = false;

    static data_type* Load(xresource::mgr& mgr, const full_guid& guid);
    static void Destroy(xresource::mgr& mgr, data_type&& data, const full_guid& guid);
};

/**
 * @brief Material loader specialization.
 */
template<>
struct xresource::loader<Engine::ResourceGUID::material_type_guid_v> {
    constexpr static inline auto type_name_v = L"Material";
    using data_type = Engine::MaterialResource;
    constexpr static inline auto use_death_march_v = false;

    static data_type* Load(xresource::mgr& mgr, const full_guid& guid);
    static void Destroy(xresource::mgr& mgr, data_type&& data, const full_guid& guid);
};

/**
 * @brief Audio loader specialization.
 */
template<>
struct xresource::loader<Engine::ResourceGUID::audio_type_guid_v> {
    constexpr static inline auto type_name_v = L"Audio";
    using data_type = Engine::AudioResource;
    constexpr static inline auto use_death_march_v = true;  // Use death march for audio

    static data_type* Load(xresource::mgr& mgr, const full_guid& guid);
    static void Destroy(xresource::mgr& mgr, data_type&& data, const full_guid& guid);
};

/**
 * @brief Shader loader specialization.
 */
template<>
struct xresource::loader<Engine::ResourceGUID::shader_type_guid_v> {
    constexpr static inline auto type_name_v = L"Shader";
    using data_type = Engine::ShaderResource;
    constexpr static inline auto use_death_march_v = false;

    static data_type* Load(xresource::mgr& mgr, const full_guid& guid);
    static void Destroy(xresource::mgr& mgr, data_type&& data, const full_guid& guid);
};

// ========== LOADER REGISTRATIONS ==========
// These register the loaders with xresource_mgr
inline xresource::loader_registration<Engine::ResourceGUID::texture_type_guid_v> texture_loader;
inline xresource::loader_registration<Engine::ResourceGUID::mesh_type_guid_v> mesh_loader;
inline xresource::loader_registration<Engine::ResourceGUID::material_type_guid_v> material_loader;
inline xresource::loader_registration<Engine::ResourceGUID::audio_type_guid_v> audio_loader;
inline xresource::loader_registration<Engine::ResourceGUID::shader_type_guid_v> shader_loader;


#endif // __RESOURCE_DATA_H__
#pragma once
#ifndef __RESOURCE_HELPERS_H__
#define __RESOURCE_HELPERS_H__

//External libraries 
#include "../xresource_guid/include/xresource_guid.h"

//c++ libraries 
#include <string>
#include <filesystem>


//engine files
#include "ResourceTypes.h"
#include "../Utility/AssetPath.h"
#include "AssetManager.h"

namespace Engine {
    /**
     * @brief Convert an instance_guid to a full_guid with the appropriate type_guid
     * @param instance The instance_guid to convert
     * @param type The resource type (MESH, TEXTURE, etc.)
     * @return A full_guid with both instance and type information
     *
     * @example
     *   auto instance = AM.getGuidFromName("cube.fbx");
     *   auto full = convertToFullGuid(instance, ResourceType::MESH);
     *   MeshResource* mesh = RM.loadResource<MeshResource>(full);
     */
    inline xresource::full_guid convertToFullGuid(xresource::instance_guid instance, ResourceType type) {
        xresource::full_guid result;
        result.m_Instance = instance;
        result.m_Type = ResourceGUID::getTypeGUID(type);
        return result;
    }

    /**
     * @brief Overload: Convert instance_guid to full_guid for MESH resources
     * @param instance The instance_guid to convert
     * @return A full_guid configured for mesh resources
     */
    inline xresource::full_guid convertToMeshGuid(xresource::instance_guid instance) {
        xresource::full_guid result;
        result.m_Instance = instance;
        result.m_Type = ResourceGUID::mesh_type_guid_v;
        return result;
    }

    /**
     * @brief Overload: Convert instance_guid to full_guid for TEXTURE resources
     * @param instance The instance_guid to convert
     * @return A full_guid configured for texture resources
     */
    inline xresource::full_guid convertToTextureGuid(xresource::instance_guid instance) {
        xresource::full_guid result;
        result.m_Instance = instance;
        result.m_Type = ResourceGUID::texture_type_guid_v;
        return result;
    }

    /**
     * @brief Overload: Convert instance_guid to full_guid for MATERIAL resources
     * @param instance The instance_guid to convert
     * @return A full_guid configured for material resources
     */
    inline xresource::full_guid convertToMaterialGuid(xresource::instance_guid instance) {
        xresource::full_guid result;
        result.m_Instance = instance;
        result.m_Type = ResourceGUID::material_type_guid_v;
        return result;
    }

    /**
     * @brief Overload: Convert instance_guid to full_guid for AUDIO resources
     * @param instance The instance_guid to convert
     * @return A full_guid configured for audio resources
     */
    inline xresource::full_guid convertToAudioGuid(xresource::instance_guid instance) {
        xresource::full_guid result;
        result.m_Instance = instance;
        result.m_Type = ResourceGUID::audio_type_guid_v;
        return result;
    }

    /**
     * @brief Overload: Convert instance_guid to full_guid for SHADER resources
     * @param instance The instance_guid to convert
     * @return A full_guid configured for shader resources
     */
    inline xresource::full_guid convertToShaderGuid(xresource::instance_guid instance) {
        xresource::full_guid result;
        result.m_Instance = instance;
        result.m_Type = ResourceGUID::shader_type_guid_v;
        return result;
    }

    /**
     * @brief Get path to compiled binary file
     * @param guid Resource GUID
     * @param type Resource type
     * @return Full path: Compiled/Meshes/1234567890ABCDEF.mesh
     */
    inline std::string getCompiledFilePath(const xresource::full_guid& guid, ResourceType type) {


        // Get repository root and build Compiled path
        std::filesystem::path compiledRoot = AM.config().compiledPath; 


        // Get type folder
        std::string typeFolder = resourceTypeToString(type);

        // Format GUID as hex string
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0')
            << std::setw(16) << guid.m_Instance.m_Value;

        // Get extension
        std::string extension;
        switch (type) {
        case ResourceType::TEXTURE:  extension = ".tex"; break;
        case ResourceType::MESH:     extension = ".mesh"; break;
        case ResourceType::MATERIAL: extension = ".mat"; break;
        case ResourceType::AUDIO:    extension = ".audio"; break;
        case ResourceType::SHADER:   extension = ".shader"; break;
        default:                     extension = ".bin"; break;
        }

        // Build path: Compiled/Type/GUID.ext (flat structure)
        std::filesystem::path result = compiledRoot / typeFolder / (ss.str() + extension);
        return result.string();
    }

    /**
     * @brief Check if file exists
     */
    inline bool fileExists(const std::string& path) {
        return std::filesystem::exists(path);
    }

}// end of namespace Engine

#endif
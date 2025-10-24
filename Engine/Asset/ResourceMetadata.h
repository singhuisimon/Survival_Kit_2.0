/**
* @file ResourceMetadata.h
* @brief Resource properties definitions
*	Defines property structure for different resource types (descriptor files data)
* @author Wai Lwin Thit
* @date
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/

#pragma once

//include header files
#include "ResourceTypes.h"

//include the libraries 
#include <string>
#include <vector>
#include <memory>


#ifndef _RESOURCE_METADATA_H_
#define _RESOURCE_METADATA_H_

namespace Engine {

         /**
         * @brief Base class for resource properties.
         * @details Contains common properties shared by all resource types.
         */
    struct ResourceProperties {

        std::string resourceName; //human readable name
        std::string intermediateFilePath; 
        std::vector<std::string> tags; //for categorization
        ResourceType resourceType; //type of resource

        //Time stamps for change detection
        uint64_t lastModified = 0; 
        uint64_t lastImported = 0;

        ResourceProperties() : resourceType(ResourceType::UNKNOWN) {}
        virtual ~ResourceProperties() = default;

        /**
         * @brief Create a copy of this properties object.
         * @return Unique pointer to the copied properties.
         */
        virtual std::unique_ptr<ResourceProperties> clone() const = 0;
    };

    /**
     * @brief Texture-specific properties.
     */

    struct TextureProperties : public ResourceProperties {
        int maxWidth = 1024;                ///< Maximum texture width
        int maxHeight = 1024;               ///< Maximum texture height
        std::string compressionFormat;      ///< Compression format (DXT1, DXT5, etc.)
        bool srgb = false;                  ///< Whether to use sRGB color space
        bool generateMipmaps = true;        ///< Whether to generate mipmaps
        int compressionQuality = 80;        ///< Compression quality (0-100)

        TextureProperties() {
            resourceType = ResourceType::TEXTURE;
        }

        std::unique_ptr<ResourceProperties> clone() const override {
            return std::make_unique<TextureProperties>(*this);
        }
    };
        /**
     * @brief Mesh-specific properties.
     */
        struct MeshProperties : public ResourceProperties {
            float scaleFactor = 1.0f;           ///< Scale to apply during import
            bool optimizeVertices = true;       ///< Whether to optimize vertex data
            bool generateNormals = true;        ///< Whether to generate normals if missing
            bool generateTangents = true;       ///< Whether to generate tangents
            bool flipUVs = false;               ///< Whether to flip UV coordinates
            std::string animationImportMode = "default";  ///< How to handle animations

            // Mesh optimization settings
            bool removeDegenerate = true;       ///< Remove degenerate triangles
            bool weldVertices = true;           ///< Weld duplicate vertices
            float weldThreshold = 0.0001f;      ///< Threshold for vertex welding

            MeshProperties() {
                resourceType = ResourceType::MESH;
            }

            std::unique_ptr<ResourceProperties> clone() const override {
                return std::make_unique<MeshProperties>(*this);
            }
        };

        /**
         * @brief Material-specific properties.
         */
        struct MaterialProperties : public ResourceProperties {
            std::string shaderName;             ///< Name of associated shader
            xresource::full_guid diffuseTexture;   ///< Diffuse texture GUID
            xresource::full_guid normalTexture;    ///< Normal map GUID
            xresource::full_guid specularTexture;  ///< Specular map GUID

            // Material parameters
            float shininess = 32.0f;            ///< Shininess factor
            float opacity = 1.0f;               ///< Opacity (0-1)
            bool doubleSided = false;           ///< Whether material is double-sided

            MaterialProperties() {
                resourceType = ResourceType::MATERIAL;
            }

            std::unique_ptr<ResourceProperties> clone() const override {
                return std::make_unique<MaterialProperties>(*this);
            }
        };

        /**
         * @brief Audio-specific properties.
         */
        struct AudioProperties : public ResourceProperties {
            int sampleRate = 44100;             ///< Sample rate in Hz
            int bitDepth = 16;                  ///< Bit depth (8, 16, 24, 32)
            int channels = 2;                   ///< Number of channels (1=mono, 2=stereo)
            std::string compressionFormat = "ogg"; ///< Audio compression format
            float compressionQuality = 0.7f;    ///< Compression quality (0-1)
            bool looping = false;               ///< Whether audio should loop
            float volume = 1.0f;                ///< Default volume (0-1)

            AudioProperties() {
                resourceType = ResourceType::AUDIO;
            }

            std::unique_ptr<ResourceProperties> clone() const override {
                return std::make_unique<AudioProperties>(*this);
            }
        };

        /**
         * @brief Shader-specific properties.
         */
        struct ShaderProperties : public ResourceProperties {
            std::string vertexShaderPath;      ///< Path to vertex shader
            std::string fragmentShaderPath;    ///< Path to fragment shader
            std::string geometryShaderPath;    ///< Path to geometry shader (optional)
            std::vector<std::string> defines;  ///< Preprocessor defines
            bool enableDebugInfo = false;      ///< Include debug information

            ShaderProperties() {
                resourceType = ResourceType::SHADER;
            }

            std::unique_ptr<ResourceProperties> clone() const override {
                return std::make_unique<ShaderProperties>(*this);
            }
        };

        /**
        * @brief Factory function to create appropriate properties for a resource type.
        * @param type The resource type.
        * @return Unique pointer to the created properties object.
        */
        inline std::unique_ptr<ResourceProperties> createResourceProperties(ResourceType type) {
            switch (type) {
            case ResourceType::TEXTURE:
                return std::make_unique<TextureProperties>();
            case ResourceType::MESH:
                return std::make_unique<MeshProperties>();
            case ResourceType::MATERIAL:
                return std::make_unique<MaterialProperties>();
            case ResourceType::AUDIO:
                return std::make_unique<AudioProperties>();
            case ResourceType::SHADER:
                return std::make_unique<ShaderProperties>();
            default:
                return std::make_unique<TextureProperties>(); //return a concrete implementation instead of abstract 
                //or we could return a nullptr?
            }
        }
        /**
         * @brief Cast base properties to specific type.
         * @tparam T The target properties type.
         * @param properties Base properties pointer.
         * @return Pointer to cast properties, or nullptr if cast fails.
         */
        template<typename T>
        T* cast_properties(ResourceProperties* properties) {
            return dynamic_cast<T*>(properties);
        }

        template<typename T>
        const T* cast_properties(const ResourceProperties* properties) {
            return dynamic_cast<const T*>(properties);
        }
} // end of namespace gam300

#endif // _RESOURCE_METADATA_H
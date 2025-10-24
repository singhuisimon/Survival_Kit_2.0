/**
 * @file CompiledResourceFormats.h
 * @brief Binary format definitions for compiled resources
 * @details Defines the structure of compiled binary files that ResourceLoaders read
 * @author [Your Name]
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */

#pragma once
#ifndef __COMPILED_RESOURCE_FORMATS_H__
#define __COMPILED_RESOURCE_FORMATS_H__

#include <cstdint>
#include "ResourceTypes.h"

namespace Engine {

    /**
     * @brief Common header for all compiled resource files
     * @details This header appears at the start of every compiled binary file
     */
    struct CompiledResourceHeader {
        uint32_t magic;           // Magic number for validation: 0x52455347 ('RESG')
        uint32_t version;         // Format version (currently 1)
        uint32_t resourceType;    // ResourceType enum value cast to uint32_t
        uint32_t flags;           // Compression, mipmaps, etc.
        uint64_t dataSize;        // Size of data section in bytes
        uint64_t metadataSize;    // Size of metadata section in bytes
        uint64_t guid_instance;   // GUID instance value
        uint64_t guid_type;       // GUID type value
        uint32_t checksum;        // CRC32 or similar checksum
        uint32_t reserved;        // For future use

        static constexpr uint32_t MAGIC_NUMBER = 0x52455347;  // 'RESG'
        static constexpr uint32_t CURRENT_VERSION = 1;
    };

    /**
     * @brief Header for compiled mesh data
     * @details Follows CompiledResourceHeader in .mesh files
     */
    struct CompiledMeshData {
        char magic[4] = { 'M', 'S', 'H', '\0' };  // Magic number "MSH"
        uint32_t version = 1;                      // Format version

        uint32_t vertexCount = 0;                  // Number of vertices
        uint32_t indexCount = 0;                   // Number of indices

        uint32_t hasPositions = 1;                 // Always 1 (positions required)
        uint32_t hasNormals = 0;                   // 1 if normals present
        uint32_t hasColors = 0;                    // 1 if colors present
        uint32_t hasTexCoords = 0;                 // 1 if UVs present

        uint32_t vertexStride = 0;                 // Bytes per vertex (if interleaved)
        uint32_t indexSize = 4;                    // 2 for uint16, 4 for uint32

        uint32_t reserved[6] = { 0 };              // For future use
    };

    /**
     * @brief Header for compiled texture data
     * @details Follows CompiledResourceHeader in .tex files
     */
    struct CompiledTextureData {
        char magic[4] = { 'T', 'E', 'X', '\0' };  // Magic number "TEX"
        uint32_t version = 1;                      // Format version

        uint32_t width = 0;                        // Texture width in pixels
        uint32_t height = 0;                       // Texture height in pixels
        uint32_t channels = 4;                     // Number of channels (3=RGB, 4=RGBA)
        uint32_t mipLevels = 1;                    // Number of mipmap levels

        uint32_t format = 0;                       // Internal format (GL_RGBA8, etc.)
        uint32_t dataFormat = 0;                   // Data format (GL_RGBA, etc.)
        uint32_t dataType = 0;                     // Data type (GL_UNSIGNED_BYTE, etc.)

        uint32_t srgb = 0;                         // 1 if sRGB, 0 if linear
        uint32_t compressed = 0;                   // 1 if compressed, 0 if not

        uint32_t reserved[5] = { 0 };              // For future use
    };

    /**
     * @brief Header for compiled audio data
     * @details Follows CompiledResourceHeader in .audio files
     */
    struct CompiledAudioData {
        char magic[4] = { 'A', 'U', 'D', '\0' };  // Magic number "AUD"
        uint32_t version = 1;                      // Format version

        uint32_t sampleRate = 44100;               // Samples per second
        uint32_t channels = 2;                     // Number of channels (1=mono, 2=stereo)
        uint32_t bitDepth = 16;                    // Bits per sample (8, 16, 24, 32)
        uint32_t sampleCount = 0;                  // Total number of samples

        uint32_t format = 0;                       // OpenAL format (AL_FORMAT_STEREO16, etc.)
        uint32_t compressed = 0;                   // 1 if compressed, 0 if PCM

        uint32_t reserved[6] = { 0 };              // For future use
    };

    /**
     * @brief Header for compiled shader data
     * @details Follows CompiledResourceHeader in .shader files
     */
    struct CompiledShaderData {
        char magic[4] = { 'S', 'H', 'D', '\0' };  // Magic number "SHD"
        uint32_t version = 1;                      // Format version

        uint32_t vertexShaderSize = 0;             // Size of vertex shader source
        uint32_t fragmentShaderSize = 0;           // Size of fragment shader source
        uint32_t geometryShaderSize = 0;           // Size of geometry shader source (0 if none)

        uint32_t shaderType = 0;                   // Shader type flags
        uint32_t reserved[6] = { 0 };              // For future use
    };

    /**
     * @brief Flags for compiled resources
     */
    namespace CompileFlags {
        constexpr uint32_t COMPRESSED = 1 << 0;    // Resource data is compressed
        constexpr uint32_t HAS_MIPMAPS = 1 << 1;   // Texture has mipmaps
        constexpr uint32_t SRGB = 1 << 2;          // Texture uses sRGB color space
    }

} // namespace Engine

#endif // __COMPILED_RESOURCE_FORMATS_H__
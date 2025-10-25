/**
 * @file ResourceLoaders.cpp
 * @brief Complete implementation of xresource_mgr loaders with OpenGL integration
 * @details Loads compiled binary resources and creates OpenGL handles
 * @author 
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */

#include "ResourceData.h"
#include "ResourceManager.h"
#include "CompiledResourceFormat.h"
#include "ResourceHelpers.h"

#include "../include/glad/glad.h" // OpenGL functions
#include "../glm/glm/glm.hpp"

#include <fstream>
#include <memory>


 // Loader registrations - defined once
xresource::loader_registration<Engine::ResourceGUID::texture_type_guid_v> texture_loader;
xresource::loader_registration<Engine::ResourceGUID::mesh_type_guid_v> mesh_loader;
xresource::loader_registration<Engine::ResourceGUID::material_type_guid_v> material_loader;
xresource::loader_registration<Engine::ResourceGUID::audio_type_guid_v> audio_loader;
xresource::loader_registration<Engine::ResourceGUID::shader_type_guid_v> shader_loader;

namespace Engine {

    // Helper to get ResourceManager from xresource::mgr
    ResourceManager* getResourceManager(xresource::mgr& mgr) {
        return &mgr.getUserData<ResourceManager>();
    }

    // Helper to read compiled resource header
    bool readCompiledHeader(std::ifstream& file, CompiledResourceHeader& header) {
        file.read(reinterpret_cast<char*>(&header), sizeof(CompiledResourceHeader));

        if (!file) {
           /* LM.writeLog("ResourceLoader - Failed to read compiled resource header");*/
            return false;
        }

        // Validate magic number
        if (header.magic != CompiledResourceHeader::MAGIC_NUMBER) {
         //   LM.writeLog("ResourceLoader - Invalid magic number: 0x%X", header.magic);
            return false;
        }

        // Check version compatibility
        if (header.version != CompiledResourceHeader::CURRENT_VERSION) {
         //   LM.writeLog("ResourceLoader - Unsupported version: %u", header.version);
            return false;
        }

        return true;
    }

} // namespace Engine


// ========== TEXTURE LOADER IMPLEMENTATION ==========

xresource::loader<Engine::ResourceGUID::texture_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::texture_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    Engine::ResourceManager* rm = Engine::getResourceManager(mgr);

    // Get compiled file path
    std::string compiled_path = getCompiledFilePath(guid, Engine::ResourceType::TEXTURE);

    if (!Engine::fileExists(compiled_path)) {
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }

    // Read texture-specific header
    Engine::CompiledTextureData texHeader;
    file.read(reinterpret_cast<char*>(&texHeader), sizeof(Engine::CompiledTextureData));

    if (!file) {
        return nullptr;
    }

    //validate magic number
    if (strncmp(texHeader.magic, "TEX", 3) != 0) return nullptr;

    // Create texture resource
    auto texture = std::make_unique<data_type>();
    texture->width = texHeader.width;
    texture->height = texHeader.height;
    texture->channels = texHeader.channels;

    // Determine OpenGL format
    GLenum internalFormat, format, type;
    if (texHeader.srgb) {
        internalFormat = (texHeader.channels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
    }
    else {
        internalFormat = (texHeader.channels == 4) ? GL_RGBA8 : GL_RGB8;
    }
    format = (texHeader.channels == 4) ? GL_RGBA : GL_RGB;
    type = GL_UNSIGNED_BYTE;

    // Generate OpenGL texture
    glGenTextures(1, &texture->textureID);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        texHeader.mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Read and upload each mip level
    int currentWidth = texHeader.width;
    int currentHeight = texHeader.height;

    for (uint32_t mipLevel = 0; mipLevel < texHeader.mipLevels; ++mipLevel) {
        // Calculate size of this mip level
        size_t mipSize = static_cast<size_t>(currentWidth) * currentHeight * texHeader.channels;

        // Read mip data
        std::vector<unsigned char> mipData(mipSize);
        file.read(reinterpret_cast<char*>(mipData.data()), mipSize);

        if (!file) {
            glDeleteTextures(1, &texture->textureID);
            return nullptr;
        }

        // Upload to GPU
        glTexImage2D(GL_TEXTURE_2D, mipLevel, internalFormat,
            currentWidth, currentHeight, 0, format, type, mipData.data());

        // Calculate next mip dimensions
        currentWidth = std::max(1, currentWidth / 2);
        currentHeight = std::max(1, currentHeight / 2);
    }

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        glDeleteTextures(1, &texture->textureID);
        return nullptr;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    texture->format = texHeader.srgb ? "sRGB" : "RGB";
    return texture.release();
}

void xresource::loader<Engine::ResourceGUID::texture_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // Delete OpenGL texture
    if (data.textureID != 0) {
        glDeleteTextures(1, &data.textureID);
    }

    delete& data;
}


// ========== MESH LOADER IMPLEMENTATION ==========


xresource::loader<Engine::ResourceGUID::mesh_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::mesh_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    Engine::ResourceManager* rm = Engine::getResourceManager(mgr);

    // Get compiled file path
    std::string compiled_path = getCompiledFilePath(guid, Engine::ResourceType::MESH);

    if (!Engine::fileExists(compiled_path)) {
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }

    // Read compiled mesh data header
    Engine::CompiledMeshData meshHeader;
    file.read(reinterpret_cast<char*>(&meshHeader), sizeof(meshHeader));
    if (!file) {
        return nullptr;
    }

    //validate magic number
    if (strncmp(meshHeader.magic, "MSH", 3) != 0) return nullptr;

    // Create mesh resource
    auto mesh = std::make_unique<data_type>();
    // Prepare interleaved vertex data
      // Format: pos(3) + normal(3) + color(3) + uv(2) = 11 floats per vertex
    mesh->vertices.resize(meshHeader.vertexCount * 11);

    // Read interleaved vertex data
    for (uint32_t i = 0; i < meshHeader.vertexCount; ++i) {
        size_t offset = i * 11;

        // Read position (always present)
        if (meshHeader.hasPositions) {
            glm::vec3 pos;
            file.read(reinterpret_cast<char*>(&pos), sizeof(glm::vec3));
            mesh->vertices[offset + 0] = pos.x;
            mesh->vertices[offset + 1] = pos.y;
            mesh->vertices[offset + 2] = pos.z;
        }

        // Read normal (if present)
        if (meshHeader.hasNormals) {
            glm::vec3 normal;
            file.read(reinterpret_cast<char*>(&normal), sizeof(glm::vec3));
            mesh->vertices[offset + 3] = normal.x;
            mesh->vertices[offset + 4] = normal.y;
            mesh->vertices[offset + 5] = normal.z;
        }
        else {
            mesh->vertices[offset + 3] = 0.0f;
            mesh->vertices[offset + 4] = 0.0f;
            mesh->vertices[offset + 5] = 0.0f;
        }

        // Read color (if present)
        if (meshHeader.hasColors) {
            glm::vec3 color;
            file.read(reinterpret_cast<char*>(&color), sizeof(glm::vec3));
            mesh->vertices[offset + 6] = color.x;
            mesh->vertices[offset + 7] = color.y;
            mesh->vertices[offset + 8] = color.z;
        }
        else {
            mesh->vertices[offset + 6] = 1.0f;
            mesh->vertices[offset + 7] = 1.0f;
            mesh->vertices[offset + 8] = 1.0f;
        }

        // Read texcoord (if present)
        if (meshHeader.hasTexCoords) {
            glm::vec2 uv;
            file.read(reinterpret_cast<char*>(&uv), sizeof(glm::vec2));
            mesh->vertices[offset + 9] = uv.x;
            mesh->vertices[offset + 10] = uv.y;
        }
        else {
            mesh->vertices[offset + 9] = 0.0f;
            mesh->vertices[offset + 10] = 0.0f;
        }
    }


    // Read indices
    mesh->indices.resize(meshHeader.indexCount);
    if (meshHeader.indexSize == 2) {
        // Read UINT16 indices
        std::vector<uint16_t> indices16(meshHeader.indexCount);
        file.read(reinterpret_cast<char*>(indices16.data()),
            meshHeader.indexCount * sizeof(uint16_t));

        // Convert to uint32
        for (uint32_t i = 0; i < meshHeader.indexCount; ++i) {
            mesh->indices[i] = static_cast<uint32_t>(indices16[i]);
        }
    }
    else {
        // Read UINT32 indices directly
        file.read(reinterpret_cast<char*>(mesh->indices.data()),
            meshHeader.indexCount * sizeof(uint32_t));
    }
    if (!file) {
      //  LM.writeLog("MeshLoader - Failed to read mesh data");
        return nullptr;
    }

#if 0
    // Convert to interleaved format for OpenGL
    // Format: pos(3) + normal(3) + color(3) + uv(2) = 11 floats per vertex
    mesh->vertices.resize(meshHeader.vertexCount * 11);

    for (uint32_t i = 0; i < meshHeader.vertexCount; ++i) {
        size_t offset = i * 11;

        // Position (3 floats)
        mesh->vertices[offset + 0] = positions[i].x;
        mesh->vertices[offset + 1] = positions[i].y;
        mesh->vertices[offset + 2] = positions[i].z;

        // Normal (3 floats)
        mesh->vertices[offset + 3] = normals[i].x;
        mesh->vertices[offset + 4] = normals[i].y;
        mesh->vertices[offset + 5] = normals[i].z;

        // Color (3 floats)
        mesh->vertices[offset + 6] = colors[i].x;
        mesh->vertices[offset + 7] = colors[i].y;
        mesh->vertices[offset + 8] = colors[i].z;

        // TexCoord (2 floats)
        mesh->vertices[offset + 9] = texcoords[i].x;
        mesh->vertices[offset + 10] = texcoords[i].y;
    }
#endif
    // Create OpenGL buffers
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    // Upload interleaved vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER,
        mesh->vertices.size() * sizeof(float),
        mesh->vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh->indices.size() * sizeof(unsigned int),
        mesh->indices.data(), GL_STATIC_DRAW);

    // Setup vertex attributes - interleaved format
    size_t stride = 11 * sizeof(float);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute (location = 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // TexCoord attribute (location = 3)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
     //   LM.writeLog("MeshLoader - OpenGL error: 0x%X", error);
        glDeleteVertexArrays(1, &mesh->VAO);
        glDeleteBuffers(1, &mesh->VBO);
        glDeleteBuffers(1, &mesh->EBO);
        return nullptr;
    }

    return mesh.release();
}



void xresource::loader<Engine::ResourceGUID::mesh_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // Delete OpenGL buffers
    if (data.VAO != 0) {
        glDeleteVertexArrays(1, &data.VAO);
    }
    if (data.VBO != 0) {
        glDeleteBuffers(1, &data.VBO);
    }
    if (data.EBO != 0) {
        glDeleteBuffers(1, &data.EBO);
    }

    delete& data;
}


// ========== MATERIAL LOADER IMPLEMENTATION ==========

xresource::loader<Engine::ResourceGUID::material_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::material_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    Engine::ResourceManager* rm = Engine::getResourceManager(mgr);

    // Get compiled file path
    std::string compiled_path = getCompiledFilePath(guid, Engine::ResourceType::MATERIAL);

    if (!Engine::fileExists(compiled_path)) {
     //   LM.writeLog("MaterialLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
      //  LM.writeLog("MaterialLoader - Failed to open: %s", compiled_path.c_str());
        return nullptr;
    }

    // Read header
    Engine::CompiledResourceHeader header;
    if (!Engine::readCompiledHeader(file, header)) {
        return nullptr;
    }

    // Create material resource
    auto material = std::make_unique<data_type>();

    // Read material properties
    file.read(reinterpret_cast<char*>(&material->diffuseTexture), sizeof(xresource::full_guid));
    file.read(reinterpret_cast<char*>(&material->normalTexture), sizeof(xresource::full_guid));
    file.read(reinterpret_cast<char*>(&material->specularTexture), sizeof(xresource::full_guid));
    file.read(reinterpret_cast<char*>(&material->shininess), sizeof(float));
    file.read(reinterpret_cast<char*>(&material->opacity), sizeof(float));
    file.read(reinterpret_cast<char*>(&material->doubleSided), sizeof(bool));

    // Read shader name length and string
    uint32_t nameLength;
    file.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
    material->shaderName.resize(nameLength);
    file.read(material->shaderName.data(), nameLength);

    if (!file) {
   //     LM.writeLog("MaterialLoader - Failed to read material data");
        return nullptr;
    }

   // LM.writeLog("MaterialLoader - Loaded material GUID: %llX", guid.m_Instance.m_Value);

    return material.release();
}

void xresource::loader<Engine::ResourceGUID::material_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    //LM.writeLog("MaterialLoader - Destroyed material GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}


// ========== AUDIO LOADER IMPLEMENTATION ==========

xresource::loader<Engine::ResourceGUID::audio_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::audio_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    Engine::ResourceManager* rm = Engine::getResourceManager(mgr);

    std::string compiled_path = getCompiledFilePath(guid, Engine::ResourceType::AUDIO);

    if (!Engine::fileExists(compiled_path)) {
    //    LM.writeLog("AudioLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
     //   LM.writeLog("AudioLoader - Failed to open: %s", compiled_path.c_str());
        return nullptr;
    }
    Engine::CompiledResourceHeader header;
    if (!Engine::readCompiledHeader(file, header)) {
        return nullptr;
    }

    auto audio = std::make_unique<data_type>();

    // Read audio properties
    file.read(reinterpret_cast<char*>(&audio->sampleRate), sizeof(int));
    file.read(reinterpret_cast<char*>(&audio->channels), sizeof(int));
    file.read(reinterpret_cast<char*>(&audio->bitDepth), sizeof(int));

    // Read audio data size and data
    uint32_t dataSize;
    file.read(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t));
    audio->audioData.resize(dataSize);
    file.read(audio->audioData.data(), dataSize);

    if (!file) {
     //   LM.writeLog("AudioLoader - Failed to read audio data");
        return nullptr;
    }

    // TODO: Create OpenAL buffer if using OpenAL
    // For now, just keep data in memory

 ///   LM.writeLog("AudioLoader - Loaded audio GUID: %llX, Size: %u bytes",
   //     guid.m_Instance.m_Value, dataSize);

    return audio.release();
}

void xresource::loader<Engine::ResourceGUID::audio_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // TODO: Delete OpenAL buffer if created
    //LM.writeLog("AudioLoader - Destroyed audio GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}


// ========== SHADER LOADER IMPLEMENTATION ==========

xresource::loader<Engine::ResourceGUID::shader_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::shader_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    Engine::ResourceManager* rm = Engine::getResourceManager(mgr);

    std::string compiled_path = getCompiledFilePath(guid, Engine::ResourceType::SHADER);

    if (!Engine::fileExists(compiled_path)) {
   //     LM.writeLog("ShaderLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
     //   LM.writeLog("ShaderLoader - Failed to open: %s", compiled_path.c_str());
        return nullptr;
    }

    Engine::CompiledResourceHeader header;
    if (!Engine::readCompiledHeader(file, header)) {
        return nullptr;
    }

    auto shader = std::make_unique<data_type>();

    // Read shader source lengths and sources
    uint32_t vertLength, fragLength, geomLength;
    file.read(reinterpret_cast<char*>(&vertLength), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&fragLength), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&geomLength), sizeof(uint32_t));

    if (vertLength > 0) {
        shader->vertexSource.resize(vertLength);
        file.read(shader->vertexSource.data(), vertLength);
    }

    if (fragLength > 0) {
        shader->fragmentSource.resize(fragLength);
        file.read(shader->fragmentSource.data(), fragLength);
    }

    if (geomLength > 0) {
        shader->geometrySource.resize(geomLength);
        file.read(shader->geometrySource.data(), geomLength);
    }

    if (!file) {
     // LM.writeLog("ShaderLoader - Failed to read shader sources");
        return nullptr;
    }

    // TODO: Compile and link OpenGL shader program
    // For now, just keep sources in memory

 //   LM.writeLog("ShaderLoader - Loaded shader GUID: %llX", guid.m_Instance.m_Value);

    return shader.release();
}

void xresource::loader<Engine::ResourceGUID::shader_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // TODO: Delete OpenGL shader program if created
    if (data.programID != 0) {
        glDeleteProgram(data.programID);
    }

  // LM.writeLog("ShaderLoader - Destroyed shader GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}


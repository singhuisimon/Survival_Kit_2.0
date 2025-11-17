/**
 * @file ResourceLoaders.cpp
 * @brief Complete implementation of xresource_mgr loaders with OpenGL integration
 * @details Loads compiled binary resources and creates OpenGL handles
 * @author Wai Lwin Thit
 * @date 11 October 2025
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "ResourceData.h"
#include "ResourceManager.h"
#include "CompiledResourceFormat.h"
#include "ResourceHelpers.h"
#include "../Utility/Logger.h"

 // RapidJSON includes
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "../include/glad/glad.h" // OpenGL functions
#include "../glm/glm/glm.hpp"

#include <fstream>
#include <memory>

#include "../../AssetCompiler/CompilerCore/MeshCompiler.h"


namespace Engine { 

    // Helper to get ResourceManager from xresource::mgr
    ResourceManager* getResourceManager(xresource::mgr& mgr) {
        return &mgr.getUserData<ResourceManager>();
    }

    // Helper to read compiled resource header
    bool readCompiledHeader(std::ifstream& file, CompiledResourceHeader& header) {
        file.read(reinterpret_cast<char*>(&header), sizeof(CompiledResourceHeader));

        if (!file) {
            return false;
        }

        // Validate magic number
        if (header.magic != CompiledResourceHeader::MAGIC_NUMBER) {
            return false;
        }

        // Check version compatibility
        if (header.version != CompiledResourceHeader::CURRENT_VERSION) {
            return false;
        }

        return true;
    }

} // namespace Engine


// ========== TEXTURE LOADER IMPLEMENTATION ==========

xresource::loader<Engine::ResourceGUID::texture_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::texture_type_guid_v>::Load(
    xresource::mgr& /*mgr*/, const full_guid& guid)
{

    // Get compiled file path
    std::string compiled_path = getCompiledFilePath(guid, Engine::ResourceType::TEXTURE);

    if (!Engine::fileExists(compiled_path)) {
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LOG_WARNING("Failed to open binary file");
        return nullptr;
    }

    // Read texture-specific header
    Engine::CompiledTextureData texHeader;
    file.read(reinterpret_cast<char*>(&texHeader), sizeof(Engine::CompiledTextureData));

    if (!file) {
		LOG_WARNING("Failed to read texture header");
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
            LOG_WARNING("Failed to read texture");
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
		LOG_WARNING("OpenGL error occurred while creating texture: 0x%X", error);
        return nullptr;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    texture->format = texHeader.srgb ? "sRGB" : "RGB";
    return texture.release();
}

void xresource::loader<Engine::ResourceGUID::texture_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& /*guid*/)
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
    xresource::mgr& /*mgr*/, const full_guid& guid)
{


    std::string compiled_path = getCompiledFilePath(guid, Engine::ResourceType::MESH);

    //LOG_INFO("MESHFILE PATH : ", compiled_path);

    if (!Engine::fileExists(compiled_path)) {
        Engine::listCompiledFiles(Engine::ResourceType::MESH);  
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("File cannot be opened ", compiled_path);

        return nullptr;
    }

    LOG_INFO("File Opened Successfully"); 

    // Read compiled mesh data header
    Engine::CompiledMeshData meshHeader;
	assert(sizeof(Engine::CompiledMeshData) == 68); // Ensure no padding issues
    file.read(reinterpret_cast<char*>(&meshHeader), sizeof(meshHeader));
    if (!file) {
        LOG_ERROR("Failed to read mesh header from file");
        return nullptr;
    }
    LOG_INFO("Mesh header read successfully");

    //validate magic number
    if (strncmp(meshHeader.magic, "MSH", 3) != 0) {
        LOG_ERROR("Invalid magic number in mesh file. Expected 'MSH', got: ",
            meshHeader.magic[0], meshHeader.magic[1], meshHeader.magic[2]);
        return nullptr;
    }
    LOG_INFO("Magic number validated");

    LOG_INFO("Mesh Header Info - Meshes: ", meshHeader.meshCount);

    std::vector<Engine::SubMeshDescriptor> submeshes;

    if (meshHeader.meshCount > 0) {
        submeshes.resize(meshHeader.meshCount);
        file.read(reinterpret_cast<char*>(submeshes.data()),
			meshHeader.meshCount * sizeof(Engine::SubMeshDescriptor));

		// Check for read errors
        if (!file) {
            LOG_ERROR("Failed to read submesh descriptors from file");
			return nullptr;
        }

        LOG_INFO("Read ", meshHeader.meshCount, " submesh descriptors");

        for (uint32_t i = 0; i < meshHeader.meshCount; ++i) {

            LOG_INFO("  Submesh ", i, ": ", submeshes[i].name,
                     " (indices: ", submeshes[i].startIndex, "-",
                     (submeshes[i].startIndex + submeshes[i].indexCount - 1), ")");

        }
    }

    // Create mesh resource
    auto mesh = std::make_unique<data_type>();
    // Prepare interleaved vertex data
      // Format: pos(3) + normal(3) + color(3) + uv(2) = 11 floats per vertex
    mesh->vertices.resize(meshHeader.vertexCount * 11);
    mesh->subMeshes = submeshes;
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
        LOG_ERROR("Failed to read mesh indices from file");
        return nullptr;
    }
    LOG_INFO("Mesh data loaded - Vertices: ", meshHeader.vertexCount, " Indices: ", meshHeader.indexCount);
    LOG_INFO("Starting OpenGL buffer creation...");


    // Create OpenGL buffers
    glGenVertexArrays(1, &mesh->VAO);

    glGenBuffers(1, &mesh->VBO);

    glGenBuffers(1, &mesh->EBO);
    LOG_INFO("OpenGL buffers generated: VAO=", mesh->VAO, " VBO=", mesh->VBO, " EBO=", mesh->EBO);

    glBindVertexArray(mesh->VAO);
    LOG_INFO("VAO bound successfully");

    // Upload interleaved vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER,
        mesh->vertices.size() * sizeof(float),
        mesh->vertices.data(), GL_STATIC_DRAW);
    LOG_INFO("VBO data uploaded: ", mesh->vertices.size() * sizeof(float), " bytes");

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh->indices.size() * sizeof(unsigned int),
        mesh->indices.data(), GL_STATIC_DRAW);
    LOG_INFO("EBO data uploaded: ", mesh->indices.size() * sizeof(unsigned int), " bytes");


    // Setup vertex attributes - interleaved format
    size_t stride = (11 * sizeof(float));

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute (location = 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // TexCoord attribute (location = 3)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error) {
        LOG_ERROR("OpenGL error after creating buffers: ", error);
    }
 
    LOG_INFO("OpenGL buffers created successfully");
    LOG_INFO("Mesh VAO: ", mesh->VAO, " VBO: ", mesh->VBO, " EBO: ", mesh->EBO);
    LOG_INFO("Returning mesh pointer...");

    LOG_INFO("Vertex count: ", (mesh->vertices.size() / 11));
    LOG_INFO("Index count: ", mesh->indices.size());

    auto* meshPtr = mesh.release();
    LOG_INFO("mesh.release() returned pointer: ", static_cast<void*>(meshPtr));
    return meshPtr;
}



void xresource::loader<Engine::ResourceGUID::mesh_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& /*guid*/)
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
    xresource::mgr& /*mgr*/, const full_guid& guid)
{

    // Get the source file path
    std::string filepath = Engine::AM.getNameFromGuid(guid.m_Instance);
    std::string prefix = Engine::AssetManager::GetSourceResourcesPath() + "/Sources/Material/";
	filepath = prefix + filepath;

    // Check if file exists
    if (!Engine::fileExists(filepath)) {
		//LOG_WARNING("MaterialLoader - Source file not found: ", filepath);
        return nullptr;
    }

    // Open the file 
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        //LOG_WARNING("Material Loader - unable to open file: ", filepath);
        return nullptr;
    }

	// Read entire file into a string
    std::string jsonString((std::istreambuf_iterator<char>(ifs)),
		std::istreambuf_iterator<char>());
    ifs.close();

	// Parse into a JSON document
    using namespace  rapidjson;
    Document doc;
    doc.Parse(jsonString.c_str());

    if (doc.HasParseError()) {
        //LOG_ERROR("JSON Parse error at offset ", doc.GetErrorOffset());
        return nullptr;
    }

    // Create material resource
    auto material = std::make_unique<data_type>(); // Assumes data_type is your new MaterialResource

    // Deserialize material properties from JSON
    if (doc.HasMember("shaderName"))
        material->shaderName = doc["shaderName"].GetString();

    // --- Texture Maps ---
    if (doc.HasMember("baseMap"))
        material->baseMap = xresource::instance_guid{ std::stoull(doc["baseMap"].GetString(), nullptr, 16) };

    if (doc.HasMember("normalMap"))
        material->normalMap = xresource::instance_guid{ std::stoull(doc["normalMap"].GetString(), nullptr, 16) };

    if (doc.HasMember("metallicMap"))
        material->metallicMap = xresource::instance_guid{ std::stoull(doc["metallicMap"].GetString(), nullptr, 16) };

    if (doc.HasMember("roughnessMap"))
        material->roughnessMap = xresource::instance_guid{ std::stoull(doc["roughnessMap"].GetString(), nullptr, 16) };

    if (doc.HasMember("emissionMap"))
        material->emissionMap = xresource::instance_guid{ std::stoull(doc["emissionMap"].GetString(), nullptr, 16) };

    if (doc.HasMember("occlusionMap"))
        material->occlusionMap = xresource::instance_guid{ std::stoull(doc["occlusionMap"].GetString(), nullptr, 16) };

    // --- Color Properties (3-component) ---
    if (doc.HasMember("baseColor"))
    {
        const Value& bColor = doc["baseColor"];
        material->baseColor[0] = bColor[0].GetFloat();
        material->baseColor[1] = bColor[1].GetFloat();
        material->baseColor[2] = bColor[2].GetFloat();
    }

    if (doc.HasMember("emissionColor"))
    {
        const Value& eColor = doc["emissionColor"];
        material->emissionColor[0] = eColor[0].GetFloat();
        material->emissionColor[1] = eColor[1].GetFloat();
        material->emissionColor[2] = eColor[2].GetFloat();
    }

    // --- Float Properties ---
    if (doc.HasMember("metallic"))
        material->metallic = doc["metallic"].GetFloat();

    if (doc.HasMember("roughness"))
        material->roughness = doc["roughness"].GetFloat();

    if (doc.HasMember("opacity"))
        material->opacity = doc["opacity"].GetFloat();

    if (doc.HasMember("emissionStrength"))
        material->emissionStrength = doc["emissionStrength"].GetFloat();

    if (doc.HasMember("alphaThreshold"))
        material->alphaThreshold = doc["alphaThreshold"].GetFloat();

    if (doc.HasMember("ambientOcclusion"))
		material->ambientOcclusion = doc["ambientOcclusion"].GetFloat();

    // --- UV Transforms ---
    if (doc.HasMember("tiling"))
    {
        const Value& tiling = doc["tiling"];
        material->tiling[0] = tiling[0].GetFloat();
        material->tiling[1] = tiling[1].GetFloat();
    }

    if (doc.HasMember("offset"))
    {
        const Value& offset = doc["offset"];
        material->offset[0] = offset[0].GetFloat();
        material->offset[1] = offset[1].GetFloat();
    }

    // --- Bools / Render Flags ---
    if (doc.HasMember("enableEmission"))
        material->enableEmission = doc["enableEmission"].GetBool();

    if (doc.HasMember("alphaTest"))
        material->alphaTest = doc["alphaTest"].GetBool();

    if (doc.HasMember("doubleSided"))
        material->doubleSided = doc["doubleSided"].GetBool();

    if (doc.HasMember("receiveShadows"))
        material->receiveShadows = doc["receiveShadows"].GetBool();

    if (doc.HasMember("castShadows"))
        material->castShadows = doc["castShadows"].GetBool();

    return material.release();
}

void xresource::loader<Engine::ResourceGUID::material_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& /*guid*/)
{
    //LM.writeLog("MaterialLoader - Destroyed material GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}


// ========== AUDIO LOADER IMPLEMENTATION ==========

xresource::loader<Engine::ResourceGUID::audio_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::audio_type_guid_v>::Load(
    xresource::mgr& /*mgr*/, const full_guid& guid)
{

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

    return audio.release();
}

void xresource::loader<Engine::ResourceGUID::audio_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& /*guid*/)
{
    delete& data;
}


// ========== SHADER LOADER IMPLEMENTATION ==========

xresource::loader<Engine::ResourceGUID::shader_type_guid_v>::data_type*
xresource::loader<Engine::ResourceGUID::shader_type_guid_v>::Load(
    xresource::mgr& /*mgr*/, const full_guid& guid)
{
  

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

 //   LM.writeLog("ShaderLoader - Loaded shader GUID: %llX", guid.m_Instance.m_Value);

    return shader.release();
}

void xresource::loader<Engine::ResourceGUID::shader_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& /*guid*/)
{
    // TODO: Delete OpenGL shader program if created
    if (data.programID != 0) {
        glDeleteProgram(data.programID);
    }

  // LM.writeLog("ShaderLoader - Destroyed shader GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}


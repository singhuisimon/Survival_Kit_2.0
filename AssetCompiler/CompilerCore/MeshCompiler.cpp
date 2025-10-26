#include "MeshCompiler.h"
#include "../Utility/DescriptorParser.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdarg>
#include "../rapidjson/document.h"
#include "../rapidjson/istreamwrapper.h"


//open FBX 
#include "../openFBX/openFBX/src/ofbx.h"


namespace fs = std::filesystem;

namespace AssetCompiler {

// ============================================================================
// PUBLIC API
// ============================================================================

    bool MeshCompiler::compile(const std::string& descriptorPath,
        const std::string& outputPath,
        bool verbose) {
        verbose_ = verbose;

        log("=== Compiling Mesh ===");
        log("Descriptor: %s", descriptorPath.c_str());

        // Step 1: Parse descriptor to get source path and settings
        std::string sourcePath;
        MeshSettingsCompiler settings;

        if (!parseSettings(descriptorPath, sourcePath, settings)) {
            log("ERROR: Failed to parse descriptor settings");
            return false;
        }

        // Fix path separators (handle \\ vs /)
        sourcePath = fixPathSeparators(sourcePath);

        log("Source: %s", sourcePath.c_str());
        log("Settings: scale=%.2f, optimize=%d, normals=%d",
            settings.scale, settings.optimizeVertices, settings.generateNormals);

        // Step 2: Check if source file exists
        if (!fs::exists(sourcePath)) {
            log("ERROR: Source file not found: %s", sourcePath.c_str());
            return false;
        }

        // Step 3: Load mesh data from source file
        MeshData meshData;

        std::string ext = fs::path(sourcePath).extension().string();
        bool loadSuccess = false;

        if (ext == ".fbx" || ext == ".FBX") {
            loadSuccess = loadFBXMesh(sourcePath, meshData);
        }
        else if (ext == ".obj" || ext == ".OBJ") {
            loadSuccess = loadOBJMesh(sourcePath, meshData);
        }
        else {
            log("ERROR: Unsupported mesh format: %s", ext.c_str());
            return false;
        }

        if (!loadSuccess || meshData.isEmpty()) {
            log("ERROR: Failed to load mesh data");
            return false;
        }

        log("Loaded: %zu vertices, %zu indices, %zu triangles",
            meshData.getVertexCount(), meshData.getIndexCount(), meshData.getTriangleCount());

        // Step 4: Apply processing based on settings
        if (settings.scale != 1.0f) {
            scaleMesh(meshData, settings.scale);
            log("Applied scale: %.2f", settings.scale);
        }

        if (settings.generateNormals) {
            generateNormals(meshData);
            log("Generated normals");
        }

        if (settings.flipUVs) {
            flipUVs(meshData);
            log("Flipped UVs");
        }

        if (settings.removeDegenerate) {
            size_t beforeCount = meshData.getTriangleCount();
            removeDegenerate(meshData);
            log("Removed degenerate triangles: %zu -> %zu",
                beforeCount, meshData.getTriangleCount());
        }

        if (settings.weldVertices) {
            size_t beforeCount = meshData.getVertexCount();
            weldVertices(meshData, settings.weldThreshold);
            log("Welded vertices: %zu -> %zu (threshold: %.5f)",
                beforeCount, meshData.getVertexCount(), settings.weldThreshold);
        }

        if (settings.optimizeVertices) {
            optimizeVertexCache(meshData);
            log("Optimized vertex cache");
        }

        // Step 5: Prepare binary header
        CompiledMeshHeader header;
        header.vertexCount = static_cast<uint32_t>(meshData.getVertexCount());
        header.indexCount = static_cast<uint32_t>(meshData.getIndexCount());
        header.hasPositions = settings.includePos ? 1 : 0;
        header.hasNormals = (settings.includeNormals && !meshData.normals.empty()) ? 1 : 0;
        header.hasColors = (settings.includeColors && !meshData.colors.empty()) ? 1 : 0;
        header.hasTexCoords = (settings.includeTexCoords && !meshData.texCoords.empty()) ? 1 : 0;

        // Calculate vertex stride (bytes per vertex)
        header.vertexStride = 0;
        if (header.hasPositions) header.vertexStride += sizeof(glm::vec3);
        if (header.hasNormals) header.vertexStride += sizeof(glm::vec3);
        if (header.hasColors) header.vertexStride += sizeof(glm::vec3);
        if (header.hasTexCoords) header.vertexStride += sizeof(glm::vec2);

        header.indexSize = (settings.indexType == "UINT16") ? 2 : 4;

        // Step 6: Create output directory if needed
        fs::path outPath(outputPath);
        if (!fs::exists(outPath.parent_path())) {
            fs::create_directories(outPath.parent_path());
        }

        // Step 7: Write binary file
        if (!writeBinaryMesh(outputPath, header, meshData)) {
            log("ERROR: Failed to write binary mesh");
            return false;
        }

        log("Success! Compiled mesh: %s", outputPath.c_str());
        log("Output size: %.2f KB", fs::file_size(outputPath) / 1024.0f);

        return true;
    }

    // ============================================================================
    // LOADING
    // ============================================================================

    bool MeshCompiler::loadFBXMesh(const std::string& path, MeshData& meshData) {
        log("Loading FBX mesh: %s", path.c_str());

        log("Loading FBX mesh: %s", path.c_str());

        // Load file into memory
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            log("ERROR: Failed to open FBX file: %s", path.c_str());
            return false;
        }

        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<ofbx::u8> content(fileSize);
        if (!file.read(reinterpret_cast<char*>(content.data()), fileSize)) {
            log("ERROR: Failed to read FBX file: %s", path.c_str());
            return false;
        }
        file.close();

        // Load FBX using OpenFBX
        ofbx::IScene* scene = ofbx::load(content.data(), fileSize, (ofbx::u16)ofbx::LoadFlags::NONE);

        if (!scene) {
            log("ERROR: Failed to parse FBX: %s (Error: %s)", path.c_str(), ofbx::getError());
            return false;
        }

        log("FBX loaded: %d meshes found", scene->getMeshCount());

        // Process all meshes in the scene
        int meshCount = scene->getMeshCount();
        for (int mesh_idx = 0; mesh_idx < meshCount; mesh_idx++) {
            const ofbx::Mesh* mesh = scene->getMesh(mesh_idx);
            const ofbx::GeometryData& geom = mesh->getGeometryData();

            log("Processing mesh %d: %s", mesh_idx, mesh->name);

            size_t vertex_offset = meshData.positions.size();

            // Get vertex attributes
            ofbx::Vec3Attributes positions = geom.getPositions();
            ofbx::Vec3Attributes normals = geom.getNormals();
            ofbx::Vec2Attributes uvs = geom.getUVs();

            // Process each partition (submesh with same material)
            for (int partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx) {
                const ofbx::GeometryPartition& partition = geom.getPartition(partition_idx);

                // Process each polygon in the partition
                for (int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
                    const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];

                    // Extract vertices for this polygon
                    for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
                        // Position
                        ofbx::Vec3 pos = positions.get(i);
                        meshData.positions.push_back(glm::vec3(
                            static_cast<float>(pos.x),
                            static_cast<float>(pos.y),
                            static_cast<float>(pos.z)
                        ));

                        // Normal (if available)
                        if (normals.values != nullptr) {
                            ofbx::Vec3 normal = normals.get(i);
                            meshData.normals.push_back(glm::vec3(
                                static_cast<float>(normal.x),
                                static_cast<float>(normal.y),
                                static_cast<float>(normal.z)
                            ));
                        }
                        else {
                            meshData.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        }

                        // Color (default gray)
                        meshData.colors.push_back(glm::vec3(0.5f));

                        // UV coordinates (if available)
                        if (uvs.values != nullptr) {
                            ofbx::Vec2 uv = uvs.get(i);
                            meshData.texCoords.push_back(glm::vec2(
                                static_cast<float>(uv.x),
                                static_cast<float>(uv.y)
                            ));
                        }
                        else {
                            meshData.texCoords.push_back(glm::vec2(0.0f, 0.0f));
                        }
                    }

                    // Triangulate the polygon
                    if (polygon.vertex_count == 3) {
                        // Already a triangle
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 0));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 1));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 2));
                    }
                    else if (polygon.vertex_count == 4) {
                        // Quad - split into two triangles
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 0));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 1));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 2));

                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 0));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 2));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 3));
                    }
                    else if (polygon.vertex_count > 4) {
                        // N-gon - use OpenFBX triangulate function
                        std::vector<int> tri_indices(polygon.vertex_count * 3);
                        ofbx::u32 tri_count = ofbx::triangulate(geom, polygon, tri_indices.data());

                        for (ofbx::u32 t = 0; t < tri_count; ++t) {
                            meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + tri_indices[t]));
                        }
                    }
                }
            }

            vertex_offset = meshData.positions.size();
        }

        // Clean up
        scene->destroy();

        log("FBX parsing complete: %zu positions, %zu normals, %zu colors, %zu texCoords, %zu indices",
            meshData.positions.size(), meshData.normals.size(), meshData.colors.size(),
            meshData.texCoords.size(), meshData.indices.size());

        return !meshData.positions.empty();
        return false;
    }

    bool MeshCompiler::loadOBJMesh(const std::string& path, MeshData& meshData) {

        (void)meshData;

        log("Loading OBJ mesh: %s", path.c_str());

        // TODO: Implement OBJ loading
        // Simple OBJ parser or use tinyobjloader

        log("ERROR: OBJ loading not yet implemented");
        return false;
    }

    // ============================================================================
    // PROCESSING
    // ============================================================================

    void MeshCompiler::scaleMesh(MeshData& meshData, float scale) {
        for (auto& pos : meshData.positions) {
            pos *= scale;
        }
    }

    void MeshCompiler::generateNormals(MeshData& meshData) {
        meshData.normals.resize(meshData.positions.size(), glm::vec3(0.0f));

        // Calculate face normals and accumulate
        for (size_t i = 0; i < meshData.indices.size(); i += 3) {
            uint32_t i0 = meshData.indices[i];
            uint32_t i1 = meshData.indices[i + 1];
            uint32_t i2 = meshData.indices[i + 2];

            if (i0 >= meshData.positions.size() ||
                i1 >= meshData.positions.size() ||
                i2 >= meshData.positions.size()) {
                continue;  // Skip invalid indices
            }

            glm::vec3 edge1 = meshData.positions[i1] - meshData.positions[i0];
            glm::vec3 edge2 = meshData.positions[i2] - meshData.positions[i0];
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            meshData.normals[i0] += faceNormal;
            meshData.normals[i1] += faceNormal;
            meshData.normals[i2] += faceNormal;
        }

        // Normalize accumulated normals
        for (auto& normal : meshData.normals) {
            if (glm::length(normal) > 0.0001f) {
                normal = glm::normalize(normal);
            }
        }
    }

    void MeshCompiler::flipUVs(MeshData& meshData) {
        for (auto& uv : meshData.texCoords) {
            uv.y = 1.0f - uv.y;
        }
    }

    void MeshCompiler::removeDegenerate(MeshData& meshData) {
        std::vector<uint32_t> validIndices;

        for (size_t i = 0; i < meshData.indices.size(); i += 3) {
            uint32_t i0 = meshData.indices[i];
            uint32_t i1 = meshData.indices[i + 1];
            uint32_t i2 = meshData.indices[i + 2];

            // Skip degenerate triangles (shared vertices)
            if (i0 != i1 && i1 != i2 && i2 != i0) {
                validIndices.push_back(i0);
                validIndices.push_back(i1);
                validIndices.push_back(i2);
            }
        }

        meshData.indices = std::move(validIndices);
    }

    void MeshCompiler::weldVertices(MeshData& meshData, float threshold) {
        std::vector<glm::vec3> uniquePos;
        std::vector<glm::vec3> uniqueNormals;
        std::vector<glm::vec3> uniqueColors;
        std::vector<glm::vec2> uniqueTexCoords;
        std::vector<uint32_t> remap(meshData.positions.size());

        float thresholdSq = threshold * threshold;

        for (size_t i = 0; i < meshData.positions.size(); ++i) {
            bool found = false;

            for (size_t j = 0; j < uniquePos.size(); ++j) {
                glm::vec3 diff = meshData.positions[i] - uniquePos[j];
                float distSq = glm::dot(diff, diff);

                if (distSq < thresholdSq) {
                    remap[i] = static_cast<uint32_t>(j);
                    found = true;
                    break;
                }
            }

            if (!found) {
                remap[i] = static_cast<uint32_t>(uniquePos.size());
                uniquePos.push_back(meshData.positions[i]);
                if (i < meshData.normals.size()) uniqueNormals.push_back(meshData.normals[i]);
                if (i < meshData.colors.size()) uniqueColors.push_back(meshData.colors[i]);
                if (i < meshData.texCoords.size()) uniqueTexCoords.push_back(meshData.texCoords[i]);
            }
        }

        // Remap indices
        for (auto& index : meshData.indices) {
            if (index < remap.size()) {
                index = remap[index];
            }
        }

        meshData.positions = std::move(uniquePos);
        meshData.normals = std::move(uniqueNormals);
        meshData.colors = std::move(uniqueColors);
        meshData.texCoords = std::move(uniqueTexCoords);
    }

    void MeshCompiler::optimizeVertexCache(MeshData& meshData) {
        // TODO: Implement Forsyth or similar vertex cache optimization
        // For now, this is a placeholder
        log("Vertex cache optimization: placeholder (implement Forsyth algorithm)");
    }

    // ============================================================================
    // SERIALIZATION
    // ============================================================================

    bool MeshCompiler::writeBinaryMesh(const std::string& outputPath,
        const CompiledMeshHeader& header,
        const MeshData& meshData) {
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            log("ERROR: Failed to open output file: %s", outputPath.c_str());
            return false;
        }

        // Write header
        file.write(reinterpret_cast<const char*>(&header), sizeof(CompiledMeshHeader));

        // Write vertex data (interleaved)
        for (size_t i = 0; i < meshData.positions.size(); ++i) {
            if (header.hasPositions) {
                file.write(reinterpret_cast<const char*>(&meshData.positions[i]), sizeof(glm::vec3));
            }
            if (header.hasNormals && i < meshData.normals.size()) {
                file.write(reinterpret_cast<const char*>(&meshData.normals[i]), sizeof(glm::vec3));
            }
            if (header.hasColors && i < meshData.colors.size()) {
                file.write(reinterpret_cast<const char*>(&meshData.colors[i]), sizeof(glm::vec3));
            }
            if (header.hasTexCoords && i < meshData.texCoords.size()) {
                file.write(reinterpret_cast<const char*>(&meshData.texCoords[i]), sizeof(glm::vec2));
            }
        }

        // Write indices
        if (header.indexSize == 2) {
            // Convert to UINT16
            for (uint32_t idx : meshData.indices) {
                uint16_t idx16 = static_cast<uint16_t>(idx);
                file.write(reinterpret_cast<const char*>(&idx16), sizeof(uint16_t));
            }
        }
        else {
            // Write as UINT32
            file.write(reinterpret_cast<const char*>(meshData.indices.data()),
                meshData.indices.size() * sizeof(uint32_t));
        }

        file.close();
        return true;
    }

    // ============================================================================
    // HELPERS
    // ============================================================================

    bool MeshCompiler::parseSettings(const std::string& descriptorPath,
        std::string& sourcePath,
        MeshSettingsCompiler& settings) {
        rapidjson::Document doc;

        std::ifstream ifs(descriptorPath);
        if (!ifs.is_open()) {
            log("ERROR: Could not open descriptor: %s", descriptorPath.c_str());
            return false;
        }

        rapidjson::IStreamWrapper isw(ifs);
        doc.ParseStream(isw);

        if (doc.HasParseError()) {
            log("ERROR: JSON parse error in descriptor");
            return false;
        }

        // Extract source path
        if (doc.HasMember("sourcePath") && doc["sourcePath"].IsString()) {
            sourcePath = doc["sourcePath"].GetString();
        }
        else {
            log("ERROR: No 'sourcePath' in descriptor");
            return false;
        }

        // Extract mesh settings
        if (doc.HasMember("meshSettings") && doc["meshSettings"].IsObject()) {
            const auto& ms = doc["meshSettings"];

            if (ms.HasMember("outputFormat")) settings.outputFormat = ms["outputFormat"].GetString();
            if (ms.HasMember("includePos")) settings.includePos = ms["includePos"].GetBool();
            if (ms.HasMember("includeNormals")) settings.includeNormals = ms["includeNormals"].GetBool();
            if (ms.HasMember("includeColors")) settings.includeColors = ms["includeColors"].GetBool();
            if (ms.HasMember("includeTexCoords")) settings.includeTexCoords = ms["includeTexCoords"].GetBool();
            if (ms.HasMember("indexType")) settings.indexType = ms["indexType"].GetString();
            if (ms.HasMember("scale")) settings.scale = ms["scale"].GetFloat();
            if (ms.HasMember("optimizeVertices")) settings.optimizeVertices = ms["optimizeVertices"].GetBool();
            if (ms.HasMember("generateNormals")) settings.generateNormals = ms["generateNormals"].GetBool();
            if (ms.HasMember("flipUVs")) settings.flipUVs = ms["flipUVs"].GetBool();
            if (ms.HasMember("removeDegenerate")) settings.removeDegenerate = ms["removeDegenerate"].GetBool();
            if (ms.HasMember("weldVertices")) settings.weldVertices = ms["weldVertices"].GetBool();
            if (ms.HasMember("weldThreshold")) settings.weldThreshold = ms["weldThreshold"].GetFloat();
        }

        return true;
    }

    std::string MeshCompiler::fixPathSeparators(const std::string& path) {
        std::string fixed = path;
        std::replace(fixed.begin(), fixed.end(), '\\', '/');

        // Remove leading slash/backslash if present
        if (!fixed.empty() && (fixed[0] == '/' || fixed[0] == '\\')) {
            fixed = fixed.substr(1);
        }

        return fixed;
    }

    void MeshCompiler::log(const char* format, ...) {
        if (!verbose_) return;

        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        std::cout << "  [MeshCompiler] " << buffer << "\n";
    }



} //end of namespace AssetCompiler
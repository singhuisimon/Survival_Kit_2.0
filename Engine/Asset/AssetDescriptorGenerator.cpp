#include "AssetDescriptorGenerator.h"
#include "AssetDatabase.h"
#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"


#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;


        // ==================== PUBLIC API ====================



    void AssetDescriptorGenerator::SetOutputRoot(const std::string& root) {
        m_outputRoot = root;
    }


        std::string AssetDescriptorGenerator::GetDescriptorFolderPath(const AssetRecord& rec) const {
        // Convert GUID to hex string (16 characters, uppercase)
        std::ostringstream guidStream;
        guidStream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') 
                   << rec.guid.m_Value;
        std::string guidHex = guidStream.str();

        // Extract last 4 characters for folder structure
        // Example: 5F7ED05B14224003 -> folders: "40/03"
        std::string dir1 = guidHex.substr(12, 2);  // Characters 13-14
        std::string dir2 = guidHex.substr(14, 2);  // Characters 15-16

        // Get resource type folder name
        std::string typeFolder = resourceTypeToString(rec.type);

        // Build path using filesystem::path to handle separators correctly
        fs::path descriptorPath = fs::path(m_outputRoot) / typeFolder / dir1 / dir2 / guidHex;

        // Convert to generic string with forward slashes
        std::string result = descriptorPath.generic_string();

        // Ensure trailing slash
        if (!result.empty() && result.back() != '/') {
            result += '/';
        }
        return result;
        }
 // ==================== INFO.TXT GENERATION ====================
    bool AssetDescriptorGenerator::WriteInfoFile(
        const std::string& folderPath,
        const AssetRecord& rec,
        const DescriptorExtras* extras) const
    {
        std::string infoPath = folderPath + "Info.txt";
        std::string json = BuildInfoJson(rec, extras);

        bool result =  WriteText(infoPath, json);

        if (!result) {
            LOG_ERROR("Failed to write Info.txt");
        }

        return result;
    }

    std::string AssetDescriptorGenerator::BuildInfoJson(
        const AssetRecord& rec,
        const DescriptorExtras* extras) const
    {
        std::ostringstream ss;

        ss << "{\n";

        // Display name
        if (extras && !extras->displayName.empty()) {
            ss << "  \"name\": \"" << EscapeJson(extras->displayName) << "\",\n";
        } else {
            // Default: use filename without extension
            fs::path p(rec.sourcePath);
            ss << "  \"name\": \"" << EscapeJson(p.stem().string()) << "\",\n";
        }

        // Comment (optional)
        if (extras && !extras->comment.empty()) {
            ss << "  \"comment\": \"" << EscapeJson(extras->comment) << "\",\n";
        }

        // GUID (for debugging/reference)
        ss << "  \"guid\": {\n";
        ss << "    \"instance\": \"" << std::uppercase << std::hex << std::setw(16) 
           << std::setfill('0') << rec.guid.m_Value << "\",\n";
        
        // Get type GUID
        xresource::type_guid typeGuid = ResourceGUID::getTypeGUID(rec.type);
        ss << "    \"type\": \"" << std::uppercase << std::hex << std::setw(16) 
           << std::setfill('0') << typeGuid.m_Value << "\"\n";
        ss << "  },\n";

        // Tags
        ss << "  \"tags\": [";
        if (extras && !extras->tags.empty()) {
            for (size_t i = 0; i < extras->tags.size(); ++i) {
                ss << "\"" << EscapeJson(extras->tags[i]) << "\"";
                if (i < extras->tags.size() - 1) ss << ", ";
            }
        }
        ss << "],\n";

        // Last imported timestamp
        if (extras) {
            ss << "  \"lastImported\": " << extras->lastImported << ",\n";
        } else {
            ss << "  \"lastImported\": 0,\n";
        }

        // Resource links (optional dependencies)
        ss << "  \"resourceLinks\": [";
        if (extras && !extras->resourceLinks.empty()) {
            ss << "\n";
            for (size_t i = 0; i < extras->resourceLinks.size(); ++i) {
                const auto& link = extras->resourceLinks[i];
                ss << "    {\n";
                ss << "      \"instance\": \"" << std::uppercase << std::hex << std::setw(16) 
                   << std::setfill('0') << link.m_Instance.m_Value << "\",\n";
                ss << "      \"type\": \"" << std::uppercase << std::hex << std::setw(16) 
                   << std::setfill('0') << link.m_Type.m_Value << "\"\n";
                ss << "    }";
                if (i < extras->resourceLinks.size() - 1) ss << ",";
                ss << "\n";
            }
            ss << "  ]\n";
        } else {
            ss << "]\n";
        }

        ss << "}\n";

        return ss.str();
    }

// ==================== DESCRIPTOR.TXT GENERATION ====================

//****TEXTURE*****/
    std::string AssetDescriptorGenerator::BuildDescriptorJson(
        const std::string& sourcePath,
        const TextureSettings& settings) const
    {
        std::ostringstream ss;

        //change to relative path
        std::string relativePath = Engine::getRelativeAssetPath(sourcePath);

        ss << "{\n";
        ss << "  \"sourcePath\": \"" << EscapeJson(relativePath) << "\",\n";
        ss << "  \"textureSettings\": {\n";
        ss << "    \"usageType\": \"" << EscapeJson(settings.usageType) << "\",\n";
        ss << "    \"compression\": \"" << EscapeJson(settings.compression) << "\",\n";
        ss << "    \"quality\": " << settings.quality << ",\n";
        ss << "    \"generateMipmaps\": " << (settings.generateMipmaps ? "true" : "false") << ",\n";
        ss << "    \"srgb\": " << (settings.srgb ? "true" : "false") << "\n";
        ss << "  }\n";
        ss << "}\n";
        
        return ss.str();
    }

//*****AUDIO*******/
    std::string AssetDescriptorGenerator::BuildDescriptorJson(
            const std::string& sourcePath,
            const AudioSettings& settings) const 
        {

            std::ostringstream ss; 
            std::string relativePath = Engine::getRelativeAssetPath(sourcePath);

            ss << "{\n";
            ss << "  \"sourcePath\": \"" << EscapeJson(relativePath) << "\",\n";
            ss << "  \"audioSettings\": {\n";
            ss << "    \"outputFormat\": \"" << EscapeJson(settings.outputFormat) << "\",\n";
            ss << "    \"compression\": \"" << EscapeJson(settings.compression) << "\",\n";
            ss << "    \"quality\": " << settings.quality << ",\n";
            ss << "    \"sampleRate\": " << settings.sampleRate << ",\n";
            ss << "    \"channelMode\": \"" << EscapeJson(settings.channelMode) << "\"\n";
            ss << "  }\n";
            ss << "}\n";

            return ss.str();
                
    }

//*******MESH******* */
    std::string AssetDescriptorGenerator::BuildDescriptorJson(
        const std::string& sourcePath,
        const MeshSettings& settings) const
    {
        std::ostringstream ss;
        std::string relativePath = Engine::getRelativeAssetPath(sourcePath);
        ss << "{\n";
        ss << "  \"sourcePath\": \"" << EscapeJson(relativePath) << "\",\n";
        ss << "  \"meshSettings\": {\n";
        ss << "    \"outputFormat\": \"" << EscapeJson(settings.outputFormat) << "\",\n";
        ss << "    \"includePos\": " << (settings.includePos ? "true" : "false") << ",\n";
        ss << "    \"includeNormals\": " << (settings.includeNormals ? "true" : "false") << ",\n";
        ss << "    \"includeColors\": " << (settings.includeColors ? "true" : "false") << ",\n";
        ss << "    \"includeTexCoords\": " << (settings.includeTexCoords ? "true" : "false") << ",\n";
        ss << "    \"indexType\": \"" << EscapeJson(settings.indexType) << "\",\n";
        ss << "    \"scale\": " << settings.scale << ",\n";
        ss << "    \"optimizeVertices\": " << (settings.optimizeVertices ? "true" : "false") << ",\n";
        ss << "    \"generateNormals\": " << (settings.generateNormals ? "true" : "false") << "\n";
        ss << "  }\n";
        ss << "}\n";

        return ss.str();
    }
//*******SHADER******* */
    std::string AssetDescriptorGenerator::BuildDescriptorJson(
        const std::string& sourcePath,
        const ShaderSettings& settings) const
    {
        std::ostringstream ss;
        std::string relativePath = Engine::getRelativeAssetPath(sourcePath);
        ss << "{\n";
        ss << "  \"sourcePath\": \"" << EscapeJson(relativePath) << "\",\n";
        ss << "  \"shaderSettings\": {\n";

        if (!settings.vertexShader.empty()) {
            ss << "    \"vertexShader\": \"" << EscapeJson(settings.vertexShader) << "\",\n";
        }
        if (!settings.fragmentShader.empty()) {
            ss << "    \"fragmentShader\": \"" << EscapeJson(settings.fragmentShader) << "\",\n";
        }

        ss << "    \"outputFormat\": \"" << EscapeJson(settings.outputFormat) << "\",\n";
        ss << "    \"targetAPI\": \"" << EscapeJson(settings.targetAPI) << "\",\n";
        ss << "    \"targetVersion\": \"" << EscapeJson(settings.targetVersion) << "\",\n";
        ss << "    \"optimizationLevel\": \"" << EscapeJson(settings.optimizationLevel) << "\",\n";
        ss << "    \"stripDebugInfo\": " << (settings.stripDebugInfo ? "true" : "false") << "\n";
        ss << "  }\n";
        ss << "}\n";

        return ss.str();
    }

    // ==================== HELPER FUNCTIONS ====================
    std::string AssetDescriptorGenerator::EscapeJson(const std::string& s) {
        std::string escaped;
        escaped.reserve(s.size());

        for (char c : s) {
            switch (c) {
                case '\"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\b': escaped += "\\b"; break;
                case '\f': escaped += "\\f"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default:
                    if (c < 0x20) {
                        // Escape control characters
                        char buf[7];
                        snprintf(buf, sizeof(buf), "\\u%04x", c);
                        escaped += buf;
                    } else {
                        escaped += c;
                    }
                    break;
            }
        }

        return escaped;
    }


    bool AssetDescriptorGenerator::EnsureDirectory(const std::string& path) {
        try {
            fs::path p(path);


            // Check if already exists
            if (fs::exists(p)) {
                if (fs::is_directory(p)) {
                    return true;
                }
                else {
                    LOG_ERROR("Path exists but is not a directory!", path);
                    return false;
                }
            }

            // Try to create with error code
            std::error_code ec;
            bool created = fs::create_directories(p, ec);

            if (ec) {
                LOG_ERROR("Failed to create directory: ", path);
                LOG_ERROR("  Error: ", ec.message(), " (code: ", ec.value(), ")");
                return false;
            }

            // Verify creation
            if (fs::exists(p) && fs::is_directory(p)) {
                LOG_DEBUG("Directory created successfully");
                return true;
            }

            LOG_ERROR("Creation succeeded but directory doesn't exist!", path);
            return false;

        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception creating directory: ", path, " - ", e.what());
            return false;
        }
    }

    bool AssetDescriptorGenerator::WriteText(const std::string& path, const std::string& text) {
        try {
            LOG_DEBUG("Opening file for writing: ", path);

            std::ofstream file(path);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file: ", path);
                return false;
            }

            file << text;

            if (!file.good()) {
                LOG_ERROR("Failed to write to file: ", path);
                return false;
            }

            file.close();
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception writing file: ", path, " - ", e.what());
            return false;
        }
    }
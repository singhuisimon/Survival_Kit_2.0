/**
 * @file ResourcePaths.cpp
 * @brief Implementation of ResourcePaths class.
 * @details File system organization and path management implementation.
 * @author Wai Lwin Thit
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


 //include header files here
#include "ResourcePaths.h"
#include "AssetManager.h"

//include libraries 
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>


namespace Engine {

    //constructor 
    ResourcePaths::ResourcePaths() {
        //initialize the paths to directory assets
        std::string asset_path = getAssetsPath();

        m_descriptors_root_path = asset_path + "Descriptors/";
        m_intermediate_root_path = asset_path + "/Cache/Intermediate/";

        // CHANGED: Use Compiled folder at the same level as Assets folder
        // Get the parent directory of Assets (the repository root containing Assets folder)
        std::filesystem::path assets_dir(asset_path);
        std::filesystem::path repo_root = assets_dir.parent_path();
        m_compiled_root_path = (repo_root / "Compiled").string() + "/";
    }

    //initialize directory structure
    bool ResourcePaths::initializeDirectories() {
        try {
            // Create root directories
            if (!createDirectoryIfNotExists(m_descriptors_root_path)) return false;
            if (!createDirectoryIfNotExists(m_intermediate_root_path)) return false;
            if (!createDirectoryIfNotExists(m_compiled_root_path)) return false;

            // Create type-specific directories
            if (!createResourceTypeDirectories(m_descriptors_root_path)) return false;
            if (!createResourceTypeDirectories(m_intermediate_root_path)) return false;
            if (!createResourceTypeDirectories(m_compiled_root_path)) return false;

            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }



    void ResourcePaths::setDescriptorRootPath(const std::string& path) {
        m_descriptors_root_path = ensureTrailingSeparator(path);
    }

    void ResourcePaths::setIntermediateRootPath(const std::string& path) {
        m_intermediate_root_path = ensureTrailingSeparator(path);
    }
    void ResourcePaths::setCompiledRootPath(const std::string& path) {
        m_compiled_root_path = ensureTrailingSeparator(path);
    }

    const std::string& ResourcePaths::getDescriptorRootPath() const {
        return m_descriptors_root_path;
    }

    const std::string& ResourcePaths::getIntermediateRootPath() const {
        return m_intermediate_root_path;
    }

    const std::string& ResourcePaths::getCompiledRootPath() const {
        return m_compiled_root_path;
    }

    std::string ResourcePaths::getDescriptorFilePath(const xresource::full_guid& guid, ResourceType type) const {
        std::string type_folder = getResourceTypeFolder(type);
        std::string sub_dir = generateGUIDSubdirectory(guid);

        // Generate hex filename from GUID
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << guid.m_Instance.m_Value;
        std::string filename = ss.str() + ".desc";

        return m_descriptors_root_path + type_folder + "/" + sub_dir + filename;
    }

    std::string ResourcePaths::getDescriptorDirectoryPath(const xresource::full_guid& guid, ResourceType type) const {
        std::string type_folder = getResourceTypeFolder(type);
        std::string sub_dir = generateGUIDSubdirectory(guid);

        return m_descriptors_root_path + type_folder + "/" + sub_dir;
    }

    std::string ResourcePaths::getIntermediateFilePath(const std::string& path) const {
        return m_intermediate_root_path + normalizePath(path);
    }


    //compiled file paths
    std::string ResourcePaths::getCompiledFilePath(const xresource::full_guid& guid, ResourceType type) const {
        std::string type_folder = getResourceTypeFolder(type);
       // std::string sub_dir = generateGUIDSubdirectory(guid);

        // Generate filename with type-specific extension
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << guid.m_Instance.m_Value;

        std::string extension;
        switch (type) {
        case ResourceType::TEXTURE:  extension = ".tex"; break;
        case ResourceType::MESH:     extension = ".mesh"; break;
        case ResourceType::MATERIAL: extension = ".mat"; break;
        case ResourceType::AUDIO:    extension = ".audio"; break;
        case ResourceType::SHADER:   extension = ".shader"; break;
        default:                     extension = ".bin"; break;
        }

        std::string filename = ss.str() + extension;
        return m_compiled_root_path + type_folder + "/" + filename;
    }

    bool ResourcePaths::createDirectoryIfNotExists(const std::string& path) const {
        try {
            if (!std::filesystem::exists(path)) {
                return std::filesystem::create_directories(path);
            }
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    std::string ResourcePaths::getResourceTypeFolder(ResourceType type) const {
        return resourceTypeToString(type);
    }

    bool ResourcePaths::createResourceTypeDirectories(const std::string& root_path) const {

        try {
            //create directories for each resource type
            for (int i = 0; i <= static_cast<int>(ResourceType::SHADER); ++i) {
                ResourceType type = static_cast<ResourceType>(i);
                if (type == ResourceType::UNKNOWN) continue;

                std::string type_folder = root_path + getResourceTypeFolder(type) + "/";
                if (!createDirectoryIfNotExists(type_folder)) {
                    return false;
                }
            }
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    //path utilities 
    bool ResourcePaths::fileExists(const std::string& file_path) const {
        return std::filesystem::exists(file_path);
    }

    uint64_t ResourcePaths::getFileModificationTime(const std::string& file_path) const {

        try {
            if (!std::filesystem::exists(file_path)) {
                return 0;
            }

            auto ftime = std::filesystem::last_write_time(file_path);
            auto duration = ftime.time_since_epoch();
            return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        }
        catch (const std::exception&) {
            return 0;
        }
    }

    uint64_t ResourcePaths::getFileSize(const std::string& file_path) const {
        try {
            if (!std::filesystem::exists(file_path)) {
                return 0;
            }
            return std::filesystem::file_size(file_path);
        }
        catch (const std::exception&) {
            return 0;
        }
    }

    std::string ResourcePaths::getRelativePath(const std::string& from, const std::string& to) const {
        try {
            std::filesystem::path from_path(from);
            std::filesystem::path to_path(to);
            return std::filesystem::relative(to_path, from_path).generic_string();
        }
        catch (const std::exception&) {
            return to;
        }
    }

    std::string ResourcePaths::normalizePath(const std::string& path) const {
        std::string normalized = path;
        std::replace(normalized.begin(), normalized.end(), '\\', '/');

        //remove trailing slash
        while (!normalized.empty() && normalized.back() == '/') {
            normalized.pop_back();
        }

        return normalized;
    }

    std::string ResourcePaths::generateGUIDSubdirectory(const xresource::full_guid& guid) const {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << guid.m_Instance.m_Value;
        std::string guid_str = ss.str();

        //Extract two-character directories from GUID
        //using last 4 characters for two levels of directories
        if (guid_str.length() >= 4) {
            std::string dir2 = guid_str.substr(guid_str.length() - 2, 2);
            std::string dir1 = guid_str.substr(guid_str.length() - 4, 2);
            return dir1 + "/" + dir2 + "/";
        }

        return "";
    }

    std::string ResourcePaths::ensureTrailingSeparator(const std::string& path) const {
        if (path.empty()) {
            return path;
        }

        std::string normalized = normalizePath(path);
        if (normalized.back() != '/') {
            normalized += '/';
        }

        return normalized;
    }
} // end of namespace gam300
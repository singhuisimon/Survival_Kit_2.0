/**
 * @file AssetPath.cpp
 * @brief Implementation of asset path management functions.
 * @author Simon Chan, Liliana Hanawardani, Saw Hui Shan
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Utility/AssetPath.h"

#include "../Utility/Logger.h"
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace fs = std::filesystem;

namespace Engine {

    // Base path to the assets directory - modify this to match your project structure
    const std::string BASE_ASSETS_PATH = std::filesystem::current_path().string() + "\\Resources\\";


    // Get the directory where the executable is located
    static fs::path GetExecutableDirectory() {
#ifdef _WIN32
        // Windows: Get the full path to the .exe
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        fs::path exePath(buffer);
        return exePath.parent_path();
#else
        // Linux/Mac: Read /proc/self/exe or use readlink
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            fs::path exePath(buffer);
            return exePath.parent_path();
        }
        return fs::current_path();
#endif
    }

    // Format the filepath for the assets directory
    std::string getAssetsPath() {

        // Get the directory where the executable is running from
        fs::path exeDir = GetExecutableDirectory();
        LOG_INFO("Executable directory: ", exeDir.string());


        // Resources should be next to the executable
        fs::path resourcesPath = exeDir / "Resources";
        LOG_INFO("Looking for Resources at: ", resourcesPath.string());

        if (fs::exists(resourcesPath) && fs::is_directory(resourcesPath)) {
            // Verify it has the expected structure
            fs::path sourcesPath = resourcesPath / "Sources";

            if (fs::exists(sourcesPath)) {
                std::string result = resourcesPath.generic_string();
                if (!result.empty() && result.back() != '/') {
                    result += '/';
                }
                LOG_INFO("Assets path: ", result);  // Only show this in normal operation
                return result;
            }
            else {
                LOG_WARNING("Resources folder exists but missing Sources/ subfolder");
            }
        }
        else {
            LOG_ERROR("Resources folder not found at: ", resourcesPath.string());
        }

        // Fallback
        std::string fallback = resourcesPath.generic_string();
        if (!fallback.empty() && fallback.back() != '/') {
            fallback += '/';
        }

        LOG_ERROR("Using fallback path: ", fallback);
        LOG_ERROR("Assets may not load correctly!");

        return fallback;
    }

    // Get the absolute filepath for file in assets directory
    std::string getAssetFilePath(const std::string& relativePath) {

        std::string formattedPath = relativePath;
        std::string currentPath = getAssetsPath();

        for (char& c : formattedPath) {
            if (c == '\\') c = '/';
        }

        // Remove leading slash if present to avoid double slash
        if (!formattedPath.empty() && (formattedPath[0] == '/' || formattedPath[0] == '\\')) {
            formattedPath = formattedPath.substr(1);
        }

        return currentPath + formattedPath;
    }

    // For Descriptor Generation for Assets

    //get repo root by finding the Survival_Kit folder structure
    std::string getRepositoryRoot() {
        fs::path currentPath = fs::current_path();

        //search upwards
        while (!currentPath.empty()) {
            //check if able to find Assets folder
            if (fs::exists(currentPath / "Assets")) {
                //verification
                if (currentPath.filename() == "Survival_Kit" ||
                    fs::exists(currentPath / "Survival_Kit.sln") ||
                    fs::exists(currentPath / ".git")) {
                    return currentPath.string();
                }
            }

            //check for nested structure
            if (fs::exists(currentPath / "Survival_Kit" / "Survival_Kit" / "Survival_Kit" / "Assets")) {
                return (currentPath / "Survival_Kit" / "Survival_Kit" / "Survival_Kit").string();
            }

            currentPath = currentPath.parent_path();
        }

        return fs::current_path().string();
    }


    //get local cache directory (for intermediate files, NOT descriptors
    std::string getLocalCachePath() {
        fs::path repoRoot = getRepositoryRoot();
        fs::path cachePath = repoRoot / "Cache";

        // create the cache directory if it dosen't exist
        if (!fs::exists(cachePath)) {
            std::error_code ec;
            fs::create_directories(cachePath, ec);
        }

        return cachePath.generic_string();
    }


    //get intermediate directory for processed assets
    std::string getIntermediatePath()
    {
        fs::path cachePath = getLocalCachePath();
        fs::path intermediatePath = cachePath / "Intermediate";

        //create directory if it dosen't exist
        if (!fs::exists(intermediatePath))
        {
            std::error_code ec;
            fs::create_directories(intermediatePath, ec);
        }

        return intermediatePath.generic_string();
    }

    std::string getDescriptorsPath() {
        // Get the base Assets path and append Descriptors
        std::string assetsPath = getAssetsPath();

        // Ensure path ends with separator
        if (!assetsPath.empty() && assetsPath.back() != '/' && assetsPath.back() != '\\') {
            assetsPath += "/";
        }

        return assetsPath + "Descriptors/";
    }

    //build a descriptor file path with the required structure: Assets/Descriptors/AssetType/Dir1/Dir2/GUID.desc/filename
    std::string buildDescriptorPath(const std::string& assetType,
        const std::string& guid,
        const std::string& filename) {
        // Ensure GUID is at least 16 characters
        if (guid.length() < 16) {
            // If GUID is too short, pad with zeros or return error path
            return "";
        }

        // Extract Dir1 (last 2 chars) and Dir2 (second-to-last 2 chars)
        std::string dir1 = guid.substr(12, 2);
        std::string dir2 = guid.substr(14, 2);

        // Get the base Assets path
        fs::path assetsPath = getAssetsPath();

        // Build the complete path: Assets/Descriptors/AssetType/Dir1/Dir2/GUID.desc/
        fs::path descriptorDir = assetsPath / "Descriptors" / assetType / dir1 / dir2 / (guid + ".desc");

        // Create all necessary directories
        if (!fs::exists(descriptorDir)) {
            std::error_code ec;
            fs::create_directories(descriptorDir, ec);
        }

        // Return the full path including the filename
        return (descriptorDir / filename).generic_string();
    }

    std::string getRepository() {
        fs::path currentPath = fs::current_path();

        while (!currentPath.empty()) {
            if (fs::exists(currentPath / "Survival_Kit.sln") ||
                fs::exists(currentPath / ".git")) {
                std::cout << "[DEBUG] Repo root found: " << currentPath << std::endl;
                return currentPath.string();
            }
            currentPath = currentPath.parent_path();
        }

        std::cout << "[WARN] Could not find repo root, using current path: "
            << fs::current_path() << std::endl;
        return fs::current_path().string();
    }

    std::string getManagedScriptsPath() {
        fs::path repoRoot = getRepository();

        // Look for both "ManagedScripts" and "managedscripts"
        fs::path managedScriptsPath = repoRoot / "ManagedScripts";
        if (!fs::exists(managedScriptsPath)) {
            fs::path alt = repoRoot / "managedscripts";
            if (fs::exists(alt)) managedScriptsPath = alt;
        }

        std::cout << "[DEBUG] ManagedScripts path: " << managedScriptsPath << std::endl;
        return managedScriptsPath.generic_string();
    }

    std::vector<std::string> getAvailableScripts() {
        std::vector<std::string> scripts;
        fs::path scriptsPath = getManagedScriptsPath();

        if (!fs::exists(scriptsPath)) {
            std::cerr << "[ERROR] ManagedScripts folder not found: " << scriptsPath << std::endl;
            return scripts;
        }

        for (const auto& entry : fs::directory_iterator(scriptsPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cs") {
                scripts.push_back(entry.path().stem().string()); // remove ".cs"
            }
        }

        // Print what we found
        std::cout << "[DEBUG] Found " << scripts.size() << " script(s):\n";
        for (const auto& s : scripts)
            std::cout << "  - " << s << std::endl;

        return scripts;
    }

    std::string getRelativeAssetPath(const std::string& absolutePath)
    {
        std::filesystem::path full(absolutePath);
        std::string norm = full.lexically_normal().string();

        // normalize slashes
        std::replace(norm.begin(), norm.end(), '/', '\\');

        // look for "\Resources\" to strip everything before it
        size_t pos = norm.find("\\Resources\\");
        if (pos != std::string::npos)
        {
            // keep everything from "\Resources" onwards
            return norm.substr(pos);
        }

        // fallback: just filename
        return "\\Resources\\" + full.filename().string();
    }

    std::string escapeBackslashesForJSON(const std::string& input)
    {
        std::string escaped;
        escaped.reserve(input.size() * 2);

        for (char c : input)
        {
            if (c == '\\')
                escaped += "\\\\";
            else
                escaped += c;
        }

        return escaped;
    }

} // end of namespace gam300

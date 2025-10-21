/**
 * @file AssetPath.h
 * @brief Helper functions for asset path management.
 * @author Simon Chan, Liliana Hanawardani, Saw Hui Shan
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __ASSET_PATH_H__
#define __ASSET_PATH_H__

#include <string>
#include <vector>

    /**
     * @brief Get the absolute path to the assets directory.
     * @return The absolute path to the assets directory.
     */
    std::string getAssetsPath();

    /**
     * @brief Get the absolute path to a file in the assets directory.
     * @param relativePath The path relative to the assets directory.
     * @return The absolute path to the asset file.
     */
    std::string getAssetFilePath(const std::string& relativePath);

    /**
     * @brief Get the repository root directory by finding the Survival_Kit folder.
     * @return The absolute path to the repository root.
     */
    std::string getRepositoryRoot();

    /**
     * @brief Get the local cache directory path (for intermediate files).
     * @return The absolute path to the cache directory.
     */
    std::string getLocalCachePath();

    /**
     * @brief Get the intermediate directory for processed/compiled assets.
     * @return The absolute path to the intermediate directory.
     */
    std::string getIntermediatePath();

    /**
     * @brief Get the descriptors folder path (Assets/Descriptors/).
     * @return The absolute path to the descriptors folder.
     */
    std::string getDescriptorsPath();


    /**
     * @brief Build a descriptor file path using the standard structure.
     * Creates path: Assets/Descriptors/AssetType/Dir1/Dir2/GUID.desc/filename
     * @param assetType The type of asset (e.g., "Texture", "Mesh")
     * @param guid The full GUID string in hex
     * @param filename The descriptor filename (default: "Descriptor.txt")
     * @return The full path to the descriptor file.
     */
    std::string buildDescriptorPath(const std::string& assetType,
        const std::string& guid,
        const std::string& filename = "Descriptor.txt");

    std::string getRepository();
    std::string getManagedScriptsPath();
    std::vector<std::string> getAvailableScripts();

    std::string getRelativeAssetPath(const std::string& absolutePath);
    std::string escapeBackslashesForJSON(const std::string& input);

#endif // __ASSET_PATH_H__
/**
 * @file ResourcePaths.h
 * @brief File system organization utilities for the resource pipeline.
 * @details Handles directory structure creation, path resolution, and file system utilities.
 * @author Wai Lwin Thit
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#ifndef _RESOURCE_PATHS_H
#define _RESOURCE_PATHS_H

//include necessary headers
#include "ResourceTypes.h"
#include "../Utility/AssetPath.h"

//include c libraries
#include <string>
#include <filesystem>

namespace Engine {
	/**
	*/

	class ResourcePaths {

	private: 
		std::string m_descriptors_root_path; //descriptor files root path
		std::string m_intermediate_root_path;	//intermediate files root path
		std::string m_compiled_root_path;	//compiled files root path

	public: 

		ResourcePaths(); 

		bool initializeDirectories();


		//path setters

		/**
		 * @brief Set the root path for descriptor files.
		 * @param path Root path for descriptors.
		 */
		void setDescriptorRootPath(const std::string& path); 

		/**
		 * @brief Set the root path for intermediate files.
		 * @param path Root path for intermediate files.
		 */
		void setIntermediateRootPath(const std::string& path);

		/**
		 * @brief Set the root path for compiled files.
		 * @param path Root path for compiled files.
		 */
		void setCompiledRootPath(const std::string& path);


		//path getters
		/**
		 * @brief Get the root path for descriptor files.
		 * @return Root path for descriptors.
		 */
		const std::string& getDescriptorRootPath() const;

		/**
		 * @brief Get the root path for intermediate files.
		 * @return Root path for intermediate files.
		 */
		const std::string& getIntermediateRootPath() const;

		/**
		 * @brief Get the root path for compiled binary files.
		 * @return Root path for compiled files.
		 */
		const std::string& getCompiledRootPath() const;

		/**
		 * @brief Get the full path to a descriptor file.
		 * @param guid The resource GUID.
		 * @param type The resource type.
		 * @return Full path to the descriptor file.
		 */
		std::string getDescriptorFilePath(const xresource::full_guid& guid, ResourceType type) const;

		/**
		 * @brief Get the directory path for a descriptor file.
		 * @param guid The resource GUID.
		 * @param type The resource type.
		 * @return Directory path for the descriptor file.
		 */
		std::string getDescriptorDirectoryPath(const xresource::full_guid& guid, ResourceType type) const;

		// ========== Resource File Paths ==========

		/**
		 * @brief Get the full path to an intermediate resource file.
		 * @param relative_path Path relative to intermediate root.
		 * @return Full path to the intermediate file.
		 */
		std::string getIntermediateFilePath(const std::string& relative_path) const;

		/**
		 * @brief Get the full path to a compiled resource file.
		 * @param guid The resource GUID.
		 * @param type The resource type.
		 * @return Full path to the compiled file.
		 */
		std::string getCompiledFilePath(const xresource::full_guid& guid, ResourceType type) const;

		//directory operations

		/**
		 * @brief Create directory if it doesn't exist.
		 * @param path Directory path to create.
		 * @return True if successful or already exists, false otherwise.
		 */
		bool createDirectoryIfNotExists(const std::string& path) const;

		/**
		 * @brief Get the resource type folder name.
		 * @param type The resource type.
		 * @return Folder name for the resource type.
		 */
		std::string getResourceTypeFolder(ResourceType type) const;

		/**
		 * @brief Create all type-specific directories.
		 * @param root_path Root path under which to create type folders.
		 * @return True if successful, false otherwise.
		 */
		bool createResourceTypeDirectories(const std::string& root_path) const;

		//Path Utilities

		/**
		 * @brief Check if a file exists.
		 * @param file_path Path to the file.
		 * @return True if file exists, false otherwise.
		 */
		bool fileExists(const std::string& file_path) const;

		/**
		 * @brief Get file modification time.
		 * @param file_path Path to the file.
		 * @return Modification time as timestamp, or 0 if file doesn't exist.
		 */
		uint64_t getFileModificationTime(const std::string& file_path) const;

		/**
		 * @brief Get file size in bytes.
		 * @param file_path Path to the file.
		 * @return File size in bytes, or 0 if file doesn't exist.
		 */
		uint64_t getFileSize(const std::string& file_path) const;

		/**
		 * @brief Get relative path from one path to another.
		 * @param from Source path.
		 * @param to Target path.
		 * @return Relative path from 'from' to 'to'.
		 */
		std::string getRelativePath(const std::string& from, const std::string& to) const;


	    std::string normalizePath(const std::string& path) const;

	private:
			 /**
			  * @brief Generate GUID-based subdirectory structure.
			  * @param guid The GUID to use for directory structure.
			  * @return Subdirectory path (e.g., "AB/CD/").
			  */
			 std::string generateGUIDSubdirectory(const xresource::full_guid& guid) const;

			 /**
			  * @brief Ensure path ends with a separator.
			  * @param path Path to process.
			  * @return Path with trailing separator.
			  */
			 std::string ensureTrailingSeparator(const std::string& path) const;
	};
}

#endif // !_RESOURCE_PATHS_H

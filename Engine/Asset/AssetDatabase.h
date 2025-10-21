/**
 * @file AssetDatabase.h
 * @brief Defines asset metadata storage and lookup utilities. 
 * @author 
 * @date 15/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __ASSET_DATABASE_H__
#define __ASSET_DATABASE_H__

#include <string>
#include <unordered_map>
#include <vector> // iteration helper

//files
#include "../xresource_guid/include/xresource_guid.h"
#include "ResourceTypes.h"  // Use existing ResourceType enum

namespace gam300{

    struct AssetRecord{

        //GUID 
        xresource::instance_guid guid;
        ResourceType type = ResourceType::UNKNOWN;

        //filepaths 
        std::string sourcePath;
        
        //metadata 
        std::string ext; //file extension
        std::string contentHash;
        std::time_t lastWriteTime; 

        //Status
        bool valid = false;
        
    };

	/**
	* @brief Map of GUID <-> AssetRecord with helpers for path lookups.
	*/
    struct AssetDatabase {


		/** Load database from a text file. Returns false on I/O failure. */
		bool Load(const std::string& file);

		/** Save database to a text file. Returns false on I/O failure. */
		bool Save(const std::string& file) const;

		/**
		* @brief Ensure there is an id for a given *source* path and return it.
		* @details If the path is new, a record is created with a fresh id.
		* Path is normalized to forward slashes for the key.
		*/
		xresource::instance_guid EnsureIdForPath(const std::string& path);

        //Look up (const)
		const AssetRecord* Find(xresource::instance_guid id) const;
		const AssetRecord* FindBySource(const std::string& path) const;

        //Look up (mutable)
		AssetRecord* FindMutable(xresource::instance_guid id);
		AssetRecord* FindBySourceMutable(const std::string& path);


        //Remove function 

		/** Remove a record by id. Returns true if a record was erased. */
		bool Remove(xresource::instance_guid id);

		/** Remove a record by source path. Returns true if a record was erased. */
		bool RemoveBySource(const std::string& path);

		/** @return A convenience vector of mutable pointers to all records. */
		std::vector<AssetRecord*> AllMutable();

		/** Clear the whole database (in-memory). */
		void Clear();

        //Utility functions
		/** @return Number of records currently stored. */
		size_t Count() const { return byId.size(); }

		/** Normalize a path to forward slashes and no trailing slash. */
		static std::string NormalizePath(const std::string& path);

		/** Extract the lowercase extension (including the dot) from a path. */
		static std::string ExtensionLower(const std::string& path);


        //storage
		std::unordered_map<xresource::instance_guid, AssetRecord> byId; //!< id -> record
		std::unordered_map<std::string, xresource::instance_guid> bySourcePath; //!< normalized source -> id

    };

}//endof nnamespace gam300

#endif
/**
 * @file AssetDatabase.cpp
 * @brief Implements asset database management functions. 
 * @author
 * @date 15/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "AssetDatabase.h"
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <algorithm>

//external library for GUID
#include "../xresource_guid/include/xresource_guid.h"

namespace gam300 {

	//generate GUIDs using xresource_guid library
	static xresource::instance_guid GenId()
	{
        return xresource::instance_guid::GenerateGUIDCopy();
	}

	std::string AssetDatabase::NormalizePath(const std::string& path)
	{
		// Convert to a canonical, forward-slash style so map keys are stable
		namespace fs = std::filesystem;
		fs::path p(path);
		p = p.lexically_normal();
		return p.generic_string();
	}

	std::string AssetDatabase::ExtensionLower(const std::string& path)
	{
		namespace fs = std::filesystem;
		std::string ext = fs::path(path).extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
			});
		return ext;
	}

	bool AssetDatabase::Load(const std::string& file)
	{

        //TODO:: update this for after the storage format is finalized

		// Reset in-memory state first to avoid mixing sessions
		byId.clear();
		bySourcePath.clear();

		return true;
	}

	bool AssetDatabase::Save(const std::string& file) const
	{
        //TODO:: update later after finalizing the storage format
        return true;
	}

	xresource::instance_guid AssetDatabase::EnsureIdForPath(const std::string& path)
	{
		const std::string key = NormalizePath(path);


		if (auto it = bySourcePath.find(key); it != bySourcePath.end())
			return it->second;


		// Create new record with generated guid
		xresource::instance_guid guid = GenId();
		AssetRecord rec;
		rec.guid = guid;
		rec.sourcePath = key;
		rec.ext = ExtensionLower(key);


		byId[guid] = rec;
		bySourcePath[key] = guid;
		return guid;
	}

	const AssetRecord* AssetDatabase::Find(xresource::instance_guid guid) const
	{
		auto it = byId.find(guid);
		return (it == byId.end()) ? nullptr : &it->second;
	}

	AssetRecord* AssetDatabase::FindMutable(xresource::instance_guid guid)
	{
		auto it = byId.find(guid);
		return (it == byId.end()) ? nullptr : &it->second;
	}

	const AssetRecord* AssetDatabase::FindBySource(const std::string& path) const
	{
		const std::string key = NormalizePath(path);
		auto it = bySourcePath.find(key);
		if (it == bySourcePath.end()) return nullptr;
		return Find(it->second);
	}

	AssetRecord* AssetDatabase::FindBySourceMutable(const std::string& path)
	{
		const std::string key = NormalizePath(path);
		auto it = bySourcePath.find(key);
		if (it == bySourcePath.end()) return nullptr;
		return FindMutable(it->second);
	}

	bool AssetDatabase::Remove(xresource::instance_guid guid)
	{
		auto it = byId.find(guid);
		if (it == byId.end()) return false;
		bySourcePath.erase(it->second.sourcePath);
		byId.erase(it);
		return true;
	}

	bool AssetDatabase::RemoveBySource(const std::string& path)
	{
		const std::string key = NormalizePath(path);
		auto it = bySourcePath.find(key);
		if (it == bySourcePath.end()) return false;
		return Remove(it->second);
	}

	std::vector<AssetRecord*> AssetDatabase::AllMutable()
	{
		std::vector<AssetRecord*> v;
		v.reserve(byId.size());
		for (auto& [guid, rec] : byId)
			v.push_back(&rec);
		return v;
	}

	void AssetDatabase::Clear()
	{
		byId.clear();
		bySourcePath.clear();
	}

} //end of namespace gam300
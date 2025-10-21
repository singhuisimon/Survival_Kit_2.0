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
#include "../Utility/Logger.h"

namespace fs = std::filesystem;

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
		std::ifstream in(file);
		if (!in.is_open()) {
			LOG_DEBUG("Asset database not found (first run): ", file);
			return false;
		}

		Clear();

		std::string line;
		int lineNum = 0;
		int loadedCount = 0;

		while (std::getline(in, line)) {
			lineNum++;

			// Skip comments and empty lines
			if (line.empty() || line[0] == '#') {
				continue;
			}

			// Parse line: guid|type|sourcePath|ext|contentHash|lastWriteTime|valid
			std::stringstream ss(line);
			std::string guidStr, typeStr, sourcePath, ext, contentHash, timeStr, validStr;

			if (!std::getline(ss, guidStr, '|')) continue;
			if (!std::getline(ss, typeStr, '|')) continue;
			if (!std::getline(ss, sourcePath, '|')) continue;
			if (!std::getline(ss, ext, '|')) continue;
			if (!std::getline(ss, contentHash, '|')) continue;
			if (!std::getline(ss, timeStr, '|')) continue;
			if (!std::getline(ss, validStr)) continue;

			try {
				AssetRecord rec;
				rec.guid.m_Value = std::stoull(guidStr, nullptr, 16);
				rec.type = static_cast<ResourceType>(std::stoi(typeStr));
				rec.sourcePath = sourcePath;
				rec.ext = ext;
				rec.contentHash = contentHash;
				rec.lastWriteTime = static_cast<std::time_t>(std::stoll(timeStr));
				rec.valid = (validStr == "1");

				byId[rec.guid] = rec;
				bySourcePath[rec.sourcePath] = rec.guid;
				loadedCount++;
			}
			catch (const std::exception& e) {
				LOG_WARNING("Failed to parse database line ", lineNum, ": ", e.what());
			}
		}

		LOG_INFO("Loaded ", loadedCount, " asset records from database");
		return true;
	}

	bool AssetDatabase::Save(const std::string& file) const
	{

		// Ensure directory exists
		fs::path filepath(file);
		if (filepath.has_parent_path()) {
			fs::create_directories(filepath.parent_path());
		}

		std::ofstream out(file, std::ios::trunc);
		if (!out.is_open()) {
			LOG_ERROR("Failed to open database file for writing: ", file);
			return false;
		}

		// Write header
		out << "# Asset Database\n";
		out << "# Format: guid|type|sourcePath|ext|contentHash|lastWriteTime|valid\n";
		out << "# Version: 1.0\n\n";

		// Write records
		for (const auto& [guid, rec] : byId) {
			out << std::hex << guid.m_Value << std::dec << '|'
				<< static_cast<int>(rec.type) << '|'
				<< rec.sourcePath << '|'
				<< rec.ext << '|'
				<< rec.contentHash << '|'
				<< rec.lastWriteTime << '|'
				<< (rec.valid ? '1' : '0') << '\n';
		}

		LOG_DEBUG("Saved ", byId.size(), " asset records to database");
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


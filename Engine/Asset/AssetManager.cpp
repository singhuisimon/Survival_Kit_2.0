/**
 * @file AssetManager.cpp
 * @brief Implements the asset management system and compilation pipeline.
 * @author Wai Lwin Thit(20%), Simon Chan(30%), Rio Shannon Yvon Leonardo(50%)
 * @date 18/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#include "AssetManager.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include "../Utility/AssetPath.h"
#include "../Utility/Logger.h"

namespace fs = std::filesystem;


	// Singleton plumbing
	//AssetManager::AssetManager() {
	//	setType("AssetManager");
	//}

	AssetManager& AssetManager::getInstance() {
		static AssetManager instance;
		return instance;
	}

	//============================== COMPILER ===================================

	//=========================== ASSET MANAGER ===================================================
	// Configuration
	void AssetManager::setConfig(const Config& cfg) {
		m_cfg = cfg;
	}

	// startup
	int AssetManager::startUp() {
	/*	if (Manager::startUp())
			return -1;*/

		//LM.writeLog("AssetManager::startUp() - Initializing");

		//set up the configuration
		if (m_cfg.sourceRoots.empty()) {
			m_cfg = createDefaultConfig();
		}

		//get assetpath (with the utility fun)
		std::string assetsPath = Engine::getAssetsPath();

		//create directories 
		try {
			fs::create_directories(assetsPath + "Descriptors");
			fs::create_directories(assetsPath + "DB");

			//LM.writeLog("AssetManager::startUp() - Created working directories");
		}
		catch (const std::exception& e) {
			//LM.writeLog("AssetManager::startUp() - Failed to create directories: %s", e.what());
			return -1;
		}
		// Configure scanner
		m_scanner.setRoots(m_cfg.sourceRoots);
		m_scanner.setExtensions(m_cfg.scanExtensions);
		m_scanner.setIgnoreSubstrings(m_cfg.ignoreSubstrings);
		m_scanner.setIncludeHidden(m_cfg.includeHidden);

		// load snap shot AM.startUp()
		if (!m_cfg.snapshotFile.empty())
			m_scanner.LoadSnapshot(m_cfg.snapshotFile);

		//RegisterDefaultImporters(m_importers);

		// load database file AM.startUp();
		if (!m_cfg.databaseFile.empty())
			m_db.Load(m_cfg.databaseFile);

		//set up the root for descriptor generator
		m_descGen.SetOutputRoot(m_cfg.descriptorRoot);

		// NEW: Initialize compiler system
		//initializeCompilers();

		//LM.writeLog("AssetManager::startUp() - complete");
		return 0;
	}

	//shutdown
	void AssetManager::shutDown() {
		// NEW: Shutdown compilers before other systems
		//shutdownCompilers();

		//save database
		if (!m_cfg.databaseFile.empty())
			m_db.Save(m_cfg.databaseFile);

		//save snapshot
		if (!m_cfg.snapshotFile.empty()) {
			const size_t snapCount = m_scanner.GetSnapshotSize();
			bool success = m_scanner.SaveSnapshot(m_cfg.snapshotFile);
			/*LM.writeLog("AssetManager::shutDown() - Saved snapshot: %zu files, success=%d, path=%s",
				snapCount, success, m_cfg.snapshotFile.c_str());*/
		}

		//LM.writeLog("AssetManager::shutDown() - complete");
		//Manager::shutDown();
	}

	// ==================== COMPILER SYSTEM ====================

	// ==================== CHANGE HANDLING ====================

	//this handleAddedOrModified is 
	void AssetManager::handleAddedOrModified(const std::string& src) {

		// Ensure we have a guid for this source path
		xresource::instance_guid guid = m_db.EnsureIdForPath(src);
		auto* rec = m_db.FindMutable(guid);
		if (!rec) {
			LOG_ERROR("Failed to create/find record for: ", src);
			return;
		}

		//detect the resource type from file extension
		rec->type = detectResourceTypeFromPath(src);
		if (rec->type == ResourceType::UNKNOWN) {
			LOG_WARNING("Unknown resource type: ", src);
			rec->valid = false;
			return;
		}

		//get the basic extension for metadata
		rec->ext = AssetDatabase::ExtensionLower(src);

		//get file timestamp
		try {
			if (fs::exists(src)) {
				auto ftime = fs::last_write_time(src);
				auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
					ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
				rec->lastWriteTime = std::chrono::system_clock::to_time_t(sctp);
			}
		}
		catch (const std::exception& e) {
			LOG_WARNING("Could not get timestamp for ", src);
			rec->lastWriteTime = std::time(nullptr);
		}


		// 4. Mark as valid (we'll generate descriptors next)
		rec->valid = true;

		// 5. Generate descriptor files (Info.txt + Descriptor.txt)
		if (m_cfg.writeDescriptors) {
			// Prepare extras for Info.txt
			DescriptorExtras extras;
			extras.displayName = fs::path(src).filename().string();
			extras.category = resourceTypeToString(rec->type);
			extras.lastImported = std::time(nullptr);

			// Generate type-specific descriptors based on ResourceType
			bool descriptorGenerated = false;
			std::string descriptorPath;

			switch (rec->type) {
			case ResourceType::TEXTURE: {
				TextureSettings settings;
				// Use sensible defaults
				settings.usageType = "COLOR";
				settings.compression = "BC7";
				settings.quality = 0.8f;
				settings.generateMipmaps = true;
				settings.srgb = true;

				descriptorGenerated = m_descGen.GenerateFor(*rec, &extras, settings, &descriptorPath);
				break;
			}

			case ResourceType::MESH: {
				MeshSettings settings;
				// Use sensible defaults
				settings.outputFormat = "CUSTOM";
				settings.includePos = true;
				settings.includeNormals = true;
				settings.includeTexCoords = true;
				settings.indexType = "UINT32";
				settings.optimizeVertices = true;

				descriptorGenerated = m_descGen.GenerateFor(*rec, &extras, settings, &descriptorPath);
				break;
			}

			case ResourceType::AUDIO: {
				AudioSettings settings;
				// Use sensible defaults
				settings.outputFormat = "OGG";
				settings.compression = "VORBIS";
				settings.quality = 0.7f;
				settings.sampleRate = 44100;
				settings.channelMode = "STEREO";

				descriptorGenerated = m_descGen.GenerateFor(*rec, &extras, settings, &descriptorPath);
				break;
			}

			case ResourceType::SHADER: {
				ShaderSettings settings;
				// Use sensible defaults
				settings.outputFormat = "GLSL";
				settings.targetAPI = "OPENGL";
				settings.targetVersion = "460";
				settings.optimizationLevel = "PERFORMANCE";
				settings.stripDebugInfo = true;

				descriptorGenerated = m_descGen.GenerateFor(*rec, &extras, settings);
				break;
			}

			case ResourceType::MATERIAL: {
				// Materials might not need settings initially, just Info.txt
				MeshSettings settings;
				descriptorGenerated = m_descGen.GenerateFor(*rec, &extras, settings, &descriptorPath);
				break;
			}

			default:
				LOG_WARNING("No descriptor settings defined for type: ", resourceTypeToString(rec->type));
				break;
			}

			if (!descriptorGenerated) {
				LOG_ERROR("Failed to generate descriptors for: ", src);
			}
		}

		LOG_INFO("Asset processed: ", src, " (GUID: ", std::hex, rec->guid.m_Value, std::dec,
			", Type: ", resourceTypeToString(rec->type), ")");
	}

	void AssetManager::handleRemoved(const std::string& src) {
		// Find the record before removing it
		const AssetRecord* rec = m_db.FindBySource(src);

		if (!rec) {
			//LM.writeLog("AssetManager - WARNING: No record found for removed file: %s", src.c_str());
			//return;
		}


			//store guid and type for logging
			xresource::instance_guid guid = rec->guid;
			ResourceType type = rec->type;

			// Delete descriptor file
			if (m_cfg.writeDescriptors) {
				std::string descriptorPath = m_descGen.GetDescriptorFolderPath(*rec);

				if (fs::exists(descriptorPath)) {
					fs::remove(descriptorPath);
					//LM.writeLog("AssetManager - Deleted descriptor file: %s", descriptorPath.c_str());
				}

				// Clean up empty parent folders for descriptors
				fs::path currentFolder = fs::path(descriptorPath).parent_path();
				fs::path descriptorsRoot = fs::absolute(m_cfg.descriptorRoot);

				while (currentFolder.has_parent_path()) {
					if (fs::equivalent(currentFolder, descriptorsRoot)) {
						break;
					}

					if (fs::exists(currentFolder) && fs::is_empty(currentFolder)) {
						fs::remove(currentFolder);
						//LM.writeLog("AssetManager - Deleted empty descriptor folder: %s",
						//	currentFolder.string().c_str());
						currentFolder = currentFolder.parent_path();
					}
					else {
						break;
					}
				}
			}
		

		// Remove from database
		if (m_db.RemoveBySource(src)) {
			//LM.writeLog("AssetManager - Removed from DB: %s (GUID: %016llx, Type: %s"
			//	, src.c_str(), guid.m_Value, resourceTypeToString(type).c_str());

			if (!m_cfg.databaseFile.empty()) {
				//save the final databasefile
				m_db.Save(m_cfg.databaseFile);
			}

			if (!m_cfg.snapshotFile.empty()) {
				m_scanner.SaveSnapshot(m_cfg.snapshotFile);

			}
		}
		else {
			//LM.writeLog("AssetManager - ERROR: Failed to remove from database: %s", src.c_str());
		}
	}

	void AssetManager::scanAndProcess() {

		LOG_INFO("===========================================");
		LOG_INFO("  Asset Scan & Process");
		LOG_INFO("===========================================");
		LOG_DEBUG("Snapshot has ", m_scanner.GetSnapshotSize(), " files before scan");

		// Iterate changes from the scanner and act on them
		int addedCount = 0;
		int modifiedCount = 0;
		int removedCount = 0;

		// Iterate changes from the scanner and act on them
		for (const auto& c : m_scanner.Scan()) {
			switch (c.kind) {
			case ScanChange::Kind::Added:
				handleAddedOrModified(c.sourcePath);
				addedCount++;
				break;

			case ScanChange::Kind::Modified:
				handleAddedOrModified(c.sourcePath);
				modifiedCount++;
				break;

			case ScanChange::Kind::Removed:
				handleRemoved(c.sourcePath);
				removedCount++;
				break;
			}
		}

		LOG_INFO("Scan complete:");
		LOG_INFO("  Added: ", addedCount);
		LOG_INFO("  Modified: ", modifiedCount);
		LOG_INFO("  Removed: ", removedCount);
		LOG_INFO("  Total assets: ", m_db.Count());

		// Persist after a pass
		if (!m_cfg.databaseFile.empty()) {

			if (!m_db.Save(m_cfg.databaseFile)) {
				LOG_ERROR("Failed to save asset database");
			}
		}
	
		if (!m_cfg.snapshotFile.empty()) {
			if (!m_scanner.SaveSnapshot(m_cfg.snapshotFile)) {
				LOG_ERROR("Failed to save scan snapshot");
			}
		}

		LOG_INFO("===========================================");
	}



	xresource::instance_guid AssetManager::getAssetId(const std::string& sourcePath) const {
		const AssetRecord* rec = m_db.FindBySource(sourcePath);
		return rec ? rec->guid : 0;
	}

	xresource::instance_guid AssetManager::getAssetIdByFilename(const std::string& filename) const {
		auto allRecords = const_cast<AssetDatabase&>(m_db).AllMutable();

		for (const auto* rec : allRecords) {
			if (!rec) continue;

			fs::path p(rec->sourcePath);
			if (p.filename().string() == filename) {
				return rec->guid;
			}
		}
		return 0;
	}

	const AssetRecord* AssetManager::getAssetRecord(xresource::instance_guid id) const {
		return m_db.Find(id);
	}

	bool AssetManager::assetExists(const std::string& sourcePath) const {
		return m_db.FindBySource(sourcePath) != nullptr;
	}


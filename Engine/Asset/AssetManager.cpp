/**
 * @file AssetManager.cpp
 * @brief Implements the asset management system and compilation pipeline.
 * @author Wai Lwin Thit
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

namespace Engine {



	AssetManager& AssetManager::getInstance() {
		static AssetManager instance;
		return instance;
	}

	//=========================== ASSET MANAGER ===================================================
	// Configuration
	void AssetManager::setConfig(const Config& cfg) {
		m_cfg = cfg;
	}

	// startup
	int AssetManager::startUp() {

		//set up the configuration
		if (m_cfg.sourceRoots.empty()) {
			m_cfg = createDefaultConfig();
		}

		LOG_INFO("Asset Manager StartUp 1");

		//create directories 
		try {
			
			if (!m_cfg.descriptorRoot.empty()) {
				fs::create_directories(m_cfg.descriptorRoot);
			}

			// Create database directory from config
			if (!m_cfg.databaseFile.empty()) {
				fs::path dbPath(m_cfg.databaseFile);
				fs::create_directories(dbPath.parent_path());  // FROM CONFIG!
			}
		}
		catch (const std::exception& e) {
			LOG_ERROR("Failed to create directories: ", e.what());
			return -1;
		}

		LOG_INFO("Asset Manager StartUp 2");

		// Configure scanner
		m_scanner.setRoots(m_cfg.sourceRoots);
		m_scanner.setExtensions(m_cfg.scanExtensions);
		m_scanner.setIgnoreSubstrings(m_cfg.ignoreSubstrings);
		m_scanner.setIncludeHidden(m_cfg.includeHidden);

		LOG_INFO("Asset Manager StartUp 3");

		// load snap shot AM.startUp()
		if (!m_cfg.snapshotFile.empty())
			m_scanner.LoadSnapshot(m_cfg.snapshotFile);

		//RegisterDefaultImporters(m_importers);
		LOG_INFO("Asset Manager StartUp 4");

		// load database file AM.startUp();
		if (!m_cfg.databaseFile.empty())
			m_db.Load(m_cfg.databaseFile);

		LOG_INFO("Asset Manager StartUp 5");

		//set up the root for descriptor generator
		m_descGen.SetOutputRoot(m_cfg.descriptorRoot);

		LOG_INFO("Asset Manager StartUp 6");

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
			if (success)
				LOG_INFO("Snapshot has: ", snapCount);
			else 
				LOG_ERROR("Snapshot not saved at shutdown");
		}

	
	}

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
			LOG_WARNING("Could not get timestamp for ", src, e.what());
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
			LOG_WARNING("No record found for removed file: ", src);
			return;
		}


			//store guid and type for logging
			xresource::instance_guid guid = rec->guid;
			ResourceType type = rec->type;

			// Delete descriptor file
			if (m_cfg.writeDescriptors) {
				std::string descriptorPath = m_descGen.GetDescriptorFolderPath(*rec);

				if (fs::exists(descriptorPath)) {
					try {
						fs::remove_all(descriptorPath);
						LOG_DEBUG("Deleted descriptor folder: ", descriptorPath);

					}
					catch(const std::exception& e){
						LOG_ERROR("Failed to delete descriptor folder: ", descriptorPath,
							" Error: ", e.what());
					}
				}

				// Clean up empty parent folders for descriptors
				try {

					fs::path currentFolder = fs::path(descriptorPath).parent_path();
					fs::path descriptorsRoot = fs::absolute(m_cfg.descriptorRoot);

					while (currentFolder.has_parent_path()) {
						// Check if both paths exist before using equivalent
						if (!fs::exists(currentFolder) || !fs::exists(descriptorsRoot)) {
							break;
						}

						if (fs::equivalent(currentFolder, descriptorsRoot)) {
							break;
						}

						if (fs::exists(currentFolder) && fs::is_empty(currentFolder)) {
							fs::remove(currentFolder);
							LOG_DEBUG("Deleted empty descriptor folder: ",
								currentFolder.string());
							currentFolder = currentFolder.parent_path();
							currentFolder = currentFolder.parent_path();
						}
						else {
							break;
						}
					}

				}
				catch (const std::exception& e) {
					LOG_WARNING("Error cleaning up descriptor folders: ", e.what());
				}

			}
		

		// Remove from database
		if (m_db.RemoveBySource(src)) {
			LOG_INFO("Removed from DB: ", src, " (GUID: ", std::hex, guid.m_Value,
				std::dec, ", Type: ", resourceTypeToString(type), ")");

			if (!m_cfg.databaseFile.empty()) {
				//save the final databasefile
				m_db.Save(m_cfg.databaseFile);
			}

			if (!m_cfg.snapshotFile.empty()) {
				m_scanner.SaveSnapshot(m_cfg.snapshotFile);

			}
		}
		else {
			LOG_ERROR("Failed to remove from database: ", src);
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


	// ========== RESOURCE LOADING HELPER FUNCTIONS ==========

	xresource::instance_guid AssetManager::getGuidFromName(const std::string& filename) const {
		// Use the existing getAssetIdByFilename function
		return getAssetIdByFilename(filename);
	}

	std::string AssetManager::getNameFromGuid(xresource::instance_guid guid) const {
		// Find the asset record
		const AssetRecord* rec = m_db.Find(guid);
		if (!rec) {
			return ""; // Return empty string if not found
		}

		// Extract filename from source path
		fs::path p(rec->sourcePath);
		return p.filename().string();
	}

	bool AssetManager::CompileSingleAsset(xresource::instance_guid guid, bool verbose) {

		//find the asset in the record and get the type 
		const AssetRecord* rec = m_db.Find(guid); 

		if (!rec) {
			LOG_ERROR("AssetManager: Asset not found for GUID: ", std::hex, guid.m_Value); 
			return false;
		}

		if (!rec->valid) {
			LOG_ERROR("AssetManager: Asset is invalid: ", std::hex, guid.m_Value);
			return false;
		}

		//get the path to the Resources folder in the root 
		std::string descriptorPath = getDescriptorRoot();
		std::string compiledPath = getCompiledPath();
		//get the resource type string
		std::string resourceTypeFolder = resourceTypeToString(rec->type);

		//build the GUID hex string
		std::ostringstream guidHex;
		guidHex << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << guid.m_Value;
		std::string guidString = guidHex.str();

		//build the command 
		std::ostringstream command; 

		fs::path exeDir = GetExecutableDirectory();

		//std::filesystem::current_path
		fs::path compilerPath = exeDir / "AssetCompiler.exe";

		//if that does not exist 
		if (!fs::exists(compilerPath)) {
			LOG_ERROR("AssetManager: AssetCompiler.exe not found at: ", compilerPath.string()); 
			LOG_ERROR("BUILD THE ASSET COMPILER"); 
			return false;
		}
		command << "\"";
		command << "\"" << compilerPath.string() << "\"";
		//command << "\AssetCompiler";
		command << " --guid " << guidString;
		command << " --type " << resourceTypeFolder;
		command << " --input \"" << descriptorPath << "\"";
		command << " --output \"" << compiledPath << "\"";
		if (verbose) {
			command << " --verbose";
		}
		command << "\"";
		LOG_DEBUG("Running command: ", command.str());

		// Execute the compiler
		int result = std::system(command.str().c_str());

		if (result == 0) {
			LOG_INFO("Asset compiled successfully: ", guidString);

			// Update the database
			AssetRecord* mutableRec = m_db.FindMutable(guid);
			if (mutableRec) {
				mutableRec->needsRecompile = false;
				mutableRec->descriptorModifiedTime = std::time(nullptr);

				std::string compiledFilePath = getCompiledFilePath(guid, mutableRec->type);
				if (fs::exists(compiledFilePath)) {
					mutableRec->lastCompiledTime = fs::last_write_time(compiledFilePath).time_since_epoch().count();
				}
			}

			return true;
		}
		else {
			LOG_ERROR("Asset compilation failed with code: ", result);
			return false;
		}

	}

	bool AssetManager::CompileAllAsset(bool verbose) {
		//build the command 
		std::ostringstream command;

		fs::path exeDir = GetExecutableDirectory();

		//std::filesystem::current_path
		fs::path compilerPath = exeDir / "AssetCompiler.exe";

		//if that does not exist 
		if (!fs::exists(compilerPath)) {
			LOG_ERROR("AssetManager: AssetCompiler.exe not found at: ", compilerPath.string());
			LOG_ERROR("BUILD THE ASSET COMPILER");
			return false;
		}

		std::string descriptorPath = getDescriptorRoot();
		std::string compiledPath = getCompiledPath();


		command << "\"" << compilerPath.string() << "\"";
		command << " --input " << descriptorPath;
		command << " --output " << compiledPath;

		if (verbose) {
			command << " --verbose";
		}
		LOG_DEBUG("Running command: ", command.str());

		// Execute the compiler
		int result = std::system(command.str().c_str());

		if (result == 0) {
			LOG_INFO("All Assets Compiled Successfully"); 
			return true;
		}
		else {
			LOG_DEBUG("All Assets Failed to Compile"); 
			return false;
		}


	}

	std::string AssetManager::getCompiledFilePath(xresource::instance_guid guid, ResourceType type)const {
		std::ostringstream guidHex;
		guidHex << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << guid.m_Value;

		std::string extension;
		switch (type) {
		case ResourceType::MESH: extension = ".mesh"; break;
		case ResourceType::TEXTURE: extension = ".tex"; break;
		case ResourceType::AUDIO: extension = ".audio"; break;
		case ResourceType::SHADER: extension = ".shader"; break;
		default: extension = ".bin"; break;
		}

		fs::path compiledPath = fs::path(getCompiledPath())
			/ resourceTypeToString(type)
			/ (guidHex.str() + extension);

		return compiledPath.string();
	}



}// end of namespace Engine

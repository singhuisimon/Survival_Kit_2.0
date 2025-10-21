#pragma once


/**
 * @file AssetManager.h
 * @brief Declares the AssetManager for coordinating asset pipeline tasks.
 * @author
 * @date 18/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#ifndef __ASSET_MANAGER_H__
#define __ASSET_MANAGER_H__

#include <string>
#include <vector>
#include <ctime>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

// // Manager base + logging
//#include "Manager.h"
//#include "LogManager.h"

// Pipeline headers (In Pipeline folder)
#include "AssetDatabase.h"
#include "AssetScanner.h"
#include "AssetDescriptorGenerator.h" 


//asset path
#include "../Utility/AssetPath.h"


//define acronym for easier access 
#define AM AssetManager::getInstance()




	/**
	* @class AssetManager
	* @brief Central editor tool coordinating scanning, importing and DB.
	*/
	class AssetManager {
	private:
		AssetManager()=default; // singleton
		AssetManager(const AssetManager&) = delete; // no copy
		void operator=(const AssetManager&) = delete; // no assign

	public:
		// Singleton accessor (same pattern as other managers)
		static AssetManager& getInstance();


		// ---------------- Configuration ----------------
		struct Config {

			//=============== SOURCE PATH ============
			std::vector<std::string> sourceRoots; // Folders to scan for assets

			//=============== OUTPUT PATHS ===========
			std::string descriptorRoot; //Where to write descriptor files to 
			std::string compiledPath; 

			//=============== SCANNING OPTIONS =========
			std::vector<std::string> scanExtensions; // Allowed extensions (no dots); empty = all
			std::vector<std::string> ignoreSubstrings; //Path patterns to ignore
			bool includeHidden = false; // Scan dot-files on POSIX

			//=============== INTERNAL PATHS ===========
			std::string databaseFile; //Asset database persistence
			std::string snapshotFile; //scanner state for incremental scans

			//================ DESCRIPTOR OPTIONS ==================
			bool writeDescriptors = true; // generate descriptor files
		};

		/**
		* @brief Create default configuration for the project
		* @return Configured AssetManager::Config with sensible defaults
		*/
		static Config createDefaultConfig() {
			Config cfg{};

			//returns "Resources/"
			std::string assetsPath = getAssetsPath();

			//=============== SOURCE ROOTS ===============
			cfg.sourceRoots = {
				assetsPath + "Sources/Scenes", //every folder under Sources is a source
				assetsPath + "Sources/Shaders",
				assetsPath + "Sources/Meshes",
				assetsPath + "Sources/Textures",
				assetsPath + "Sources/Audio"

			};

			//================= OUTPUT PATHS ===============
			cfg.descriptorRoot = assetsPath + "Descriptors";
			cfg.compiledPath = assetsPath + "Compiled";

			//================= INTERNAL PATHS ===================
			cfg.databaseFile = assetsPath + "DB/assetdb.txt";
			cfg.snapshotFile = assetsPath + "DB/scan.snapshot";

			// ==================== IGNORE PATTERNS ====================
			cfg.ignoreSubstrings = {};

			// ==================== DESCRIPTOR OPTIONS ====================
			cfg.writeDescriptors = true;

			return cfg;
		}

		/** Apply configuration before startUp() */
		void setConfig(const Config& cfg);


		// --------------- Lifetime (Manager API) ---------------
		int startUp(); // Configure pipeline and warm-load DB
		void shutDown(); // Persist DB and snapshot


		// --------------- Main work ---------------
		/** Scan source roots, import changes, update DB, optionally emit .desc */
		void scanAndProcess();


		// --------------- Accessors ---------------
		AssetDatabase& db() { return m_db; }
		const AssetDatabase& db() const { return m_db; }
		const Config& config() const { return m_cfg; }
		AssetDescriptorGenerator& descriptorGenerator() { return m_descGen; }
		const AssetDescriptorGenerator& descriptorGenerator() const { return m_descGen; }

		//--------------Validating Descriptors ----------
		//void validateExistingDescriptors();

		/**
		 * @brief Get the AssetId for a given source path.
		 * @param sourcePath Path to the asset file (e.g., "Assets/Textures/rock.png")
		 * @return AssetId (0 if not found)
		 */
		xresource::instance_guid getAssetId(const std::string& sourcePath) const;

		/**
		 * @brief Get the xresource::instance_guid for a given filename (searches all assets).
		 * @param filename Just the filename (e.g., "rock.png")
		 * @return xresource::instance_guid (0 if not found, first match if multiple with same name)
		 */
		xresource::instance_guid getAssetIdByFilename(const std::string& filename) const;

		/**
		 * @brief Get the AssetRecord for a given xresource::instance_guid.
		 * @param id The asset ID
		 * @return Pointer to AssetRecord (nullptr if not found)
		 */
		const AssetRecord* getAssetRecord(xresource::instance_guid id) const;

		/**
		 * @brief Check if an asset exists in the database.
		 * @param sourcePath Path to the asset file
		 * @return True if asset exists
		 */
		bool assetExists(const std::string& sourcePath) const;

		//************************** COMPILER SYSTEM ***************************************** */

#if 0
/**
 * @brief Initialize the compiler system
 * @details Call this in startUp() after scanner is initialized
 */
		void initializeCompilers();

		/**
		 * @brief Queue an asset for compilation
		 * @param rec Asset record to compile
		 */
		void queueCompilation(const AssetRecord* rec);

		/**
		 * @brief Process compilation queue (call in scanAndProcess)
		 */
		void processCompilationQueue();

		/**
		 * @brief Get completed compilation results
		 */
		std::vector<CompileResult> getCompletedCompilations();

		/**
		 * @brief Shut down compiler thread safely
		 */
		void shutdownCompilers();

#endif
	private:

		void handleAddedOrModified(const std::string& src);
		void handleRemoved(const std::string& src);
		//static const char* typeName(ResourceType t);

		// State
		Config m_cfg{};
		AssetScanner m_scanner;
		AssetDatabase m_db; //database for the asset records to keep track of
		AssetDescriptorGenerator m_descGen; //for descriptor generator

	};


#endif // __ASSET_MANAGER_H__

#pragma once
#ifndef __ASSET_SCANNER_H__
#define __ASSET_SCANNER_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include <filesystem>

namespace gam300 {

	/**
	* @brief Change description for a scanned source file.
	*/
	struct ScanChange {
		enum class Kind { 
            Added, 
            Modified, 
            Removed } kind; // Change category
		std::string sourcePath; // Absolute/normalized path
	};

	/**
	* @brief Lightweight snapshot data kept for each tracked file.
	* @details We use last write time (as time_t) and file size for a
	* cheap-but-effective change heuristic.
	*/
	struct FileStamp {
		std::time_t lastWrite = 0; // Last write time converted to system_clock
		std::uintmax_t size = 0; // Size in bytes
	};

	/**
	* @class AssetScanner
	* @brief Recursively scans roots and reports file changes since last scan.
	* @details This is a small, focused utility that the AssetImporter can call
	* each frame (or on-demand). It maintains an in-memory snapshot and
	* computes a diff on every Scan().
	*/
	class AssetScanner {
	public:

		/**
		* @brief Replace the set of root directories to scan.
		*/
		void setRoots(const std::vector<std::string>& roots);

		/**
		* @brief Add a single root directory to scan.
		*/
		void addRoot(const std::string& root);

		/**
		* @brief Remove all configured roots.
		*/
		void clearRoots();

		/**
		* @brief Restrict scanning to these extensions (case-insensitive, without dot).
		* @details Example: {"png", "jpg", "fbx", "wav"}. If empty, all files
		* are considered.
		*/
		void setExtensions(const std::vector<std::string>& exts);

		/**
		* @brief Ignore files/directories if any of these substrings appear in the path.
		* @details Simple, fast filter (not glob/regex). Example: {"/Temp/", ".git", ".meta"}.
		*/
		void setIgnoreSubstrings(const std::vector<std::string>& patterns);

		/**
		* @brief Include or skip hidden files (names starting with '.') on POSIX.
		*/
		void setIncludeHidden(bool include_hidden);

		/**
		* @brief Scan all roots and return the changes since the previous scan.
		*/
		std::vector<ScanChange> Scan();

		/**
		* @brief Number of tracked files in the internal snapshot.
		*/
		size_t GetSnapshotSize() const { return m_snapshot.size(); }

		/**
		* @brief Save the internal snapshot to a text file for warm start.
		* @details Format: one record per line => path|timestamp|size
		* @return True on success.
		*/
		bool SaveSnapshot(const std::string& file) const;

		/**
		* @brief Load a snapshot previously saved with saveSnapshot().
		* @return True on success.
		*/
		bool LoadSnapshot(const std::string& file);

	private:

		//helpers
		static std::time_t toTimeT(std::filesystem::file_time_type ftime);
		bool shouldIgnore(const std::filesystem::path& p) const;
		bool extAllowed(const std::filesystem::path& p) const;
		bool isHidden(const std::filesystem::path& p) const;

		// State
		std::vector<std::string> m_roots; // Roots to traverse
		std::unordered_map<std::string, FileStamp> m_snapshot; //  Path -> stamp
		std::unordered_set<std::string> m_exts; //  Lowercase extensions (no dot)
		std::vector<std::string> m_ignore_substrings; //  Cheap ignore filters
		bool m_include_hidden = false; //  Include dotfiles
	};


} //end of namespace gam300

#endif // __ASSET_SCANNER_H__
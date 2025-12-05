/**
 * @file AssetScanner.cpp
 * @brief Incremental file system scanner for detecting asset changes
 *
 * @details
 * Recursively scans configured source directories and detects file changes
 * (additions, modifications, deletions) by comparing against a persistent
 * snapshot. Uses timestamp and file size heuristics for efficient change detection.
 *
 * @author Wai Lwin Thit
 * @date September 17 2025
 *
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "AssetScanner.h"
#include "../Utility/AssetPath.h"
#include "AssetManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>



namespace fs = std::filesystem;

namespace Engine {


	// Convert filesystem clock to system_clock-backed time_t (portable approach)
	std::time_t AssetScanner::toTimeT(fs::file_time_type ftime) {
		try {
			auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
				ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
			return std::chrono::system_clock::to_time_t(sctp);
		}
		catch (...) {
			return 0;
		}
	}

	void AssetScanner::setRoots(const std::vector<std::string>& roots) {
		m_roots = roots;
	}


	void AssetScanner::addRoot(const std::string& root) {
		m_roots.push_back(root);
	}


	void AssetScanner::clearRoots() {
		m_roots.clear();
	}

	void AssetScanner::setExtensions(const std::vector<std::string>& exts) {
		m_exts.clear();
		for (auto e : exts) {
			std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) { return (char)std::tolower(c); });
			if (!e.empty() && e[0] == '.') e.erase(e.begin());
			if (!e.empty()) m_exts.insert(e);
		}
	}


	void AssetScanner::setIgnoreSubstrings(const std::vector<std::string>& patterns) {
		m_ignore_substrings = patterns;
	}


	void AssetScanner::setIncludeHidden(bool include_hidden) {
		m_include_hidden = include_hidden;
	}

	bool AssetScanner::extAllowed(const fs::path& p) const {
		if (m_exts.empty()) return true; // Allow all if unset
		auto ext = p.extension().string();
		if (!ext.empty() && ext[0] == '.') ext.erase(ext.begin());
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
		return m_exts.find(ext) != m_exts.end();
	}


	bool AssetScanner::isHidden(const fs::path& p) const {
		auto name = p.filename().string();
		return !name.empty() && name[0] == '.'; // POSIX heuristic
	}


	bool AssetScanner::shouldIgnore(const fs::path& p) const {
		const auto s = p.string();
		for (const auto& pat : m_ignore_substrings) {
			if (!pat.empty() && s.find(pat) != std::string::npos)
				return true;
		}
		if (!m_include_hidden && isHidden(p))
			return true;
		return false;
	}

	//scanning
	std::vector<ScanChange> AssetScanner::Scan()
	{
		std::vector<ScanChange> changes;                 
		std::unordered_set<std::string> seen;            

		fs::directory_options opts = fs::directory_options::skip_permission_denied;

		// Iterate over source roots 
		for (const auto& root : m_roots)
		{
			if (!fs::exists(root))
				continue;

			try
			{
				for (auto it = fs::recursive_directory_iterator(root, opts);
					it != fs::recursive_directory_iterator(); ++it)
				{
					const auto& entry = *it;
					if (!entry.is_regular_file())
						continue;

					const auto& path = entry.path();
					if (shouldIgnore(path) || !extAllowed(path))
						continue;

                    //get the file path (metadata)
					 std::string pathStr = path.string();

					// Normalize path to forward slashes for consistent comparison
					std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

					const std::time_t t = toTimeT(entry.last_write_time());
					const std::uintmax_t sz = entry.file_size();

                    //mark the file as seen
					seen.insert(pathStr);

                    //check with the previous snapshot
					auto found = m_snapshot.find(pathStr);
                    // not found in the prev snap, it's a new file
					if (found == m_snapshot.end())
					{
                        //push the new file into the changes 
						m_snapshot[pathStr] = { t, sz };
						changes.push_back({ ScanChange::Kind::Added, pathStr });
					}
					else {
						// File exists, check if changed
						FileStamp& stamp = found->second;

						// Add tolerance for timestamp comparison (1 second)
						// Filesystem timestamps can have rounding differences
						bool timeChanged = std::abs(static_cast<long long>(stamp.lastWrite) -
							static_cast<long long>(t)) > 1;
						bool sizeChanged = stamp.size != sz;

						if (timeChanged || sizeChanged) {
							stamp.lastWrite = t;
							stamp.size = sz;
							changes.push_back({ ScanChange::Kind::Modified, pathStr });
						}
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e)
            {
				LOG_ERROR("Filesystem error: ", e.what());
            }
            
		} //finished iterating the Source folders

		// Detect removed files (present in snapshot but not seen now)
		for (auto it = m_snapshot.begin(); it != m_snapshot.end(); )
		{
			if (seen.find(it->first) == seen.end())
			{
                //file was in snapshot but it's NOT seen in this scan = deleted file
				changes.push_back({ ScanChange::Kind::Removed, it->first });
				it = m_snapshot.erase(it); //remove from snapshot
			}
			else
			{
				++it; //or else keep the file in the snapshot
			}
		}

        //finally return the changes
		return changes;
	}

#pragma region LoadSave
	bool AssetScanner::SaveSnapshot(const std::string& file) const
	{
		std::ofstream out(file, std::ios::trunc);
		if (!out.is_open()) return false;
		for (const auto& kv : m_snapshot)
		{

			//convert absolute path to relative path before writing 
			std::string pathToWrite = getRelativeAssetPath(kv.first);
			// Write one record per line: path|timestamp|size\n
			out << pathToWrite << '|' << kv.second.lastWrite << '|' << kv.second.size << '\n';
		}
		return true;
	}

	bool AssetScanner::LoadSnapshot(const std::string& file)
	{
		std::ifstream in(file);
		if (!in.is_open()) return false;
		
		m_snapshot.clear();

		std::string line;
		while (std::getline(in, line))
		{
			std::stringstream ss(line);
			std::string path;
			std::string ts;
			std::string sz;

			if (!std::getline(ss, path, '|')) continue;
			if (!std::getline(ss, ts, '|')) continue;
			std::getline(ss, sz); // read the rest of the line as size

			//convert the relative path back to absolute path for internal storage 
			std::string absolutePath; 
			if (path.find("\\Resources\\") == 0 || path.find("/Resources/") == 0) {
				// Remove the leading \Resources\ or /Resources/ part
				std::string pathWithoutResources = path;
				if (pathWithoutResources.find("\\Resources\\") == 0) {
					pathWithoutResources = pathWithoutResources.substr(11); // Length of "\Resources\"
				}
				else if (pathWithoutResources.find("/Resources/") == 0) {
					pathWithoutResources = pathWithoutResources.substr(11); // Length of "/Resources/"
				}
				std::string sourceRoot = Engine::getAssetsPath();
				fs::path fullPath = fs::path(sourceRoot) / pathWithoutResources;

				//normalize here as well for safety
				std::string result = fullPath.string(); 
				std::replace(result.begin(), result.end(), '\\', '/');
				absolutePath = result;
			}
			else {
				absolutePath = path;
			}

			FileStamp st{};
			try { st.lastWrite = static_cast<std::time_t>(std::stoll(ts)); }
			catch (...) { st.lastWrite = 0; }
			try { st.size = static_cast<std::uintmax_t>(std::stoull(sz)); }
			catch (...) { st.size = 0; }

			m_snapshot[absolutePath] = st;
		}
		return true;
	}
#pragma endregion

}//end of namespace engine

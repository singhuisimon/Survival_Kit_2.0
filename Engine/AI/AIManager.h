#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace Engine {

	//forward declarations
	class BehaviourTree;
	class BehaviourTreeLoader;

	class AIManager {

	public:
		AIManager();
		~AIManager();

		/**
		 * @brief Initialize the AI Manager
		 * @return True if initialization successful
		 */
		bool Init();

		/**
		 * @brief Shutdown and cleanup all resources
		 */
		void Shutdown();

		/**
		 * @brief Load a behaviour tree from file
		 * @param filepath Path to behaviour tree file (e.g., "Loveletter.txt")
		 * @return Pointer to loaded tree, or nullptr if failed
		 * @details If tree is already loaded, returns cached version
		 */
		//BehaviourTree* LoadTree(const std::string& filepath);

		/**
		 * @brief Get a loaded behaviour tree
		 * @param filepath Path to the tree file
		 * @return Pointer to tree if loaded, nullptr otherwise
		 */
		//BehaviourTree* GetTree(const std::string& filepath);

		/**
		 * @brief Reload a behaviour tree from file
		 * @param filepath Path to tree file
		 * @return True if reload successful
		 * @details Useful for hot-reloading during development
		 */
		//bool ReloadTree(const std::string& filepath);

		/**
		 * @brief Unload a specific behaviour tree
		 * @param filepath Path to tree file
		 */
		//void UnloadTree(const std::string& filepath);

		/**
		 * @brief Unload all behaviour trees
		 */
		//void UnloadAllTrees();

		/**
		 * @brief Check if a tree is loaded
		 * @param filepath Path to tree file
		 * @return True if tree is in cache
		 */
		//bool IsTreeLoaded(const std::string& filepath) const;

		/**
		 * @brief Get number of loaded trees
		 */
		//size_t GetLoadedTreeCount() const { return m_TreeCache.size(); }


		/**
		 * @brief Check if manager is initialized
		 */
		bool IsInitialized() const { return m_Initialized; }

	private:

		/**
		* @brief Get full path to tree file
		* @param filepath Relative path
		* @return Full path to asset
		*/
		std::string GetFullPath(const std::string& filepath) const;

		/**
		* @brief Log error helper
		*/
		void LogError(const std::string& message) const;

		bool m_Initialized = false;

		// Cache of loaded behaviour trees
		// Key: filepath, Value: owned tree pointer
		//std::unordered_map<std::string, std::unique_ptr<BehaviourTree>> m_TreeCache;

		//TO BE IMPLEMENTED LATER
		//std::unique_ptr<BehaviourTreeLoader> m_Loader;
	};

} //end of namespace Engine
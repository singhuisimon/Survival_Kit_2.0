#include "AIManager.h"
//#include "BehaviourTree/BehaviourTree.h"
//#include "BehaviourTree/BehaviourTreeLoader.h"
#include "Utility/Logger.h"
#include "Utility/AssetPath.h"

namespace Engine {

    AIManager::AIManager() {}

    AIManager::~AIManager() {
        Shutdown();
    }

    //bool AIManager::Init() {
    //    if (m_Initialized) {
    //        LOG_WARNING("AIManager already initialized.");
    //        return true;
    //    }

    //    LOG_INFO("Initializing AIManager.");

    //    // Create the tree loader
    //    m_Loader = std::make_unique<BehaviourTreeLoader>();

    //    m_Initialized = true;
    //    LOG_INFO("AIManager initialized successfully.");
    //    return true;
    //}

    void AIManager::Shutdown() {
        if (!m_Initialized) {
            return;
        }

        LOG_INFO("AIManager shutting down.");

        // Clear all cached trees
        //UnloadAllTrees();

        //m_Loader.reset();

        m_Initialized = false;
        LOG_INFO("AIManager shutdown completed.");
    }

    //COMMENTED FOR NOW BECAUSE BEHAVIOUR TREE FILES NOT CODED YET

    //BehaviourTree* AIManager::LoadTree(const std::string& filepath) {
    //    if (!m_Initialized) {
    //        LogError("AIManager not initialized");
    //        return nullptr;
    //    }

    //    if (filepath.empty()) {
    //        LogError("Cannot load tree with empty filepath");
    //        return nullptr;
    //    }

    //    // Check if already loaded
    //    auto it = m_TreeCache.find(filepath);
    //    if (it != m_TreeCache.end()) {
    //        LOG_TRACE("Tree already loaded: ", filepath);
    //        return it->second.get();
    //    }

    //    // Get full path to asset
    //    std::string fullPath = GetFullPath(filepath);

    //    LOG_INFO("Loading behaviour tree: ", filepath);

    //    // Load the tree using the loader
    //    std::unique_ptr<BehaviourTree> tree = m_Loader->LoadFromFile(fullPath);

    //    if (!tree) {
    //        LogError("Failed to load tree: " + filepath);
    //        return nullptr;
    //    }

    //    // Cache the tree
    //    BehaviourTree* treePtr = tree.get();
    //    m_TreeCache[filepath] = std::move(tree);

    //    LOG_INFO("Successfully loaded tree: ", filepath);
    //    return treePtr;
    //}

    //BehaviourTree* AIManager::GetTree(const std::string& filepath) {
    //    if (!m_Initialized) {
    //        return nullptr;
    //    }

    //    auto it = m_TreeCache.find(filepath);
    //    if (it != m_TreeCache.end()) {
    //        return it->second.get();
    //    }

    //    return nullptr;
    //}

    //bool AIManager::ReloadTree(const std::string& filepath) {
    //    if (!m_Initialized) {
    //        LogError("AIManager not initialized");
    //        return false;
    //    }

    //    LOG_INFO("Reloading behaviour tree: ", filepath);

    //    // Unload existing tree
    //    UnloadTree(filepath);

    //    // Load fresh copy
    //    BehaviourTree* tree = LoadTree(filepath);

    //    if (tree) {
    //        LOG_INFO("Successfully reloaded tree: ", filepath);
    //        return true;
    //    }

    //    LogError("Failed to reload tree: " + filepath);
    //    return false;
    //}

    //void AIManager::UnloadTree(const std::string& filepath) {
    //    if (!m_Initialized) {
    //        return;
    //    }

    //    auto it = m_TreeCache.find(filepath);
    //    if (it != m_TreeCache.end()) {
    //        LOG_INFO("Unloading tree: ", filepath);
    //        m_TreeCache.erase(it);
    //    }
    //}

    //void AIManager::UnloadAllTrees() {
    //    if (!m_Initialized) {
    //        return;
    //    }

    //    LOG_INFO("Unloading all behaviour trees (", m_TreeCache.size(), " trees)");
    //    m_TreeCache.clear();
    //}

    //bool AIManager::IsTreeLoaded(const std::string& filepath) const {
    //    return m_TreeCache.find(filepath) != m_TreeCache.end();
    //}

    //std::string AIManager::GetFullPath(const std::string& filepath) const {
    //    // Use AssetPath helper to get full path to behaviour tree files
    //    // Assumes trees are stored in: Resources/BehaviourTrees/
    //    return getAssetFilePath("BehaviourTrees/" + filepath);
    //}

    void AIManager::LogError(const std::string& message) const {
        LOG_ERROR("AIManager: ", message);
    }

}	//end of namespace Engine
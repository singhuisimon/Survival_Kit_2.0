#include "BehaviourTreeSerializer.h"
#include "Utility/Logger.h"
#include <filesystem>

namespace Engine {

    BehaviourTreeSerializer::BehaviourTreeSerializer() = default;
    BehaviourTreeSerializer::~BehaviourTreeSerializer() { Shutdown(); }

    bool BehaviourTreeSerializer::Init() {
        if (m_Initialized) {
            LOG_WARNING("[BTSerializer] Already initialized");
            return true;
        }

        m_Loader = std::make_unique<BehaviourTreeLoader>();
        m_Initialized = true;
        LOG_INFO("[BTSerializer] Initialized successfully");
        return true;
    }

    void BehaviourTreeSerializer::Shutdown() {
        if (!m_Initialized) return;
        
        LOG_INFO("[BTSerializer] Shutting down, unloading ", m_TreeCache.size(), " trees");
        UnloadAllTrees();
        m_Loader.reset();
        m_Initialized = false;
        LOG_INFO("[BTSerializer] Shutdown complete");
    }

    BehaviourTree* BehaviourTreeSerializer::LoadTree(const std::string& filepath) {
        if (!m_Initialized) {
            LOG_ERROR("[BTSerializer] Cannot load tree - serializer not initialized");
            return nullptr;
        }

        if (filepath.empty()) {
            LOG_ERROR("[BTSerializer] Cannot load tree - empty filepath");
            return nullptr;
        }

        // Check if already cached
        auto it = m_TreeCache.find(filepath);
        if (it != m_TreeCache.end()) {
            LOG_TRACE("[BTSerializer] Returning cached tree: ", filepath);
            return it->second.get();
        }

        // Check if file exists
        if (!std::filesystem::exists(filepath)) {
            LOG_ERROR("[BTSerializer] File does not exist: ", filepath);
            return nullptr;
        }

        // Load new tree
        LOG_INFO("[BTSerializer] Loading tree from: ", filepath);
        std::unique_ptr<BehaviourTree> tree = m_Loader->LoadFromFile(filepath);

        if (!tree) {
            LOG_ERROR("[BTSerializer] Failed to parse tree: ", filepath);
            return nullptr;
        }

        if (!tree->HasRoot()) {
            LOG_ERROR("[BTSerializer] Loaded tree has no root node: ", filepath);
            return nullptr;
        }

        // Cache and return
        BehaviourTree* rawPtr = tree.get();
        m_TreeCache[filepath] = std::move(tree);
        LOG_INFO("[BTSerializer] Successfully loaded and cached tree: ", filepath);

        return rawPtr;
    }

    BehaviourTree* BehaviourTreeSerializer::GetTree(const std::string& filepath) {
        auto it = m_TreeCache.find(filepath);
        return (it != m_TreeCache.end()) ? it->second.get() : nullptr;
    }

    bool BehaviourTreeSerializer::ReloadTree(const std::string& filepath) {
        if (!m_Initialized) {
            LOG_ERROR("[BTSerializer] Cannot reload - not initialized");
            return false;
        }

        if (filepath.empty()) {
            LOG_ERROR("[BTSerializer] Cannot reload - empty filepath");
            return false;
        }

        LOG_INFO("[BTSerializer] Reloading tree: ", filepath);

        // Remove old cached version
        auto it = m_TreeCache.find(filepath);
        if (it != m_TreeCache.end()) {
            LOG_TRACE("[BTSerializer] Removing old cached tree");
            m_TreeCache.erase(it);
        }

        // Load fresh version
        BehaviourTree* tree = LoadTree(filepath);

        if (tree) {
            LOG_INFO("[BTSerializer] Tree reloaded successfully: ", filepath);
            return true;
        }

        LOG_ERROR("[BTSerializer] Failed to reload tree: ", filepath);
        return false;
    }

    void BehaviourTreeSerializer::UnloadTree(const std::string& filepath) {
        auto it = m_TreeCache.find(filepath);
        if (it != m_TreeCache.end()) {
            LOG_INFO("[BTSerializer] Unloading tree: ", filepath);
            m_TreeCache.erase(it);
        }
        else {
            LOG_TRACE("[BTSerializer] Tree not in cache, nothing to unload: ", filepath);
        }
    }

    void BehaviourTreeSerializer::UnloadAllTrees() {
        if (m_TreeCache.empty()) {
            LOG_TRACE("[BTSerializer] No trees to unload");
            return;
        }

        LOG_INFO("[BTSerializer] Unloading all trees (count: ", m_TreeCache.size(), ")");
        m_TreeCache.clear();
    }

    bool BehaviourTreeSerializer::ValidateTree(const std::string& filepath, std::string& errorMsg) {
        if (!m_Initialized) {
            errorMsg = "Serializer not initialized";
            return false;
        }

        if (filepath.empty()) {
            errorMsg = "Empty filepath";
            return false;
        }

        if (!std::filesystem::exists(filepath)) {
            errorMsg = "File does not exist: " + filepath;
            return false;
        }

        // Try to load the tree
        std::unique_ptr<BehaviourTree> tree = m_Loader->LoadFromFile(filepath);
        if (!tree) {
            errorMsg = "Failed to parse tree file";
            return false;
        }

        if (!tree->HasRoot()) {
            errorMsg = "Tree has no root node";
            return false;
        }

        errorMsg = "Tree is valid";
        return true;
    }

    std::vector<std::string> BehaviourTreeSerializer::GetLoadedTreePaths() const {
        std::vector<std::string> paths;
        paths.reserve(m_TreeCache.size());

        for (const auto& [path, tree] : m_TreeCache) {
            paths.push_back(path);
        }

        return paths;
    }

}

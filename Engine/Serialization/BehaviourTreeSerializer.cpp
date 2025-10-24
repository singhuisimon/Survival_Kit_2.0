#include "BehaviourTreeSerializer.h"
#include "Utility/Logger.h"

namespace Engine {

    BehaviourTreeSerializer::BehaviourTreeSerializer() = default;
    BehaviourTreeSerializer::~BehaviourTreeSerializer() { Shutdown(); }

    bool BehaviourTreeSerializer::Init() {
        if (m_Initialized) return true;
        m_Loader = std::make_unique<BehaviourTreeLoader>();
        m_Initialized = true;
        LOG_INFO("[BTSerializer] Initialized");
        return true;
    }

    void BehaviourTreeSerializer::Shutdown() {
        if (!m_Initialized) return;
        UnloadAllTrees();
        m_Loader.reset();
        m_Initialized = false;
        LOG_INFO("[BTSerializer] Shutdown complete");
    }

    BehaviourTree* BehaviourTreeSerializer::LoadTree(const std::string& filepath) {
        if (!m_Initialized || filepath.empty()) {
            LOG_ERROR("[BTSerializer] Cannot load tree: invalid state or empty path");
            return nullptr;
        }

        auto it = m_TreeCache.find(filepath);
        if (it != m_TreeCache.end())
            return it->second.get();

        std::unique_ptr<BehaviourTree> tree = m_Loader->LoadFromFile(filepath);
        if (!tree) {
            LOG_ERROR("[BTSerializer] Failed to load: ", filepath);
            return nullptr;
        }

        BehaviourTree* raw = tree.get();
        m_TreeCache[filepath] = std::move(tree);
        LOG_INFO("[BTSerializer] Cached new tree: ", filepath);
        return raw;
    }

    BehaviourTree* BehaviourTreeSerializer::GetTree(const std::string& filepath) {
        auto it = m_TreeCache.find(filepath);
        return (it != m_TreeCache.end()) ? it->second.get() : nullptr;
    }

    bool BehaviourTreeSerializer::IsTreeLoaded(const std::string& filepath) const {
        return m_TreeCache.find(filepath) != m_TreeCache.end();
    }

    void BehaviourTreeSerializer::UnloadAllTrees() {
        LOG_INFO("[BTSerializer] Unloading all (", m_TreeCache.size(), ")");
        m_TreeCache.clear();
    }

}

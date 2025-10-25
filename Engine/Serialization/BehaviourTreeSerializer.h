#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "AI/BehaviourTree/BehaviourTree.h"
#include "AI/BehaviourTree/BehaviourTreeLoader.h"

namespace Engine {

    class BehaviourTreeSerializer {
    public:
        BehaviourTreeSerializer();
        ~BehaviourTreeSerializer();

        bool Init();
        void Shutdown();

        BehaviourTree* LoadTree(const std::string& filepath);
        BehaviourTree* GetTree(const std::string& filepath);
        bool ReloadTree(const std::string& filepath);

        // Scene lifecycle - unload specific trees
        void UnloadTree(const std::string& filepath);
        void UnloadAllTrees();

        // Editor validation
        bool ValidateTree(const std::string& filepath, std::string& errorMsg);

        // Get all loaded tree paths (for editor UI)
        std::vector<std::string> GetLoadedTreePaths() const;

        bool IsInitialized() const { return m_Initialized; }

    private:
        bool m_Initialized = false;
        std::unique_ptr<BehaviourTreeLoader> m_Loader;
        std::unordered_map<std::string, std::unique_ptr<BehaviourTree>> m_TreeCache;

        // Track which trees are used by which scenes (for cleanup)
        std::unordered_map<std::string, std::unordered_set<std::string>> m_TreeSceneUsage;
    };

} // namespace Engine

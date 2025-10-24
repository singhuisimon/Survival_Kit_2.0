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
        bool IsTreeLoaded(const std::string& filepath) const;
        void UnloadAllTrees();

        bool IsInitialized() const { return m_Initialized; }

    private:
        bool m_Initialized = false;
        std::unique_ptr<BehaviourTreeLoader> m_Loader;
        std::unordered_map<std::string, std::unique_ptr<BehaviourTree>> m_TreeCache;
    };

} // namespace Engine

#pragma once

#include "ECS/Scene.h"
#include "ECS/System.h"
#include "ECS/Entity.h"
#include "AI/BehaviourTree/BehaviourNode.h"
#include "ECS/Components.h"

namespace Engine
{

    /**
     * @brief One frame of execution on the stack
     */
    struct NodeFrame {
        BehaviourNode* node;
        std::size_t childIndex = 0;
    };

    /**
     * @class BehaviourTree
     * @brief Stack-based behaviour tree runtime
     */
    class BehaviourTree {
    public:
        explicit BehaviourTree(const std::string& name = "BehaviourTree");
        ~BehaviourTree() = default;

        NodeResult Tick(Blackboard& blackboard, float deltaTime);
        void Reset();

        // --- Structure ---
        void SetRoot(std::unique_ptr<BehaviourNode> root);
        BehaviourNode* GetRoot() const { return m_Root.get(); }
        bool HasRoot() const { return m_Root != nullptr; }

        // --- State ---
        bool IsRunning() const { return !m_Stack.empty(); }

        // --- Metadata (for editor) ---
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& n) { m_Name = n; }
        
        // Editor/Debug support
        const std::vector<NodeFrame>& GetStack() const { return m_Stack; }
        size_t GetStackDepth() const { return m_Stack.size(); }

        // Get currently executing node
        BehaviourNode* GetCurrentNode() const {
            return m_Stack.empty() ? nullptr : m_Stack.back().node;
        }

        std::string GetExecutionTrace() const;

    private:
        std::string m_Name;
        std::unique_ptr<BehaviourNode> m_Root;

        std::vector<NodeFrame> m_Stack; // execution stack
    };

}	//end of namespace Engine
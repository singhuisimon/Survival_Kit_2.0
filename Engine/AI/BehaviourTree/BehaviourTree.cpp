#include "AI/BehaviourTree/BehaviourTree.h"
#include "Utility/Logger.h"
#include "ECS/Components.h"

namespace Engine {

    BehaviourTree::BehaviourTree(const std::string& name)
        : m_Name(name), m_Description("") {
    }

    void BehaviourTree::SetRoot(std::unique_ptr<BehaviourNode> root) {
        if (!root) {
            LOG_WARNING("[BT] Tried to set null root for tree: ", m_Name);
            return;
        }
        m_Root = std::move(root);
        m_Stack.clear();
        LOG_INFO("[BT] Root set for ", m_Name, ": ", m_Root->GetName());
    }

    NodeResult BehaviourTree::Tick(Blackboard& blackboard, float deltaTime) {
        if (!m_Root) {
            LOG_WARNING("[BT] Tree has no root node: ", m_Name);
            return NodeResult::FAILURE;
        }

        // First tick of execution
        if (m_Stack.empty()) {
            m_Stack.push_back({ m_Root.get(), 0 });
            m_Root->Reset();
        }

        while (!m_Stack.empty()) {
            NodeFrame& frame = m_Stack.back();
            BehaviourNode* node = frame.node;

            NodeResult result = node->Step(blackboard, deltaTime, frame.childIndex, m_Stack);

            if (result == NodeResult::IN_PROGRESS)
                return NodeResult::IN_PROGRESS; // pause for this frame

            m_Stack.pop_back(); // finished node

            if (result == NodeResult::FAILURE)
                return NodeResult::FAILURE; // bubble failure up
        }

        return NodeResult::SUCCESS;
    }

    void BehaviourTree::Reset() {
        m_Stack.clear();
        if (m_Root)
            m_Root->Reset();
    }

}
#include "AI/BehaviourTree/BehaviourTree.h"
#include "Utility/Logger.h"
#include "ECS/Components.h"

namespace Engine {

    BehaviourTree::BehaviourTree(const std::string& name)
        : m_Name(name) {
    }

    NodeResult BehaviourTree::Tick(Blackboard& blackboard, float deltaTime) {
        if (!m_Root) {
            LOG_WARNING("[BT] Tree has no root node: ", m_Name);
            return NodeResult::FAILURE;
        }

        // Initialize execution stack on first tick
        if (m_Stack.empty()) {
            m_Root->Reset();
            m_Stack.push_back({ m_Root.get(), 0 });
            LOG_TRACE("[BT] Starting tree '", m_Name, "' execution");
        }

        while (!m_Stack.empty()) {
            NodeFrame& frame = m_Stack.back();
            BehaviourNode* node = frame.node;

            if (!node) {
                LOG_ERROR("[BT] Null node in stack for tree '", m_Name, "'");
                m_Stack.pop_back();
                return NodeResult::FAILURE;
            }

            NodeResult result = node->Step(blackboard, deltaTime, frame.childIndex, m_Stack);

            // Handle result
            switch (result) {
            case NodeResult::IN_PROGRESS:
                // Node is still running, pause execution for this frame
                return NodeResult::IN_PROGRESS;

            case NodeResult::SUCCESS:
                // Node succeeded, pop it off and continue
                LOG_TRACE("[BT] Node '", node->GetName(), "' succeeded");
                m_Stack.pop_back();

                // If stack is empty, entire tree succeeded
                if (m_Stack.empty()) {
                    LOG_TRACE("[BT] Tree '", m_Name, "' completed successfully");
                    return NodeResult::SUCCESS;
                }
                // Otherwise continue with parent node
                break;

            case NodeResult::FAILURE:
                // Node failed, pop it and return failure
                LOG_TRACE("[BT] Node '", node->GetName(), "' failed");
                m_Stack.pop_back();

                // Failure bubbles up - clear stack and return
                m_Stack.clear();
                LOG_TRACE("[BT] Tree '", m_Name, "' failed");
                return NodeResult::FAILURE;
            }
        }

        // Stack empty means tree completed successfully
        return NodeResult::SUCCESS;
    }

    void BehaviourTree::Reset() {
        LOG_TRACE("[BT] Resetting tree '", m_Name, "'");
        m_Stack.clear();

        if (m_Root) {
            m_Root->Reset();
        }
    }

    void BehaviourTree::SetRoot(std::unique_ptr<BehaviourNode> root) {
        if (!root) {
            LOG_WARNING("[BT] Tried to set null root for tree: ", m_Name);
            return;
        }

        LOG_INFO("[BT] Setting root node '", root->GetName(), "' for tree '", m_Name, "'");
        m_Root = std::move(root);
        m_Stack.clear();
    }

    std::string BehaviourTree::GetExecutionTrace() const {
        if (m_Stack.empty()) {
            return "Tree not running";
        }

        std::string trace;
        for (size_t i = 0; i < m_Stack.size(); ++i) {
            const auto& frame = m_Stack[i];
            if (frame.node) {
                trace += frame.node->GetName();
                if (i < m_Stack.size() - 1) {
                    trace += " -> ";
                }
            }
        }

        return trace.empty() ? "Empty trace" : trace;
    }
}
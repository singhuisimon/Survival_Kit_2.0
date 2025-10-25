#include "Utility/Logger.h"
#include "AI/BehaviourTree/BehaviourNode.h"

namespace Engine {

    BehaviourNode::BehaviourNode(const std::string& name)
        : m_Name(name),
        m_Status(NodeStatus::IDLE),
        m_Result(NodeResult::IN_PROGRESS) {
    }

    void BehaviourNode::AddChild(std::unique_ptr<BehaviourNode> child) {
        if (child)
            m_Children.push_back(std::move(child));
        else
            LOG_WARNING("[BT] Null child added to node: ", m_Name);
    }

    BehaviourNode* BehaviourNode::GetChild(std::size_t index) const {
        return index < m_Children.size() ? m_Children[index].get() : nullptr;
    }

    void BehaviourNode::Reset() {
        m_Status = NodeStatus::IDLE;
        m_Result = NodeResult::IN_PROGRESS;

        //recursively reset all children
        for (auto& child : m_Children) {
            if (child) {
                child->Reset();
            }
        }
    }

    void BehaviourNode::OnEnter(Blackboard& blackboard) {
        (void)blackboard;
        m_Status = NodeStatus::READY;
        LOG_TRACE("[BT] OnEnter: ", m_Name);
    }

    void BehaviourNode::OnUpdate(Blackboard& blackboard, float deltaTime) {
        (void)blackboard;
        (void)deltaTime;

        // Base implementation: instantly succeed
        // Derived classes should override this with actual logic
        m_Status = NodeStatus::RUNNING;
    }

    void BehaviourNode::OnExit(Blackboard& blackboard) {
        (void)blackboard;
        m_Status = NodeStatus::EXITING;
        LOG_TRACE("[BT] OnExit: ", m_Name, " with result: ",
            (m_Result == NodeResult::SUCCESS ? "SUCCESS" :
                m_Result == NodeResult::FAILURE ? "FAILURE" : "IN_PROGRESS"));
    }

    NodeResult BehaviourNode::Step(Blackboard& blackboard, float deltaTime,
        std::size_t& childIndex,
        std::vector<NodeFrame>& stack) {
        
        (void)childIndex; // Base nodes don't use children
        (void)stack;      // Base nodes don't push to stack

        // Enter node if not yet started
        if (m_Status == NodeStatus::IDLE) {
            OnEnter(blackboard);
        }

        // Update node if still running
        if (m_Status == NodeStatus::RUNNING && m_Result == NodeResult::IN_PROGRESS) {
            OnUpdate(blackboard, deltaTime);
        }

        // Exit node if completed
        if (m_Result != NodeResult::IN_PROGRESS && m_Status != NodeStatus::EXITING) {
            OnExit(blackboard);
        }

        return m_Result;
    }

}
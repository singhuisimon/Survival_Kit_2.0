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
        for (auto& child : m_Children)
            child->Reset();
    }

    void BehaviourNode::OnEnter(Blackboard& blackboard) {
        (void)blackboard;
        m_Status = NodeStatus::ENTERING;
        LOG_TRACE("[BT] OnEnter: ", m_Name);
    }

    void BehaviourNode::OnUpdate(Blackboard& blackboard, float deltaTime) {
        (void)blackboard;
        (void)deltaTime;
        m_Status = NodeStatus::RUNNING;
        // Default node instantly succeeds
        m_Result = NodeResult::SUCCESS;
    }

    void BehaviourNode::OnExit(Blackboard& blackboard) {
        (void)blackboard;
        m_Status = NodeStatus::EXITING;
        LOG_TRACE("[BT] OnExit: ", m_Name);
        m_Status = NodeStatus::COMPLETED;
    }

    NodeResult BehaviourNode::Step(Blackboard& blackboard, float deltaTime,
        std::size_t& childIndex,
        std::vector<NodeFrame>& stack) {
        (void)childIndex;
        (void)stack;

        if (m_Status == NodeStatus::IDLE || m_Status == NodeStatus::ENTERING)
            OnEnter(blackboard);

        if (m_Status == NodeStatus::ENTERING || m_Status == NodeStatus::RUNNING)
            OnUpdate(blackboard, deltaTime);

        if (m_Result != NodeResult::IN_PROGRESS) {
            OnExit(blackboard);
        }

        return m_Result;
    }

}
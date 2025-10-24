#pragma once

#include "ECS/Scene.h"
#include "ECS/System.h"
#include "ECS/Entity.h"
#include "AI/AIManager.h"
#include "ECS/Components.h"

namespace Engine
{
    /**
     * @brief Execution phase of the node (for editor/debug visualization)
     */
    enum class NodeStatus {
        IDLE,       // Has not started
        ENTERING,   // Just began (OnEnter called)
        RUNNING,    // Currently executing
        EXITING,    // Finishing (OnExit called)
        COMPLETED   // Done (SUCCESS or FAILURE)
    };

    /**
     * @brief Logical outcome of execution (for BT control flow)
     */
    enum class NodeResult {
        IN_PROGRESS, // Still running
        SUCCESS,     // Completed successfully
        FAILURE      // Completed unsuccessfully
    };

    struct NodeFrame; // forward declare for stack

    /**
     * @class BehaviourNode
     * @brief Base class for all nodes in a behaviour tree (composite, decorator, leaf)
     */
    class BehaviourNode {
    public:
        explicit BehaviourNode(const std::string& name = "Node");
        virtual ~BehaviourNode() = default;

        // --- Stack-driven execution step ---
        virtual NodeResult Step(Blackboard& blackboard, float deltaTime,
            std::size_t& childIndex,
            std::vector<NodeFrame>& stack);

        // --- Node lifecycle (overridable by derived types) ---
        virtual void OnEnter(Blackboard& blackboard);
        virtual void OnUpdate(Blackboard& blackboard, float deltaTime);
        virtual void OnExit(Blackboard& blackboard);
        virtual void Reset();

        // --- Hierarchy management ---
        void AddChild(std::unique_ptr<BehaviourNode> child);
        BehaviourNode* GetChild(std::size_t index) const;
        const std::vector<std::unique_ptr<BehaviourNode>>& GetChildren() const { return m_Children; }
        std::size_t GetChildCount() const { return m_Children.size(); }

        // --- Metadata for editor ---
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& n) { m_Name = n; }
        virtual std::string GetType() const { return "Node"; }

        NodeStatus GetStatus() const { return m_Status; }
        NodeResult GetResult() const { return m_Result; }

    protected:
        std::string m_Name;
        NodeStatus  m_Status;
        NodeResult  m_Result;
        std::vector<std::unique_ptr<BehaviourNode>> m_Children;
    };

}	//end of namespace Engine
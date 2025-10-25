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
        IDLE,       // Node hasn't started yet
        READY,      // Node is ready to be entered
        RUNNING,    // Node is currently executing
        EXITING,    // Node has completed (success/failure)
        SUSPENDED   // Node is paused/inactive
    };

    /**
     * @brief Logical outcome of execution (for BT control flow)
     */
    enum class NodeResult {
        IN_PROGRESS, // Still executing
        SUCCESS,     // Completed successfully
        FAILURE      // Failed to complete
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

        /**
         * @brief Stack-driven execution step (called by BehaviourTree)
         * @param deltaTime Frame time
         * @param childIndex Current child being processed (for composites)
         * @param stack Execution stack (composites push children here)
         * @return Current result of this node
         */
        virtual NodeResult Step(Blackboard& blackboard, float deltaTime,
            std::size_t& childIndex,
            std::vector<NodeFrame>& stack);

        // --- Node lifecycle (overridable by derived types) ---
        /**
         * @brief Called once when node starts
         * @details Use this to initialize state, set up timers, etc.
         */
        virtual void OnEnter(Blackboard& blackboard);

        /**
         * @brief Called every frame while node is running
         * @details Set m_Result to SUCCESS/FAILURE when done
         */
        virtual void OnUpdate(Blackboard& blackboard, float deltaTime);

        /**
         * @brief Called once when node completes
         * @details Use this for cleanup, logging, etc.
         */
        virtual void OnExit(Blackboard& blackboard);

        /**
         * @brief Reset node to initial state
         * @details Called when tree restarts or node needs to be reused
         */
        virtual void Reset();

        // --- Hierarchy management ---
        void AddChild(std::unique_ptr<BehaviourNode> child);    //Take note leaf aren't suppose to have child!
        BehaviourNode* GetChild(std::size_t index) const;
        const std::vector<std::unique_ptr<BehaviourNode>>& GetChildren() const { return m_Children; }
        std::size_t GetChildCount() const { return m_Children.size(); }

        // --- Metadata for editor ---
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& n) { m_Name = n; }

        /**
         * @brief Get the type name of this node (e.g., "Sequence", "Selector")
         * @details Override this in derived classes for proper type identification
         */
        virtual std::string GetType() const { return "Node"; }

        NodeStatus GetStatus() const { return m_Status; }
        NodeResult GetResult() const { return m_Result; }

    protected:

        // === HELPER METHODS ===

        /**
         * @brief Mark this node as succeeded
         * @details Sets m_Result to SUCCESS. Call this in OnUpdate when task completes.
         */
        void Succeed() { m_Result = NodeResult::SUCCESS; }

        /**
         * @brief Mark this node as failed
         * @details Sets m_Result to FAILURE. Call this in OnUpdate when task fails.
         */
        void Fail() { m_Result = NodeResult::FAILURE; }

        /**
         * @brief Check if this node has completed (success or failure)
         */
        bool IsComplete() const {
            return m_Result == NodeResult::SUCCESS || m_Result == NodeResult::FAILURE;
        }

        std::string m_Name;
        NodeStatus  m_Status;
        NodeResult  m_Result;
        std::vector<std::unique_ptr<BehaviourNode>> m_Children;
    };
}	//end of namespace Engine
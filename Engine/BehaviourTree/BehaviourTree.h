/**
 * @file BehaviourTree.h
 * @brief Main behavior tree class with stack-based execution
 * @author Amanda Leow Boon Suan (100%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#pragma once

#include "BTNode.h"
#include "BTNodeRegistry.h"
#include "../Asset/ResourceTypes.h"
#include <stack>
#include <memory>

namespace Engine {

    /**
     * @brief Main behavior tree class
     * @details Manages root node and provides stack-based execution
     */
    class BehaviourTree {
    public:
        BehaviourTree();

        explicit BehaviourTree(std::shared_ptr<BTNode> rootNode);

        /**
         * @brief Get unique identifier for this tree
         */
        xresource::instance_guid GetGUID() const;

        /**
         * @brief Set unique identifier
         */
        void SetGUID(xresource::instance_guid guid);

        /**
         * @brief Get tree name
         */
        const std::string& GetName() const;

        /**
         * @brief Set tree name
         */
        void SetName(const std::string& name);

        /**
         * @brief Set the root node of the tree
         */
        void SetRootNode(std::shared_ptr<BTNode> root); 

        /**
         * @brief Get the root node
         */
        std::shared_ptr<BTNode> GetRootNode() const; 

        /**
         * @brief Execute the behavior tree using stack-based approach
         * @param context Execution context
         * @return Final status of the tree execution
         */
        BTStatus Execute(BTContext& context);

        /**
         * @brief Reset the tree to initial state
         */
        void Reset();

        /**
         * @brief Get the current execution status (for debugging)
         */
        size_t GetStackDepth() const;

    private:
        /**
         * @brief Process a composite node's execution logic
         * @details This handles the node-specific logic for composites/decorators
         */
        BTStatus ProcessCompositeNode(BTStackFrame& frame, BTContext& context);

        std::shared_ptr<BTNode> m_RootNode;
        std::stack<BTStackFrame> m_ExecutionStack;
        xresource::instance_guid m_GUID;
        std::string m_Name = "BehaviourTree";
    };

} // namespace Engine

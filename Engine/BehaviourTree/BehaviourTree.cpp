/**
 * @file BehaviourTree.h
 * @brief Main behavior tree class with stack-based execution
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BehaviourTree.h"
#include "../Asset/ResourceTypes.h"
#include <entt/entt.hpp>
#include "ECS/Scene.h"
#include <stack>
#include <memory>

namespace Engine {

    BehaviourTree::BehaviourTree() : m_GUID(xresource::instance_guid::GenerateGUIDCopy()) {}

    BehaviourTree::BehaviourTree(std::shared_ptr<BTNode> rootNode)
        : m_RootNode(rootNode)
        , m_GUID(xresource::instance_guid::GenerateGUIDCopy()) {
    }

    /**
     * @brief Get unique identifier for this tree
     */
    xresource::instance_guid BehaviourTree::GetGUID() const { return m_GUID; }

    /**
     * @brief Set unique identifier
     */
    void BehaviourTree::SetGUID(xresource::instance_guid guid) { m_GUID = guid; }

    /**
     * @brief Get tree name
     */
    const std::string& BehaviourTree::GetName() const { return m_Name; }

    /**
     * @brief Set tree name
     */
    void BehaviourTree::SetName(const std::string& name) { m_Name = name; }

    /**
     * @brief Set the root node of the tree
     */
    void BehaviourTree::SetRootNode(std::shared_ptr<BTNode> root) {
        m_RootNode = root;
    }

    /**
     * @brief Get the root node
     */
    std::shared_ptr<BTNode> BehaviourTree::GetRootNode() const {
        return m_RootNode;
    }

    /**
     * @brief Execute the behavior tree using stack-based approach
     * @param context Execution context
     * @return Final status of the tree execution
     */
    //BTStatus BehaviourTree::Execute(BTContext& context) {
        //if (!m_RootNode) {
        //    return BTStatus::Failure;
        //}

        //// Clear the stack for fresh execution
        //while (!m_ExecutionStack.empty()) {
        //    m_ExecutionStack.pop();
        //}

        //// Push root node
        //m_ExecutionStack.push(BTStackFrame(m_RootNode));

        //// Stack-based execution loop
        //while (!m_ExecutionStack.empty()) {
        //    BTStackFrame& frame = m_ExecutionStack.top();

        //    // Call OnEnter for first time
        //    if (!frame.HasEntered) {
        //        frame.Node->OnEnter(context);
        //        frame.HasEntered = true;
        //    }

        //    if (frame.Node = m_RootNode) {
        //        context.isroot = true;
        //    }

        //    // Check if this is a composite/decorator with children
        //    auto& children = frame.Node->GetChildren();

        //    if (children.empty()) {
        //        // Leaf node - execute directly
        //        BTStatus status = frame.Node->Execute(context);

        //        // Call OnExit
        //        frame.Node->OnExit(context);

        //        // Pop this frame
        //        m_ExecutionStack.pop();

        //        // If this was the root, we're done
        //        if (m_ExecutionStack.empty()) {
        //            return status;
        //        }

        //        // Update parent's last child status
        //        m_ExecutionStack.top().LastChildStatus = status;
        //    }
        //    else {
        //        // Composite/Decorator node - handle based on type
        //        BTStatus status = ProcessCompositeNode(frame, context);

        //        // If node is still running, keep it on stack
        //        if (status == BTStatus::Running) {
        //            continue;
        //        }

        //        // Node completed - call OnExit and pop
        //        frame.Node->OnExit(context);
        //        m_ExecutionStack.pop();

        //        // If this was the root, we're done
        //        if (m_ExecutionStack.empty()) {
        //            return status;
        //        }

        //        // Update parent's last child status
        //        m_ExecutionStack.top().LastChildStatus = status;
        //    }
        //}

        //return BTStatus::Failure;
    //}
    BTStatus BehaviourTree::Execute(BTContext& context) {
        if (!m_RootNode) {
            return BTStatus::Failure;
        }

        // Only initialize stack if it's empty (allows Running nodes to continue)
        if (m_ExecutionStack.empty()) {
            m_ExecutionStack.push(BTStackFrame(m_RootNode));
        }

        // Stack-based execution loop
        while (!m_ExecutionStack.empty()) {
            BTStackFrame& frame = m_ExecutionStack.top();

            // Safety check
            if (!frame.Node) {
                m_ExecutionStack.pop();
                continue;
            }

            // Call OnEnter for first time
            if (!frame.HasEntered) {
                frame.Node->OnEnter(context);
                frame.HasEntered = true;
            }

            // Mark if this is the root node
            if (frame.Node == m_RootNode) {  // FIXED: == not =
                context.isroot = true;
            }

            // --- CAPTURE ENTITY VALIDITY BEFORE EXECUTION ---
            entt::entity entityHandle = entt::null;
            bool canValidate = (context.Entity && context.Scene);

            if (canValidate) {
                auto& registry = context.Scene->GetRegistry();
                entityHandle = static_cast<entt::entity>(*context.Entity);

                // CRITICAL: Check if entity is ALREADY invalid before executing
                if (!registry.valid(entityHandle)) {
                    LOG_TRACE("BehaviourTree: Entity already destroyed, aborting execution");
                    // Don't try to clean up - just return
                    return BTStatus::Failure;
                }
            }

            // Check if this is a composite/decorator with children
            auto& children = frame.Node->GetChildren();
            BTStatus status;

            if (children.empty()) {
                // --- LEAF NODE ---
                status = frame.Node->Execute(context);

                // --- CHECK IF ENTITY WAS DESTROYED ---
                if (canValidate && context.Scene) {
                    auto& registry = context.Scene->GetRegistry();
                    if (!registry.valid(entityHandle)) {
                        // Entity destroyed - abort immediately without cleanup
                        LOG_TRACE("BehaviourTree: Entity destroyed during leaf execution");
                        return BTStatus::Success;
                    }
                }

                // Safe to call OnExit - entity still exists
                frame.Node->OnExit(context);
                m_ExecutionStack.pop();

                // If this was the root, we're done
                if (m_ExecutionStack.empty()) {
                    return status;
                }

                // Update parent's last child status
                m_ExecutionStack.top().LastChildStatus = status;
            }
            else {
                // --- COMPOSITE/DECORATOR NODE ---
                status = ProcessCompositeNode(frame, context);

                // --- CHECK IF ENTITY WAS DESTROYED ---
                if (canValidate && context.Scene) {
                    auto& registry = context.Scene->GetRegistry();
                    if (!registry.valid(entityHandle)) {
                        // Entity destroyed - abort immediately without cleanup
                        LOG_TRACE("BehaviourTree: Entity destroyed during composite execution");
                        return BTStatus::Success;
                    }
                }

                // If node is still running, keep it on stack
                if (status == BTStatus::Running) {
                    continue;
                }

                // Node completed - call OnExit and pop
                frame.Node->OnExit(context);
                m_ExecutionStack.pop();

                // If this was the root, we're done
                if (m_ExecutionStack.empty()) {
                    return status;
                }

                // Update parent's last child status
                m_ExecutionStack.top().LastChildStatus = status;
            }
        }

        return BTStatus::Failure;
    }


    /**
     * @brief Reset the tree to initial state
     */
    void BehaviourTree::Reset() {
        if (m_RootNode) {
            m_RootNode->Reset();
        }

        // Clear execution stack
        while (!m_ExecutionStack.empty()) {
            m_ExecutionStack.pop();
        }
    }

    /**
     * @brief Get the current execution status (for debugging)
     */
    size_t BehaviourTree::GetStackDepth() const {
        return m_ExecutionStack.size();
    }

    /**
         * @brief Process a composite node's execution logic
         * @details This handles the node-specific logic for composites/decorators
         * EDITED !!!!
         */
    //BTStatus BehaviourTree::ProcessCompositeNode(BTStackFrame& frame, BTContext& context) {
    //    auto& children = frame.Node->GetChildren();

    //    // Sync frame's ChildIndex with composite's internal state
    //    if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
    //        frame.ChildIndex = composite->GetCurrentChildIndex();
    //    }

    //    // CRITICAL: Store entity handle to check validity after execution
    //    entt::entity entityHandle = entt::null;
    //    bool canValidate = (context.Entity && context.Scene);

    //    if (canValidate) {
    //        auto& registry = context.Scene->GetRegistry();
    //        entityHandle = static_cast<entt::entity>(*context.Entity);
    //    }

    //    // Execute (entity might destroy itself here)
    //    BTStatus status = frame.Node->Execute(context);

    //    // SAFE sync-back: Check entity still exists
    //    if (canValidate && context.Scene) {
    //        auto& registry = context.Scene->GetRegistry();

    //        // If entity was destroyed, don't sync back - just return
    //        if (!registry.valid(entityHandle)) {
    //            LOG_TRACE("ProcessCompositeNode: Entity destroyed during execution");
    //            // Return Success to indicate intentional destruction
    //            return BTStatus::Success;
    //        }

    //        // Entity still valid - safe to sync back
    //        if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
    //            frame.ChildIndex = composite->GetCurrentChildIndex();
    //        }
    //    }
    //    else {
    //        // No entity context, try normal sync with safety
    //        if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
    //            frame.ChildIndex = composite->GetCurrentChildIndex();
    //        }
    //    }

    //    return status;
        
        //auto& children = frame.Node->GetChildren();

        ////NEW
        //// Sync frame's ChildIndex with composite's internal state
        //// For composites that use m_CurrentChildIndex
        //if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
        //    frame.ChildIndex = composite->GetCurrentChildIndex();
        //}

        ////OLD
        //// Execute the node's logic
        //// Note: For composites, the Execute method handles child iteration
        //BTStatus status = frame.Node->Execute(context);

        ////NEW
        //// Sync back after execution
        //if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
        //    frame.ChildIndex = composite->GetCurrentChildIndex();
        //}

        //return status;
    //}
    BTStatus BehaviourTree::ProcessCompositeNode(BTStackFrame& frame, BTContext& context) {
        auto& children = frame.Node->GetChildren();

        // Sync frame's ChildIndex with composite's internal state
        if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
            frame.ChildIndex = composite->GetCurrentChildIndex();
        }

        // CRITICAL: Store entity handle to check validity after execution
        entt::entity entityHandle = entt::null;
        bool canValidate = (context.Entity && context.Scene);

        if (canValidate) {
            auto& registry = context.Scene->GetRegistry();
            entityHandle = static_cast<entt::entity>(*context.Entity);
        }

        // Execute (entity might destroy itself here)
        BTStatus status = frame.Node->Execute(context);

        // SAFE sync-back: Check entity still exists
        if (canValidate && context.Scene) {
            auto& registry = context.Scene->GetRegistry();

            // If entity was destroyed, don't sync back
            if (!registry.valid(entityHandle)) {
                LOG_TRACE("ProcessCompositeNode: Entity destroyed during execution");
                return BTStatus::Success;
            }

            // Entity still valid - safe to sync back
            if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
                frame.ChildIndex = composite->GetCurrentChildIndex();
            }
        }
        else {
            // No entity context, do normal sync
            if (auto* composite = dynamic_cast<BTComposite*>(frame.Node.get())) {
                frame.ChildIndex = composite->GetCurrentChildIndex();
            }
        }

        return status;
    }

} // namespace Engine

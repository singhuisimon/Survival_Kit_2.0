/**
 * @file BTCompositeNodes.cpp
 * @brief Definition of Behaviour Tree Composite Nodes classes
 * @author Amanda Leow Boon Suan (90%), Rio Shannon Yvon Leonardo (10%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "BTCompositeNodes.h"
#include "ECS/Scene.h"

namespace Engine {


    //COMPOSITE
    std::vector<std::shared_ptr<BTNode>>& BTComposite::GetChildren() {
        return m_Children;
    }

    const std::vector<std::shared_ptr<BTNode>>& BTComposite::GetChildren() const {
        return m_Children;
    }

    bool BTComposite::CanHaveChildren() const { return true; }

    void BTComposite::AddChild(std::shared_ptr<BTNode> child) {
        if (child) {
            m_Children.push_back(child);
			m_ChildrenEntered.push_back(false);
        }
    }

    void BTComposite::RemoveChild(size_t index) {
        if (index < m_Children.size()) {
            m_Children.erase(m_Children.begin() + index);
			m_ChildrenEntered.erase(m_ChildrenEntered.begin() + index);
        }
    }

    void BTComposite::Reset() {
        m_CurrentChildIndex = 0;
        
        // Reset all children entered flags
        for (size_t i = 0; i < m_ChildrenEntered.size(); ++i) {
            m_ChildrenEntered[i] = false;
        }

        for (auto& child : m_Children) {
            child->Reset();
        }
    }

    //new functions implementation
    size_t BTComposite::GetCurrentChildIndex() const { return m_CurrentChildIndex; }
    void BTComposite::SetCurrentChildIndex(size_t index) { m_CurrentChildIndex = index; }

    int BTComposite::FindChildIndex(const std::shared_ptr<BTNode>& child) const {
        for (size_t i = 0; i < m_Children.size(); ++i) {
            if (m_Children[i] == child) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int BTComposite::FindChildIndexByGUID(xresource::instance_guid guid) const {
        for (size_t i = 0; i < m_Children.size(); ++i) {
            if (m_Children[i]->GetGUID() == guid) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    //end of new functions implementation

    //Sequence

    BTStatus BTSequence::Execute(BTContext& context) {

        // Execute children in sequence
        while (m_CurrentChildIndex < m_Children.size()) {
            auto& child = m_Children[m_CurrentChildIndex];

            // Check if we need to call OnEnter for this child
            /*static std::unordered_map<BTNode*, bool> enteredChildren;
            auto key = child.get();

            if (enteredChildren[key] == false || enteredChildren.find(key) == enteredChildren.end()) {
                child->OnEnter(context);
                enteredChildren[key] = true;
            }*/

            //BTStatus status = child->Execute(context);

            //if (status == BTStatus::Failure) {
            //    // Child failed - call OnExit and reset sequence
            //    child->OnExit(context);
            //    enteredChildren[key] = false;

            //    if (context.Entity && context.Scene) {
            //        auto& registry = context.Scene->GetRegistry();
            //        entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
            //        if (registry.valid(entityHandle)) {
            //            Reset();
            //        }
            //    }
            //    return BTStatus::Failure;
            //}
            //else if (status == BTStatus::Running) {
            //    return BTStatus::Running;
            //}

            //// Child succeeded - call OnExit and move to next
            //child->OnExit(context);
            //enteredChildren[key] = false;
            //m_CurrentChildIndex++;


            // Call OnEnter if this is the first time executing this child
            if (!m_ChildrenEntered[m_CurrentChildIndex]) {
                child->OnEnter(context);
                m_ChildrenEntered[m_CurrentChildIndex] = true;
            }

            BTStatus status = child->Execute(context);

            if (status == BTStatus::Failure) {
                // Child failed - call OnExit and reset sequence
                child->OnExit(context);
                m_ChildrenEntered[m_CurrentChildIndex] = false;

                // Check if entity still exists before resetting
                if (context.Entity && context.Scene) {
                    auto& registry = context.Scene->GetRegistry();
                    entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                    if (registry.valid(entityHandle)) {
                        Reset();
                    }
                }
                return BTStatus::Failure;
            }
            else if (status == BTStatus::Running) {
                return BTStatus::Running;
            }

            // Child succeeded - call OnExit and move to next
            child->OnExit(context);
            m_ChildrenEntered[m_CurrentChildIndex] = false;
            m_CurrentChildIndex++;
        }

        // Execute children in sequence
        //while (m_CurrentChildIndex < m_Children.size()) {
        //    BTStatus status = m_Children[m_CurrentChildIndex]->Execute(context);

        //    if (status == BTStatus::Failure) {
        //        // Check if entity still exists before resetting
        //        if (context.Entity && context.Scene) {
        //            auto& registry = context.Scene->GetRegistry();
        //            entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
        //            if (registry.valid(entityHandle)) {
        //                Reset();
        //            }
        //        }
        //        return BTStatus::Failure;
        //    }
        //    else if (status == BTStatus::Running) {
        //        return BTStatus::Running;
        //    }

        //    // Child succeeded, move to next
        //    m_CurrentChildIndex++;
        //}

        // All children succeeded
        if (!context.isroot) {
            // Check if entity still exists before resetting
            if (context.Entity && context.Scene) {
                auto& registry = context.Scene->GetRegistry();
                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                if (registry.valid(entityHandle)) {
                    Reset();
                }
            }
        }

        return BTStatus::Success;
    }


    //BTSelector
    BTStatus BTSelector::Execute(BTContext& context) {
        // Try children in sequence until one succeeds
        while (m_CurrentChildIndex < m_Children.size()) {
            auto& child = m_Children[m_CurrentChildIndex];

            // Call OnEnter if this is the first time executing this child
            if (!m_ChildrenEntered[m_CurrentChildIndex]) {
                child->OnEnter(context);
                m_ChildrenEntered[m_CurrentChildIndex] = true;
            }

            BTStatus status = child->Execute(context);

            if (status == BTStatus::Success) {
                // Child succeeded - call OnExit and return success
                child->OnExit(context);
                m_ChildrenEntered[m_CurrentChildIndex] = false;

                // Only reset if not root - prevents crash when entity is destroyed
                if (!context.isroot) {
                    // Check if entity still exists before resetting
                    if (context.Entity && context.Scene) {
                        auto& registry = context.Scene->GetRegistry();
                        entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                        if (registry.valid(entityHandle)) {
                            Reset();
                        }
                    }
                }

                return BTStatus::Success;
            }
            else if (status == BTStatus::Running) {
                return BTStatus::Running;
            }

            // Child failed - call OnExit and try next
            child->OnExit(context);
            m_ChildrenEntered[m_CurrentChildIndex] = false;
            m_CurrentChildIndex++;

            // Check if we need to call OnEnter for this child
            //static std::unordered_map<BTNode*, bool> enteredChildren;
            //auto key = child.get();

            //if (enteredChildren[key] == false || enteredChildren.find(key) == enteredChildren.end()) {
            //    child->OnEnter(context);
            //    enteredChildren[key] = true;
            //}

            //BTStatus status = child->Execute(context);

            //if (status == BTStatus::Success) {
            //    // Child succeeded - call OnExit and return success
            //    child->OnExit(context);
            //    enteredChildren[key] = false;

            //    if (!context.isroot) {
            //        if (context.Entity && context.Scene) {
            //            auto& registry = context.Scene->GetRegistry();
            //            entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
            //            if (registry.valid(entityHandle)) {
            //                Reset();
            //            }
            //        }
            //    }

            //    return BTStatus::Success;
            //}
            //else if (status == BTStatus::Running) {
            //    return BTStatus::Running;
            //}

            //// Child failed - call OnExit and try next
            //child->OnExit(context);
            //enteredChildren[key] = false;
            //m_CurrentChildIndex++;
        }

        //// Try children in sequence until one succeeds
        //while (m_CurrentChildIndex < m_Children.size()) {
        //    BTStatus status = m_Children[m_CurrentChildIndex]->Execute(context);

        //    if (status == BTStatus::Success) {

        //        // Only reset if not root - prevents crash when entity is destroyed
        //        if (!context.isroot) {
        //            // Check if entity still exists before resetting
        //            if (context.Entity && context.Scene) {
        //                auto& registry = context.Scene->GetRegistry();
        //                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
        //                if (registry.valid(entityHandle)) {
        //                    Reset();
        //                }
        //            }
        //        }

        //        return BTStatus::Success;
        //    }
        //    else if (status == BTStatus::Running) {
        //        return BTStatus::Running;
        //    }

        //    // Child failed, try next
        //    m_CurrentChildIndex++;
        //}

        // All children failed
        if (!context.isroot) {
            // Check if entity still exists before resetting
            if (context.Entity && context.Scene) {
                auto& registry = context.Scene->GetRegistry();
                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                if (registry.valid(entityHandle)) {
                    Reset();
                }
            }
        }

        return BTStatus::Failure;
    }


    //Parallel

    BTParallel::BTParallel(Policy successPolicy, Policy failurePolicy)
        : m_SuccessPolicy(successPolicy), m_FailurePolicy(failurePolicy) {
    }

    BTStatus BTParallel::Execute(BTContext& context) {
        if (m_Children.empty()) {
            return BTStatus::Success;
        }

        size_t successCount = 0;
        size_t failureCount = 0;
        size_t runningCount = 0;

        // Execute all children
        for (size_t i = 0; i < m_Children.size(); ++i) {
            auto& child = m_Children[i];

            // Call OnEnter if this is the first time executing this child
            if (!m_ChildrenEntered[i]) {
                child->OnEnter(context);
                m_ChildrenEntered[i] = true;
            }

            BTStatus status = child->Execute(context);

            switch (status) {
            case BTStatus::Success:
                // Call OnExit for completed child
                if (m_ChildrenEntered[i]) {
                    child->OnExit(context);
                    m_ChildrenEntered[i] = false;
                }
                successCount++;
                break;
            case BTStatus::Failure:
                // Call OnExit for completed child
                if (m_ChildrenEntered[i]) {
                    child->OnExit(context);
                    m_ChildrenEntered[i] = false;
                }
                failureCount++;
                break;
            case BTStatus::Running:
                runningCount++;
                break;
            }
        }

        // Check failure condition
        if (m_FailurePolicy == Policy::RequireOne && failureCount > 0) {
            // Parallel failed - call OnExit for all children still entered
            for (size_t i = 0; i < m_Children.size(); ++i) {
                if (m_ChildrenEntered[i]) {
                    m_Children[i]->OnExit(context);
                    m_ChildrenEntered[i] = false;
                }
            }

            // Check if entity still exists before resetting
            if (context.Entity && context.Scene) {
                auto& registry = context.Scene->GetRegistry();
                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                if (registry.valid(entityHandle)) {
                    Reset();
                }
            }
            return BTStatus::Failure;
        }
        if (m_FailurePolicy == Policy::RequireAll && failureCount == m_Children.size()) {
            // All failed - call OnExit for all children still entered
            for (size_t i = 0; i < m_Children.size(); ++i) {
                if (m_ChildrenEntered[i]) {
                    m_Children[i]->OnExit(context);
                    m_ChildrenEntered[i] = false;
                }
            }

            // Check if entity still exists before resetting
            if (context.Entity && context.Scene) {
                auto& registry = context.Scene->GetRegistry();
                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                if (registry.valid(entityHandle)) {
                    Reset();
                }
            }
            return BTStatus::Failure;
        }

        // Check success condition
        if (m_SuccessPolicy == Policy::RequireAll && successCount == m_Children.size()) {
            // All succeeded - call OnExit for all children still entered
            for (size_t i = 0; i < m_Children.size(); ++i) {
                if (m_ChildrenEntered[i]) {
                    m_Children[i]->OnExit(context);
                    m_ChildrenEntered[i] = false;
                }
            }

            if (!context.isroot) {
                // Check if entity still exists before resetting
                if (context.Entity && context.Scene) {
                    auto& registry = context.Scene->GetRegistry();
                    entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                    if (registry.valid(entityHandle)) {
                        Reset();
                    }
                }
            }
            return BTStatus::Success;
        }
        if (m_SuccessPolicy == Policy::RequireOne && successCount > 0 && runningCount == 0) {
            // At least one succeeded and none running - call OnExit for all children still entered
            for (size_t i = 0; i < m_Children.size(); ++i) {
                if (m_ChildrenEntered[i]) {
                    m_Children[i]->OnExit(context);
                    m_ChildrenEntered[i] = false;
                }
            }

            if (!context.isroot) {
                // Check if entity still exists before resetting
                if (context.Entity && context.Scene) {
                    auto& registry = context.Scene->GetRegistry();
                    entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                    if (registry.valid(entityHandle)) {
                        Reset();
                    }
                }
            }
            return BTStatus::Success;
        }

        // Still running
        return BTStatus::Running;
    }

	////working but not for multiple entities
 //   BTStatus BTParallel::Execute(BTContext& context) {
 //       if (m_Children.empty()) {
 //           return BTStatus::Success;
 //       }

 //       size_t successCount = 0;
 //       size_t failureCount = 0;
 //       size_t runningCount = 0;

 //       static std::unordered_map<BTNode*, bool> enteredChildren;

 //       // Execute all children
 //       for (auto& child : m_Children) {
 //           auto key = child.get();

 //           if (enteredChildren[key] == false || enteredChildren.find(key) == enteredChildren.end()) {
 //               child->OnEnter(context);
 //               enteredChildren[key] = true;
 //           }

 //           BTStatus status = child->Execute(context);

 //           switch (status) {
 //           case BTStatus::Success:
 //               if (enteredChildren[key]) {
 //                   child->OnExit(context);
 //                   enteredChildren[key] = false;
 //               }
 //               successCount++;
 //               break;
 //           case BTStatus::Failure:
 //               if (enteredChildren[key]) {
 //                   child->OnExit(context);
 //                   enteredChildren[key] = false;
 //               }
 //               failureCount++;
 //               break;
 //           case BTStatus::Running:
 //               runningCount++;
 //               break;
 //           }
 //       }

 //       // Check failure condition
 //       if (m_FailurePolicy == Policy::RequireOne && failureCount > 0) {
 //           for (auto& child : m_Children) {
 //               auto key = child.get();
 //               if (enteredChildren[key]) {
 //                   child->OnExit(context);
 //                   enteredChildren[key] = false;
 //               }
 //           }
 //           if (context.Entity && context.Scene) {
 //               auto& registry = context.Scene->GetRegistry();
 //               entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
 //               if (registry.valid(entityHandle)) {
 //                   Reset();
 //               }
 //           }
 //           return BTStatus::Failure;
 //       }
 //       if (m_FailurePolicy == Policy::RequireAll && failureCount == m_Children.size()) {
 //           for (auto& child : m_Children) {
 //               auto key = child.get();
 //               if (enteredChildren[key]) {
 //                   child->OnExit(context);
 //                   enteredChildren[key] = false;
 //               }
 //           }
 //           if (context.Entity && context.Scene) {
 //               auto& registry = context.Scene->GetRegistry();
 //               entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
 //               if (registry.valid(entityHandle)) {
 //                   Reset();
 //               }
 //           }
 //           return BTStatus::Failure;
 //       }

 //       // Check success condition
 //       if (m_SuccessPolicy == Policy::RequireAll && successCount == m_Children.size()) {
 //           for (auto& child : m_Children) {
 //               auto key = child.get();
 //               if (enteredChildren[key]) {
 //                   child->OnExit(context);
 //                   enteredChildren[key] = false;
 //               }
 //           }
 //           if (!context.isroot) {
 //               if (context.Entity && context.Scene) {
 //                   auto& registry = context.Scene->GetRegistry();
 //                   entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
 //                   if (registry.valid(entityHandle)) {
 //                       Reset();
 //                   }
 //               }
 //           }
 //           return BTStatus::Success;
 //       }
 //       if (m_SuccessPolicy == Policy::RequireOne && successCount > 0 && runningCount == 0) {
 //           for (auto& child : m_Children) {
 //               auto key = child.get();
 //               if (enteredChildren[key]) {
 //                   child->OnExit(context);
 //                   enteredChildren[key] = false;
 //               }
 //           }
 //           if (!context.isroot) {
 //               if (context.Entity && context.Scene) {
 //                   auto& registry = context.Scene->GetRegistry();
 //                   entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
 //                   if (registry.valid(entityHandle)) {
 //                       Reset();
 //                   }
 //               }
 //           }
 //           return BTStatus::Success;
 //       }

 //       // Still running
 //       return BTStatus::Running;
 //   }

    //BTStatus BTParallel::Execute(BTContext& context) {
    //    if (m_Children.empty()) {
    //        return BTStatus::Success;
    //    }

    //    size_t successCount = 0;
    //    size_t failureCount = 0;
    //    size_t runningCount = 0;

    //    // Execute all children
    //    for (auto& child : m_Children) {
    //        BTStatus status = child->Execute(context);

    //        switch (status) {
    //        case BTStatus::Success:
    //            successCount++;
    //            break;
    //        case BTStatus::Failure:
    //            failureCount++;
    //            break;
    //        case BTStatus::Running:
    //            runningCount++;
    //            break;
    //        }
    //    }

    //    // Check failure condition
    //    if (m_FailurePolicy == Policy::RequireOne && failureCount > 0) {
    //        // Check if entity still exists before resetting
    //        if (context.Entity && context.Scene) {
    //            auto& registry = context.Scene->GetRegistry();
    //            entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
    //            if (registry.valid(entityHandle)) {
    //                Reset();
    //            }
    //        }
    //        return BTStatus::Failure;
    //    }
    //    if (m_FailurePolicy == Policy::RequireAll && failureCount == m_Children.size()) {
    //        // Check if entity still exists before resetting
    //        if (context.Entity && context.Scene) {
    //            auto& registry = context.Scene->GetRegistry();
    //            entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
    //            if (registry.valid(entityHandle)) {
    //                Reset();
    //            }
    //        }
    //        return BTStatus::Failure;
    //    }

    //    // Check success condition
    //    if (m_SuccessPolicy == Policy::RequireAll && successCount == m_Children.size()) {
    //        if (!context.isroot) {
    //            // Check if entity still exists before resetting
    //            if (context.Entity && context.Scene) {
    //                auto& registry = context.Scene->GetRegistry();
    //                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
    //                if (registry.valid(entityHandle)) {
    //                    Reset();
    //                }
    //            }
    //        }
    //        return BTStatus::Success;
    //    }
    //    if (m_SuccessPolicy == Policy::RequireOne && successCount > 0 && runningCount == 0) {
    //        if (!context.isroot) {
    //            // Check if entity still exists before resetting
    //            if (context.Entity && context.Scene) {
    //                auto& registry = context.Scene->GetRegistry();
    //                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
    //                if (registry.valid(entityHandle)) {
    //                    Reset();
    //                }
    //            }
    //        }
    //        return BTStatus::Success;
    //    }

    //    // Still running
    //    return BTStatus::Running;
    //}

    void BTParallel::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "SuccessPolicy", m_SuccessPolicy == Policy::RequireAll ? "RequireAll" : "RequireOne" });
        properties.push_back({ "FailurePolicy", m_FailurePolicy == Policy::RequireAll ? "RequireAll" : "RequireOne" });
    }

    void BTParallel::SetProperty(const std::string& name, const std::string& value) {
        if (name == "SuccessPolicy") {
            m_SuccessPolicy = (value == "RequireAll") ? Policy::RequireAll : Policy::RequireOne;
        }
        else if (name == "FailurePolicy") {
            m_FailurePolicy = (value == "RequireAll") ? Policy::RequireAll : Policy::RequireOne;
        }
    }

} // namespace Engine

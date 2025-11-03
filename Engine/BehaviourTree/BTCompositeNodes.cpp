/**
 * @file BTCompositeNodes.h
 * @brief Composite behaviour tree nodes (Sequence, Selector, Parallel)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTCompositeNodes.h"

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
        }
    }

    void BTComposite::RemoveChild(size_t index) {
        if (index < m_Children.size()) {
            m_Children.erase(m_Children.begin() + index);
        }
    }

    void BTComposite::Reset() {
        m_CurrentChildIndex = 0;
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
            BTStatus status = m_Children[m_CurrentChildIndex]->Execute(context);

            if (status == BTStatus::Failure) {
                Reset();
                return BTStatus::Failure;
            }
            else if (status == BTStatus::Running) {
                return BTStatus::Running;
            }

            // Child succeeded, move to next
            m_CurrentChildIndex++;
        }

        // All children succeeded
        if (!context.isroot) {
            Reset();
        }
        return BTStatus::Success;
    }


    //BTSelector
    BTStatus BTSelector::Execute(BTContext& context) {
        // Try children in sequence until one succeeds
        while (m_CurrentChildIndex < m_Children.size()) {
            BTStatus status = m_Children[m_CurrentChildIndex]->Execute(context);

            if (status == BTStatus::Success) {
                Reset();
                return BTStatus::Success;
            }
            else if (status == BTStatus::Running) {
                return BTStatus::Running;
            }

            // Child failed, try next
            m_CurrentChildIndex++;
        }

        // All children failed
        if (!context.isroot) {
            Reset();
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
        for (auto& child : m_Children) {
            BTStatus status = child->Execute(context);

            switch (status) {
            case BTStatus::Success:
                successCount++;
                break;
            case BTStatus::Failure:
                failureCount++;
                break;
            case BTStatus::Running:
                runningCount++;
                break;
            }
        }

        // Check failure condition
        if (m_FailurePolicy == Policy::RequireOne && failureCount > 0) {
            Reset();
            return BTStatus::Failure;
        }
        if (m_FailurePolicy == Policy::RequireAll && failureCount == m_Children.size()) {
            Reset();
            return BTStatus::Failure;
        }

        // Check success condition
        if (m_SuccessPolicy == Policy::RequireAll && successCount == m_Children.size()) {
            if (!context.isroot) {
                Reset();
            }
            return BTStatus::Success;
        }
        if (m_SuccessPolicy == Policy::RequireOne && successCount > 0 && runningCount == 0) {
            if (!context.isroot) {
                Reset();
            }
            return BTStatus::Success;
        }

        // Still running
        return BTStatus::Running;
    }

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

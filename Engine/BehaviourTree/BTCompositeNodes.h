/**
 * @file BTCompositeNodes.h
 * @brief Composite behaviour tree nodes (Sequence, Selector, Parallel)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTNode.h"

namespace Engine {

    /**
     * @brief Base class for composite nodes (nodes with children)
     */
    class BTComposite : public BTNode {
    public:
        std::vector<std::shared_ptr<BTNode>>& GetChildren() override {
            return m_Children;
        }

        const std::vector<std::shared_ptr<BTNode>>& GetChildren() const override {
            return m_Children;
        }

        bool CanHaveChildren() const override { return true; }

        void AddChild(std::shared_ptr<BTNode> child) override {
            if (child) {
                m_Children.push_back(child);
            }
        }

        void RemoveChild(size_t index) override {
            if (index < m_Children.size()) {
                m_Children.erase(m_Children.begin() + index);
            }
        }

        void Reset() override {
            m_CurrentChildIndex = 0;
            for (auto& child : m_Children) {
                child->Reset();
            }
        }

    protected:
        std::vector<std::shared_ptr<BTNode>> m_Children;
        size_t m_CurrentChildIndex = 0;
    };

    /**
     * @brief Sequence node - executes children in order, fails if any child fails
     * @details Returns Success only if all children succeed
     */
    class BTSequence : public BTComposite {
    public:
        const char* GetTypeName() const override { return "Sequence"; }

        BTStatus Execute(BTContext& context) override {
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
            Reset();
            return BTStatus::Success;
        }
    };

    /**
     * @brief Selector node - executes children until one succeeds
     * @details Returns Success if any child succeeds, Failure if all fail
     */
    class BTSelector : public BTComposite {
    public:
        const char* GetTypeName() const override { return "Selector"; }

        BTStatus Execute(BTContext& context) override {
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
            Reset();
            return BTStatus::Failure;
        }
    };

    /**
     * @brief Parallel node - executes all children simultaneously
     * @details Can be configured to succeed/fail based on number of child successes/failures
     */
    class BTParallel : public BTComposite {
    public:
        enum class Policy {
            RequireAll,     ///< Succeed only if all children succeed
            RequireOne      ///< Succeed if at least one child succeeds
        };

        BTParallel(Policy successPolicy = Policy::RequireAll, Policy failurePolicy = Policy::RequireOne)
            : m_SuccessPolicy(successPolicy), m_FailurePolicy(failurePolicy) {}

        const char* GetTypeName() const override { return "Parallel"; }

        BTStatus Execute(BTContext& context) override {
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
                Reset();
                return BTStatus::Success;
            }
            if (m_SuccessPolicy == Policy::RequireOne && successCount > 0 && runningCount == 0) {
                Reset();
                return BTStatus::Success;
            }

            // Still running
            return BTStatus::Running;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "SuccessPolicy", m_SuccessPolicy == Policy::RequireAll ? "RequireAll" : "RequireOne" });
            properties.push_back({ "FailurePolicy", m_FailurePolicy == Policy::RequireAll ? "RequireAll" : "RequireOne" });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "SuccessPolicy") {
                m_SuccessPolicy = (value == "RequireAll") ? Policy::RequireAll : Policy::RequireOne;
            }
            else if (name == "FailurePolicy") {
                m_FailurePolicy = (value == "RequireAll") ? Policy::RequireAll : Policy::RequireOne;
            }
        }

        // Allow direct property access for registration macros
        Policy m_SuccessPolicy;
        Policy m_FailurePolicy;
    };

} // namespace Engine

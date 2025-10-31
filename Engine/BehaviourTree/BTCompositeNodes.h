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
        std::vector<std::shared_ptr<BTNode>>& GetChildren() override;

        const std::vector<std::shared_ptr<BTNode>>& GetChildren() const override;

        bool CanHaveChildren() const override;

        void AddChild(std::shared_ptr<BTNode> child) override;

        void RemoveChild(size_t index) override;

        void Reset() override;

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

        BTStatus Execute(BTContext& context) override;
    };

    /**
     * @brief Selector node - executes children until one succeeds
     * @details Returns Success if any child succeeds, Failure if all fail
     */
    class BTSelector : public BTComposite {
    public:
        const char* GetTypeName() const override { return "Selector"; }

        BTStatus Execute(BTContext& context) override;
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

        BTParallel(Policy successPolicy = Policy::RequireAll, Policy failurePolicy = Policy::RequireOne);

        const char* GetTypeName() const override { return "Parallel"; }

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        Policy m_SuccessPolicy;
        Policy m_FailurePolicy;
    };

} // namespace Engine

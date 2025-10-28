/**
 * @file BTDecoratorNodes.h
 * @brief Decorator behaviour tree nodes (modify child behaviour)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTNode.h"

namespace Engine {

    /**
     * @brief Base class for decorator nodes (single child with modified behaviour)
     */
    class BTDecorator : public BTNode {
    public:
        bool CanHaveChildren() const override { return true; }

        void AddChild(std::shared_ptr<BTNode> child) override {
            m_Child = child;
        }

        void RemoveChild(size_t index) override {
            if (index == 0) {
                m_Child.reset();
            }
        }

        std::vector<std::shared_ptr<BTNode>>& GetChildren() override {
            m_ChildVector.clear();
            if (m_Child) {
                m_ChildVector.push_back(m_Child);
            }
            return m_ChildVector;
        }

        const std::vector<std::shared_ptr<BTNode>>& GetChildren() const override {
            m_ChildVector.clear();
            if (m_Child) {
                m_ChildVector.push_back(m_Child);
            }
            return m_ChildVector;
        }

        void Reset() override {
            if (m_Child) {
                m_Child->Reset();
            }
        }

    protected:
        std::shared_ptr<BTNode> m_Child;
        mutable std::vector<std::shared_ptr<BTNode>> m_ChildVector; // For GetChildren interface
    };

    /**
     * @brief Inverter - inverts the result of its child
     */
    class BTInverter : public BTDecorator {
    public:
        const char* GetTypeName() const override { return "Inverter"; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Child) {
                return BTStatus::Failure;
            }

            BTStatus status = m_Child->Execute(context);

            if (status == BTStatus::Success) {
                return BTStatus::Failure;
            }
            else if (status == BTStatus::Failure) {
                return BTStatus::Success;
            }

            return BTStatus::Running;
        }
    };

    /**
     * @brief Succeeder - always returns success regardless of child result
     */
    class BTSucceeder : public BTDecorator {
    public:
        const char* GetTypeName() const override { return "Succeeder"; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Child) {
                return BTStatus::Success;
            }

            BTStatus status = m_Child->Execute(context);
            return (status == BTStatus::Running) ? BTStatus::Running : BTStatus::Success;
        }
    };

    /**
     * @brief Failer - always returns failure regardless of child result
     */
    class BTFailer : public BTDecorator {
    public:
        const char* GetTypeName() const override { return "Failer"; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Child) {
                return BTStatus::Failure;
            }

            BTStatus status = m_Child->Execute(context);
            return (status == BTStatus::Running) ? BTStatus::Running : BTStatus::Failure;
        }
    };

    /**
     * @brief Repeater - repeats child execution N times or infinitely
     */
    class BTRepeater : public BTDecorator {
    public:
        BTRepeater(int repeatCount = -1) // -1 = infinite
            : m_RepeatCount(repeatCount), m_CurrentCount(0) {}

        const char* GetTypeName() const override { return "Repeater"; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Child) {
                return BTStatus::Failure;
            }

            // Infinite repeat
            if (m_RepeatCount < 0) {
                BTStatus status = m_Child->Execute(context);
                if (status != BTStatus::Running) {
                    m_Child->Reset(); // Reset child for next iteration
                }
                return BTStatus::Running; // Always running
            }

            // Limited repeat
            while (m_CurrentCount < m_RepeatCount) {
                BTStatus status = m_Child->Execute(context);

                if (status == BTStatus::Running) {
                    return BTStatus::Running;
                }

                m_Child->Reset();
                m_CurrentCount++;
            }

            // Completed all repeats
            m_CurrentCount = 0;
            return BTStatus::Success;
        }

        void Reset() override {
            m_CurrentCount = 0;
            BTDecorator::Reset();
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "RepeatCount", std::to_string(m_RepeatCount) });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "RepeatCount") {
                m_RepeatCount = std::stoi(value);
            }
        }

        // Allow direct property access for registration macros
        int m_RepeatCount;
        int m_CurrentCount;
    };

    /**
     * @brief RepeatUntilFail - repeats child until it fails
     */
    class BTRepeatUntilFail : public BTDecorator {
    public:
        const char* GetTypeName() const override { return "RepeatUntilFail"; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Child) {
                return BTStatus::Failure;
            }

            BTStatus status = m_Child->Execute(context);

            if (status == BTStatus::Failure) {
                m_Child->Reset();
                return BTStatus::Success; // We succeed when child fails
            }
            else if (status == BTStatus::Success) {
                m_Child->Reset(); // Reset for next iteration
            }

            return BTStatus::Running;
        }
    };

    /**
     * @brief Cooldown - prevents child from running more than once per cooldown period
     */
    class BTCooldown : public BTDecorator {
    public:
        BTCooldown(float cooldownTime = 1.0f)
            : m_CooldownTime(cooldownTime), m_TimeSinceLastRun(cooldownTime) {}

        const char* GetTypeName() const override { return "Cooldown"; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Child) {
                return BTStatus::Failure;
            }

            m_TimeSinceLastRun += context.DeltaTime;

            // Still in cooldown
            if (m_TimeSinceLastRun < m_CooldownTime) {
                return BTStatus::Failure;
            }

            // Execute child
            BTStatus status = m_Child->Execute(context);

            // Reset cooldown when child completes
            if (status != BTStatus::Running) {
                m_TimeSinceLastRun = 0.0f;
            }

            return status;
        }

        void Reset() override {
            m_TimeSinceLastRun = m_CooldownTime; // Ready to execute
            BTDecorator::Reset();
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "CooldownTime", std::to_string(m_CooldownTime) });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "CooldownTime") {
                m_CooldownTime = std::stof(value);
            }
        }

        // Allow direct property access for registration macros
        float m_CooldownTime;
        float m_TimeSinceLastRun;
    };

} // namespace Engine

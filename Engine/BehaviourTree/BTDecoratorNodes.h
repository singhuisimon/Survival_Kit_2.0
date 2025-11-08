/**
 * @file BTDecoratorNodes.h
 * @brief Decorator behaviour tree nodes (modify child behaviour)
 * @author Amanda Leow Boon Suan (90%), Rio Shannon Yvon Leonardo (10%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#pragma once

#include "BTNode.h"

namespace Engine {

    /**
     * @brief Base class for decorator nodes (single child with modified behaviour)
     */
    class BTDecorator : public BTNode {
    public:
        bool CanHaveChildren() const override;

        void AddChild(std::shared_ptr<BTNode> child);

        void RemoveChild(size_t index) override;

        std::vector<std::shared_ptr<BTNode>>& GetChildren() override;

        const std::vector<std::shared_ptr<BTNode>>& GetChildren() const override;

        void Reset() override;

    protected:
        std::shared_ptr<BTNode> m_Child;
        mutable std::vector<std::shared_ptr<BTNode>> m_ChildVector; // For GetChildren interface
    };

    /**
     * @brief Inverter - inverts the result of its child
     */
    class BTInverter : public BTDecorator {
    public:
        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;
    };

    /**
     * @brief Succeeder - always returns success regardless of child result
     */
    class BTSucceeder : public BTDecorator {
    public:
        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;
    };

    /**
     * @brief Failer - always returns failure regardless of child result
     */
    class BTFailer : public BTDecorator {
    public:
        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;
    };

    /**
     * @brief Repeater - repeats child execution N times or infinitely
     */
    class BTRepeater : public BTDecorator {
    public:
        BTRepeater(int repeatCount = -1); // -1 = infinite

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void Reset() override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        int m_RepeatCount;
        int m_CurrentCount;
    };

    /**
     * @brief RepeatUntilFail - repeats child until it fails
     */
    class BTRepeatUntilFail : public BTDecorator {
    public:
        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;
    };

    /**
     * @brief Cooldown - prevents child from running more than once per cooldown period
     */
    class BTCooldown : public BTDecorator {
    public:
        BTCooldown(float cooldownTime = 1.0f);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void Reset() override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        float m_CooldownTime;
        float m_TimeSinceLastRun;
    };

} // namespace Engine

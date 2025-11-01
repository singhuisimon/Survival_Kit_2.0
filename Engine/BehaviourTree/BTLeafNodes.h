/**
 * @file BTLeafNodes.h
 * @brief Leaf behaviour tree nodes (Actions and Conditions)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTNode.h"
#include <functional>

namespace Engine {

    /**
     * @brief Action node - executes a custom action
     * @details Leaf node that performs actual game logic
     */
    class BTAction : public BTNode {
    public:
        using ActionFunc = std::function<BTStatus(BTContext&)>;

        BTAction(ActionFunc action = nullptr);

        const char* GetTypeName() const override;

        void SetAction(ActionFunc action);

        BTStatus Execute(BTContext& context) override;

    private:
        ActionFunc m_Action;
    };

    /**
     * @brief Condition node - checks a condition and returns success/failure
     */
    class BTCondition : public BTNode {
    public:
        using ConditionFunc = std::function<bool(BTContext&)>;

        BTCondition(ConditionFunc condition = nullptr);

        const char* GetTypeName() const override;

        void SetCondition(ConditionFunc condition);

        BTStatus Execute(BTContext& context) override;

    private:
        ConditionFunc m_Condition;
    };

    /**
     * @brief Wait node - waits for a specified duration
     */
    class BTWait : public BTNode {
    public:
        BTWait(float duration = 1.0f);

        const char* GetTypeName() const override;

        void OnEnter(BTContext& context) override;

        BTStatus Execute(BTContext& context) override;

        void Reset() override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        float m_Duration;
        float m_ElapsedTime;
    };

    /**
     * @brief Log node - outputs a debug message
     */
    class BTLog : public BTNode {
    public:
        BTLog(const std::string& message = "");

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        std::string m_Message;
        float accumulatedtime;
        float time = 60.0f;
    };

    /**
     * @brief SetBlackboard node - sets a blackboard value with type support
     */
    class BTSetBlackboard : public BTNode {
    public:
        BTSetBlackboard(const std::string& key = "", const std::string& value = "", const std::string& type = "string");

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        std::string m_Key;
        std::string m_Value;
        std::string m_Type;  // "bool", "int", "float", "string", "vec2", "vec3", "vec4", "entity"
    };

    /**
     * @brief CheckBlackboard node - checks if a blackboard value matches (string comparison)
     */
    class BTCheckBlackboard : public BTNode {
    public:
        BTCheckBlackboard(const std::string& key = "", const std::string& expectedValue = "");

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        std::string m_Key;
        std::string m_ExpectedValue;
    };

    /**
     * @brief SetBlackboardInt - convenience node for setting integer values
     */
    class BTSetBlackboardInt : public BTNode {
    public:
        BTSetBlackboardInt(const std::string& key = "", int value = 0);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        int m_Value;
    };

    /**
     * @brief SetBlackboardFloat - convenience node for setting float values
     */
    class BTSetBlackboardFloat : public BTNode {
    public:
        BTSetBlackboardFloat(const std::string& key = "", float value = 0.0f);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        float m_Value;
    };

    /**
     * @brief SetBlackboardBool - convenience node for setting boolean values
     */
    class BTSetBlackboardBool : public BTNode {
    public:
        BTSetBlackboardBool(const std::string& key = "", bool value = false);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        bool m_Value;
    };

    /**
     * @brief SetBlackboardVec3 - convenience node for setting vec3 values
     */
    class BTSetBlackboardVec3 : public BTNode {
    public:
        BTSetBlackboardVec3(const std::string& key = "", const glm::vec3& value = glm::vec3(0.0f));

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        glm::vec3 m_Value;
    };

} // namespace Engine

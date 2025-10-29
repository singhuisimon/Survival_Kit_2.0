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

        BTAction(ActionFunc action = nullptr)
            : m_Action(action) {}

        const char* GetTypeName() const override { return "Action"; }

        void SetAction(ActionFunc action) { m_Action = action; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Action) {
                return BTStatus::Failure;
            }
            return m_Action(context);
        }

    private:
        ActionFunc m_Action;
    };

    /**
     * @brief Condition node - checks a condition and returns success/failure
     */
    class BTCondition : public BTNode {
    public:
        using ConditionFunc = std::function<bool(BTContext&)>;

        BTCondition(ConditionFunc condition = nullptr)
            : m_Condition(condition) {}

        const char* GetTypeName() const override { return "Condition"; }

        void SetCondition(ConditionFunc condition) { m_Condition = condition; }

        BTStatus Execute(BTContext& context) override {
            if (!m_Condition) {
                return BTStatus::Failure;
            }
            return m_Condition(context) ? BTStatus::Success : BTStatus::Failure;
        }

    private:
        ConditionFunc m_Condition;
    };

    /**
     * @brief Wait node - waits for a specified duration
     */
    class BTWait : public BTNode {
    public:
        BTWait(float duration = 1.0f)
            : m_Duration(duration), m_ElapsedTime(0.0f) {}

        const char* GetTypeName() const override { return "Wait"; }

        void OnEnter(BTContext& context) override {
            (void)context;
            m_ElapsedTime = 0.0f;
        }

        BTStatus Execute(BTContext& context) override {
            m_ElapsedTime += context.DeltaTime;

            if (m_ElapsedTime >= m_Duration) {
                return BTStatus::Success;
            }

            return BTStatus::Running;
        }

        void Reset() override {
            m_ElapsedTime = 0.0f;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Duration", std::to_string(m_Duration) });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Duration") {
                m_Duration = std::stof(value);
            }
        }

        // Allow direct property access for registration macros
        float m_Duration;
        float m_ElapsedTime;
    };

    /**
     * @brief Log node - outputs a debug message
     */
    class BTLog : public BTNode {
    public:
        BTLog(const std::string& message = "")
            : m_Message(message) {}

        const char* GetTypeName() const override { return "Log"; }

        BTStatus Execute(BTContext& context) override {
            (void)context;
            // In real implementation, use your Logger system
            // LOG_INFO("BehaviourTree: ", m_Message);
            return BTStatus::Success;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Message", m_Message });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Message") {
                m_Message = value;
            }
        }

        // Allow direct property access for registration macros
        std::string m_Message;
    };

    /**
     * @brief SetBlackboard node - sets a blackboard value with type support
     */
    class BTSetBlackboard : public BTNode {
    public:
        BTSetBlackboard(const std::string& key = "", const std::string& value = "", const std::string& type = "string")
            : m_Key(key), m_Value(value), m_Type(type) {
        }

        const char* GetTypeName() const override { return "SetBlackboard"; }

        BTStatus Execute(BTContext& context) override {
            if (m_Key.empty()) {
                return BTStatus::Failure;
            }

            // Use the blackboard's FromString method to set the value with proper type
            if (context.Blackboard.FromString(m_Key, m_Type, m_Value)) {
                return BTStatus::Success;
            }

            return BTStatus::Failure;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Key", m_Key });
            properties.push_back({ "Type", m_Type });
            properties.push_back({ "Value", m_Value });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Key") {
                m_Key = value;
            }
            else if (name == "Type") {
                m_Type = value;
            }
            else if (name == "Value") {
                m_Value = value;
            }
        }

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
        BTCheckBlackboard(const std::string& key = "", const std::string& expectedValue = "")
            : m_Key(key), m_ExpectedValue(expectedValue) {
        }

        const char* GetTypeName() const override { return "CheckBlackboard"; }

        BTStatus Execute(BTContext& context) override {
            if (m_Key.empty()) {
                return BTStatus::Failure;
            }

            if (!context.Blackboard.Has(m_Key)) {
                return BTStatus::Failure;
            }

            // Convert to string and compare
            std::string actualValue = context.Blackboard.ToString(m_Key);
            return (actualValue == m_ExpectedValue) ? BTStatus::Success : BTStatus::Failure;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Key", m_Key });
            properties.push_back({ "ExpectedValue", m_ExpectedValue });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Key") {
                m_Key = value;
            }
            else if (name == "ExpectedValue") {
                m_ExpectedValue = value;
            }
        }

        // Allow direct property access for registration macros
        std::string m_Key;
        std::string m_ExpectedValue;
    };

    /**
     * @brief SetBlackboardInt - convenience node for setting integer values
     */
    class BTSetBlackboardInt : public BTNode {
    public:
        BTSetBlackboardInt(const std::string& key = "", int value = 0)
            : m_Key(key), m_Value(value) {
        }

        const char* GetTypeName() const override { return "SetBlackboardInt"; }

        BTStatus Execute(BTContext& context) override {
            if (m_Key.empty()) return BTStatus::Failure;
            context.Blackboard.Set(m_Key, m_Value);
            return BTStatus::Success;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Key", m_Key });
            properties.push_back({ "Value", std::to_string(m_Value) });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Key") m_Key = value;
            else if (name == "Value") m_Value = std::stoi(value);
        }

        std::string m_Key;
        int m_Value;
    };

    /**
     * @brief SetBlackboardFloat - convenience node for setting float values
     */
    class BTSetBlackboardFloat : public BTNode {
    public:
        BTSetBlackboardFloat(const std::string& key = "", float value = 0.0f)
            : m_Key(key), m_Value(value) {
        }

        const char* GetTypeName() const override { return "SetBlackboardFloat"; }

        BTStatus Execute(BTContext& context) override {
            if (m_Key.empty()) return BTStatus::Failure;
            context.Blackboard.Set(m_Key, m_Value);
            return BTStatus::Success;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Key", m_Key });
            properties.push_back({ "Value", std::to_string(m_Value) });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Key") m_Key = value;
            else if (name == "Value") m_Value = std::stof(value);
        }

        std::string m_Key;
        float m_Value;
    };

    /**
     * @brief SetBlackboardBool - convenience node for setting boolean values
     */
    class BTSetBlackboardBool : public BTNode {
    public:
        BTSetBlackboardBool(const std::string& key = "", bool value = false)
            : m_Key(key), m_Value(value) {
        }

        const char* GetTypeName() const override { return "SetBlackboardBool"; }

        BTStatus Execute(BTContext& context) override {
            if (m_Key.empty()) return BTStatus::Failure;
            context.Blackboard.Set(m_Key, m_Value);
            return BTStatus::Success;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Key", m_Key });
            properties.push_back({ "Value", m_Value ? "true" : "false" });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Key") m_Key = value;
            else if (name == "Value") m_Value = (value == "true" || value == "1");
        }

        std::string m_Key;
        bool m_Value;
    };

    /**
     * @brief SetBlackboardVec3 - convenience node for setting vec3 values
     */
    class BTSetBlackboardVec3 : public BTNode {
    public:
        BTSetBlackboardVec3(const std::string& key = "", const glm::vec3& value = glm::vec3(0.0f))
            : m_Key(key), m_Value(value) {
        }

        const char* GetTypeName() const override { return "SetBlackboardVec3"; }

        BTStatus Execute(BTContext& context) override {
            if (m_Key.empty()) return BTStatus::Failure;
            context.Blackboard.Set(m_Key, m_Value);
            return BTStatus::Success;
        }

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override {
            properties.push_back({ "Key", m_Key });
            properties.push_back({ "X", std::to_string(m_Value.x) });
            properties.push_back({ "Y", std::to_string(m_Value.y) });
            properties.push_back({ "Z", std::to_string(m_Value.z) });
        }

        void SetProperty(const std::string& name, const std::string& value) override {
            if (name == "Key") m_Key = value;
            else if (name == "X") m_Value.x = std::stof(value);
            else if (name == "Y") m_Value.y = std::stof(value);
            else if (name == "Z") m_Value.z = std::stof(value);
        }

        std::string m_Key;
        glm::vec3 m_Value;
    };

} // namespace Engine

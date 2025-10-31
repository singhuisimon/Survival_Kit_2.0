/**
 * @file BTLeafNodes.h
 * @brief Leaf behaviour tree nodes (Actions and Conditions)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTLeafNodes.h"
#include <functional>

namespace Engine {

    //Action

    BTAction::BTAction(ActionFunc action): m_Action(action) {}

    const char* BTAction::GetTypeName() const { return "Action"; }

    void BTAction::SetAction(ActionFunc action) { m_Action = action; }

    BTStatus BTAction::Execute(BTContext& context) {
        if (!m_Action) {
            return BTStatus::Failure;
        }
        return m_Action(context);
    }


    //BTCondition
    BTCondition::BTCondition(ConditionFunc condition) : m_Condition(condition) {}
    
    const char* BTCondition::GetTypeName() const { return "Condition"; }
    
    void BTCondition::SetCondition(ConditionFunc condition) { m_Condition = condition; }
    
    BTStatus BTCondition::Execute(BTContext& context) {
        if (!m_Condition) {
            return BTStatus::Failure;
        }
        return m_Condition(context) ? BTStatus::Success : BTStatus::Failure;
    }


    //BTWait
    BTWait::BTWait(float duration) : m_Duration(duration), m_ElapsedTime(0.0f) {}
    
    const char* BTWait::GetTypeName() const { return "Wait"; }
    
    void BTWait::OnEnter(BTContext& context) {
        (void)context;
        m_ElapsedTime = 0.0f;
    }
    
    BTStatus BTWait::Execute(BTContext& context) {
        m_ElapsedTime += context.DeltaTime;
    
        if (m_ElapsedTime >= m_Duration) {
            return BTStatus::Success;
        }
    
        return BTStatus::Running;
    }
    
    void BTWait::Reset() {
        m_ElapsedTime = 0.0f;
    }
    
    void BTWait::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Duration", std::to_string(m_Duration) });
    }
    
    void BTWait::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Duration") {
            m_Duration = std::stof(value);
        }
    }
    

    //BTLog:
    BTLog::BTLog(const std::string& message) : m_Message(message) {}

    const char* BTLog::GetTypeName() const { return "Log"; }

    BTStatus BTLog::Execute(BTContext& context) {
        (void)context;
        // In real implementation, use your Logger system
        // LOG_INFO("BehaviourTree: ", m_Message);
        return BTStatus::Success;
    }
    
    void BTLog::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Message", m_Message });
    }
    
    void BTLog::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Message") {
            m_Message = value;
        }
    }

    //BTSetBlackboard
    BTSetBlackboard::BTSetBlackboard(const std::string& key, const std::string& value, const std::string& type)
        : m_Key(key), m_Value(value), m_Type(type) {}

    const char* BTSetBlackboard::GetTypeName() const { return "SetBlackboard"; }
    
    BTStatus BTSetBlackboard::Execute(BTContext& context) {
        if (m_Key.empty()) {
            return BTStatus::Failure;
        }
    
        // Use the blackboard's FromString method to set the value with proper type
        if (context.Blackboard.FromString(m_Key, m_Type, m_Value)) {
            return BTStatus::Success;
        }
    
        return BTStatus::Failure;
    }
    
    void BTSetBlackboard::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Type", m_Type });
        properties.push_back({ "Value", m_Value });
    }
    
    void BTSetBlackboard::SetProperty(const std::string& name, const std::string& value) {
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
    

    //BTCheckBlackboard
    BTCheckBlackboard::BTCheckBlackboard(const std::string& key, const std::string& expectedValue)
        : m_Key(key), m_ExpectedValue(expectedValue) {
    }
    
    const char* BTCheckBlackboard::GetTypeName() const { return "CheckBlackboard"; }
    
    BTStatus BTCheckBlackboard::Execute(BTContext& context) {
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
    
    void BTCheckBlackboard::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "ExpectedValue", m_ExpectedValue });
    }
    
    void BTCheckBlackboard::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") {
            m_Key = value;
        }
        else if (name == "ExpectedValue") {
            m_ExpectedValue = value;
        }
    }


    //BTSetBlackboardInt
    BTSetBlackboardInt::BTSetBlackboardInt(const std::string& key, int value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardInt::GetTypeName() const { return "SetBlackboardInt"; }
    
    BTStatus BTSetBlackboardInt::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardInt::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Value", std::to_string(m_Value) });
    }
    
    void BTSetBlackboardInt::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "Value") m_Value = std::stoi(value);
    }
    


    //BTSetBlackboardFloat
    BTSetBlackboardFloat::BTSetBlackboardFloat(const std::string& key, float value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardFloat::GetTypeName() const { return "SetBlackboardFloat"; }
    
    BTStatus BTSetBlackboardFloat::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardFloat::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Value", std::to_string(m_Value) });
    }
    
    void BTSetBlackboardFloat::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "Value") m_Value = std::stof(value);
    }



    //BTSetBlackboardBool
    BTSetBlackboardBool::BTSetBlackboardBool(const std::string& key, bool value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardBool::GetTypeName() const { return "SetBlackboardBool"; }
    
    BTStatus BTSetBlackboardBool::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardBool::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Value", m_Value ? "true" : "false" });
    }
    
    void BTSetBlackboardBool::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "Value") m_Value = (value == "true" || value == "1");
    }
    

    //BTSetBlackboardVec3:
    BTSetBlackboardVec3::BTSetBlackboardVec3(const std::string& key, const glm::vec3& value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardVec3::GetTypeName() const { return "SetBlackboardVec3"; }
    
    BTStatus BTSetBlackboardVec3::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardVec3::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "X", std::to_string(m_Value.x) });
        properties.push_back({ "Y", std::to_string(m_Value.y) });
        properties.push_back({ "Z", std::to_string(m_Value.z) });
    }
    
    void BTSetBlackboardVec3::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "X") m_Value.x = std::stof(value);
        else if (name == "Y") m_Value.y = std::stof(value);
        else if (name == "Z") m_Value.z = std::stof(value);
    }
    

} // namespace Engine

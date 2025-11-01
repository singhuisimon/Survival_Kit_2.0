/**
 * @file BTNodeRegistry.h
 * @brief Registry for behaviour tree node types
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTNodeRegistry.h"
#include "../Utility/Logger.h"

#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

namespace Engine {


    //BTNodeRegistry

    /**
     * @brief Register a node type
     * @param setupProperties Optional callback to register node properties
     */
    template<typename T>
    void BTNodeRegistry::RegisterNodeType(
        const std::string& category, 
        const std::string& description,
        std::function<void(BTNodeMetadata&)> setupProperties) {
        
        // Create a temporary instance to get the type name
        auto temp = std::make_shared<T>();
        std::string typeName = temp->GetTypeName();
    
        BTNodeTypeMetadata metadata(
            typeName,
            category,
            description,
            []() { return std::make_shared<T>(); },
            temp->CanHaveChildren()
        );
    
        // Setup properties if callback provided
        if (setupProperties && metadata.PropertyMetadata) {
            setupProperties(*metadata.PropertyMetadata);
        }
    
        m_NodeTypes[typeName] = metadata;
        LOG_INFO("BTNodeRegistry: Registered node type '", typeName, "' (Category: ", category, ")");
    }
    
    /**
     * @brief Create a node instance by type name
     */
    std::shared_ptr<BTNode> BTNodeRegistry::CreateNode(const std::string& typeName) const {
        auto it = m_NodeTypes.find(typeName);
        if (it == m_NodeTypes.end()) {
            LOG_ERROR("BTNodeRegistry: Unknown node type '", typeName, "'");
            return nullptr;
        }
    
        auto node = it->second.Factory();
        if (node) {
            LOG_TRACE("BTNodeRegistry: Created node of type '", typeName, "'");
        }
        return node;
    }
    
    /**
     * @brief Get metadata for a node type
     */
    const BTNodeTypeMetadata* BTNodeRegistry::GetMetadata(const std::string& typeName) const {
        auto it = m_NodeTypes.find(typeName);
        if (it == m_NodeTypes.end()) {
            return nullptr;
        }
        return &it->second;
    }
    
    /**
     * @brief Get all registered node types
     */
    const std::unordered_map<std::string, BTNodeTypeMetadata>& BTNodeRegistry::GetAllNodeTypes() const {
        return m_NodeTypes;
    }
    
    /**
     * @brief Get all node types in a specific category
     */
    std::vector<std::string> BTNodeRegistry::GetNodeTypesByCategory(const std::string& category) const {
        std::vector<std::string> types;
        for (const auto& [typeName, metadata] : m_NodeTypes) {
            if (metadata.Category == category) {
                types.push_back(typeName);
            }
        }
        return types;
    }
    
    /**
     * @brief Check if a node type is registered
     */
    bool BTNodeRegistry::IsNodeTypeRegistered(const std::string& typeName) const {
        return m_NodeTypes.find(typeName) != m_NodeTypes.end();
    }
    
    /**
     * @brief Clear all registered types
     */
    void BTNodeRegistry::Clear() {
        m_NodeTypes.clear();
        LOG_INFO("BTNodeRegistry: Cleared all node types");
    }
    
    /**
     * @brief Register all built-in node types
     */
    void BTNodeRegistry::RegisterBuiltInNodes() {
        auto& registry = Get();
    
        // Composite nodes (no parameters)
        registry.RegisterNodeType<BTSequence>("Composite", "Executes children in sequence until one fails");
        registry.RegisterNodeType<BTSelector>("Composite", "Executes children until one succeeds");
        
        // Parallel with properties
        registry.RegisterNodeType<BTParallel>("Composite", "Executes all children simultaneously",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_ENUM_PROPERTY(BTParallel, "SuccessPolicy", m_SuccessPolicy,
                    PolicyToString, StringToPolicy);
                BT_REGISTER_ENUM_PROPERTY(BTParallel, "FailurePolicy", m_FailurePolicy,
                    PolicyToString, StringToPolicy);
            });
    
        // Decorator nodes
        registry.RegisterNodeType<BTInverter>("Decorator", "Inverts the result of its child");
        registry.RegisterNodeType<BTSucceeder>("Decorator", "Always returns success");
        registry.RegisterNodeType<BTFailer>("Decorator", "Always returns failure");
        
        // Repeater with properties
        registry.RegisterNodeType<BTRepeater>("Decorator", "Repeats child N times or infinitely",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_INT_PROPERTY(BTRepeater, "RepeatCount", m_RepeatCount);
            });
        
        registry.RegisterNodeType<BTRepeatUntilFail>("Decorator", "Repeats child until it fails");
        
        // Cooldown with properties
        registry.RegisterNodeType<BTCooldown>("Decorator", "Prevents child from running during cooldown",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTCooldown, "CooldownTime", m_CooldownTime);
            });
    
        // Leaf nodes
        registry.RegisterNodeType<BTAction>("Action", "Executes custom action logic");
        registry.RegisterNodeType<BTCondition>("Condition", "Checks a condition");
        
        // Wait with properties
        registry.RegisterNodeType<BTWait>("Action", "Waits for a duration",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTWait, "Duration", m_Duration);
            });
        
        // Log with properties
        registry.RegisterNodeType<BTLog>("Action", "Outputs a debug message",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTLog, "Message", m_Message);
            });
        
        // SetBlackboard with properties
        registry.RegisterNodeType<BTSetBlackboard>("Action", "Sets a blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboard, "Key", m_Key);
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboard, "Type", m_Type);
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboard, "Value", m_Value);
            });
    
        
        // CheckBlackboard with properties
        registry.RegisterNodeType<BTCheckBlackboard>("Condition", "Checks a blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCheckBlackboard, "Key", m_Key);
                BT_REGISTER_STRING_PROPERTY(BTCheckBlackboard, "ExpectedValue", m_ExpectedValue);
            });
    
        // Blackboard convenience nodes
        registry.RegisterNodeType<BTSetBlackboardInt>("Action", "Sets an integer blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardInt, "Key", m_Key);
                BT_REGISTER_INT_PROPERTY(BTSetBlackboardInt, "Value", m_Value);
            });
    
        registry.RegisterNodeType<BTSetBlackboardFloat>("Action", "Sets a float blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardFloat, "Key", m_Key);
                BT_REGISTER_FLOAT_PROPERTY(BTSetBlackboardFloat, "Value", m_Value);
            });
    
        registry.RegisterNodeType<BTSetBlackboardBool>("Action", "Sets a bool blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardBool, "Key", m_Key);
                BT_REGISTER_BOOL_PROPERTY(BTSetBlackboardBool, "Value", m_Value);
            });
    
        registry.RegisterNodeType<BTSetBlackboardVec3>("Action", "Sets a vec3 blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardVec3, "Key", m_Key);
                // optional: you could expose XYZ too, but usually Key+Vec3 is fine
            });
    
        LOG_INFO("BTNodeRegistry: Registered Blackboard Set node types (Int, Float, Bool, Vec3)");
    
    
        LOG_INFO("BTNodeRegistry: Registered ", registry.m_NodeTypes.size(), " built-in node types");
    }
    

} // namespace Engine

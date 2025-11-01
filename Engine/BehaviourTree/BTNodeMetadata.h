/**
 * @file BTNodeMetadata.h
 * @brief Property metadata system for behaviour tree nodes
 * @author AI System Team
 * @date 2025
 * 
 * Integrates BT nodes with the engine's Property/Reflection system
 */

#pragma once

#include "BTNode.h"
#include "../Serialization/Property.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace Engine {

    /**
     * @brief Property descriptor for a node parameter
     */
    struct BTNodePropertyDescriptor {
        std::string Name;
        PropertyType Type;
        std::function<std::string(BTNode*)> Getter;   // Returns string representation
        std::function<void(BTNode*, const std::string&)> Setter;
        
        BTNodePropertyDescriptor() = default;
        
        BTNodePropertyDescriptor(
            const std::string& name,
            PropertyType type,
            std::function<std::string(BTNode*)> getter,
            std::function<void(BTNode*, const std::string&)> setter)
            : Name(name), Type(type), Getter(getter), Setter(setter) {}
    };

    /**
     * @brief Metadata for node properties (similar to ComponentMetadata)
     */
    class BTNodeMetadata {
    public:
        BTNodeMetadata(const std::string& typeName, const std::string& category)
            : m_TypeName(typeName), m_Category(category) {}

        const std::string& GetTypeName() const { return m_TypeName; }
        const std::string& GetCategory() const { return m_Category; }
        
        void AddProperty(const BTNodePropertyDescriptor& descriptor) {
            m_Properties.push_back(descriptor);
        }

        const std::vector<BTNodePropertyDescriptor>& GetProperties() const {
            return m_Properties;
        }

        /**
         * @brief Get property value from a node instance
         */
        std::string GetPropertyValue(BTNode* node, const std::string& propertyName) const {
            for (const auto& prop : m_Properties) {
                if (prop.Name == propertyName && prop.Getter) {
                    return prop.Getter(node);
                }
            }
            return "";
        }

        /**
         * @brief Set property value on a node instance
         */
        bool SetPropertyValue(BTNode* node, const std::string& propertyName, const std::string& value) {
            for (const auto& prop : m_Properties) {
                if (prop.Name == propertyName && prop.Setter) {
                    prop.Setter(node, value);
                    return true;
                }
            }
            return false;
        }

    private:
        std::string m_TypeName;
        std::string m_Category;
        std::vector<BTNodePropertyDescriptor> m_Properties;
    };

    /**
     * @brief Helper macros for registering node properties
     */
    
    // Register a float property
    #define BT_REGISTER_FLOAT_PROPERTY(NodeType, PropertyName, MemberVar) \
        metadata.AddProperty(BTNodePropertyDescriptor( \
            PropertyName, \
            PropertyType::Float, \
            [](BTNode* node) { \
                auto* n = static_cast<NodeType*>(node); \
                return std::to_string(n->MemberVar); \
            }, \
            [](BTNode* node, const std::string& value) { \
                auto* n = static_cast<NodeType*>(node); \
                n->MemberVar = std::stof(value); \
            } \
        ))

    // Register an int property
    #define BT_REGISTER_INT_PROPERTY(NodeType, PropertyName, MemberVar) \
        metadata.AddProperty(BTNodePropertyDescriptor( \
            PropertyName, \
            PropertyType::Int, \
            [](BTNode* node) { \
                auto* n = static_cast<NodeType*>(node); \
                return std::to_string(n->MemberVar); \
            }, \
            [](BTNode* node, const std::string& value) { \
                auto* n = static_cast<NodeType*>(node); \
                n->MemberVar = std::stoi(value); \
            } \
        ))

    // Register a bool property
    #define BT_REGISTER_BOOL_PROPERTY(NodeType, PropertyName, MemberVar) \
        metadata.AddProperty(BTNodePropertyDescriptor( \
            PropertyName, \
            PropertyType::Bool, \
            [](BTNode* node) { \
                auto* n = static_cast<NodeType*>(node); \
                return n->MemberVar ? "true" : "false"; \
            }, \
            [](BTNode* node, const std::string& value) { \
                auto* n = static_cast<NodeType*>(node); \
                n->MemberVar = (value == "true" || value == "1"); \
            } \
        ))

    // Register a string property
    #define BT_REGISTER_STRING_PROPERTY(NodeType, PropertyName, MemberVar) \
        metadata.AddProperty(BTNodePropertyDescriptor( \
            PropertyName, \
            PropertyType::String, \
            [](BTNode* node) { \
                auto* n = static_cast<NodeType*>(node); \
                return n->MemberVar; \
            }, \
            [](BTNode* node, const std::string& value) { \
                auto* n = static_cast<NodeType*>(node); \
                n->MemberVar = value; \
            } \
        ))

    // === PROPERTY MACROS ========================================================

    // Enum property (uses conversion helpers)
    #define BT_REGISTER_ENUM_PROPERTY(NodeType, PropertyName, MemberVar, EnumToStr, StrToEnum) \
        metadata.AddProperty(BTNodePropertyDescriptor{ \
            PropertyName, \
            PropertyType::String, \
            [](BTNode* node) { \
                auto* n = static_cast<NodeType*>(node); \
                return EnumToStr(n->MemberVar); \
            }, \
            [](BTNode* node, const std::string& value) { \
                auto* n = static_cast<NodeType*>(node); \
                n->MemberVar = StrToEnum(value); \
            } \
        })

} // namespace Engine

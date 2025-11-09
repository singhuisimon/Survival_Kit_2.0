/**
 * @file BTNodeRegistry.h
 * @brief Registry for behaviour tree node types
 * @author Amanda Leow Boon Suan (80%), Rio Shannon Yvon Leonardo (20%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "BTNode.h"
#include "BTNodeMetadata.h"
#include "BTCompositeNodes.h"
#include "BTDecoratorNodes.h"
#include "BTLeafNodes.h"
#include "../Utility/Logger.h"

#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

namespace Engine {

    inline std::string PolicyToString(BTParallel::Policy policy) {
        switch (policy) {
        case BTParallel::Policy::RequireOne: return "RequireOne";
        case BTParallel::Policy::RequireAll: return "RequireAll";
        default: return "Unknown";
        }
    }

    inline BTParallel::Policy StringToPolicy(const std::string& s) {
        if (s == "RequireOne") return BTParallel::Policy::RequireOne;
        if (s == "RequireAll") return BTParallel::Policy::RequireAll;
        return BTParallel::Policy::RequireOne;  // default fallback
    }

    inline std::string ComparisonToString(BTCheckHealth::Comparison comparison) {
        switch (comparison) {
        case BTCheckHealth::Comparison::Greater: return "Greater";
        case BTCheckHealth::Comparison::Less: return "Less";
        case BTCheckHealth::Comparison::Equal: return "Equal";
        case BTCheckHealth::Comparison::GreaterOrEqual: return "GreaterOrEqual";
        case BTCheckHealth::Comparison::LessOrEqual: return "LessOrEqual";
        default: return "Unknown";
        }
    }

    inline BTCheckHealth::Comparison StringToComparison(const std::string& s) {
        if (s == "Greater") return BTCheckHealth::Comparison::Greater;
        if (s == "Less") return BTCheckHealth::Comparison::Less;
        if (s == "Equal") return BTCheckHealth::Comparison::Equal;
        if (s == "GreaterOrEqual") return BTCheckHealth::Comparison::GreaterOrEqual;
        if (s == "LessOrEqual") return BTCheckHealth::Comparison::LessOrEqual;
        return BTCheckHealth::Comparison::Greater;  // default fallback
    }


    /**
     * @brief Factory function type for creating nodes
     */
    using BTNodeFactory = std::function<std::shared_ptr<BTNode>()>;

    /**
     * @brief Metadata for a behaviour tree node type
     */
    struct BTNodeTypeMetadata {
        std::string TypeName;
        std::string Category;           ///< For editor organization (Composite, Decorator, Action, etc.)
        std::string Description;
        BTNodeFactory Factory;
        bool CanHaveChildren;
        std::shared_ptr<BTNodeMetadata> PropertyMetadata;  ///< Property reflection data

        BTNodeTypeMetadata() = default;

        BTNodeTypeMetadata(const std::string& typeName,
            const std::string& category,
            const std::string& description,
            BTNodeFactory factory,
            bool canHaveChildren = false)
            : TypeName(typeName)
            , Category(category)
            , Description(description)
            , Factory(factory)
            , CanHaveChildren(canHaveChildren)
            , PropertyMetadata(std::make_shared<BTNodeMetadata>(typeName, category)) {
        }
    };

    /**
     * @brief Singleton registry for all behaviour tree node types
     */
    class BTNodeRegistry {
    public:
        static BTNodeRegistry& Get() {
            static BTNodeRegistry instance;
            return instance;
        }

        BTNodeRegistry(const BTNodeRegistry&) = delete;
        BTNodeRegistry& operator=(const BTNodeRegistry&) = delete;

        /**
         * @brief Register a node type
         * @param setupProperties Optional callback to register node properties
         */
        template<typename T>
        void RegisterNodeType(
            const std::string& category,
            const std::string& description,
            std::function<void(BTNodeMetadata&)> setupProperties = nullptr);

        /**
         * @brief Create a node instance by type name
         */
        std::shared_ptr<BTNode> CreateNode(const std::string& typeName) const;

        /**
         * @brief Get metadata for a node type
         */
        const BTNodeTypeMetadata* GetMetadata(const std::string& typeName) const;

        /**
         * @brief Get all registered node types
         */
        const std::unordered_map<std::string, BTNodeTypeMetadata>& GetAllNodeTypes() const;

        /**
         * @brief Get all node types in a specific category
         */
        std::vector<std::string> GetNodeTypesByCategory(const std::string& category) const;

        /**
         * @brief Check if a node type is registered
         */
        bool IsNodeTypeRegistered(const std::string& typeName) const;

        /**
         * @brief Clear all registered types
         */
        void Clear();

        /**
         * @brief Register all built-in node types
         */
        static void RegisterBuiltInNodes();

    private:
        BTNodeRegistry() = default;
        ~BTNodeRegistry() = default;

        std::unordered_map<std::string, BTNodeTypeMetadata> m_NodeTypes;
    };

} // namespace Engine

/**
 * @file BTNode.h
 * @brief Base class for all behaviour tree nodes with stack-based execution
 * @author AI System Team
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "../Asset/ResourceTypes.h"
#include "../Serialization/Property.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Engine {

    // Forward declarations
    class Entity;
    class Scene;

    /**
     * @brief Execution status of a behaviour tree node
     */
    enum class BTStatus {
        Success,    ///< Node completed successfully
        Failure,    ///< Node failed
        Running     ///< Node is still executing
    };

    /**
     * @brief Execution context passed down the tree
     */
    struct BTContext {
        Entity* Entity = nullptr;           ///< The entity this tree is running on
        Scene* Scene = nullptr;              ///< The scene the entity belongs to
        float DeltaTime = 0.0f;             ///< Time since last frame
        
        // Blackboard - shared data storage for the tree
        std::unordered_map<std::string, std::string> Blackboard;
    };

    /**
     * @brief Base class for all behaviour tree nodes
     * @details Uses stack-based execution instead of recursion
     */
    class BTNode {
    public:
        virtual ~BTNode() = default;

        /**
         * @brief Get the type name of this node (for serialization/registry)
         */
        virtual const char* GetTypeName() const = 0;

        /**
         * @brief Execute this node with stack-based approach
         * @param context Execution context
         * @return Node execution status
         */
        virtual BTStatus Execute(BTContext& context) = 0;

        /**
         * @brief Called when node is first entered (optional override)
         */
        virtual void OnEnter(BTContext& context) { (void)context; }

        /**
         * @brief Called when node exits (optional override)
         */
        virtual void OnExit(BTContext& context) { (void)context; }

        /**
         * @brief Reset node state (optional override)
         */
        virtual void Reset() {}

        /**
         * @brief Get node's unique identifier
         */
        xresource::instance_guid GetGUID() const { return m_GUID; }

        /**
         * @brief Set node's unique identifier
         */
        void SetGUID(xresource::instance_guid guid) { m_GUID = guid; }

        /**
         * @brief Get node name (for editor display)
         */
        const std::string& GetName() const { return m_Name; }

        /**
         * @brief Set node name
         */
        void SetName(const std::string& name) { m_Name = name; }

        /**
         * @brief Get child nodes (for composite nodes)
         */
        virtual std::vector<std::shared_ptr<BTNode>>& GetChildren() {
            static std::vector<std::shared_ptr<BTNode>> empty;
            return empty;
        }

        /**
         * @brief Get child nodes (const)
         */
        virtual const std::vector<std::shared_ptr<BTNode>>& GetChildren() const {
            static std::vector<std::shared_ptr<BTNode>> empty;
            return empty;
        }

        /**
         * @brief Check if this node can have children
         */
        virtual bool CanHaveChildren() const { return false; }

        /**
         * @brief Add a child node (for composite nodes)
         */
        virtual void AddChild(std::shared_ptr<BTNode> child) {
            (void)child;
            // Override in composite nodes
        }

        /**
         * @brief Remove a child node by index
         */
        virtual void RemoveChild(size_t index) {
            (void)index;
            // Override in composite nodes
        }

        /**
         * @brief Get serializable properties for this node
         * @details Used for editor and serialization
         * @note This is kept for simple string-based serialization
         */
        virtual void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
            (void)properties;
            // Override in derived classes to expose parameters
        }

        /**
         * @brief Set a property value from string
         * @note This is kept for simple string-based serialization
         */
        virtual void SetProperty(const std::string& name, const std::string& value) {
            (void)name;
            (void)value;
            // Override in derived classes
        }

        /**
         * @brief Get property metadata for reflection system
         * @details Override to expose typed properties to the editor
         */
        virtual void GetPropertyMetadata(std::vector<std::tuple<std::string, PropertyType, std::function<std::string()>, std::function<void(const std::string&)>>>& properties) {
            (void)properties;
            // Override in derived classes for full property support
        }

    protected:
        BTNode() : m_GUID(xresource::instance_guid::GenerateGUIDCopy()) {}

        xresource::instance_guid m_GUID;
        std::string m_Name = "Node";
    };

    /**
     * @brief Stack frame for stack-based execution
     */
    struct BTStackFrame {
        std::shared_ptr<BTNode> Node;
        size_t ChildIndex = 0;          ///< Current child being processed
        BTStatus LastChildStatus = BTStatus::Success;
        bool HasEntered = false;        ///< Whether OnEnter was called

        BTStackFrame(std::shared_ptr<BTNode> node)
            : Node(node) {}
    };

} // namespace Engine

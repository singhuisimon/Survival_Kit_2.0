/**
 * @file PrefabComponent.h
 * @brief Prefab component - tracks prefab instances and their overrides
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "../Asset/ResourceTypes.h"
#include <vector>
#include <string>

namespace Engine {

    /**
     * @brief Prefab component - tracks prefab instances and their overrides
     * @note This is an invisible component that marks an entity as a prefab instance
     * @details Stores information about which prefab this entity is an instance of,
     *          and tracks any modifications (overrides, additions, deletions) made to
     *          the instance that differ from the original prefab.
     */
    struct PrefabComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        /// Reference to the prefab resource this entity is an instance of
        xresource::instance_guid PrefabGUID;

        /// Track added components (components not in original prefab)
        std::vector<xresource::instance_guid> AddedComponents;

        /// Track deleted components (components removed from prefab)
        std::vector<xresource::instance_guid> DeletedComponents;

        /**
         * @brief Represents a single overridden property in a prefab instance
         */
        struct OverriddenProperty {
            xresource::instance_guid ComponentGUID;  ///< Which component contains this property
            std::string PropertyPath;                 ///< Which property (e.g., "Position.x")
            std::string Value;                        ///< Serialized value of the override
        };

        /// Track overridden properties (properties modified from prefab defaults)
        std::vector<OverriddenProperty> OverriddenProperties;

        /**
         * @brief Default constructor - creates an invalid/unlinked prefab component
         */
        PrefabComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , PrefabGUID(xresource::instance_guid{}) {
        }

        /**
         * @brief Constructor with prefab GUID - links this instance to a specific prefab
         * @param prefabGuid GUID of the prefab this entity is an instance of
         */
        explicit PrefabComponent(xresource::instance_guid prefabGuid)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , PrefabGUID(prefabGuid) {
        }

        /**
         * @brief Check if this component is linked to a valid prefab
         * @return True if PrefabGUID is valid (non-zero)
         */
        bool IsValid() const {
            return PrefabGUID.m_Value != 0;
        }

        /**
         * @brief Check if this instance has any local modifications
         * @return True if there are any overrides, additions, or deletions
         */
        bool HasModifications() const {
            return !OverriddenProperties.empty() ||
                !AddedComponents.empty() ||
                !DeletedComponents.empty();
        }

        /**
         * @brief Clear all local modifications (reset to prefab defaults)
         */
        void ClearModifications() {
            OverriddenProperties.clear();
            AddedComponents.clear();
            DeletedComponents.clear();
        }

        /**
         * @brief Add a property override
         * @param componentGuid GUID of the component containing the property
         * @param propertyPath Path to the property (e.g., "Position.x")
         * @param value Serialized value of the override
         */
        void AddPropertyOverride(xresource::instance_guid componentGuid,
            const std::string& propertyPath,
            const std::string& value) {
            // Check if this property is already overridden
            for (auto& override : OverriddenProperties) {
                if (override.ComponentGUID.m_Value == componentGuid.m_Value &&
                    override.PropertyPath == propertyPath) {
                    // Update existing override
                    override.Value = value;
                    return;
                }
            }

            // Add new override
            OverriddenProperties.push_back({ componentGuid, propertyPath, value });
        }

        /**
         * @brief Remove a property override
         * @param componentGuid GUID of the component containing the property
         * @param propertyPath Path to the property
         * @return True if an override was removed
         */
        bool RemovePropertyOverride(xresource::instance_guid componentGuid,
            const std::string& propertyPath) {
            for (auto it = OverriddenProperties.begin(); it != OverriddenProperties.end(); ++it) {
                if (it->ComponentGUID.m_Value == componentGuid.m_Value &&
                    it->PropertyPath == propertyPath) {
                    OverriddenProperties.erase(it);
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Mark a component as added (not in original prefab)
         * @param componentGuid GUID of the added component
         */
        void MarkComponentAdded(xresource::instance_guid componentGuid) {
            // Check if already marked
            for (const auto& guid : AddedComponents) {
                if (guid.m_Value == componentGuid.m_Value) {
                    return;
                }
            }
            AddedComponents.push_back(componentGuid);
        }

        /**
         * @brief Mark a component as deleted (removed from prefab)
         * @param componentGuid GUID of the deleted component
         */
        void MarkComponentDeleted(xresource::instance_guid componentGuid) {
            // Check if already marked
            for (const auto& guid : DeletedComponents) {
                if (guid.m_Value == componentGuid.m_Value) {
                    return;
                }
            }
            DeletedComponents.push_back(componentGuid);
        }

        /**
         * @brief Check if a component was added to this instance
         * @param componentGuid GUID of the component to check
         * @return True if the component is marked as added
         */
        bool IsComponentAdded(xresource::instance_guid componentGuid) const {
            for (const auto& guid : AddedComponents) {
                if (guid.m_Value == componentGuid.m_Value) {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Check if a component was deleted from this instance
         * @param componentGuid GUID of the component to check
         * @return True if the component is marked as deleted
         */
        bool IsComponentDeleted(xresource::instance_guid componentGuid) const {
            for (const auto& guid : DeletedComponents) {
                if (guid.m_Value == componentGuid.m_Value) {
                    return true;
                }
            }
            return false;
        }
    };

} // namespace Engine
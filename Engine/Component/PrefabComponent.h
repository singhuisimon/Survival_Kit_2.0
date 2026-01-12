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
#include "../Serialization/ComponentSerializer.h"
#include <vector>
#include <string>
#include <algorithm>

namespace Engine {
	struct ComponentOverride {
		ComponentTypeID componentType;
		bool isAddedComponent;      // Component added to instance
		bool isRemovedComponent;    // Component removed from instance

		std::string originalComponentJSON;  // Full component state from prefab
		std::string currentComponentJSON;   // Current component state (for comparison)

		// List of property names that were modified (for UI display only)
		std::vector<std::string> modifiedPropertyNames;

		ComponentOverride()
			: componentType(ComponentTypeID::None)
			, isAddedComponent(false)
			, isRemovedComponent(false) {
		}

		bool HasOverrides() const {
			return !modifiedPropertyNames.empty() || isAddedComponent || isRemovedComponent;
		}
	};

	struct PrefabComponent
	{
		static constexpr ComponentTypeID TypeID = ComponentTypeID::Prefab;
		static constexpr const char* TypeName = "PrefabComponent";

		xresource::instance_guid ComponentGUID;
		xresource::instance_guid PrefabAssetGuid;
		std::vector<ComponentOverride> componentOverrides;

		bool isPrefabRoot;
		bool isNestedPrefab;
		xresource::instance_guid parentPrefabGuid;
		std::vector<u32> childEntityIDs;

		std::string prefabName;
		u32 prefabVersion;

		PrefabComponent()
			: ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())  
			, PrefabAssetGuid(0)
			, isPrefabRoot(true)
			, isNestedPrefab(false)
			, parentPrefabGuid(0)
			, prefabVersion(1)
		{
		}

		// Constructor with prefab asset GUID
		explicit PrefabComponent(xresource::instance_guid assetGuid,
			const std::string& name = "",
			bool root = true)
			: ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())  
			, PrefabAssetGuid(assetGuid)
			, isPrefabRoot(root)
			, isNestedPrefab(false)
			, parentPrefabGuid(0)
			, prefabName(name)
			, prefabVersion(1)
		{
		}

		bool HasOverrides() const {
			for (const auto& override : componentOverrides) {
				if (override.HasOverrides()) return true;
			}
			return false;
		}

		bool IsComponentOverridden(ComponentTypeID type) const {
			for (const auto& override : componentOverrides) {
				if (override.componentType == type && override.HasOverrides()) {
					return true;
				}
			}
			return false;
		}

		std::vector<std::string> GetModifiedProperties(ComponentTypeID type) const {
			for (const auto& override : componentOverrides) {
				if (override.componentType == type) {
					return override.modifiedPropertyNames;
				}
			}
			return {};
		}

		void MarkComponentModified(ComponentTypeID type, const std::string& propertyName = "") {
			ComponentOverride* compOverride = nullptr;

			// Find existing override for this component type
			for (auto& override : componentOverrides) {
				if (override.componentType == type) {
					compOverride = &override;
					break;
				}
			}

			// If not found, create new override
			if (!compOverride) {
				componentOverrides.emplace_back();
				compOverride = &componentOverrides.back();
				compOverride->componentType = type;
			}

			//// FIXED: Always add the component type itself as a property if no name given
			//if (propertyName.empty()) {
			//	// Add a generic marker to show component was modified
			//	std::string marker = "ComponentModified";
			//	auto it = std::find(compOverride->modifiedPropertyNames.begin(),
			//		compOverride->modifiedPropertyNames.end(),
			//		marker);
			//	if (it == compOverride->modifiedPropertyNames.end()) {
			//		compOverride->modifiedPropertyNames.push_back(marker);
			//	}
			//}
			//else {
			//	// Add specific property name if provided
			//	auto it = std::find(compOverride->modifiedPropertyNames.begin(),
			//		compOverride->modifiedPropertyNames.end(),
			//		propertyName);
			//	if (it == compOverride->modifiedPropertyNames.end()) {
			//		compOverride->modifiedPropertyNames.push_back(propertyName);
			//	}
			//}
			if (!propertyName.empty()) {
				auto it = std::find(compOverride->modifiedPropertyNames.begin(),
					compOverride->modifiedPropertyNames.end(),
					propertyName);
				if (it == compOverride->modifiedPropertyNames.end()) {
					compOverride->modifiedPropertyNames.push_back(propertyName);
				}
			}
		}

		void StoreOriginalComponent(ComponentTypeID type, const std::string& componentJSON) {
			ComponentOverride* compOverride = nullptr;
			for (auto& override : componentOverrides) {
				if (override.componentType == type) {
					compOverride = &override;
					break;
				}
			}

			if (!compOverride) {
				componentOverrides.emplace_back();
				compOverride = &componentOverrides.back();
				compOverride->componentType = type;
			}

			compOverride->originalComponentJSON = componentJSON;
		}

		std::string GetOriginalComponentJSON(ComponentTypeID type) const {
			for (const auto& override : componentOverrides) {
				if (override.componentType == type) {
					return override.originalComponentJSON;
				}
			}
			return "";
		}

		void ClearAllOverrides() {
			componentOverrides.clear();
		}

		void ClearComponentOverride(ComponentTypeID type) {
			componentOverrides.erase(
				std::remove_if(componentOverrides.begin(), componentOverrides.end(),
					[type](const ComponentOverride& o) {
						return o.componentType == type;
					}),
				componentOverrides.end()
			);
		}

		void MarkComponentRemoved(ComponentTypeID componentType, const std::string& originalComponentJSON = "") {
			ComponentOverride* compOverride = nullptr;

			for (auto& override : componentOverrides) {
				if (override.componentType == componentType) {
					compOverride = &override;
					break;
				}
			}

			if (!compOverride) {
				componentOverrides.emplace_back();
				compOverride = &componentOverrides.back();
				compOverride->componentType = componentType;
			}

			// Store the original component state BEFORE removal
			if (!originalComponentJSON.empty()) {
				compOverride->originalComponentJSON = originalComponentJSON;
			}

			// Mark as removed
			compOverride->isRemovedComponent = true;
			compOverride->isAddedComponent = false;
			compOverride->modifiedPropertyNames.clear();

			LOG_DEBUG("Marked component as removed: ",
				ComponentSerializer::GetComponentTypeName(componentType));
		}

		// ADD: MarkComponentAdded method
		void MarkComponentAdded(ComponentTypeID componentType, const std::string& componentJSON = "") {
			ComponentOverride* compOverride = nullptr;

			for (auto& override : componentOverrides) {
				if (override.componentType == componentType) {
					compOverride = &override;
					break;
				}
			}

			if (!compOverride) {
				componentOverrides.emplace_back();
				compOverride = &componentOverrides.back();
				compOverride->componentType = componentType;
			}

			// Mark as added
			compOverride->isAddedComponent = true;
			compOverride->isRemovedComponent = false;
			compOverride->modifiedPropertyNames.clear();

			// Store the added component's state for potential revert
			if (!componentJSON.empty()) {
				compOverride->currentComponentJSON = componentJSON;
			}

			LOG_DEBUG("Marked component as added: ",
				ComponentSerializer::GetComponentTypeName(componentType));
		}

		// ADD: Helper methods
		bool IsComponentRemoved(ComponentTypeID type) const {
			for (const auto& override : componentOverrides) {
				if (override.componentType == type && override.isRemovedComponent) {
					return true;
				}
			}
			return false;
		}

		bool IsComponentAdded(ComponentTypeID type) const {
			for (const auto& override : componentOverrides) {
				if (override.componentType == type && override.isAddedComponent) {
					return true;
				}
			}
			return false;
		}

		std::vector<ComponentTypeID> GetRemovedComponents() const {
			std::vector<ComponentTypeID> removed;
			for (const auto& override : componentOverrides) {
				if (override.isRemovedComponent) {
					removed.push_back(override.componentType);
				}
			}
			return removed;
		}

		std::vector<ComponentTypeID> GetAddedComponents() const {
			std::vector<ComponentTypeID> added;
			for (const auto& override : componentOverrides) {
				if (override.isAddedComponent) {
					added.push_back(override.componentType);
				}
			}
			return added;
		}

		// Clear removal flag (for revert functionality)
		void ClearComponentRemoval(ComponentTypeID type) {
			for (auto& override : componentOverrides) {
				if (override.componentType == type) {
					override.isRemovedComponent = false;
					override.modifiedPropertyNames.clear();
					break;
				}
			}
		}

		// ADD: Clear addition flag
		void ClearComponentAddition(ComponentTypeID type) {
			for (auto& override : componentOverrides) {
				if (override.componentType == type) {
					override.isAddedComponent = false;
					override.modifiedPropertyNames.clear();
					override.currentComponentJSON.clear();
					break;
				}
			}
		}
	};
} // namespace Engine
/**
 * @file BehaviourTreePrefab.h
 * @brief Integration with prefab system for behaviour trees
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "../BehaviourTree/BehaviourTree.h"
#include "../Serialization/BehaviourTreeSerializer.h"
#include "BehaviourTreePrefab.h"
#include "Prefab.h"
#include "PrefabRegistry.h"
#include "../Utility/Logger.h"
#include <memory>

namespace Engine {

    /**
     * @brief Create a prefab from a behaviour tree
     * @param tree The behaviour tree to convert to prefab
     * @param name Name for the prefab
     * @return Shared pointer to the created prefab
     */
    std::shared_ptr<Prefab> BehaviourTreePrefab::CreatePrefab(const BehaviourTree& tree, const std::string& name) {
        // Serialize the tree to JSON string
        std::string treeData = BehaviourTreeSerializer::SerializeToString(tree);

        // Create a prefab with the serialized data
        auto prefab = std::make_shared<Prefab>(PrefabType::Entity);
        prefab->SetName(name);
        prefab->SetEntityData(treeData); // Reusing entity data field for tree data

        LOG_INFO("BehaviourTreePrefab: Created prefab '", name, "' from tree '", tree.GetName(), "'");
        return prefab;
    }

    /**
     * @brief Load a behaviour tree from a prefab
     * @param prefabGUID GUID of the prefab
     * @return Shared pointer to the loaded behaviour tree, or nullptr if failed
     */
    std::shared_ptr<BehaviourTree> BehaviourTreePrefab::LoadFromPrefab(xresource::instance_guid prefabGUID) {
        auto prefab = PrefabRegistry::Get().GetPrefab(prefabGUID);
        if (!prefab) {
            LOG_ERROR("BehaviourTreePrefab: Prefab not found (GUID: 0x",
                std::hex, prefabGUID.m_Value, std::dec, ")");
            return nullptr;
        }

        // Deserialize tree from prefab data
        std::string treeData = prefab->GetEntityData();
        auto tree = BehaviourTreeSerializer::DeserializeFromString(treeData);

        if (tree) {
            LOG_INFO("BehaviourTreePrefab: Loaded tree '", tree->GetName(),
                "' from prefab '", prefab->GetName(), "'");
        }

        return tree;
    }

    /**
     * @brief Save a behaviour tree as a prefab and register it
     * @param tree The behaviour tree to save
     * @param name Name for the prefab
     * @return GUID of the created prefab
     */
    xresource::instance_guid BehaviourTreePrefab::SaveAsPrefab(const BehaviourTree& tree, const std::string& name) {
        auto prefab = CreatePrefab(tree, name);
        if (!prefab) {
            return xresource::instance_guid{};
        }

        // Register the prefab
        PrefabRegistry::Get().RegisterPrefab(prefab);

        LOG_INFO("BehaviourTreePrefab: Registered prefab '", name, "' (GUID: 0x",
            std::hex, prefab->GetGUID().m_Value, std::dec, ")");

        return prefab->GetGUID();
    }

    /**
     * @brief Create a runtime instance of a tree from a prefab
     * @param prefabGUID GUID of the prefab
     * @return New tree instance (independent copy)
     */
    std::shared_ptr<BehaviourTree> BehaviourTreePrefab::Instantiate(xresource::instance_guid prefabGUID) {
        auto tree = LoadFromPrefab(prefabGUID);
        if (!tree) {
            return nullptr;
        }

        // Generate new GUID for the instance (make it unique)
        tree->SetGUID(xresource::instance_guid::GenerateGUIDCopy());

        LOG_TRACE("BehaviourTreePrefab: Instantiated tree from prefab (GUID: 0x",
            std::hex, prefabGUID.m_Value, std::dec, ")");

        return tree;
    }

    /**
     * @brief Update a prefab with a modified tree
     * @param prefabGUID GUID of the prefab to update
     * @param tree The updated tree
     * @return True if successful
     */
    bool BehaviourTreePrefab::UpdatePrefab(xresource::instance_guid prefabGUID, const BehaviourTree& tree) {
        auto prefab = PrefabRegistry::Get().GetPrefab(prefabGUID);
        if (!prefab) {
            LOG_ERROR("BehaviourTreePrefab: Prefab not found for update (GUID: 0x",
                std::hex, prefabGUID.m_Value, std::dec, ")");
            return false;
        }

        // Serialize and update prefab data
        std::string treeData = BehaviourTreeSerializer::SerializeToString(tree);
        prefab->SetEntityData(treeData);

        LOG_INFO("BehaviourTreePrefab: Updated prefab '", prefab->GetName(), "'");
        return true;
    }

    /**
     * @brief Save a behaviour tree prefab to file
     * @param prefabGUID GUID of the prefab
     * @param filepath Path to save to
     * @return True if successful
     */
    bool BehaviourTreePrefab::SavePrefabToFile(xresource::instance_guid prefabGUID, const std::string& filepath) {
        auto tree = LoadFromPrefab(prefabGUID);
        if (!tree) {
            return false;
        }

        return BehaviourTreeSerializer::SerializeToFile(*tree, filepath);
    }

    /**
     * @brief Load a behaviour tree prefab from file and register it
     * @param filepath Path to load from
     * @param name Name for the prefab
     * @return GUID of the registered prefab
     */
    xresource::instance_guid BehaviourTreePrefab::LoadPrefabFromFile(const std::string& filepath, const std::string& name) {
        auto tree = BehaviourTreeSerializer::DeserializeFromFile(filepath);
        if (!tree) {
            return xresource::instance_guid{};
        }

        return SaveAsPrefab(*tree, name);
    }

} // namespace Engine

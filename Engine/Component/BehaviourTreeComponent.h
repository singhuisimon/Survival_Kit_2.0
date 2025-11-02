/**
 * @file BehaviourTreeComponent.h
 * @brief Component that attaches a behavior tree to an entity
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "../BehaviourTree/BehaviourTree.h"
#include "../Asset/ResourceTypes.h"

namespace Engine {

    /**
     * @brief Component that holds a behavior tree instance
     * @details Attach this to an entity to give it AI behavior
     */
    struct BehaviourTreeComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        /// Reference to the behavior tree prefab/asset
        //xresource::instance_guid TreeAssetGUID;
        std::string TreeAssetPath;

        /// Runtime instance of the behavior tree
        std::shared_ptr<BehaviourTree> TreeInstance;

        /// Whether the tree should execute every frame
        bool Active = true;

        /// Whether to reset the tree when it completes
        bool ResetOnComplete = true;

        /// Last execution status (for debugging)
        BTStatus LastStatus = BTStatus::Success;

        BTBlackboard PersistantBlackboard;

        /**
         * @brief Default constructor
         */
        BehaviourTreeComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            //, TreeAssetGUID(xresource::instance_guid{})
            , TreeAssetPath("")
            , TreeInstance(nullptr) {
        }

        /**
         * @brief Constructor with tree asset
         */
        explicit BehaviourTreeComponent(const std::string& treeAssetPath)//xresource::instance_guid treeAssetGuid)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            //, TreeAssetGUID(treeAssetGuid)
            , TreeAssetPath(treeAssetPath)
            , TreeInstance(nullptr) {
        }

        /**
         * @brief Constructor with tree instance
         */
        explicit BehaviourTreeComponent(std::shared_ptr<BehaviourTree> tree)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            //, TreeAssetGUID(xresource::instance_guid{})
            , TreeAssetPath("")
            , TreeInstance(tree) {
        }

        /**
         * @brief Check if tree is valid and ready to execute
         */
        bool IsValid() const {
            return TreeInstance != nullptr && TreeInstance->GetRootNode() != nullptr;
        }

        /**
         * @brief Reset the tree to initial state
         */
        void Reset() {
            if (TreeInstance) {
                TreeInstance->Reset();
            }
            LastStatus = BTStatus::Success;
        }
    };

} // namespace Engine

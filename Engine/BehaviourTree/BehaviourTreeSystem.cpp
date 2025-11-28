/**
 * @file BehaviourTreeSystem.cpp
 * @brief Definition of BehaviourTreeSystem class for managing behavior tree execution in the ECS.
 * @author Amanda Leow Boon Suan (100%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "../ECS/System.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include "../Component/BehaviourTreeComponent.h"
#include "../Component/TransformComponent.h"
#include "../Prefab/BehaviourTreePrefab.h"
#include "BehaviourTreeSystem.h"
#include "../Utility/Logger.h"

namespace Engine {


    void BehaviourTreeSystem::OnInit(Scene* scene) {
        LOG_INFO("BehaviourTreeSystem: Initialized");
        //(void)scene;

        if (scene) {
            LoadBehaviourTrees(scene);
        }
    }

    void BehaviourTreeSystem::LoadBehaviourTrees(Scene* scene) {
        if (!scene) return;

        auto& registry = scene->GetRegistry();
        auto view = registry.view<BehaviourTreeComponent>();

        int loadedCount = 0;
        int failedCount = 0;

        for (auto entity : view) {
            Entity ent(entity, &registry);

            if (!ent.HasComponent<BehaviourTreeComponent>()) {
                continue;
            }

            auto& btComp = ent.GetComponent<BehaviourTreeComponent>();

            // Load tree from file if needed
            if (btComp.TreeInstance == nullptr && !btComp.TreeAssetPath.empty()) {
                btComp.TreeInstance = Engine::BehaviourTreeSerializer::DeserializeFromFile(
                    btComp.TreeAssetPath
                );

                if (btComp.TreeInstance) {
                    //LOG_INFO("BehaviourTreeSystem: Loaded tree from file ", btComp.TreeAssetPath);
                    loadedCount++;
                }
                else {
                    LOG_WARNING("BehaviourTreeSystem: Failed to load tree from ", btComp.TreeAssetPath);
                    failedCount++;
                }
            }
        }

        if (loadedCount > 0 || failedCount > 0) {
            LOG_INFO("BehaviourTreeSystem: Loaded ", loadedCount, " trees, ", failedCount, " failed");
        }
    }

    void BehaviourTreeSystem::OnUpdate(Scene* scene, Timestep ts) {
        if (!scene) return;

        auto& registry = scene->GetRegistry();
        auto view = registry.view<BehaviourTreeComponent>();

        // CRITICAL: Collect entities first to avoid iterator invalidation
        std::vector<entt::entity> entitiesToProcess;
        entitiesToProcess.reserve(view.size());

        for (auto entity : view) {
            entitiesToProcess.push_back(entity);
        }

        // Now process each entity with validation
        for (auto entityHandle : entitiesToProcess) {
            // CRITICAL: Validate entity still exists
            if (!registry.valid(entityHandle)) {
                LOG_TRACE("BehaviourTreeSystem: Entity destroyed before processing");
                continue;
            }

            Entity ent(entityHandle, &registry);

            // Check if entity still has BehaviourTreeComponent
            if (!ent.HasComponent<BehaviourTreeComponent>()) {
                LOG_TRACE("BehaviourTreeSystem: Entity lost BehaviourTreeComponent");
                continue;
            }

            auto& btComp = ent.GetComponent<BehaviourTreeComponent>();

            // Load tree from file if needed
            if (btComp.TreeInstance == nullptr && !btComp.TreeAssetPath.empty()) {
                btComp.TreeInstance = Engine::BehaviourTreeSerializer::DeserializeFromFile(btComp.TreeAssetPath);
                //if (btComp.TreeInstance)
                //    //LOG_INFO("BehaviourTreeSystem: Loaded tree from file ", btComp.TreeAssetPath);
                //else
                //    LOG_WARNING("BehaviourTreeSystem: Failed to load tree from ", btComp.TreeAssetPath);
            }

            // Skip if not active or invalid
            if (!btComp.Active || !btComp.IsValid()) {
                continue;
            }

            // Setup execution context
            BTContext context;
            context.Entity = &ent;
            context.Scene = scene;
            context.DeltaTime = ts;
            context.Blackboard = btComp.PersistantBlackboard;

            // Execute the behaviour tree
            BTStatus status = btComp.TreeInstance->Execute(context);

            // CRITICAL: Validate entity still exists after execution
            if (!registry.valid(entityHandle)) {
                LOG_TRACE("BehaviourTreeSystem: Entity destroyed during tree execution");
                // Don't try to access btComp - it's gone!
                continue;
            }

            if (!ent.HasComponent<BehaviourTreeComponent>()) {
				LOG_TRACE("BehaviourTreeSystem: Entity lost BehaviourTreeComponent during execution");
				continue;
            }

            // Safe to update component - entity still exists
            auto& btCompAfterExecution = ent.GetComponent<BehaviourTreeComponent>();
            btCompAfterExecution.LastStatus = status;
            btCompAfterExecution.PersistantBlackboard = context.Blackboard;

            // Reset on completion if configured
            if (status != BTStatus::Running && btComp.ResetOnComplete) {
                btCompAfterExecution.TreeInstance->Reset();
            }
        }
    }

    void BehaviourTreeSystem::OnShutdown(Scene* scene) {
        LOG_INFO("BehaviourTreeSystem: Shutdown");
        (void)scene;
    }

} // namespace Engine

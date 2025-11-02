/**
 * @file BehaviourTreeSystem.h
 * @brief System that updates all behaviour trees in the scene
 * @author AI System Team
 * @date 2025
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
        (void)scene;
    }

    void BehaviourTreeSystem::OnUpdate(Scene* scene, Timestep ts) {
        if (!scene) return;

        auto& registry = scene->GetRegistry();
        auto view = registry.view<BehaviourTreeComponent>();

        for (auto entity : view) {
            Entity ent(entity, &registry);
            auto& btComp = ent.GetComponent<BehaviourTreeComponent>();

            if (btComp.TreeInstance == nullptr && !btComp.TreeAssetPath.empty()) {
                btComp.TreeInstance = Engine::BehaviourTreeSerializer::DeserializeFromFile(btComp.TreeAssetPath);
                if (btComp.TreeInstance)
                    LOG_INFO("BehaviourTreeSystem: Loaded tree from file ", btComp.TreeAssetPath);
                else
                    LOG_WARNING("BehaviourTreeSystem: Failed to load tree from ", btComp.TreeAssetPath);
            }


            // Skip if not active or invalid
            if (!btComp.Active || !btComp.IsValid()) {
                LOG_WARNING("NOT ACTIVE/VALID");
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
            btComp.LastStatus = status;

            btComp.PersistantBlackboard = context.Blackboard;

            // Reset on completion if configured
            if (status != BTStatus::Running && btComp.ResetOnComplete) {
                btComp.TreeInstance->Reset();
            }
        }
    }

    void BehaviourTreeSystem::OnShutdown(Scene* scene) {
        LOG_INFO("BehaviourTreeSystem: Shutdown");
        (void)scene;
    }

} // namespace Engine

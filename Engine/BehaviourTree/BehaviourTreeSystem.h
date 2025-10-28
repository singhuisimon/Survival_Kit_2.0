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
#include "../Utility/Logger.h"

namespace Engine {

    /**
     * @brief System that executes all active behaviour trees
     * @details Runs every frame and updates all entities with BehaviourTreeComponent
     */
    class BehaviourTreeSystem : public System {
    public:
        BehaviourTreeSystem() = default;

        void OnInit(Scene* scene) override {
            LOG_INFO("BehaviourTreeSystem: Initialized");
            (void)scene;
        }

        void OnUpdate(Scene* scene, Timestep ts) override {
            if (!scene) return;

            auto& registry = scene->GetRegistry();
            auto view = registry.view<BehaviourTreeComponent>();

            for (auto entity : view) {
                Entity ent(entity, &registry);
                auto& btComp = ent.GetComponent<BehaviourTreeComponent>();

                // Skip if not active or invalid
                if (!btComp.Active || !btComp.IsValid()) {
                    continue;
                }

                // Setup execution context
                BTContext context;
                context.Entity = &ent;
                context.Scene = scene;
                context.DeltaTime = ts;

                // Execute the behaviour tree
                BTStatus status = btComp.TreeInstance->Execute(context);
                btComp.LastStatus = status;

                // Reset on completion if configured
                if (status != BTStatus::Running && btComp.ResetOnComplete) {
                    btComp.TreeInstance->Reset();
                }
            }
        }

        void OnShutdown(Scene* scene) override {
            LOG_INFO("BehaviourTreeSystem: Shutdown");
            (void)scene;
        }

        int GetPriority() const override {
            return 60; // Run after physics (10-19) and transforms (30-49), before rendering
        }

        const char* GetName() const override {
            return "BehaviourTreeSystem";
        }
    };

} // namespace Engine

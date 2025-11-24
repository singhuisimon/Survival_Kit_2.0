/**
 * @file BehaviourTreeSystem.h
 * @brief System that updates all behaviour trees in the scene
 * @author Amanda Leow Boon Suan (90%), Rio Shannon Yvon Leonardo (10%)
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
#include "../Utility/Logger.h"

namespace Engine {

    /**
     * @brief System that executes all active behaviour trees
     * @details Runs every frame and updates all entities with BehaviourTreeComponent
     */
    class BehaviourTreeSystem : public System {
    public:
        BehaviourTreeSystem() = default;

        void OnInit(Scene* scene) override;

        void OnUpdate(Scene* scene, Timestep ts) override;

        void OnShutdown(Scene* scene) override;

        int GetPriority() const override {
            return 60; // Run after physics (10-19) and transforms (30-49), before rendering
        }

        const char* GetName() const override {
            return "BehaviourTreeSystem";
        }

        void LoadBehaviourTrees(Scene* scene);
    };

} // namespace Engine

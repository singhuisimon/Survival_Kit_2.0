#include "AISystem.h"
//#include "BehaviourTree/BehaviourTree.h"
//#include "BehaviourTree/BehaviourNode.h"
#include "Utility/Logger.h"
#include "ECS/Components.h"

namespace Engine {

    AISystem::AISystem(AIManager* aiManager)
        : m_AIManager(aiManager)
        , m_Initialized(false)
        , m_GlobalTickRate(0.0f)
        , m_DebugDrawAll(false) {
    }

    AISystem::~AISystem() {
        onShutdown(nullptr);
    }

    void AISystem::onInit(Scene* scene) {
        if (!scene || !m_AIManager) {
            LOG_ERROR("AISystem::OnInit - Invalid scene or AIManager");
            return;
        }

        if (!m_AIManager->IsInitialized()) {
            LOG_ERROR("AISystem::OnInit - AIManager not initialized");
            return;
        }

        auto& registry = scene->GetRegistry();

        // Register cleanup callback for when AI components are destroyed
        registry.on_destroy<AIComponent>().connect<&AISystem::OnAIComponentRemoved>(*this);

        m_Initialized = true;
        LOG_INFO("AISystem initialized successfully");
    }

    void AISystem::onUpdate(Scene* scene, Timestep ts) {
        if (!m_Initialized || !scene || !m_AIManager) {
            return;
        }

        float deltaTime = ts.GetSeconds();

        // Process all AI entities
        ProcessAIEntities(scene, deltaTime);
    }

    void AISystem::onShutdown(Scene* scene) {
        if (!m_Initialized) {
            return;
        }

        if (scene) {
            auto& registry = scene->GetRegistry();
            registry.on_destroy<AIComponent>().disconnect<&AISystem::OnAIComponentRemoved>(*this);
        }

        LOG_INFO("AISystem shutting down.");
        m_Initialized = false;
    }

    void AISystem::ProcessAIEntities(Scene* scene, float deltaTime) {
        auto& registry = scene->GetRegistry();
        auto view = registry.view<AIComponent>();

        for (auto entityHandle : view) {
            Entity entity(entityHandle, &registry);
            auto& ai = entity.GetComponent<AIComponent>();

            // Skip inactive AI
            if (!ai.Active) {
                continue;
            }

            // Ensure behaviour tree is loaded
            //if (!EnsureBehaviourTreeLoaded(entity, ai)) {
            //    continue;
            //}

            // Check if should tick this frame
            if (!ShouldTick(ai, deltaTime)) {
                continue;
            }

            // Get optional components
            TransformComponent* transform = entity.HasComponent<TransformComponent>()
                ? &entity.GetComponent<TransformComponent>()
                : nullptr;

            // Tick the behaviour tree
            //TickBehaviourTree(entity, ai, transform);
        }
    }

    //COMMENTED BECAUSE BEHAVIOUR TREE FILES NOT CODED YET

    //bool AISystem::EnsureBehaviourTreeLoaded(Entity entity, AIComponent& ai) {
    //    // Already has a tree
    //    if (ai.Tree != nullptr) {
    //        return true;
    //    }

    //    // No tree path specified
    //    if (ai.TreeAssetPath.empty()) {
    //        LOG_WARNING("AISystem - Entity ", (uint32_t)entity, " has no behaviour tree path");
    //        return false;
    //    }

    //    // Try to load/get the tree
    //    BehaviourTree* tree = m_AIManager->GetTree(ai.TreeAssetPath);
    //    if (!tree) {
    //        // Not in cache, try loading it
    //        tree = m_AIManager->LoadTree(ai.TreeAssetPath);
    //    }

    //    if (!tree) {
    //        LOG_ERROR("AISystem - Failed to load behaviour tree: ", ai.TreeAssetPath);
    //        return false;
    //    }

    //    // Assign tree to AI component
    //    ai.Tree = tree;

    //    // Initialize blackboard with common values
    //    InitializeBlackboard(entity, ai);

    //    LOG_INFO("AISystem - Loaded behaviour tree '", ai.TreeAssetPath, "' for entity ", (uint32_t)entity);
    //    return true;
    //}

    //void AISystem::TickBehaviourTree(Entity entity, AIComponent& ai, TransformComponent* transform) {
    //    if (!ai.Tree) {
    //        return;
    //    }

    //    // Update blackboard with current entity info
    //    ai.SetBlackboardValue("self", (entt::entity)entity);

    //    if (transform) {
    //        ai.SetBlackboardValue("position", transform->Position);
    //        ai.SetBlackboardValue("rotation", glm::vec3(transform->Rotation.x, transform->Rotation.y, transform->Rotation.z));
    //    }

    //    // Tick the root node
    //    NodeStatus status = ai.Tree->Tick(ai.Data);

    //    // Update AI state based on result
    //    switch (status) {
    //    case NodeStatus::SUCCESS:
    //        ai.CurrentState = "Success";
    //        LOG_TRACE("AISystem - Entity ", (uint32_t)entity, " behaviour tree: SUCCESS");
    //        break;

    //    case NodeStatus::FAILURE:
    //        ai.CurrentState = "Failure";
    //        LOG_TRACE("AISystem - Entity ", (uint32_t)entity, " behaviour tree: FAILURE");
    //        break;

    //    case NodeStatus::RUNNING:
    //        ai.CurrentState = "Running";
    //        // Tree is still executing
    //        break;

    //    case NodeStatus::IDLE:
    //        ai.CurrentState = "Idle";
    //        break;
    //    }

    //    // Debug drawing
    //    if (ai.DebugDraw || m_DebugDrawAll) {
    //        LOG_TRACE("AISystem - Entity ", (uint32_t)entity, " State: ", ai.CurrentState);
    //    }
    //}

    bool AISystem::ShouldTick(AIComponent& ai, float deltaTime) {
        // Determine tick rate (use component's rate if set, otherwise use global)
        float tickRate = (ai.TickRate > 0.0f) ? ai.TickRate : m_GlobalTickRate;

        // If tick rate is 0, tick every frame
        if (tickRate <= 0.0f) {
            return true;
        }

        // Accumulate time
        ai.TimeSinceLastTick += deltaTime;

        // Check if enough time has passed
        if (ai.TimeSinceLastTick >= tickRate) {
            ai.TimeSinceLastTick = 0.0f;
            return true;
        }

        return false;
    }

    void AISystem::OnAIComponentRemoved(entt::registry& registry, entt::entity entity) {
        Entity e(entity, &registry);

        if (!e.HasComponent<AIComponent>()) {
            return;
        }

        auto& ai = e.GetComponent<AIComponent>();

        LOG_INFO("AISystem - Cleanup for entity ", (uint32_t)entity, " (Tree: ", ai.TreeAssetPath, ")");

        // Note: We don't delete the tree itself - it's owned by AIManager
        // Just clear the pointer
        ai.Tree = nullptr;
        ai.CurrentNode = nullptr;
        ai.Data.clear();
    }

    void AISystem::InitializeBlackboard(Entity entity, AIComponent& ai) {
        // Set up common blackboard values
        //ai.SetBlackboardValue("self", (entt::entity)entity);
        //ai.SetBlackboardValue("isAlerted", false);
        //ai.SetBlackboardValue("target", entt::null);

        LOG_TRACE("AISystem - Initialized blackboard for entity ", (uint32_t)entity);
    }

}
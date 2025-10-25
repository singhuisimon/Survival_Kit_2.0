#include "AISystem.h"
//#include "BehaviourTree/BehaviourTree.h"
//#include "BehaviourTree/BehaviourNode.h"
#include "Utility/Logger.h"
#include "ECS/Components.h"

namespace Engine {

    AISystem::AISystem(BehaviourTreeSerializer* treeSerializer)
        : m_TreeSerializer(treeSerializer)
        , m_Initialized(false)
        , m_GlobalTickRate(0.0f)
        , m_DebugDrawAll(false) {

        if (!m_TreeSerializer) {
            LOG_ERROR("[AISystem] Constructor received null BehaviourTreeSerializer!");
        }
    }

    AISystem::~AISystem() {
        OnShutdown(nullptr);
    }

    void AISystem::OnInit(Scene* scene) {
        if (!scene || !m_TreeSerializer) {
            LOG_ERROR("AISystem::OnInit - Invalid scene or AIManager");
            return;
        }

        if (!m_TreeSerializer->IsInitialized()) {
            LOG_ERROR("AISystem::OnInit - AIManager not initialized");
            return;
        }

        auto& registry = scene->GetRegistry();

        // Register cleanup callback for when AI components are destroyed
        registry.on_destroy<AIComponent>().connect<&AISystem::OnAIComponentRemoved>(*this);

        m_Initialized = true;
        m_ActiveTreePaths.clear();
        LOG_INFO("AISystem initialized successfully");
    }

    void AISystem::OnUpdate(Scene* scene, Timestep ts) {
        if (!m_Initialized || !scene || !m_TreeSerializer) {
            return;
        }

        float deltaTime = ts.GetSeconds();

        // Process all AI entities
        ProcessAIEntities(scene, deltaTime);
    }

    void AISystem::OnShutdown(Scene* scene) {
        if (!m_Initialized) {
            return;
        }

        if (scene) {
            auto& registry = scene->GetRegistry();
            registry.on_destroy<AIComponent>().disconnect<&AISystem::OnAIComponentRemoved>(*this);
        }

        m_ActiveTreePaths.clear();

        LOG_INFO("AISystem shutting down.");
        m_Initialized = false;
    }

    bool AISystem::ReloadTreeForEntity(Entity entity) {
        if (!entity.HasComponent<AIComponent>()) {
            return false;
        }

        auto& ai = entity.GetComponent<AIComponent>();
        if (ai.TreeAssetPath.empty()) {
            LOG_WARNING("[AISystem] Entity AIComponent has no tree path");
            return false;
        }

        LOG_INFO("[AISystem] Reloading tree for entity ", (uint32_t)entity);

        // Mark dirty and reload
        ai.MarkTreeDirty();
        
        // Reload in serializer
        bool success = m_TreeSerializer->ReloadTree(ai.TreeAssetPath);

        if (success) {
            // Clear current tree reference - will be reloaded on next tick
            ai.Tree = nullptr;
            ai.CurrentNode = nullptr;
            LOG_INFO("[AISystem] Tree reload successful");
        }
        else {
            LOG_ERROR("[AISystem] Tree reload failed");
        }

        return success;
    }

    void AISystem::ReloadAllTrees() {
        LOG_INFO("[AISystem] Reloading all active trees (count: ", m_ActiveTreePaths.size(), ")");

        for (const auto& path : m_ActiveTreePaths) {
            m_TreeSerializer->ReloadTree(path);
        }

        LOG_INFO("[AISystem] All trees reloaded");
    }

    std::vector<std::string> AISystem::GetActiveTreePaths() const {
        return std::vector<std::string>(m_ActiveTreePaths.begin(), m_ActiveTreePaths.end());
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
            if (!EnsureBehaviourTreeLoaded(entity, ai)) {
                continue;
            }

            // Check if should tick this frame
            if (!ShouldTick(ai, deltaTime)) {
                continue;
            }

            // Tick the behaviour tree
            TickBehaviourTree(entity, ai, deltaTime);
        }
    }

    //COMMENTED BECAUSE BEHAVIOUR TREE FILES NOT CODED YET

    bool AISystem::EnsureBehaviourTreeLoaded(Entity entity, AIComponent& ai) {
        // Already has a tree
        if (ai.Tree != nullptr && !ai.TreeDirty) {
            return true;
        }

        // No tree path specified
        if (ai.TreeAssetPath.empty()) {
            LOG_WARNING("AISystem - Entity ", (uint32_t)entity, " has no behaviour tree path");
            return false;
        }

        // Try to load/get the tree
        BehaviourTree* tree = m_TreeSerializer->GetTree(ai.TreeAssetPath);
        
        if (!tree) {
            // Not in cache, try loading it
            tree = m_TreeSerializer->LoadTree(ai.TreeAssetPath);
        }

        if (!tree) {
            LOG_ERROR("AISystem - Failed to load behaviour tree: ", ai.TreeAssetPath);
            return false;
        }

        // Assign tree to AI component
        ai.Tree = tree;
        ai.TreeDirty = false;

        m_ActiveTreePaths.insert(ai.TreeAssetPath);

        // Initialize blackboard with common values
        InitializeBlackboard(entity, ai);

        LOG_INFO("AISystem - Loaded behaviour tree '", ai.TreeAssetPath, "' for entity ", (uint32_t)entity);
        return true;
    }

    void AISystem::TickBehaviourTree(Entity entity, AIComponent& ai, float deltaTime){//TransformComponent* transform) {
        if (!ai.Tree) {
            return;
        }

        // Update blackboard with current entity info
        ai.SetBlackboardValue("self", (entt::entity)entity);
        ai.SetBlackboardValue("deltaTime", deltaTime);

        // Update transform data if available
        if (entity.HasComponent<TransformComponent>()) {
            auto& transform = entity.GetComponent<TransformComponent>();
            ai.SetBlackboardValue("position", transform.Position);

            // Convert quaternion to euler for easier use in BT
            glm::vec3 euler = glm::degrees(glm::eulerAngles(transform.Rotation));
            ai.SetBlackboardValue("rotation", euler);
        }

        // Tick the root node
        NodeResult result = ai.Tree->Tick(ai.Data, deltaTime);

        // Update AI state based on result
        switch (result) {
        case NodeResult::SUCCESS:
            ai.CurrentState = "Success";
            ai.Tree->Reset(); // Reset for next cycle
            LOG_TRACE("[AISystem] Entity ", (uint32_t)entity, " tree: SUCCESS");
            break;

        case NodeResult::FAILURE:
            ai.CurrentState = "Failure";
            ai.Tree->Reset(); // Reset for next cycle
            LOG_TRACE("[AISystem] Entity ", (uint32_t)entity, " tree: FAILURE");
            break;

        case NodeResult::IN_PROGRESS:
            ai.CurrentState = "Running";
            // Tree continues executing
            break;
        }

        // Store current executing node for debugging
        ai.CurrentNode = ai.Tree->GetCurrentNode();

        // Debug drawing
        if (ai.DebugDraw || m_DebugDrawAll) {
            LOG_TRACE("[AISystem] Entity ", (uint32_t)entity,
                " | State: ", ai.CurrentState,
                " | Trace: ", ai.Tree->GetExecutionTrace());
        }
    }

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

        LOG_INFO("[AISystem] Cleaning up AI component for entity ", (uint32_t)entity,
            " (Tree: ", ai.TreeAssetPath, ")");

        // Note: We don't delete the tree itself - it's owned by AIManager
        // Clear references (tree itself is owned by serializer)
        ai.Tree = nullptr;
        ai.CurrentNode = nullptr;
        ai.Data.clear();
    }

    void AISystem::InitializeBlackboard(Entity entity, AIComponent& ai) {
        // Set up common blackboard values
        ai.SetBlackboardValue("self", (entt::entity)entity);
        ai.SetBlackboardValue("deltaTime", 0.0f);
        
        // Optional: Initialize common gameplay values
        // ai.SetBlackboardValue("isAlerted", false);
        // ai.SetBlackboardValue("target", entt::null);
        // ai.SetBlackboardValue("health", 100.0f);

        LOG_TRACE("[AISystem] Initialized blackboard for entity ", (uint32_t)entity);
    }

}
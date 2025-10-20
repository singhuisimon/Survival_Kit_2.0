#include "IScriptEngineInterface.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "Utility/Logger.h"

namespace Engine {

    // Static member definitions
    IScriptEngineInterface::CreateEntityFunc IScriptEngineInterface::CreateEntity = nullptr;
    IScriptEngineInterface::DestroyEntityFunc IScriptEngineInterface::DestroyEntity = nullptr;
    IScriptEngineInterface::GetEntityNameFunc IScriptEngineInterface::GetEntityName = nullptr;

    IScriptEngineInterface::GetPositionFunc IScriptEngineInterface::GetPosition = nullptr;
    IScriptEngineInterface::SetPositionFunc IScriptEngineInterface::SetPosition = nullptr;
    IScriptEngineInterface::GetRotationFunc IScriptEngineInterface::GetRotation = nullptr;
    IScriptEngineInterface::SetRotationFunc IScriptEngineInterface::SetRotation = nullptr;
    IScriptEngineInterface::GetScaleFunc IScriptEngineInterface::GetScale = nullptr;
    IScriptEngineInterface::SetScaleFunc IScriptEngineInterface::SetScale = nullptr;

    IScriptEngineInterface::HasComponentFunc IScriptEngineInterface::HasComponent = nullptr;
    IScriptEngineInterface::AddComponentFunc IScriptEngineInterface::AddComponent = nullptr;
    IScriptEngineInterface::RemoveComponentFunc IScriptEngineInterface::RemoveComponent = nullptr;

    Scene* IScriptEngineInterface::CurrentScene = nullptr;

    void IScriptEngineInterface::Initialize(Scene* scene) {
        CurrentScene = scene;

        LOG_INFO("Initializing Script Engine Interface...");

        // Entity Management
        CreateEntity = [](const std::string& name) -> uint32_t {
            if (!CurrentScene) return 0;
            Entity entity = CurrentScene->CreateEntity(name);
            return static_cast<uint32_t>(entity);
            };

        DestroyEntity = [](uint32_t entityId) {
            if (!CurrentScene) return;
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            CurrentScene->DestroyEntity(entity);
            };

        GetEntityName = [](uint32_t entityId) -> std::string {
            if (!CurrentScene) return "Invalid";
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            if (entity.HasComponent<TagComponent>()) {
                return entity.GetComponent<TagComponent>().Tag;
            }
            return "Unnamed";
            };

        // Transform Component Access
        GetPosition = [](uint32_t entityId) -> glm::vec3 {
            if (!CurrentScene) return glm::vec3(0.0f);
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            if (entity.HasComponent<TransformComponent>()) {
                return entity.GetComponent<TransformComponent>().Position;
            }
            return glm::vec3(0.0f);
            };

        SetPosition = [](uint32_t entityId, const glm::vec3& position) {
            if (!CurrentScene) return;
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            if (entity.HasComponent<TransformComponent>()) {
                entity.GetComponent<TransformComponent>().Position = position;
            }
            };

        GetRotation = [](uint32_t entityId) -> glm::vec3 {
            if (!CurrentScene) return glm::vec3(0.0f);
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            if (entity.HasComponent<TransformComponent>()) {
                return entity.GetComponent<TransformComponent>().Rotation;
            }
            return glm::vec3(0.0f);
            };

        SetRotation = [](uint32_t entityId, const glm::vec3& rotation) {
            if (!CurrentScene) return;
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            if (entity.HasComponent<TransformComponent>()) {
                entity.GetComponent<TransformComponent>().Rotation = rotation;
            }
            };

        GetScale = [](uint32_t entityId) -> glm::vec3 {
            if (!CurrentScene) return glm::vec3(1.0f);
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            if (entity.HasComponent<TransformComponent>()) {
                return entity.GetComponent<TransformComponent>().Scale;
            }
            return glm::vec3(1.0f);
            };

        SetScale = [](uint32_t entityId, const glm::vec3& scale) {
            if (!CurrentScene) return;
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());
            if (entity.HasComponent<TransformComponent>()) {
                entity.GetComponent<TransformComponent>().Scale = scale;
            }
            };

        // Component Management
        HasComponent = [](uint32_t entityId, const std::string& componentType) -> bool {
            if (!CurrentScene) return false;
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());

            if (componentType == "TransformComponent") {
                return entity.HasComponent<TransformComponent>();
            }
            else if (componentType == "TagComponent") {
                return entity.HasComponent<TagComponent>();
            }
            else if (componentType == "RigidbodyComponent") {
                return entity.HasComponent<RigidbodyComponent>();
            }
            // Add more component types as needed

            return false;
            };

        AddComponent = [](uint32_t entityId, const std::string& componentType) {
            if (!CurrentScene) return;
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());

            if (componentType == "TransformComponent" && !entity.HasComponent<TransformComponent>()) {
                entity.AddComponent<TransformComponent>();
            }
            else if (componentType == "RigidbodyComponent" && !entity.HasComponent<RigidbodyComponent>()) {
                entity.AddComponent<RigidbodyComponent>();
            }
            // Add more component types as needed
            };

        RemoveComponent = [](uint32_t entityId, const std::string& componentType) {
            if (!CurrentScene) return;
            Entity entity(static_cast<entt::entity>(entityId), &CurrentScene->GetRegistry());

            if (componentType == "TransformComponent") {
                entity.RemoveComponent<TransformComponent>();
            }
            else if (componentType == "RigidbodyComponent") {
                entity.RemoveComponent<RigidbodyComponent>();
            }
            // Add more component types as needed
            };

        LOG_INFO("Script Engine Interface initialized with ",
            (CurrentScene ? CurrentScene->GetName() : "null"), " scene");
    }

    void IScriptEngineInterface::Shutdown() {
        CurrentScene = nullptr;

        // Clear all function pointers
        CreateEntity = nullptr;
        DestroyEntity = nullptr;
        GetEntityName = nullptr;
        GetPosition = nullptr;
        SetPosition = nullptr;
        GetRotation = nullptr;
        SetRotation = nullptr;
        GetScale = nullptr;
        SetScale = nullptr;
        HasComponent = nullptr;
        AddComponent = nullptr;
        RemoveComponent = nullptr;

        LOG_INFO("Script Engine Interface shut down");
    }

} // namespace Engine
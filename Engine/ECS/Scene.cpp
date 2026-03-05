#include "Scene.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/PrefabComponent.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Serialization/SceneSerializer.h"
#include "../Prefab/PrefabRegistry.h"
#include "../Utility/Logger.h"

namespace Engine {

    Scene::Scene(const std::string& name)
        : m_Name(name) {
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity entity = Entity(m_Registry.create(), &m_Registry);

        // Add default components
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();

        LOG_TRACE("Scene: Created entity '", name, "' (ID: ", static_cast<uint32_t>(entity), ")");
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        if (!entity) {
            LOG_WARNING("Scene: Attempted to destroy invalid entity");
            return;
        }

        LOG_TRACE("Scene: Destroying entity (ID: ", static_cast<uint32_t>(entity), ")");
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdate(float deltaTime) {
        m_SystemRegistry.OnUpdate(this, deltaTime);
    }

    bool Scene::SaveToFile(const std::string& filepath) {
        SceneSerializer serializer(this);
        return serializer.Serialize(filepath);
    }

    bool Scene::LoadFromFile(const std::string& filepath) {
        SceneSerializer serializer(this);
        return serializer.Deserialize(filepath);
    }

    //Entity Scene::CreateEntityFromPrefab(
    //    xresource::instance_guid prefabGUID,
    //    const std::string& name) {

    //    Entity entity = PrefabInstantiator::InstantiateEntityPrefab(this, prefabGUID);

    //    if (entity && !name.empty()) {
    //        // Override the entity name if provided
    //        if (entity.HasComponent<TagComponent>()) {
    //            entity.GetComponent<TagComponent>().Tag = name;
    //        }
    //    }

    //    return entity;
    //}

    Entity Scene::GetEntity(entt::entity entityId) {
        if (!m_Registry.valid(entityId)) {
            LOG_WARNING("Scene: Attempted to get invalid entity ID: ", (uint32_t)entityId);
            return Entity(); // Return invalid entity
        }
        return Entity(entityId, &m_Registry);
    }

    bool Scene::IsEntityValid(entt::entity entityId) const {
        return m_Registry.valid(entityId);
    }
    Entity Scene::FindEntityByName(const std::string& name) {
        auto view = m_Registry.view<TagComponent>();

        for (auto entityHandle : view) {
            Entity entity(entityHandle, &m_Registry);
            const auto& tag = entity.GetComponent<TagComponent>();

            if (tag.Name == name) {
                return entity;
            }
        }

        LOG_WARNING("Scene: Entity with name '", name, "' not found");
        return Entity(); // Invalid entity
    }

    Entity Scene::FindEntityByTag(const std::string& tag) {
        auto view = m_Registry.view<TagComponent>();

        for (auto entityHandle : view) {
            Entity entity(entityHandle, &m_Registry);
            const auto& tagComponent = entity.GetComponent<TagComponent>();

            if (tagComponent.Tag == tag) {
                return entity;
            }
        }

        LOG_WARNING("Scene: Entity with name '", tag, "' not found");
        return Entity(); // Invalid entity
    }

 
    Entity Scene::InstantiateScenePrefab(std::string filepath, Entity parent) {
        return PrefabInstantiator::InstantiatePrefabFromFile(this, filepath, parent);
    }

    bool Scene::LoadPrefabSceneFromFile(std::string filepath)
    {
        Prefab loadedPrefab;
        return PrefabRegistry::Get().LoadPrefabFromFile(filepath, loadedPrefab);
    }

    //void Scene::UnpackPrefabInstance(Entity entity) {
    //    if (!entity.HasComponent<PrefabComponent>()) {
    //        LOG_WARNING("Scene: Entity is not a prefab instance");
    //        return;
    //    }

    //    // Remove the PrefabComponent to break the prefab link
    //    entity.RemoveComponent<PrefabComponent>();

    //    LOG_INFO("Scene: Unpacked prefab instance (Entity ID: ", static_cast<uint32_t>(entity), ")");
    //}

} // namespace Engine
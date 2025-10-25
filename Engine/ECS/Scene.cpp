#include "Scene.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/PrefabComponent.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Serialization/SceneSerializer.h"
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

    Entity Scene::CreateEntityFromPrefab(
        xresource::instance_guid prefabGUID,
        const std::string& name) {

        Entity entity = PrefabInstantiator::InstantiateEntityPrefab(this, prefabGUID);

        if (entity && !name.empty()) {
            // Override the entity name if provided
            if (entity.HasComponent<TagComponent>()) {
                entity.GetComponent<TagComponent>().Tag = name;
            }
        }

        return entity;
    }

    Entity Scene::InstantiateScenePrefab(xresource::instance_guid prefabGUID) {
        return PrefabInstantiator::InstantiateScenePrefab(this, prefabGUID);
    }

    void Scene::UnpackPrefabInstance(Entity entity) {
        if (!entity.HasComponent<PrefabComponent>()) {
            LOG_WARNING("Scene: Entity is not a prefab instance");
            return;
        }

        // Remove the PrefabComponent to break the prefab link
        entity.RemoveComponent<PrefabComponent>();

        LOG_INFO("Scene: Unpacked prefab instance (Entity ID: ", static_cast<uint32_t>(entity), ")");
    }

} // namespace Engine
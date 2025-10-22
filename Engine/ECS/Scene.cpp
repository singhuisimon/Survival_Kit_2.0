#include "Scene.h"
#include "../Serialization/SceneSerializer.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Prefab/PrefabRegistry.h"
#include "../Utility/Logger.h"
#include "Components.h"

namespace Engine {

    Scene::Scene(const std::string& name)
        : m_Name(name) {
        LOG_INFO("Scene created: ", name);
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity entity(m_Registry.create(), &m_Registry);
        LOG_DEBUG("Entity created: ", name, " (ID: ", (uint32_t)entity, ")");
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        LOG_DEBUG("Entity destroyed (ID: ", (uint32_t)entity, ")");
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdate(float deltaTime) {
        // Systems will be called here in the future
        // For now, just a placeholder
        (void)deltaTime; // Suppress unused parameter warning
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
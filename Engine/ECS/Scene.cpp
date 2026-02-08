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
        if (!m_PendingSceneLoadPath.empty()) {
            LOG_INFO("Scene: Processing pending scene load: ", m_PendingSceneLoadPath);
            std::string pathToLoad = m_PendingSceneLoadPath;
            m_PendingSceneLoadPath.clear();
            LoadFromFile(pathToLoad);
            return;  // Skip normal update this frame
        }
        m_SystemRegistry.OnUpdate(this, deltaTime);
    }

    bool Scene::SaveToFile(const std::string& filepath) {
        SceneSerializer serializer(this);
        return serializer.Serialize(filepath);
    }

    bool Scene::LoadFromFile(const std::string& filepath) {
#ifndef DISABLE_EDITOR
       
        SceneSerializer serializer(this);
        return serializer.Deserialize(filepath);
#else
        LOG_INFO("Scene::LoadFromFile: Loading scene from '", filepath, "'");
        //i//f (m_Registry. > 0) {
            //LOG_INFO("Scene::LoadFromFile: Clearing existing entities");
        //Clear();
            //m_Settings = SceneSettings();
       // }

        //Clear();
        m_Registry.clear();
        
        //ShutdownSystems();
        m_Settings = SceneSettings();
        SceneSerializer serializer(this);
        bool success = serializer.Deserialize(filepath);

        if (!success)
        {
            LOG_ERROR("Scene::LoadFromFile: Failed to deserialize '", filepath, "'");
            return false;
        }

        //InitializeSystems();

        LOG_INFO("Scene::LoadFromFile: Successfully loaded '", filepath, "'");
        return true;
#endif
    }
    void Scene::SetPendingSceneLoad(const std::string& path) {
        m_PendingSceneLoadPath = path;
        LOG_INFO("Scene: Queued scene load for next frame: ", path);
    }
    

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

            if (tag.Tag == name) {
                return entity;
            }
        }

        LOG_WARNING("Scene: Entity with name '", name, "' not found");
        return Entity(); // Invalid entity
    }

 
    //Entity Scene::InstantiateScenePrefab(xresource::instance_guid prefabGUID) {
    //    return PrefabInstantiator::InstantiateScenePrefab(this, prefabGUID);
    //}

    //void Scene::UnpackPrefabInstance(Entity entity) {
    //    if (!entity.HasComponent<PrefabComponent>()) {
    //        LOG_WARNING("Scene: Entity is not a prefab instance");
    //        return;
    //    }

    //    // Remove the PrefabComponent to break the prefab link
    //    entity.RemoveComponent<PrefabComponent>();

    //    LOG_INFO("Scene: Unpacked prefab instance (Entity ID: ", static_cast<uint32_t>(entity), ")");
    //}
    void Scene::Clear()
    {
        LOG_INFO("Scene::Clear: Clearing scene '", m_Name, "'");

        // Step 1: Shutdown all systems (calls OnDestroy on scripts, etc.)
        ShutdownSystems();

        // Step 2: Destroy all entities
        m_Registry.clear();

        // Step 3: Reset scene settings to defaults
        m_Settings = SceneSettings();

        LOG_INFO("Scene::Clear: Scene cleared successfully");
    }
} // namespace Engine
#include "Scene.h"
#include "../Serialization/SceneSerializer.h"
#include "../Utility/Logger.h"

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

} // namespace Engine
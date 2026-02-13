/**
 * @file PrefabHelpers.cpp
 * @brief Implementation - SIMPLIFIED VERSION for your codebase
 */

#include "PrefabHelpers.h"
#include "../ECS/Entity.h"
#include "../ECS/Scene.h"
#include "../ECS/Components.h"
#include "../Component/PrefabComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/TagComponent.h"
#include "../Serialization/ComponentSerializer.h"
#include "../Utility/Logger.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace Engine {

    std::string PrefabHelpers::SerializeEntityToJSON(Entity entity) {
        if (!entity) {
            return "{}";
        }

        // Use RapidJSON with proper memory management
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Add entity name and tag
        if (entity.HasComponent<TagComponent>()) {
            auto& tag = entity.GetComponent<TagComponent>();
            doc.AddMember("Name", rapidjson::Value(tag.Name.c_str(), allocator), allocator);
            doc.AddMember("Tag", rapidjson::Value(tag.Tag.c_str(), allocator), allocator);
        }

        // Serialize components
        rapidjson::Value componentsArray(rapidjson::kArrayType);

        // Use ForEachComponent to avoid duplication
        ForEachComponent(entity, [&](ComponentTypeID type) {
            if (type == ComponentTypeID::Prefab) return; // Skip prefab component

            std::string compJSON = ComponentSerializer::SerializeComponent(entity, type);
            if (compJSON.empty() || compJSON == "{}") return;

            // FIX: Store string before calling c_str() to avoid dangling pointer
            std::string typeName = ComponentSerializer::GetComponentTypeName(type);

            rapidjson::Value compObj(rapidjson::kObjectType);
            compObj.AddMember("type", static_cast<int>(type), allocator);
            compObj.AddMember("typeName",
                rapidjson::Value(typeName.c_str(), allocator),
                allocator);
            compObj.AddMember("data", rapidjson::Value(compJSON.c_str(), allocator), allocator);

            componentsArray.PushBack(compObj, allocator);
            });

        doc.AddMember("Components", componentsArray, allocator);

        // Convert to string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    Entity PrefabHelpers::FindPrefabRoot(Entity entity, Scene* scene) {
        if (!scene || !entity) return Entity{};

        Entity current = entity;
        const int MAX_DEPTH = 100; // Prevent infinite loops
        int depth = 0;

        while (current && depth < MAX_DEPTH) {
            if (current.HasComponent<PrefabComponent>()) {
                auto& prefabComp = current.GetComponent<PrefabComponent>();
                if (prefabComp.isPrefabRoot) {
                    return current;
                }
            }

            // Move to parent
            if (current.HasComponent<TransformComponent>()) {
                auto& transform = current.GetComponent<TransformComponent>();
                if (transform.Parent != u32_max) {
                    current = Entity(static_cast<entt::entity>(transform.Parent), &scene->GetRegistry());
                    depth++;
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }

        return Entity{}; // No prefab root found
    }

    void PrefabHelpers::ClearPrefabComponentData(Entity entity) {
        if (!entity || !entity.HasComponent<PrefabComponent>()) return;

        auto& prefabComp = entity.GetComponent<PrefabComponent>();

        // CRITICAL: Clear all data structures that can cause memory leaks
        prefabComp.componentOverrides.clear();
        prefabComp.deletedEntities.clear();
        prefabComp.addedEntityHandles.clear();
        prefabComp.childEntityIDs.clear();

        LOG_DEBUG("Cleared prefab component data for entity: ",
            entity.HasComponent<TagComponent>() ?
            entity.GetComponent<TagComponent>().Name : "Unknown");
    }

    void PrefabHelpers::CleanupEntityForDeletion(Entity entity, Scene* scene) {
        if (!entity || !scene) return;

        // CRITICAL: Clear prefab component data to prevent memory leaks
        ClearPrefabComponentData(entity);

        // Clear parent-child relationships
        if (entity.HasComponent<TransformComponent>()) {
            auto& transform = entity.GetComponent<TransformComponent>();

            // Unparent from parent
            if (transform.Parent != u32_max) {
                Entity parentEntity(static_cast<entt::entity>(transform.Parent), &scene->GetRegistry());
                if (parentEntity && parentEntity.HasComponent<TransformComponent>()) {
                    auto& parentTransform = parentEntity.GetComponent<TransformComponent>();
                    auto& children = parentTransform.Children;

                    children.erase(
                        std::remove(children.begin(), children.end(),
                            static_cast<u32>(entity.GetHandle())),
                        children.end()
                    );
                }
            }

            // Clear children list (don't delete them, just clear references)
            transform.Children.clear();
        }
    }

    void PrefabHelpers::ForEachComponent(Entity entity,
        std::function<void(ComponentTypeID)> callback) {

        if (!entity || !callback) return;

        // Check all component types in your engine
        if (entity.HasComponent<TagComponent>())
            callback(ComponentTypeID::Tag);

        if (entity.HasComponent<TransformComponent>())
            callback(ComponentTypeID::Transform);

        if (entity.HasComponent<MeshRendererComponent>())
            callback(ComponentTypeID::MeshRenderer);

        if (entity.HasComponent<CameraComponent>())
            callback(ComponentTypeID::Camera);

        if (entity.HasComponent<LightComponent>())
            callback(ComponentTypeID::Light);

        if (entity.HasComponent<RigidbodyComponent>())
            callback(ComponentTypeID::RigidBody);

        if (entity.HasComponent<AudioComponent>())
            callback(ComponentTypeID::Audio);

        if (entity.HasComponent<ListenerComponent>())
            callback(ComponentTypeID::Listerner);

        if (entity.HasComponent<ReverbZoneComponent>())
            callback(ComponentTypeID::ReverbZone);

        if (entity.HasComponent<BehaviourTreeComponent>())
            callback(ComponentTypeID::BehaviourTree);

        if (entity.HasComponent<ParticleComponent>())
            callback(ComponentTypeID::ParticleSystem);

        if (entity.HasComponent<ScriptComponent>())
            callback(ComponentTypeID::Script);

        if (entity.HasComponent<AnimatorComponent>())
            callback(ComponentTypeID::Animator);

        if (entity.HasComponent<SpriteRendererComponent>())
            callback(ComponentTypeID::SpriteRenderer);

        if (entity.HasComponent<TrailComponent>())
            callback(ComponentTypeID::Trail);

        if (entity.HasComponent<TextComponent>())
            callback(ComponentTypeID::Text);

        // NOTE: Removed ColliderComponent since you don't have it
    }

} // namespace Engine
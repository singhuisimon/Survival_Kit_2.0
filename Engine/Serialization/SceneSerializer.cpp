#include "SceneSerializer.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/CameraComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/RigidbodyComponent.h"
#include "../Component/AudioComponent.h"
#include "../Component/ListenerComponent.h"
#include "../Component/ReverbZoneComponent.h"
#include "../Component/BehaviourTreeComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/ScriptComponent.h"
#include "../Component/LightComponent.h"
#include "../Prefab/BehaviourTreePrefab.h"
#include "../Component/PrefabComponent.h"
#include "../Component/AnimatorComponent.h"

#include "../Scripting/ScriptSerializer.h"
#include "../Scripting/MonoScriptEngine.h"
#include "ReflectionRegistry.h"
#include "../Utility/Logger.h"

// RapidJSON includes
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

// Standard library
#include <fstream>
#include <string>

// Required for quaternion to Euler conversion
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Engine {

    SceneSerializer::SceneSerializer(Scene* scene)
        : m_Scene(scene) {
    }

    bool SceneSerializer::Serialize(const std::string& filepath) {
        LOG_INFO("Serializing scene to: ", filepath);

        std::string jsonString = SerializeToString();

        // Write to file
        std::ofstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for writing: ", filepath);
            return false;
        }

        file << jsonString;
        file.close();

        LOG_INFO("Scene serialized successfully");
        return true;
    }

    std::string SceneSerializer::SerializeToString() {
        using namespace rapidjson;

        LOG_TRACE("Starting scene serialization...");

        Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Scene metadata
        LOG_TRACE("Adding scene metadata...");
        doc.AddMember("Scene", Value(m_Scene->GetName().c_str(), allocator), allocator);
        doc.AddMember("Version", "1.0", allocator);

        // Entities array
        LOG_TRACE("Creating entities array...");
        Value entitiesArray(kArrayType);

        auto& registry = m_Scene->GetRegistry();
        auto view = registry.view<TagComponent>();

        LOG_TRACE("Found ", (int)view.size(), " entities to serialize");

        int entityIndex = 0;
        for (auto entityHandle : view) {
            LOG_TRACE("Serializing entity ", entityIndex++);

            Entity entity(entityHandle, &registry);
            Value entityObj(kObjectType);

            // Entity ID
            entityObj.AddMember("ID", (uint32_t)entity, allocator);

            // Components array
            Value componentsArray(kArrayType);

            // Serialize TagComponent
            if (entity.HasComponent<TagComponent>()) {
                LOG_TRACE("  - Serializing TagComponent");
                auto& tag = entity.GetComponent<TagComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "TagComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("Tag", Value(tag.Tag.c_str(), allocator), allocator);
                componentObj.AddMember("Properties", propertiesObj, allocator);

                componentsArray.PushBack(componentObj, allocator);
            }
            if (entity.HasComponent<PrefabComponent>())
            {
                LOG_TRACE("  - Serializing PrefabComponent");
                auto& prefab = entity.GetComponent<PrefabComponent>();

                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "PrefabComponent", allocator);

                Value propertiesObj(kObjectType);

                // Prefab GUID
                propertiesObj.AddMember(
                    "PrefabGUID",
                    Value(std::to_string(prefab.PrefabGUID.m_Value).c_str(), allocator),
                    allocator
                );

                // Component GUID
                propertiesObj.AddMember(
                    "ComponentGUID",
                    Value(std::to_string(prefab.ComponentGUID.m_Value).c_str(), allocator),
                    allocator
                );

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }
            // Serialize TransformComponent
            if (entity.HasComponent<TransformComponent>()) {
                LOG_TRACE("  - Serializing TransformComponent");
                auto& transform = entity.GetComponent<TransformComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "TransformComponent", allocator);

                Value propertiesObj(kObjectType);

                // Position
                Value posArray(kArrayType);
                posArray.PushBack(transform.Position.x, allocator);
                posArray.PushBack(transform.Position.y, allocator);
                posArray.PushBack(transform.Position.z, allocator);
                propertiesObj.AddMember("Position", posArray, allocator);

                // Rotation - Convert quaternion to Euler angles
                glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(transform.Rotation));
                Value rotArray(kArrayType);
                rotArray.PushBack(eulerRotation.x, allocator);
                rotArray.PushBack(eulerRotation.y, allocator);
                rotArray.PushBack(eulerRotation.z, allocator);
                propertiesObj.AddMember("Rotation", rotArray, allocator);

                // Scale
                Value scaleArray(kArrayType);
                scaleArray.PushBack(transform.Scale.x, allocator);
                scaleArray.PushBack(transform.Scale.y, allocator);
                scaleArray.PushBack(transform.Scale.z, allocator);
                propertiesObj.AddMember("Scale", scaleArray, allocator);

                // Parent 
                propertiesObj.AddMember("Parent", transform.Parent, allocator);

                // List of children 
                Value childrenArray(kArrayType);
                for (const auto& child : transform.Children) {
                    childrenArray.PushBack(child, allocator);
                }
                propertiesObj.AddMember("Children", childrenArray, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }

            // Serialize CameraComponent
            if (entity.HasComponent<CameraComponent>()) {
                LOG_TRACE("  - Serializing CameraComponent");
                auto& camera = entity.GetComponent<CameraComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "CameraComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("Enabled", camera.Enabled, allocator);
                propertiesObj.AddMember("autoAspect", camera.autoAspect, allocator);
                propertiesObj.AddMember("isDirty", camera.isDirty, allocator);
                propertiesObj.AddMember("Depth", camera.Depth, allocator);
                propertiesObj.AddMember("Aspect", camera.Aspect, allocator);
                propertiesObj.AddMember("FOV", camera.FOV, allocator);
                propertiesObj.AddMember("NearPlane", camera.NearPlane, allocator);
                propertiesObj.AddMember("FarPlane", camera.FarPlane, allocator);

                Value targetArr(kArrayType);
                targetArr.PushBack(camera.Target.x, allocator);
                targetArr.PushBack(camera.Target.y, allocator);
                targetArr.PushBack(camera.Target.z, allocator);
                propertiesObj.AddMember("Target", targetArr, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }

            // Serialize MeshRendererComponent
            if (entity.HasComponent<MeshRendererComponent>()) {
                LOG_TRACE("  - Serializing MeshRendererComponent");
                auto& mesh = entity.GetComponent<MeshRendererComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "MeshRendererComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("MeshGuid", mesh.MeshGuid.m_Value, allocator);
                propertiesObj.AddMember("MaterialGuid", mesh.MaterialGuid.m_Value, allocator);
                propertiesObj.AddMember("TextureGuid", mesh.TextureGuid.m_Value, allocator);
                propertiesObj.AddMember("Visible", mesh.Visible, allocator);
                propertiesObj.AddMember("MeshType", mesh.MeshType, allocator);
                propertiesObj.AddMember("Material", mesh.Material, allocator);
                propertiesObj.AddMember("Texture", mesh.Texture, allocator);
                propertiesObj.AddMember("SubmeshIndex", mesh.SubmeshIndex, allocator);
                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }

            // Serialize RigidbodyComponent
            if (entity.HasComponent<RigidbodyComponent>()) {
                LOG_TRACE("  - Serializing RigidbodyComponent");
                auto& rb = entity.GetComponent<RigidbodyComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "RigidbodyComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("Mass", rb.Mass, allocator);
                propertiesObj.AddMember("IsKinematic", rb.IsKinematic, allocator);
                propertiesObj.AddMember("UseGravity", rb.UseGravity, allocator);

                Value velArray(kArrayType);
                velArray.PushBack(rb.Velocity.x, allocator);
                velArray.PushBack(rb.Velocity.y, allocator);
                velArray.PushBack(rb.Velocity.z, allocator);
                propertiesObj.AddMember("Velocity", velArray, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }

            // Serialize AudioComponent
            if (entity.HasComponent<AudioComponent>()) {
                LOG_TRACE("  - Serializing AudioComponent");
                auto& audio = entity.GetComponent<AudioComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "AudioComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("FilePath", Value(audio.AudioFilePath.c_str(), allocator), allocator);
                propertiesObj.AddMember("Type", static_cast<int>(audio.Type), allocator);
                propertiesObj.AddMember("State", static_cast<int>(audio.State), allocator);
                propertiesObj.AddMember("Volume", audio.Volume, allocator);
                propertiesObj.AddMember("Pitch", audio.Pitch, allocator);
                propertiesObj.AddMember("Loop", audio.Loop, allocator);
                propertiesObj.AddMember("Mute", audio.Mute, allocator);
                propertiesObj.AddMember("ReverbProperties", audio.ReverbProperties, allocator);
                propertiesObj.AddMember("Is3D", audio.Is3D, allocator);
                propertiesObj.AddMember("MinDistance", audio.MinDistance, allocator);
                propertiesObj.AddMember("MaxDistance", audio.MaxDistance, allocator);
                propertiesObj.AddMember("RolloffMode", static_cast<int>(audio.RolloffMode), allocator);
                propertiesObj.AddMember("DopplerLevel", audio.DopplerLevel, allocator);
                propertiesObj.AddMember("Pan2D", audio.Pan2D, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }

            // Serialize ListenerComponent
            if (entity.HasComponent<ListenerComponent>()) {
                LOG_TRACE("  - Serializing ListenerComponent");
                auto& listener = entity.GetComponent<ListenerComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "ListenerComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("Active", listener.Active, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }

            // Serialize ReverbComponent
            if (entity.HasComponent<ReverbZoneComponent>()) {
                LOG_TRACE("  - Serializing ReverbComponent");

                auto& reverb = entity.GetComponent<ReverbZoneComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "ReverbComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("Preset", static_cast<int>(reverb.Preset), allocator);
                propertiesObj.AddMember("MinDistance", reverb.MinDistance, allocator);
                propertiesObj.AddMember("MaxDistance", reverb.MaxDistance, allocator);
                propertiesObj.AddMember("DecayTime", reverb.DecayTime, allocator);
                propertiesObj.AddMember("HfDecayRatio", reverb.HfDecayRatio, allocator);
                propertiesObj.AddMember("Diffusion", reverb.Diffusion, allocator);
                propertiesObj.AddMember("Density", reverb.Density, allocator);
                propertiesObj.AddMember("WetLevel", reverb.WetLevel, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }
            // Serialize BehaviourTreeComponent
            if (entity.HasComponent<BehaviourTreeComponent>()) {
                LOG_TRACE("  - Serializing BehaviourTreeComponent");
                auto& bt = entity.GetComponent<BehaviourTreeComponent>();
                rapidjson::Value componentObj(kObjectType);
                componentObj.AddMember("Type", "BehaviourTreeComponent", allocator);

                rapidjson::Value propertiesObj(kObjectType);
                propertiesObj.AddMember("Active", bt.Active, allocator);
                propertiesObj.AddMember("ResetOnComplete", bt.ResetOnComplete, allocator);
                propertiesObj.AddMember("TreeAssetPath",
                    rapidjson::Value(bt.TreeAssetPath.c_str(), allocator), allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }
            // Serialize ParticleComponent
            if (entity.HasComponent<ParticleComponent>()) {
                LOG_TRACE("  - Serializing Particle Component");

                const auto& emitter = entity.GetComponent<ParticleComponent>();
                rapidjson::Value componentObj(kObjectType);
                componentObj.AddMember("Type", "ParticleComponent", allocator);

                rapidjson::Value propertiesObj(kObjectType);

                // Initial Velocity
                rapidjson::Value velArray(kArrayType);
                velArray.PushBack(emitter.InitialVelocity.x, allocator);
                velArray.PushBack(emitter.InitialVelocity.y, allocator);
                velArray.PushBack(emitter.InitialVelocity.z, allocator);
                propertiesObj.AddMember("Initial Velocity", velArray, allocator);

                // Min Color
                rapidjson::Value minColorArray(kArrayType);
                minColorArray.PushBack(emitter.ColorMin.x, allocator);
                minColorArray.PushBack(emitter.ColorMin.y, allocator);
                minColorArray.PushBack(emitter.ColorMin.z, allocator);
                propertiesObj.AddMember("Color Min", minColorArray, allocator);

                // Max Color
                rapidjson::Value maxColorArray(kArrayType);
                maxColorArray.PushBack(emitter.ColorMax.x, allocator);
                maxColorArray.PushBack(emitter.ColorMax.y, allocator);
                maxColorArray.PushBack(emitter.ColorMax.z, allocator);
                propertiesObj.AddMember("Color Max", maxColorArray, allocator);

                // Max Particles
                propertiesObj.AddMember("Max Particles", emitter.MaxParticles, allocator);

                // Particle Type
                propertiesObj.AddMember("Particle Type", emitter.ParticleType, allocator);

                // Emission Rate
                propertiesObj.AddMember("Emission Rate", emitter.EmissionRate, allocator);

                // Particle Lifetime
                propertiesObj.AddMember("Particle Lifetime", emitter.ParticleLifetime, allocator);

                // Emission Accumulator
                propertiesObj.AddMember("Emission Accumulator", emitter.EmissionAccumulator, allocator);

                // Particle Size
                propertiesObj.AddMember("Particle Size", emitter.ParticleSize, allocator);

                // Randomization parameters
                propertiesObj.AddMember("Velocity Randomness", emitter.VelocityRandomness, allocator);
                propertiesObj.AddMember("Lifetime Randomness", emitter.LifetimeRandomness, allocator);
                propertiesObj.AddMember("Spread Angle", emitter.SpreadAngle, allocator);
                propertiesObj.AddMember("Min Speed", emitter.MinSpeed, allocator);
                propertiesObj.AddMember("Max Speed", emitter.MaxSpeed, allocator);
                propertiesObj.AddMember("Rotation Speed", emitter.RotationSpeed, allocator);

                // Boolean parameters
                propertiesObj.AddMember("Randomize Rotation", emitter.RandomizeRotation, allocator);
                propertiesObj.AddMember("Loop", emitter.Loop, allocator);
                propertiesObj.AddMember("Active", emitter.Active, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }
            // Serialize ScriptComponent
            if (entity.HasComponent<ScriptComponent>()) {
                LOG_TRACE("  - Serializing ScriptComponent");
                auto& script = entity.GetComponent<ScriptComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "ScriptComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("ScriptClassName",
                    Value(script.ScriptClassName.c_str(), allocator), allocator);

                if (script.ScriptInstance)
                {
                    rapidjson::Value fieldsObj(kObjectType);
                    SerializeScriptFieldsToRapidJSON((MonoObject*)script.ScriptInstance, fieldsObj, allocator);
                    propertiesObj.AddMember("Fields", fieldsObj, allocator);
                }
                // --- END BLOCK ---

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }
            // Serialize LightComponent
            if (entity.HasComponent<LightComponent>()) {
                LOG_TRACE("  - Serializing LightComponent");
                auto& light = entity.GetComponent<LightComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "LightComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("Enabled", light.Enabled, allocator);
                propertiesObj.AddMember("Type", static_cast<int>(light.Type), allocator);
                //propertiesObj.AddMember("Mode", light.Mode, allocator); // For now only 1 mode, not required in scene file
                Value colorArr(kArrayType);
                colorArr.PushBack(light.Color.x, allocator);
                colorArr.PushBack(light.Color.y, allocator);
                colorArr.PushBack(light.Color.z, allocator);
                propertiesObj.AddMember("Color", colorArr, allocator);
                propertiesObj.AddMember("Intensity", light.Intensity, allocator);
                propertiesObj.AddMember("Range", light.Range, allocator);
                propertiesObj.AddMember("SpotAngleDeg", light.SpotAngleDeg, allocator);
                propertiesObj.AddMember("IndirectMultiplier", light.IndirectMultiplier, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }
            // Serialize AnimatorComponent
            if (entity.HasComponent<AnimatorComponent>()) {
                LOG_TRACE("  - Serializing AnimatorComponent");
                auto& animator = entity.GetComponent<AnimatorComponent>();
                Value componentObj(kObjectType);
                componentObj.AddMember("Type", "AnimatorComponent", allocator);

                Value propertiesObj(kObjectType);
                propertiesObj.AddMember("playing", animator.playing, allocator);
                propertiesObj.AddMember("respectClipLoop", animator.respectClipLoop, allocator);
                propertiesObj.AddMember("controller", animator.controller, allocator);
                propertiesObj.AddMember("currentClipIndex", animator.currentClipIndex, allocator);
                propertiesObj.AddMember("currentTime", animator.currentTime, allocator);
                propertiesObj.AddMember("playbackSpeed", animator.playbackSpeed, allocator);

                componentObj.AddMember("Properties", propertiesObj, allocator);
                componentsArray.PushBack(componentObj, allocator);
            }

            entityObj.AddMember("Components", componentsArray, allocator);
            entitiesArray.PushBack(entityObj, allocator);
        }

        doc.AddMember("Entities", entitiesArray, allocator);

        // Convert to string
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        doc.Accept(writer);

        LOG_TRACE("Scene serialization complete");
        return buffer.GetString();
    }

    bool SceneSerializer::Deserialize(const std::string& filepath) {
        LOG_INFO("Deserializing scene from: ", filepath);

        // Read file
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for reading: ", filepath);
            return false;
        }

        std::string jsonString((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        return DeserializeFromString(jsonString);
    }

    bool SceneSerializer::DeserializeFromString(const std::string& jsonString) {
        using namespace rapidjson;

        LOG_TRACE("Parsing JSON...");

        Document doc;
        doc.Parse(jsonString.c_str());

        if (doc.HasParseError()) {
            LOG_ERROR("JSON parse error at offset ", doc.GetErrorOffset());
            return false;
        }

        // Clear current scene
        auto& registry = m_Scene->GetRegistry();
        registry.clear();

        // Read scene name
        if (doc.HasMember("Scene")) {
            std::string sceneName = doc["Scene"].GetString();
            m_Scene->SetName(sceneName);  // Actually update the scene name
            LOG_INFO("Loading scene: ", doc["Scene"].GetString());
        }

        // Read entities
        if (!doc.HasMember("Entities") || !doc["Entities"].IsArray()) {
            LOG_ERROR("No entities array in scene file");
            return false;
        }

        const Value& entities = doc["Entities"];

        for (SizeType i = 0; i < entities.Size(); i++) {
            const Value& entityObj = entities[i];

            // Get entity name from TagComponent if available
            std::string entityName = "Entity";
            if (entityObj.HasMember("Components")) {
                const Value& components = entityObj["Components"];
                for (SizeType j = 0; j < components.Size(); j++) {
                    if (components[j]["Type"].GetString() == std::string("TagComponent")) {
                        entityName = components[j]["Properties"]["Tag"].GetString();
                        break;
                    }
                }
            }

            // Read the saved entity ID
            entt::entity entityId = entt::null;
            if (entityObj.HasMember("ID")) {
                entityId = static_cast<entt::entity>(entityObj["ID"].GetUint());
                LOG_TRACE("Restoring entity with ID: ", (uint32_t)entityId);
            }

            // Create entity
            Entity entity;
            if (entityId != entt::null) {
                // Create entity with specific ID to preserve it across saves
                entity = Entity(registry.create(entityId), &registry);
                LOG_TRACE("Created entity '", entityName, "' with preserved ID: ", (uint32_t)entity);
            }
            else {
                // Fallback to auto-generated ID (for old scene files)
                entity = m_Scene->CreateEntity(entityName);
                LOG_WARNING("Entity ID not found in scene file, auto-generating new ID");
            }

            // Deserialize components
            if (entityObj.HasMember("Components")) {
                const Value& components = entityObj["Components"];

                for (SizeType j = 0; j < components.Size(); j++) {
                    const Value& componentObj = components[j];
                    std::string componentType = componentObj["Type"].GetString();
                    const Value& properties = componentObj["Properties"];

                    // Deserialize specific component types
                    if (componentType == "TagComponent") {
                        auto& tag = entity.AddComponent<TagComponent>();
                        tag.Tag = properties["Tag"].GetString();
                    }
                    else if (componentType == "PrefabComponent")
                    {
                        auto& prefabComp = entity.AddComponent<PrefabComponent>();

                        // --- Prefab GUID ---
                        if (properties.HasMember("PrefabGUID"))
                            prefabComp.PrefabGUID = xresource::instance_guid(
                                std::stoull(properties["PrefabGUID"].GetString())
                            );

                        // --- PrefabComponent's own GUID ---
                        if (properties.HasMember("ComponentGUID"))
                            prefabComp.ComponentGUID = xresource::instance_guid(
                                std::stoull(properties["ComponentGUID"].GetString())
                            );


                    }
                    else if (componentType == "TransformComponent") {
                        auto& transform = entity.AddComponent<TransformComponent>();

                        // Position
                        if (properties.HasMember("Position")) {
                            const Value& posArray = properties["Position"];
                            transform.Position = glm::vec3(
                                posArray[0].GetFloat(),
                                posArray[1].GetFloat(),
                                posArray[2].GetFloat()
                            );
                        }

                        // Rotation - Convert Euler angles to quaternion
                        if (properties.HasMember("Rotation")) {
                            const Value& rotArray = properties["Rotation"];
                            glm::vec3 eulerRotation(
                                rotArray[0].GetFloat(),
                                rotArray[1].GetFloat(),
                                rotArray[2].GetFloat()
                            );
                            transform.Rotation = glm::quat(glm::radians(eulerRotation));
                        }

                        // Scale
                        if (properties.HasMember("Scale")) {
                            const Value& scaleArray = properties["Scale"];
                            transform.Scale = glm::vec3(
                                scaleArray[0].GetFloat(),
                                scaleArray[1].GetFloat(),
                                scaleArray[2].GetFloat()
                            );
                        }

                        if (properties.HasMember("Parent")) {
                            transform.Parent = properties["Parent"].GetUint();
                        }

                        if (properties.HasMember("Children") && properties["Children"].IsArray()) {

                            transform.Children.clear();
                            const Value& childrenArray = properties["Children"];

                            for (rapidjson::SizeType i = 0; i < childrenArray.Size(); ++i)
                                transform.Children.push_back(childrenArray[i].GetUint());

                        }
                    }
                    else if (componentType == "CameraComponent") {
                        auto& camera = entity.AddComponent<CameraComponent>();

                        if (properties.HasMember("Enabled"))
                            camera.Enabled = properties["Enabled"].GetBool();
                        if (properties.HasMember("autoAspect"))
                            camera.autoAspect = properties["autoAspect"].GetBool();
                        if (properties.HasMember("isDirty"))
                            camera.isDirty = properties["isDirty"].GetBool();
                        if (properties.HasMember("Depth"))
                            camera.Depth = properties["Depth"].GetUint();
                        if (properties.HasMember("Aspect"))
                            camera.Aspect = properties["Aspect"].GetFloat();
                        if (properties.HasMember("FOV"))
                            camera.FOV = properties["FOV"].GetFloat();
                        if (properties.HasMember("NearPlane"))
                            camera.NearPlane = properties["NearPlane"].GetFloat();
                        if (properties.HasMember("FarPlane"))
                            camera.FarPlane = properties["FarPlane"].GetFloat();

                        if (properties.HasMember("Target")) {
                            const Value& target = properties["Target"];
                            camera.Target = glm::vec3(
                                target[0].GetFloat(),
                                target[1].GetFloat(),
                                target[2].GetFloat()
                            );
                        }
                    }
                    else if (componentType == "MeshRendererComponent") {
                        auto& mesh = entity.AddComponent<MeshRendererComponent>();
                        if (properties.HasMember("MeshGuid"))
                            mesh.MeshGuid = xresource::instance_guid{ properties["MeshGuid"].GetUint64() };
                        if (properties.HasMember("MaterialGuid")) {
                            mesh.MaterialGuid = xresource::instance_guid{ properties["MaterialGuid"].GetUint64() };
                        }
                        if (properties.HasMember("TextureGuid")) {
                            mesh.TextureGuid = xresource::instance_guid{ properties["TextureGuid"].GetUint64() };
                        }
                        if (properties.HasMember("Visible")) mesh.Visible = properties["Visible"].GetBool();
                        if (properties.HasMember("MeshType")) mesh.MeshType = properties["MeshType"].GetUint();
                        if (properties.HasMember("Material")) mesh.Material = properties["Material"].GetUint();
                        if (properties.HasMember("Texture")) mesh.Texture = properties["Texture"].GetUint();
                        if (properties.HasMember("SubmeshIndex")) mesh.SubmeshIndex = properties["SubmeshIndex"].GetUint();
                    }
                    else if (componentType == "RigidbodyComponent") {
                        auto& rb = entity.AddComponent<RigidbodyComponent>();
                        if (properties.HasMember("Mass")) rb.Mass = properties["Mass"].GetFloat();
                        if (properties.HasMember("IsKinematic")) rb.IsKinematic = properties["IsKinematic"].GetBool();
                        if (properties.HasMember("UseGravity")) rb.UseGravity = properties["UseGravity"].GetBool();

                        if (properties.HasMember("Velocity")) {
                            const Value& velArray = properties["Velocity"];
                            rb.Velocity = glm::vec3(
                                velArray[0].GetFloat(),
                                velArray[1].GetFloat(),
                                velArray[2].GetFloat()
                            );
                        }
                    }
                    else if (componentType == "AudioComponent") {
                        auto& audio = entity.AddComponent<AudioComponent>();

                        if (properties.HasMember("FilePath"))
                            audio.AudioFilePath = properties["FilePath"].GetString();
                        if (properties.HasMember("Type"))
                            audio.Type = static_cast<AudioType>(properties["Type"].GetInt());
                        if (properties.HasMember("State"))
                            audio.State = static_cast<PlayState>(properties["State"].GetInt());
                        if (properties.HasMember("Volume"))
                            audio.Volume = properties["Volume"].GetFloat();
                        if (properties.HasMember("Pitch"))
                            audio.Pitch = properties["Pitch"].GetFloat();
                        if (properties.HasMember("Loop"))
                            audio.Loop = properties["Loop"].GetBool();
                        if (properties.HasMember("Mute"))
                            audio.Mute = properties["Mute"].GetBool();
                        if (properties.HasMember("Reverb"))
                            audio.ReverbProperties = properties["ReverbProperties"].GetFloat();
                        if (properties.HasMember("Is3D"))
                            audio.Is3D = properties["Is3D"].GetBool();
                        if (properties.HasMember("MinDistance"))
                            audio.MinDistance = properties["MinDistance"].GetFloat();
                        if (properties.HasMember("MaxDistance"))
                            audio.MaxDistance = properties["MaxDistance"].GetFloat();
                        if (properties.HasMember("RolloffMode"))
                            audio.RolloffMode = static_cast<AudioRolloffMode>(properties["RolloffMode"].GetInt());
                        if (properties.HasMember("DopplerLevel"))
                            audio.DopplerLevel = properties["DopplerLevel"].GetFloat();
                        if (properties.HasMember("Pan2D"))
                            audio.Pan2D = properties["Pan2D"].GetFloat();
                    }
                    else if (componentType == "ListenerComponent") {
                        auto& listener = entity.AddComponent<ListenerComponent>();

                        if (properties.HasMember("Active"))
                            listener.Active = properties["Active"].GetBool();
                    }
                    else if (componentType == "ReverbComponent") {
                        auto& reverb = entity.AddComponent<ReverbZoneComponent>();

                        if (properties.HasMember("Preset"))
                            reverb.Preset = static_cast<ReverbPreset>(properties["Preset"].GetInt());
                        if (properties.HasMember("MinDistance"))
                            reverb.MinDistance = properties["MinDistance"].GetFloat();
                        if (properties.HasMember("MaxDistance"))
                            reverb.MaxDistance = properties["MaxDistance"].GetFloat();
                        if (properties.HasMember("DecayTime"))
                            reverb.DecayTime = properties["DecayTime"].GetFloat();
                        if (properties.HasMember("HfDecayRatio"))
                            reverb.HfDecayRatio = properties["HfDecayRatio"].GetFloat();
                        if (properties.HasMember("Diffusion"))
                            reverb.Diffusion = properties["Diffusion"].GetFloat();
                        if (properties.HasMember("Density"))
                            reverb.Density = properties["Density"].GetFloat();
                        if (properties.HasMember("WetLevel"))
                            reverb.WetLevel = properties["WetLevel"].GetFloat();
                    }
                    else if (componentType == "BehaviourTreeComponent") {
                        auto& bt = entity.AddComponent<BehaviourTreeComponent>();
                        if (properties.HasMember("Active"))
                            bt.Active = properties["Active"].GetBool();
                        if (properties.HasMember("ResetOnComplete"))
                            bt.ResetOnComplete = properties["ResetOnComplete"].GetBool();
                        if (properties.HasMember("TreeAssetPath")) {
                            bt.TreeAssetPath = properties["TreeAssetPath"].GetString();
                        }

                        // Do NOT load the tree here only store the reference
                        bt.TreeInstance = nullptr;

                    }
                    else if (componentType == "ParticleComponent") {
                        auto& emitter = entity.AddComponent<ParticleComponent>();

                        // Initial Velocity
                        if (properties.HasMember("Initial Velocity") && properties["Initial Velocity"].IsArray()) {
                            const auto& velArray = properties["Initial Velocity"].GetArray();
                            if (velArray.Size() >= 3) {
                                emitter.InitialVelocity.x = velArray[0].GetFloat();
                                emitter.InitialVelocity.y = velArray[1].GetFloat();
                                emitter.InitialVelocity.z = velArray[2].GetFloat();
                            }
                        }

                        // Color Min
                        if (properties.HasMember("Color Min") && properties["Color Min"].IsArray()) {
                            const auto& minColorArray = properties["Color Min"].GetArray();
                            if (minColorArray.Size() >= 3) {
                                emitter.ColorMin.x = minColorArray[0].GetFloat();
                                emitter.ColorMin.y = minColorArray[1].GetFloat();
                                emitter.ColorMin.z = minColorArray[2].GetFloat();
                            }
                        }

                        // Color Max
                        if (properties.HasMember("Color Max") && properties["Color Max"].IsArray()) {
                            const auto& maxColorArray = properties["Color Max"].GetArray();
                            if (maxColorArray.Size() >= 3) {
                                emitter.ColorMax.x = maxColorArray[0].GetFloat();
                                emitter.ColorMax.y = maxColorArray[1].GetFloat();
                                emitter.ColorMax.z = maxColorArray[2].GetFloat();
                            }
                        }

                        // Max Particles
                        if (properties.HasMember("Max Particles"))
                            emitter.MaxParticles = properties["Max Particles"].GetUint();

                        // Particle Type
                        if (properties.HasMember("Particle Type"))
                            emitter.ParticleType = properties["Particle Type"].GetUint();

                        // Emission Rate
                        if (properties.HasMember("Emission Rate"))
                            emitter.EmissionRate = properties["Emission Rate"].GetFloat();

                        // Particle Lifetime
                        if (properties.HasMember("Particle Lifetime"))
                            emitter.ParticleLifetime = properties["Particle Lifetime"].GetFloat();

                        // Emission Accumulator
                        if (properties.HasMember("Emission Accumulator"))
                            emitter.EmissionAccumulator = properties["Emission Accumulator"].GetFloat();

                        // Particle Size
                        if (properties.HasMember("Particle Size"))
                            emitter.ParticleSize = properties["Particle Size"].GetFloat();

                        // Randomization parameters
                        if (properties.HasMember("Velocity Randomness"))
                            emitter.VelocityRandomness = properties["Velocity Randomness"].GetFloat();

                        if (properties.HasMember("Lifetime Randomness"))
                            emitter.LifetimeRandomness = properties["Lifetime Randomness"].GetFloat();

                        if (properties.HasMember("Spread Angle"))
                            emitter.SpreadAngle = properties["Spread Angle"].GetFloat();

                        if (properties.HasMember("Min Speed"))
                            emitter.MinSpeed = properties["Min Speed"].GetFloat();

                        if (properties.HasMember("Max Speed"))
                            emitter.MaxSpeed = properties["Max Speed"].GetFloat();

                        if (properties.HasMember("Rotation Speed"))
                            emitter.RotationSpeed = properties["Rotation Speed"].GetFloat();

                        // Boolean parameters
                        if (properties.HasMember("Randomize Rotation"))
                            emitter.RandomizeRotation = properties["Randomize Rotation"].GetBool();

                        if (properties.HasMember("Loop"))
                            emitter.Loop = properties["Loop"].GetBool();

                        if (properties.HasMember("Active"))
                            emitter.Active = properties["Active"].GetBool();
                    }
                    else if (componentType == "ScriptComponent") {
                        auto script = entity.AddComponent<ScriptComponent>();
                        if (properties.HasMember("ScriptClassName"))
                            script.ScriptClassName = properties["ScriptClassName"].GetString();

                        // Your script system must actually create/instantiate the MonoObject here
                        script.ScriptInstance = MonoScriptEngine::GetInstance().CreateScriptInstance(script.ScriptClassName);

                        // Now, immediately restore field values
                        if (properties.HasMember("Fields") && script.ScriptInstance)
                        {
                            const rapidjson::Value& fieldsObj = properties["Fields"];
                            DeserializeScriptFieldsFromRapidJSON((MonoObject*)script.ScriptInstance, fieldsObj);
                        }

                    } else if (componentType == "LightComponent") {
                        auto& light = entity.AddComponent<LightComponent>();

                        if (properties.HasMember("Enabled"))
                            light.Enabled = properties["Enabled"].GetBool();
                        if (properties.HasMember("Type"))
                            light.Type = static_cast<LightType>(properties["Type"].GetInt()); // 0=Dir,1=Point,2=Spot
                        // Optional: Mode is usually omitted in scene files (Realtime only), but handle if present
                        //if (properties.HasMember("Mode"))
                        //    light.Mode = static_cast<LightMode>(properties["Mode"].GetInt());
                        if (properties.HasMember("Color") && properties["Color"].IsArray()) {
                            const auto& col = properties["Color"];
                            light.Color = glm::vec3(
                                col[0].GetFloat(),
                                col[1].GetFloat(),
                                col[2].GetFloat()
                            );
                        }
                        if (properties.HasMember("Intensity"))
                            light.Intensity = properties["Intensity"].GetFloat();
                        if (properties.HasMember("Range"))
                            light.Range = properties["Range"].GetFloat();
                        if (properties.HasMember("SpotAngleDeg"))
                            light.SpotAngleDeg = properties["SpotAngleDeg"].GetFloat();
                        if (properties.HasMember("IndirectMultiplier"))
                            light.IndirectMultiplier = properties["IndirectMultiplier"].GetFloat();
                    } else if (componentType == "AnimatorComponent") {
                        auto& animator = entity.AddComponent<AnimatorComponent>();

                        if (properties.HasMember("playing"))
                            animator.playing = properties["playing"].GetBool();
                        if (properties.HasMember("respectClipLoop"))
                            animator.respectClipLoop = properties["respectClipLoop"].GetBool();
                        if (properties.HasMember("controller"))
                            animator.controller = properties["controller"].GetUint();
                        if (properties.HasMember("currentClipIndex"))
                            animator.currentClipIndex = properties["currentClipIndex"].GetUint();
                        if (properties.HasMember("currentTime"))
                            animator.currentTime = properties["currentTime"].GetFloat();
                        if (properties.HasMember("playbackSpeed"))
                            animator.playbackSpeed = properties["playbackSpeed"].GetFloat();
                    }
                }
            }
        }

        LOG_INFO("Scene deserialized successfully");
        return true;
    }

} // namespace Engine
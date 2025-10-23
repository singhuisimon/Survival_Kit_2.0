#include "SceneSerializer.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/CameraComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/RigidbodyComponent.h"
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
                propertiesObj.AddMember("FOV", camera.FOV, allocator);
                propertiesObj.AddMember("NearClip", camera.NearClip, allocator);
                propertiesObj.AddMember("FarClip", camera.FarClip, allocator);
                propertiesObj.AddMember("Primary", camera.Primary, allocator);

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
                propertiesObj.AddMember("Visible", mesh.Visible, allocator);
                propertiesObj.AddMember("MeshType", mesh.MeshType, allocator);
                propertiesObj.AddMember("Material", mesh.Material, allocator);
                propertiesObj.AddMember("Texture", mesh.Texture, allocator);

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
                propertiesObj.AddMember("Reverb", audio.Reverb, allocator);
                propertiesObj.AddMember("Is3D", audio.Is3D, allocator);
                propertiesObj.AddMember("MinDistance", audio.MinDistance, allocator);
                propertiesObj.AddMember("MaxDistance", audio.MaxDistance, allocator);

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

            // Create entity
            Entity entity = m_Scene->CreateEntity(entityName);

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
                    }
                    else if (componentType == "CameraComponent") {
                        auto& camera = entity.AddComponent<CameraComponent>();
                        if (properties.HasMember("FOV")) camera.FOV = properties["FOV"].GetFloat();
                        if (properties.HasMember("NearClip")) camera.NearClip = properties["NearClip"].GetFloat();
                        if (properties.HasMember("FarClip")) camera.FarClip = properties["FarClip"].GetFloat();
                        if (properties.HasMember("Primary")) camera.Primary = properties["Primary"].GetBool();
                    }
                    else if (componentType == "MeshRendererComponent") {
                        auto& mesh = entity.AddComponent<MeshRendererComponent>();
                        if (properties.HasMember("Visible")) mesh.Visible = properties["Visible"].GetBool();
                        if (properties.HasMember("MeshType")) mesh.MeshType = properties["MeshType"].GetUint();
                        if (properties.HasMember("Material")) mesh.Material = properties["Material"].GetUint();
                        if (properties.HasMember("Texture")) mesh.Texture = properties["Texture"].GetUint();
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

                        if(properties.HasMember("FilePath"))
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
							audio.Reverb = properties["Reverb"].GetBool();
						if (properties.HasMember("Is3D"))
							audio.Is3D = properties["Is3D"].GetBool();
						if (properties.HasMember("MinDistance"))
							audio.MinDistance = properties["MinDistance"].GetFloat();
						if (properties.HasMember("MaxDistance"))
							audio.MaxDistance = properties["MaxDistance"].GetFloat();
                    }
                    else if (componentType == "ListenerComponent") {
                        auto& listener = entity.AddComponent<ListenerComponent>();

                        if (properties.HasMember("Active"))
                            listener.Active = properties["Active"].GetBool();
                    }
                }
            }
        }

        LOG_INFO("Scene deserialized successfully");
        return true;
    }

} // namespace Engine
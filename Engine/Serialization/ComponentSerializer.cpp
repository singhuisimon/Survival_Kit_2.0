#include "ComponentSerializer.h"
#include "../ECS/Entity.h"
#include "../Asset/AssetManager.h"
#include "../Prefab/PrefabRegistry.h"

#include "../Component/PrefabComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/TagComponent.h"
#include "../Component/CameraComponent.h"
#include "../Component/RigidbodyComponent.h"
#include "../Component/AudioComponent.h"
#include "../Component/ListenerComponent.h"
#include "../Component/LightComponent.h"
#include "../Component/AnimatorComponent.h"
#include "../Component/ReverbZoneComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/BehaviourTreeComponent.h"
#include "../Component/ScriptComponent.h"

#include "../Utility/Logger.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>  // Core quaternion support
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


namespace Engine
{
    using namespace rapidjson;

    std::string ComponentSerializer::SerializeComponent(Entity entity, ComponentTypeID type)
    {
        Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        Value propertiesObj(kObjectType);

        switch (type) {
        case ComponentTypeID::Tag: {
            if (!entity.HasComponent<TagComponent>()) {
                return "{}";
            }

            auto& tag = entity.GetComponent<TagComponent>();
            propertiesObj.AddMember("ComponentGUID",
                Value(std::to_string(tag.ComponentGUID.m_Value).c_str(), allocator),
                allocator);
            propertiesObj.AddMember("Tag", Value(tag.Tag.c_str(), allocator),
                allocator);

            break;
        }
        case ComponentTypeID::Transform: {
            if (!entity.HasComponent<TransformComponent>()) {
                return "{}";
            }

            auto& transform = entity.GetComponent<TransformComponent>();

            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(transform.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );

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

            // Children
            Value childrenArray(kArrayType);
            for (const auto& child : transform.Children) {
                childrenArray.PushBack(child, allocator);
            }
            propertiesObj.AddMember("Children", childrenArray, allocator);

            break;
        }
        case ComponentTypeID::MeshRenderer: {
            if (!entity.HasComponent<MeshRendererComponent>()) {
                return "{}";
            }
            auto& mesh = entity.GetComponent<MeshRendererComponent>();
            std::string meshFilename = AM.getNameFromGuid(mesh.MeshGuid);
            std::string materialFilename = AM.getNameFromGuid(mesh.MaterialGuid);
            std::string textureFilename = AM.getNameFromGuid(mesh.TextureGuid);
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(mesh.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
            propertiesObj.AddMember("Mesh",
                Value(meshFilename.empty() ? "" : meshFilename.c_str(), allocator),
                allocator);
            propertiesObj.AddMember("Material",
                Value(materialFilename.empty() ? "" : materialFilename.c_str(), allocator),
                allocator);
            propertiesObj.AddMember("Texture",
                Value(textureFilename.empty() ? "" : textureFilename.c_str(), allocator),
                allocator);
            propertiesObj.AddMember("Visible", mesh.Visible, allocator);
            propertiesObj.AddMember("ShadowCast", mesh.ShadowCast, allocator);
            propertiesObj.AddMember("ShadowReceive", mesh.ShadowReceive, allocator);
            propertiesObj.AddMember("GlobalIlluminate", mesh.GlobalIlluminate, allocator);
            propertiesObj.AddMember("MeshType", mesh.MeshType, allocator);
            propertiesObj.AddMember("MaterialIdx", mesh.Material, allocator);
            propertiesObj.AddMember("TextureIdx", mesh.Texture, allocator);
            propertiesObj.AddMember("SubmeshIndex", mesh.SubmeshIndex, allocator);

            break;
        }
        case ComponentTypeID::Camera: {
            if (!entity.HasComponent<CameraComponent>()) {
                return "{}";
            }
            auto& camera = entity.GetComponent<CameraComponent>();
       
            Value propertiesObj(kObjectType);
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(camera.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
            propertiesObj.AddMember("Enabled", camera.Enabled, allocator);
            propertiesObj.AddMember("Primary", camera.Primary, allocator);
            propertiesObj.AddMember("Projection", camera.Projection, allocator);
            propertiesObj.AddMember("autoAspect", camera.autoAspect, allocator);

            Value sizeArr(kArrayType);
            sizeArr.PushBack(camera.Size.x, allocator);
            sizeArr.PushBack(camera.Size.y, allocator);
            propertiesObj.AddMember("Size", sizeArr, allocator);
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
            break;
        }
        case ComponentTypeID::RigidBody: {
            if (!entity.HasComponent<RigidbodyComponent>()) {
                return "{}";
            }
            auto& rb = entity.GetComponent<RigidbodyComponent>();

            // FIXED: Create propertiesObj properly (it was already declared above in your code)
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(rb.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
            propertiesObj.AddMember("Mass", rb.Mass, allocator);
            propertiesObj.AddMember("IsKinematic", rb.IsKinematic, allocator);
            propertiesObj.AddMember("IsTrigger", rb.IsTrigger, allocator);
            propertiesObj.AddMember("UseGravity", rb.UseGravity, allocator);

            Value velArray(kArrayType);
            velArray.PushBack(rb.Velocity.x, allocator);
            velArray.PushBack(rb.Velocity.y, allocator);
            velArray.PushBack(rb.Velocity.z, allocator);
            propertiesObj.AddMember("Velocity", velArray, allocator);

            Value angVel(kArrayType);
            angVel.PushBack(rb.AngularVelocity.x, allocator);
            angVel.PushBack(rb.AngularVelocity.y, allocator);
            angVel.PushBack(rb.AngularVelocity.z, allocator);
            propertiesObj.AddMember("AngularVelocity", angVel, allocator);

            propertiesObj.AddMember("LinearDamping", rb.LinearDamping, allocator);
            propertiesObj.AddMember("AngularDamping", rb.AngularDamping, allocator);
            propertiesObj.AddMember("Restitution", rb.Restitution, allocator);

            propertiesObj.AddMember("CollideType", static_cast<int>(rb.Shape), allocator);

            Value boxHalfExtent(kArrayType);
            boxHalfExtent.PushBack(rb.BoxHalfExtents.x, allocator);
            boxHalfExtent.PushBack(rb.BoxHalfExtents.y, allocator);
            boxHalfExtent.PushBack(rb.BoxHalfExtents.z, allocator);
            propertiesObj.AddMember("BoxHalfExtent", boxHalfExtent, allocator);

            propertiesObj.AddMember("SphereRadius", rb.SphereRadius, allocator);
            break; // CRITICAL: Add this break!
        
        }
        case ComponentTypeID::Audio:{
            if (!entity.HasComponent<AudioComponent>()) {
                return "{}";
            }
            auto& audio = entity.GetComponent<AudioComponent>();
            Value propertiesObj(kObjectType);
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(audio.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
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
            break;
        }
        case ComponentTypeID::Listerner: {
            if (!entity.HasComponent<ListenerComponent>()) {
                return "{}";
            }
            auto& listener = entity.GetComponent<ListenerComponent>();
            Value propertiesObj(kObjectType);
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(listener.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
            propertiesObj.AddMember("Active", listener.Active, allocator);
            break;
        }
        case ComponentTypeID::Light: {
            if (!entity.HasComponent<LightComponent>()) {
                return "{}";
            }
            auto& light = entity.GetComponent<LightComponent>();
            Value componentObj(kObjectType);
            componentObj.AddMember("Type", "LightComponent", allocator);

            Value propertiesObj(kObjectType);
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(light.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
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
            break;
        }
        case ComponentTypeID::Animator:
        {
            if (!entity.HasComponent<AnimatorComponent>()) {
                return "{}";
            }
            auto& animator = entity.GetComponent<AnimatorComponent>();

            // FIXED: Properly add to propertiesObj
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(animator.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
            propertiesObj.AddMember("playing", animator.playing, allocator);
            propertiesObj.AddMember("respectClipLoop", animator.respectClipLoop, allocator);
            propertiesObj.AddMember("controller", animator.controller, allocator);
            propertiesObj.AddMember("currentClipIndex", animator.currentClipIndex, allocator);
            propertiesObj.AddMember("currentTime", animator.currentTime, allocator);
            propertiesObj.AddMember("playbackSpeed", animator.playbackSpeed, allocator);
            break;
        }
        case ComponentTypeID::ReverbZone: {
            if (!entity.HasComponent<ReverbZoneComponent>()) {
                return "{}";
            }
            auto& reverb = entity.GetComponent<ReverbZoneComponent>();
       
            Value propertiesObj(kObjectType);
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(reverb.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
            propertiesObj.AddMember("Preset", static_cast<int>(reverb.Preset), allocator);
            propertiesObj.AddMember("MinDistance", reverb.MinDistance, allocator);
            propertiesObj.AddMember("MaxDistance", reverb.MaxDistance, allocator);
            propertiesObj.AddMember("DecayTime", reverb.DecayTime, allocator);
            propertiesObj.AddMember("HfDecayRatio", reverb.HfDecayRatio, allocator);
            propertiesObj.AddMember("Diffusion", reverb.Diffusion, allocator);
            propertiesObj.AddMember("Density", reverb.Density, allocator);
            propertiesObj.AddMember("WetLevel", reverb.WetLevel, allocator);
            break;
        }
        case ComponentTypeID::ParticleSystem: {
            if (!entity.HasComponent<ParticleComponent>()) {
                return "{}";
            }
            auto& emitter = entity.GetComponent<ParticleComponent>();
            propertiesObj.AddMember(
                "ComponentGUID",
                Value(std::to_string(emitter.ComponentGUID.m_Value).c_str(), allocator),
                allocator
            );
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
            break;
        }
        default:
            return "{}";
        }

        doc.AddMember("Properties", propertiesObj, allocator);

        // Convert to string
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    bool ComponentSerializer::DeserializeComponent(Entity entity, ComponentTypeID type, const std::string& jsonString) {
        if (jsonString.empty() || jsonString == "{}") {
            LOG_WARNING("Empty JSON string for component type: ", static_cast<u32>(type));
            return false;
        }

        Document doc;
        doc.Parse(jsonString.c_str());

        if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("Properties")) {
            LOG_ERROR("Failed to parse JSON or missing Properties for component type: ", static_cast<u32>(type));
            return false;
        }

        const Value& properties = doc["Properties"];

        switch (type) {
        case ComponentTypeID::Tag: {
            if (!entity.HasComponent<TagComponent>()) {
                entity.AddComponent<TagComponent>();
            }

            auto& tag = entity.GetComponent<TagComponent>();

            if (properties.HasMember("ComponentGUID")) {
                tag.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }

            if (properties.HasMember("Tag")) {
                tag.Tag = properties["Tag"].GetString();
            }

            return true;
        }

        case ComponentTypeID::Prefab: {
            
            return true;
        }

        case ComponentTypeID::Transform: {
            if (!entity.HasComponent<TransformComponent>()) {
                entity.AddComponent<TransformComponent>();
            }

            auto& transform = entity.GetComponent<TransformComponent>();

            if (properties.HasMember("ComponentGUID")) {
                transform.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }

            // IMPORTANT: Use setters so the scene updates properly!

            if (properties.HasMember("Position")) {
                const Value& posArray = properties["Position"];
                glm::vec3 pos(
                    posArray[0].GetFloat(),
                    posArray[1].GetFloat(),
                    posArray[2].GetFloat()
                );
                transform.SetPosition(pos);  // USE SETTER!
                LOG_DEBUG("  Set Position to: ", pos.x, ", ", pos.y, ", ", pos.z);
            }

            if (properties.HasMember("Rotation")) {
                const Value& rotArray = properties["Rotation"];
                glm::vec3 eulerRotation(
                    rotArray[0].GetFloat(),
                    rotArray[1].GetFloat(),
                    rotArray[2].GetFloat()
                );
                glm::quat quat = glm::quat(glm::radians(eulerRotation));
                transform.SetRotation(eulerRotation);  // USE SETTER with degrees!
                LOG_DEBUG("  Set Rotation to: ", eulerRotation.x, ", ", eulerRotation.y, ", ", eulerRotation.z);
            }

            if (properties.HasMember("Scale")) {
                const Value& scaleArray = properties["Scale"];
                glm::vec3 scale(
                    scaleArray[0].GetFloat(),
                    scaleArray[1].GetFloat(),
                    scaleArray[2].GetFloat()
                );
                transform.SetScale(scale);  // USE SETTER!
                LOG_DEBUG("  Set Scale to: ", scale.x, ", ", scale.y, ", ", scale.z);
            }

            LOG_DEBUG("Transform component deserialized and applied to scene");
            return true;
        }

        case ComponentTypeID::MeshRenderer: {
            if (!entity.HasComponent<MeshRendererComponent>()) {
                entity.AddComponent<MeshRendererComponent>();
            }

            auto& mesh = entity.GetComponent<MeshRendererComponent>();
            if (properties.HasMember("ComponentGUID")) {
                mesh.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }

            if (properties.HasMember("Mesh") && properties["Mesh"].IsString()) {
                std::string meshName = properties["Mesh"].GetString();
                mesh.MeshGuid = AM.getGuidFromName(meshName);
                LOG_DEBUG("  Set Mesh to: ", meshName);
            }

            if (properties.HasMember("Material") && properties["Material"].IsString()) {
                std::string matName = properties["Material"].GetString();
                mesh.MaterialGuid = AM.getGuidFromName(matName);
                LOG_DEBUG("  Set Material to: ", matName);
            }

            if (properties.HasMember("Texture") && properties["Texture"].IsString()) {
                std::string texName = properties["Texture"].GetString();
                mesh.TextureGuid = AM.getGuidFromName(texName);
                LOG_DEBUG("  Set Texture to: ", texName);
            }

            if (properties.HasMember("Visible")) {
                mesh.Visible = properties["Visible"].GetBool();
                LOG_DEBUG("  Set Visible to: ", mesh.Visible);
            }

            if (properties.HasMember("ShadowCast")) {
                mesh.ShadowCast = properties["ShadowCast"].GetBool();
                LOG_DEBUG("  Set ShadowCast to: ", mesh.ShadowCast);
            }

            if (properties.HasMember("ShadowReceive")) {
                mesh.ShadowReceive = properties["ShadowReceive"].GetBool();
                LOG_DEBUG("  Set ShadowReceive to: ", mesh.ShadowReceive);
            }

            if (properties.HasMember("GlobalIlluminate")) {
                mesh.GlobalIlluminate = properties["GlobalIlluminate"].GetBool();
                LOG_DEBUG("  Set GlobalIlluminate to: ", mesh.GlobalIlluminate);
            }

            if (properties.HasMember("MeshType")) {
                mesh.MeshType = properties["MeshType"].GetUint();
                LOG_DEBUG("  Set MeshType to: ", mesh.MeshType);
            }

            if (properties.HasMember("Material") && properties["Material"].IsNumber()) {
                mesh.Material = properties["Material"].GetUint();
            }
            else if (properties.HasMember("MaterialIdx")) {
                mesh.Material = properties["MaterialIdx"].GetUint();
            }

            if (properties.HasMember("Texture") && properties["Texture"].IsNumber()) {
                mesh.Texture = properties["Texture"].GetUint();
            }
            else if (properties.HasMember("TextureIdx")) {
                mesh.Texture = properties["TextureIdx"].GetUint();
            }

            if (properties.HasMember("SubmeshIndex")) {
                mesh.SubmeshIndex = properties["SubmeshIndex"].GetUint();
                LOG_DEBUG("  Set SubmeshIndex to: ", mesh.SubmeshIndex);
            }

            LOG_DEBUG("MeshRenderer component deserialized and applied to scene");
            return true;
        }
        case ComponentTypeID::Camera: {
            if (!entity.HasComponent<CameraComponent>()) {
                entity.AddComponent<CameraComponent>();
            }

            auto& camera = entity.GetComponent<CameraComponent>();
            if (properties.HasMember("Enabled"))
                camera.Enabled = properties["Enabled"].GetBool();
            if (properties.HasMember("Primary"))
                camera.Primary = properties["Primary"].GetBool();
            if (properties.HasMember("Projection"))
                camera.Projection = properties["Projection"].GetBool();
            if (properties.HasMember("autoAspect"))
                camera.autoAspect = properties["autoAspect"].GetBool();

            if (properties.HasMember("Size"))
            {
                const Value& size = properties["Size"];
                camera.Size = glm::vec2(
                    size[0].GetFloat(),
                    size[1].GetFloat()
                );
            }
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

            if (properties.HasMember("Target"))
            {
                const Value& target = properties["Target"];
                camera.Target = glm::vec3(
                    target[0].GetFloat(),
                    target[1].GetFloat(),
                    target[2].GetFloat()
                );
            }
            return true;
        }

        case ComponentTypeID::RigidBody: {
            if (!entity.HasComponent<RigidbodyComponent>()) {
                entity.AddComponent<RigidbodyComponent>();
            }

            auto& rb = entity.GetComponent<RigidbodyComponent>();
            if (properties.HasMember("ComponentGUID"))
            {
                rb.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }
            if (properties.HasMember("Mass"))
                rb.Mass = properties["Mass"].GetFloat();
            if (properties.HasMember("IsKinematic"))
                rb.IsKinematic = properties["IsKinematic"].GetBool();
            if (properties.HasMember("IsTrigger"))
                rb.IsTrigger = properties["IsTrigger"].GetBool();
            if (properties.HasMember("UseGravity"))
                rb.UseGravity = properties["UseGravity"].GetBool();

            if (properties.HasMember("Velocity"))
            {
                const Value& velArray = properties["Velocity"];
                rb.Velocity = glm::vec3(
                    velArray[0].GetFloat(),
                    velArray[1].GetFloat(),
                    velArray[2].GetFloat()
                );
            }
            if (properties.HasMember("AngularVelocity"))
            {
                const Value& angVel = properties["AngularVelocity"];
                rb.AngularVelocity = glm::vec3(
                    angVel[0].GetFloat(),
                    angVel[1].GetFloat(),
                    angVel[2].GetFloat()
                );
            }
            if (properties.HasMember("LinearDamping"))
                rb.LinearDamping = properties["LinearDamping"].GetFloat();
            if (properties.HasMember("AngularDamping"))
                rb.AngularDamping = properties["AngularDamping"].GetFloat();
            if (properties.HasMember("Restitution"))
                rb.Restitution = properties["Restitution"].GetFloat();
            if (properties.HasMember("CollideType"))
                rb.Shape = static_cast<ColliderType>(properties["CollideType"].GetInt());
            if (properties.HasMember("BoxHalfExtent"))
            {
                const Value& boxHalfExtent = properties["BoxHalfExtent"];
                rb.BoxHalfExtents = glm::vec3(
                    boxHalfExtent[0].GetFloat(),
                    boxHalfExtent[1].GetFloat(),
                    boxHalfExtent[2].GetFloat()
                );
            }
            if (properties.HasMember("SphereRadius"))
                rb.SphereRadius = properties["SphereRadius"].GetFloat();
            return true;
        }
        case ComponentTypeID::Audio: {
            if (!entity.HasComponent<AudioComponent>()) {
                entity.AddComponent<AudioComponent>();
            }
            auto& audio = entity.GetComponent<AudioComponent>();
            if (properties.HasMember("ComponentGUID"))
            {
                audio.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }
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
            return true;
        }
        case ComponentTypeID::Listerner: {
            if (!entity.HasComponent<ListenerComponent>()) {
                entity.AddComponent<ListenerComponent>();
            }
           
            auto& listener = entity.GetComponent<ListenerComponent>();
            if (properties.HasMember("ComponentGUID"))
            {
                listener.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }
            if (properties.HasMember("Active"))
                listener.Active = properties["Active"].GetBool();
            return true;
        }
        case ComponentTypeID::Light: {
            if (!entity.HasComponent<LightComponent>()) {
                entity.AddComponent<LightComponent>();
            }
            auto& light = entity.GetComponent<LightComponent>();
            if (properties.HasMember("ComponentGUID"))
            {
                light.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }
            if (properties.HasMember("Enabled"))
                light.Enabled = properties["Enabled"].GetBool();
            if (properties.HasMember("Type"))
                light.Type = static_cast<LightType>(properties["Type"].GetInt()); // 0=Dir,1=Point,2=Spot
            // Optional: Mode is usually omitted in scene files (Realtime only), but handle if present
            //if (properties.HasMember("Mode"))
            //    light.Mode = static_cast<LightMode>(properties["Mode"].GetInt());
            if (properties.HasMember("Color") && properties["Color"].IsArray())
            {
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
            return true;
        }

        case ComponentTypeID::Animator: {
           if (!entity.HasComponent<AnimatorComponent>()) {
                entity.AddComponent<AnimatorComponent>();
           }
           /*if (entity.HasComponent<AnimatorComponent>())
           {*/
               auto& animator = entity.GetComponent<AnimatorComponent>();
               if (properties.HasMember("ComponentGUID"))
               {
                   animator.ComponentGUID = xresource::instance_guid(
                       std::stoull(properties["ComponentGUID"].GetString())
                   );
               }
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
           //}
            return true;
        }

        case ComponentTypeID::ReverbZone: {
            if (!entity.HasComponent<ReverbZoneComponent>()) {
                entity.AddComponent<ReverbZoneComponent>();
            }
            auto& reverb = entity.GetComponent<ReverbZoneComponent>();
            
            if (properties.HasMember("ComponentGUID"))
            {
                reverb.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }
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
            return true;
        }

        case ComponentTypeID::BehaviourTree: {
            LOG_WARNING("BehaviourTree deserialization not fully implemented - skipping");
            return true;
        }

        case ComponentTypeID::ParticleSystem: {
            if (!entity.HasComponent<ParticleComponent>()) {
                entity.AddComponent<ParticleComponent>();
            }
            auto& emitter = entity.GetComponent<ParticleComponent>();
            if (properties.HasMember("ComponentGUID"))
            {
                emitter.ComponentGUID = xresource::instance_guid(
                    std::stoull(properties["ComponentGUID"].GetString())
                );
            }
            // Initial Velocity
            if (properties.HasMember("Initial Velocity") && properties["Initial Velocity"].IsArray())
            {
                const auto& velArray = properties["Initial Velocity"].GetArray();
                if (velArray.Size() >= 3)
                {
                    emitter.InitialVelocity.x = velArray[0].GetFloat();
                    emitter.InitialVelocity.y = velArray[1].GetFloat();
                    emitter.InitialVelocity.z = velArray[2].GetFloat();
                }
            }

            // Color Min
            if (properties.HasMember("Color Min") && properties["Color Min"].IsArray())
            {
                const auto& minColorArray = properties["Color Min"].GetArray();
                if (minColorArray.Size() >= 3)
                {
                    emitter.ColorMin.x = minColorArray[0].GetFloat();
                    emitter.ColorMin.y = minColorArray[1].GetFloat();
                    emitter.ColorMin.z = minColorArray[2].GetFloat();
                }
            }

            // Color Max
            if (properties.HasMember("Color Max") && properties["Color Max"].IsArray())
            {
                const auto& maxColorArray = properties["Color Max"].GetArray();
                if (maxColorArray.Size() >= 3)
                {
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
            return true;
        }

        case ComponentTypeID::Script: {
            LOG_WARNING("Script deserialization not fully implemented - skipping");
            return true;
        }
        default:
            LOG_WARNING("Unknown component type: ", static_cast<u32>(type));
            return false;
        }
    }

    const char* ComponentSerializer::GetComponentTypeName(ComponentTypeID type)
    {
        switch (type) {
        case ComponentTypeID::Transform: return "TransformComponent";
        case ComponentTypeID::MeshRenderer: return "MeshRendererComponent";
        case ComponentTypeID::Camera: return "CameraComponent";
        case ComponentTypeID::Light: return "LightComponent";
        case ComponentTypeID::ParticleSystem: return "ParticleComponent";
        case ComponentTypeID::RigidBody: return "RigidBodyComponent";
        case ComponentTypeID::Script: return "ScriptComponent";
        case ComponentTypeID::Audio: return "AudioComponent";
        case ComponentTypeID::Listerner: return "ListenerComponent";
        case ComponentTypeID::Animator: return "AnimatorComponent";
        case ComponentTypeID::ReverbZone: return "ReverbZoneComponent";
        case ComponentTypeID::BehaviourTree: return "BehaviourTreeComponent";
        case ComponentTypeID::Prefab: return "PrefabComponent";
        case ComponentTypeID::Tag: return "TagComponent";
        default: return "UnknownComponent";
        }
    }
   
}
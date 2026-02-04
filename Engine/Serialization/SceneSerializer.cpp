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
#include "../Component/SpriteRendererComponent.h"
#include "../Component/TextComponent.h"
#include "../Component/TrailComponent.h"

#include "../Scripting/ScriptSerializer.h"
#include "../Scripting/MonoScriptEngine.h"
#include "ReflectionRegistry.h"
#include "../Utility/Logger.h"
#include "../Asset/AssetManager.h"

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

	SceneSerializer::SceneSerializer(Scene *scene)
		: m_Scene(scene) {}

	bool SceneSerializer::Serialize(const std::string &filepath) {
		LOG_INFO("Serializing scene to: ", filepath);

		std::string jsonString = SerializeToString();

		// Write to file
		std::ofstream file(filepath);
		if(!file.is_open()) {
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
		auto &allocator = doc.GetAllocator();

		// Scene metadata
		LOG_TRACE("Adding scene metadata...");
		doc.AddMember("Scene", Value(m_Scene->GetName().c_str(), allocator), allocator);
		doc.AddMember("Version", "1.0", allocator);

		// Entities array
		LOG_TRACE("Creating entities array...");
		Value entitiesArray(kArrayType);

		auto &registry = m_Scene->GetRegistry();
		auto view = registry.view<TagComponent>();

		LOG_TRACE("Found ", (int)view.size(), " entities to serialize");

		int entityIndex = 0;
		for(auto entityHandle : view) {
			LOG_TRACE("Serializing entity ", entityIndex++);

			Entity entity(entityHandle, &registry);
			Value entityObj(kObjectType);

			// Entity ID
			entityObj.AddMember("ID", (uint32_t)entity, allocator);

			// Components array
			Value componentsArray(kArrayType);

			// Serialize TagComponent
			if(entity.HasComponent<TagComponent>()) {
				LOG_TRACE("  - Serializing TagComponent");
				auto &tag = entity.GetComponent<TagComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "TagComponent", allocator);

				Value propertiesObj(kObjectType);
				propertiesObj.AddMember("Name", Value(tag.Name.c_str(), allocator), allocator);
				propertiesObj.AddMember("Tag", Value(tag.Tag.c_str(), allocator), allocator);
				componentObj.AddMember("Properties", propertiesObj, allocator);

				componentsArray.PushBack(componentObj, allocator);
			}
			if(entity.HasComponent<PrefabComponent>()) {
				LOG_TRACE("  - Serializing PrefabComponent");
				auto &prefabComp = entity.GetComponent<PrefabComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "PrefabComponent", allocator);

				Value propertiesObj(kObjectType);

				// Basic prefab info
				propertiesObj.AddMember("ComponentGUID",
										Value(std::to_string(prefabComp.ComponentGUID.m_Value).c_str(), allocator),
										allocator);

				propertiesObj.AddMember("PrefabAssetGuid",
										Value(std::to_string(prefabComp.PrefabAssetGuid.m_Value).c_str(), allocator),
										allocator);

				propertiesObj.AddMember("isPrefabRoot", prefabComp.isPrefabRoot, allocator);
				propertiesObj.AddMember("isNestedPrefab", prefabComp.isNestedPrefab, allocator);

				propertiesObj.AddMember("parentPrefabGuid",
										Value(std::to_string(prefabComp.parentPrefabGuid.m_Value).c_str(), allocator),
										allocator);

				propertiesObj.AddMember("prefabName",
										Value(prefabComp.prefabName.c_str(), allocator),
										allocator);

				propertiesObj.AddMember("prefabVersion", prefabComp.prefabVersion, allocator);


				// NEW: Serialize the prefab file path
				if(prefabComp.isPrefabRoot) {
					std::string prefabPath = PrefabRegistry::Get().GetPrefabPath(prefabComp.PrefabAssetGuid);
					if(!prefabPath.empty()) {
						propertiesObj.AddMember("prefabFilePath",
												Value(prefabPath.c_str(), allocator),
												allocator);
						LOG_DEBUG("Serialized prefab path: ", prefabPath);
					}
				}
				// Serialize component overrides
				if(!prefabComp.componentOverrides.empty()) {
					Value overridesArray(kArrayType);

					for(const auto &override : prefabComp.componentOverrides) {
						Value overrideObj(kObjectType);

						overrideObj.AddMember("componentType",
											  static_cast<uint32_t>(override.componentType),
											  allocator);

						overrideObj.AddMember("isAddedComponent",
											  override.isAddedComponent,
											  allocator);

						overrideObj.AddMember("isRemovedComponent",
											  override.isRemovedComponent,
											  allocator);

						// Store original component JSON
						if(!override.originalComponentJSON.empty()) {
							overrideObj.AddMember("originalComponentJSON",
												  Value(override.originalComponentJSON.c_str(), allocator),
												  allocator);
						}

						// Store current component JSON (if tracking)
						if(!override.currentComponentJSON.empty()) {
							overrideObj.AddMember("currentComponentJSON",
												  Value(override.currentComponentJSON.c_str(), allocator),
												  allocator);
						}

						// Store modified property names
						if(!override.modifiedPropertyNames.empty()) {
							Value propsArray(kArrayType);
							for(const auto &propName : override.modifiedPropertyNames) {
								propsArray.PushBack(Value(propName.c_str(), allocator), allocator);
							}
							overrideObj.AddMember("modifiedPropertyNames", propsArray, allocator);
						}

						overridesArray.PushBack(overrideObj, allocator);
					}


					propertiesObj.AddMember("componentOverrides", overridesArray, allocator);
				}

				// Serialize child entity IDs
				if(!prefabComp.childEntityIDs.empty()) {
					Value childrenArray(kArrayType);
					for(u32 childID : prefabComp.childEntityIDs) {
						childrenArray.PushBack(childID, allocator);
					}
					propertiesObj.AddMember("childEntityIDs", childrenArray, allocator);
				}

				// Serialize deleted entities
				if(!prefabComp.deletedEntities.empty()) {
					Value deletedArray(kArrayType);

					for(const auto &deleted : prefabComp.deletedEntities) {
						Value deletedObj(kObjectType);

						deletedObj.AddMember("prefabLocalID", deleted.prefabLocalID, allocator);
						deletedObj.AddMember("entityName",
											 Value(deleted.entityName.c_str(), allocator), allocator);
						deletedObj.AddMember("serializedEntityData",
											 Value(deleted.serializedEntityData.c_str(), allocator), allocator);

						deletedArray.PushBack(deletedObj, allocator);
					}

					propertiesObj.AddMember("deletedEntities", deletedArray, allocator);
				}

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);

			}
			// Serialize TransformComponent
			if(entity.HasComponent<TransformComponent>()) {
				LOG_TRACE("  - Serializing TransformComponent");
				auto &transform = entity.GetComponent<TransformComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "TransformComponent", allocator);

				Value propertiesObj(kObjectType);
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

				// List of children 
				Value childrenArray(kArrayType);
				for(const auto &child : transform.Children) {
					childrenArray.PushBack(child, allocator);
				}
				propertiesObj.AddMember("Children", childrenArray, allocator);

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			// Serialize CameraComponent
			if(entity.HasComponent<CameraComponent>()) {
				LOG_TRACE("  - Serializing CameraComponent");
				auto &camera = entity.GetComponent<CameraComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "CameraComponent", allocator);

				Value propertiesObj(kObjectType);
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

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			// Serialize MeshRendererComponent
			if(entity.HasComponent<MeshRendererComponent>()) {
				LOG_TRACE("  - Serializing MeshRendererComponent");
				auto &mesh = entity.GetComponent<MeshRendererComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "MeshRendererComponent", allocator);

				Value propertiesObj(kObjectType);

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
				//LOG_DEBUG("TextureGuid in serializer SerializeToString(): ", mesh.TextureGuid.m_Value);
				propertiesObj.AddMember("Visible", mesh.Visible, allocator);
				propertiesObj.AddMember("GlobalIlluminate", mesh.GlobalIlluminate, allocator);
				propertiesObj.AddMember("ShadowReceive", mesh.ShadowReceive, allocator);
				propertiesObj.AddMember("MeshType", mesh.MeshType, allocator);
				propertiesObj.AddMember("MaterialIdx", mesh.Material, allocator);
				propertiesObj.AddMember("TextureIdx", mesh.Texture, allocator);
				propertiesObj.AddMember("SubmeshIndex", mesh.SubmeshIndex, allocator);
				propertiesObj.AddMember("CastType", static_cast<int>(mesh.CastType), allocator);
				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			// Serialize RigidbodyComponent
// Serialize RigidbodyComponent
			if(entity.HasComponent<RigidbodyComponent>()) {
				LOG_TRACE("  - Serializing RigidbodyComponent");
				auto &rb = entity.GetComponent<RigidbodyComponent>();
				Value componentObj(kObjectType);

				componentObj.AddMember("Type", "RigidbodyComponent", allocator);

				Value propertiesObj(kObjectType);
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

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			// Serialize AudioComponent
			if(entity.HasComponent<AudioComponent>()) {
				LOG_TRACE("  - Serializing AudioComponent");
				auto &audio = entity.GetComponent<AudioComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "AudioComponent", allocator);

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

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			// Serialize ListenerComponent
			if(entity.HasComponent<ListenerComponent>()) {
				LOG_TRACE("  - Serializing ListenerComponent");
				auto &listener = entity.GetComponent<ListenerComponent>();

				Value componentObj(kObjectType);

				componentObj.AddMember("Type", "ListenerComponent", allocator);

				Value propertiesObj(kObjectType);
				propertiesObj.AddMember(
					"ComponentGUID",
					Value(std::to_string(listener.ComponentGUID.m_Value).c_str(), allocator),
					allocator
				);
				propertiesObj.AddMember("Active", listener.Active, allocator);

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			// Serialize ReverbComponent
			if(entity.HasComponent<ReverbZoneComponent>()) {
				LOG_TRACE("  - Serializing ReverbComponent");

				auto &reverb = entity.GetComponent<ReverbZoneComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "ReverbComponent", allocator);

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

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}
			// Serialize BehaviourTreeComponent
			if(entity.HasComponent<BehaviourTreeComponent>()) {
				LOG_TRACE("  - Serializing BehaviourTreeComponent");
				auto &bt = entity.GetComponent<BehaviourTreeComponent>();
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
			if(entity.HasComponent<ParticleComponent>()) {
				LOG_TRACE("  - Serializing Particle Component");

				const auto &emitter = entity.GetComponent<ParticleComponent>();
				rapidjson::Value componentObj(kObjectType);
				componentObj.AddMember("Type", "ParticleComponent", allocator);

				rapidjson::Value propertiesObj(kObjectType);
				propertiesObj.AddMember(
					"ComponentGUID",
					Value(std::to_string(emitter.ComponentGUID.m_Value).c_str(), allocator),
					allocator
				);

				// Particle Type Advanced
				std::string particleTypeAdvancedName = AM.getNameFromGuid(emitter.ParticleTypeAdvanced);
				propertiesObj.AddMember("Particle Type Advanced",
										Value(particleTypeAdvancedName.empty() ? "" : particleTypeAdvancedName.c_str(), allocator),
										allocator);

				// Material Type
				std::string materialTypeName = AM.getNameFromGuid(emitter.MaterialType);
				propertiesObj.AddMember("Material Type",
										Value(materialTypeName.empty() ? "" : materialTypeName.c_str(), allocator),
										allocator);

				// Initial Velocity
				rapidjson::Value velArray(kArrayType);
				velArray.PushBack(emitter.InitialVelocity.x, allocator);
				velArray.PushBack(emitter.InitialVelocity.y, allocator);
				velArray.PushBack(emitter.InitialVelocity.z, allocator);
				propertiesObj.AddMember("Initial Velocity", velArray, allocator);

				// Start Size
				rapidjson::Value startSizeArray(kArrayType);
				startSizeArray.PushBack(emitter.StartSize.x, allocator);
				startSizeArray.PushBack(emitter.StartSize.y, allocator);
				startSizeArray.PushBack(emitter.StartSize.z, allocator);
				propertiesObj.AddMember("Start Size", startSizeArray, allocator);

				// Default Size
				rapidjson::Value defaultSizeArray(kArrayType);
				defaultSizeArray.PushBack(emitter.DefaultSize.x, allocator);
				defaultSizeArray.PushBack(emitter.DefaultSize.y, allocator);
				defaultSizeArray.PushBack(emitter.DefaultSize.z, allocator);
				propertiesObj.AddMember("Default Size", defaultSizeArray, allocator);

				// End Size
				rapidjson::Value endSizeArray(kArrayType);
				endSizeArray.PushBack(emitter.EndSize.x, allocator);
				endSizeArray.PushBack(emitter.EndSize.y, allocator);
				endSizeArray.PushBack(emitter.EndSize.z, allocator);
				propertiesObj.AddMember("End Size", endSizeArray, allocator);

				// Emission Box Size
				rapidjson::Value EmissionBoxSizeArray(kArrayType);
				EmissionBoxSizeArray.PushBack(emitter.EmissionBoxSize.x, allocator);
				EmissionBoxSizeArray.PushBack(emitter.EmissionBoxSize.y, allocator);
				EmissionBoxSizeArray.PushBack(emitter.EmissionBoxSize.z, allocator);
				propertiesObj.AddMember("Emission Box Size", EmissionBoxSizeArray, allocator);

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

				// Emitter Shape
				propertiesObj.AddMember("Emitter Shape", static_cast<u32>(emitter.Shape), allocator);

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

				// Grow Phase End
				propertiesObj.AddMember("Grow Phase End", emitter.GrowPhaseEnd, allocator);

				// Shrink Phase Start
				propertiesObj.AddMember("Shrink Phase Start", emitter.ShrinkPhaseStart, allocator);

				// Emission Sphere Radius
				propertiesObj.AddMember("Emission Sphere Radius", emitter.EmissionSphereRadius, allocator);

				// Randomization parameters
				propertiesObj.AddMember("Velocity Randomness", emitter.VelocityRandomness, allocator);
				propertiesObj.AddMember("Lifetime Randomness", emitter.LifetimeRandomness, allocator);
				propertiesObj.AddMember("Spread Angle", emitter.SpreadAngle, allocator);
				propertiesObj.AddMember("Min Speed", emitter.MinSpeed, allocator);
				propertiesObj.AddMember("Max Speed", emitter.MaxSpeed, allocator);
				propertiesObj.AddMember("Rotation Speed", emitter.RotationSpeed, allocator);

				// Play Delay Parameters
				propertiesObj.AddMember("Play Delay", emitter.PlayDelay, allocator);
				propertiesObj.AddMember("Delay Accumulator", emitter.DelayAccumualator, allocator);

				// Boolean parameters
				propertiesObj.AddMember("Randomize Rotation", emitter.RandomizeRotation, allocator);
				propertiesObj.AddMember("Loop", emitter.Loop, allocator);
				propertiesObj.AddMember("Active", emitter.Active, allocator);
				propertiesObj.AddMember("World Space", emitter.WorldSpace, allocator);
				propertiesObj.AddMember("Burst Mode", emitter.BurstMode, allocator);

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}
			// Serialize ScriptComponent
			if(entity.HasComponent<ScriptComponent>()) {
				LOG_TRACE("  - Serializing ScriptComponent");
				auto &script = entity.GetComponent<ScriptComponent>();

				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "ScriptComponent", allocator);

				Value propertiesObj(kObjectType);

				propertiesObj.AddMember(
					"ComponentGUID",
					Value(std::to_string(script.ComponentGUID.m_Value).c_str(), allocator),
					allocator
				);

				propertiesObj.AddMember(
					"ScriptClassName",
					Value(script.ScriptClassName.c_str(), allocator),
					allocator
				);

				// IMPORTANT:
				// Do NOT serialize ScriptInstance / GCHandle / Started.
				// Do NOT read from ScriptInstance when saving (can be stale during hotreload/STOP).
				// Persist only the cached SerializedFields.
				if(!script.SerializedFields.empty()) {
					rapidjson::Value fieldsObj(kObjectType);
					SerializeScriptFieldsFromComponentToRapidJSON(script, fieldsObj, allocator);
					propertiesObj.AddMember("Fields", fieldsObj, allocator);
				}

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			// Serialize LightComponent
			if(entity.HasComponent<LightComponent>()) {
				LOG_TRACE("  - Serializing LightComponent");
				auto &light = entity.GetComponent<LightComponent>();
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
				propertiesObj.AddMember("TypeShadow", static_cast<int>(light.TypeShadow), allocator);
				propertiesObj.AddMember("Resolution", light.Resolution, allocator);
				propertiesObj.AddMember("Strength", light.Strength, allocator);
				propertiesObj.AddMember("Bias", light.Bias, allocator);
				propertiesObj.AddMember("NearPlane", light.NearPlane, allocator);

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}
			// Serialize AnimatorComponent
			if(entity.HasComponent<AnimatorComponent>()) {
				LOG_TRACE("  - Serializing AnimatorComponent");
				auto &animator = entity.GetComponent<AnimatorComponent>();

				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "AnimatorComponent", allocator);

				Value propertiesObj(kObjectType);
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

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}
			// Serialize SpriteRendererComponent
			if(entity.HasComponent<SpriteRendererComponent>()) {
				LOG_TRACE(" - Serializing SpriteRendererComponent");
				auto &SpriteRenderer = entity.GetComponent<SpriteRendererComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "SpriteRendererComponent", allocator);

				Value propertiesObj(kObjectType);

				std::string textureFilename = AM.getNameFromGuid(SpriteRenderer.TextureGuid);

				propertiesObj.AddMember("Texture",
										Value(textureFilename.empty() ? "" : textureFilename.c_str(), allocator),
										allocator);

				Value colorArr(kArrayType);
				colorArr.PushBack(SpriteRenderer.Color.r, allocator);
				colorArr.PushBack(SpriteRenderer.Color.g, allocator);
				colorArr.PushBack(SpriteRenderer.Color.b, allocator);
				colorArr.PushBack(SpriteRenderer.Color.a, allocator);

				propertiesObj.AddMember("Color", colorArr, allocator);
				propertiesObj.AddMember("Quad", SpriteRenderer.Quad, allocator);
				propertiesObj.AddMember("Sprite Layer", SpriteRenderer.SpriteLayer, allocator);
				propertiesObj.AddMember("IsActive", SpriteRenderer.IsActive, allocator);
				propertiesObj.AddMember("IsVisible", SpriteRenderer.IsVisible, allocator);

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}
			//Serialize TextComponent
			if(entity.HasComponent<TextComponent>()) {
				LOG_TRACE(" - Serializing TextComponent");
				auto &textComp = entity.GetComponent<TextComponent>();
				Value componentObj(kObjectType);
				componentObj.AddMember("Type", "TextComponent", allocator);

				Value propertiesObj(kObjectType);

				propertiesObj.AddMember("ComponentGUID",
										Value(std::to_string(textComp.ComponentGUID.m_Value).c_str(), allocator), allocator);

				//text content
				propertiesObj.AddMember("text",
										Value(textComp.text.c_str(), allocator), allocator);

				//font name
				propertiesObj.AddMember("fontName",
										Value(textComp.fontName.c_str(), allocator), allocator);

				//font size
				propertiesObj.AddMember("fontSize", textComp.fontSize, allocator);

				//color
				Value colorArr(kArrayType);
				colorArr.PushBack(textComp.color[0], allocator);
				colorArr.PushBack(textComp.color[1], allocator);
				colorArr.PushBack(textComp.color[2], allocator);
				colorArr.PushBack(textComp.color[3], allocator);
				propertiesObj.AddMember("color", colorArr, allocator);

				//isVisible
				propertiesObj.AddMember("isVisible",
					textComp.isVisible, allocator);


				// Alignment (store as int)
				propertiesObj.AddMember("align", static_cast<int>(textComp.align), allocator);

				// Layout properties
				propertiesObj.AddMember("lineSpacing", textComp.lineSpacing, allocator);
				propertiesObj.AddMember("letterSpacing", textComp.letterSpacing, allocator);
				propertiesObj.AddMember("maxWidth", textComp.maxWidth, allocator);

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}
			// Serialize TrailComponent
			if (entity.HasComponent<TrailComponent>()) {
				LOG_TRACE("  - Serializing Trail Component");

				const auto& trail = entity.GetComponent<TrailComponent>();
				rapidjson::Value componentObj(kObjectType);
				componentObj.AddMember("Type", "TrailComponent", allocator);

				rapidjson::Value propertiesObj(kObjectType);
				propertiesObj.AddMember(
					"ComponentGUID",
					Value(std::to_string(trail.ComponentGUID.m_Value).c_str(), allocator),
					allocator
				);

				// Material Type
				std::string materialTypeName = AM.getNameFromGuid(trail.MaterialGuid);
				propertiesObj.AddMember("Material Type",
					Value(materialTypeName.empty() ? "" : materialTypeName.c_str(), allocator),
					allocator);

				// Config parameters
				propertiesObj.AddMember("Max Segments", trail.MaxSegments, allocator);
				propertiesObj.AddMember("Segment Lifetime", trail.SegmentLifetime, allocator);
				propertiesObj.AddMember("Min Distance", trail.MinDistance, allocator);
				propertiesObj.AddMember("Sample Interval", trail.SampleInterval, allocator);
				propertiesObj.AddMember("Sample Accumulator", trail.SampleAccumulator, allocator);

				// Start Color
				rapidjson::Value startColorArray(kArrayType);
				startColorArray.PushBack(trail.StartColor.x, allocator);
				startColorArray.PushBack(trail.StartColor.y, allocator);
				startColorArray.PushBack(trail.StartColor.z, allocator);
				startColorArray.PushBack(trail.StartColor.w, allocator);
				propertiesObj.AddMember("Start Color", startColorArray, allocator);

				// End Color
				rapidjson::Value endColorArray(kArrayType);
				endColorArray.PushBack(trail.EndColor.x, allocator);
				endColorArray.PushBack(trail.EndColor.y, allocator);
				endColorArray.PushBack(trail.EndColor.z, allocator);
				endColorArray.PushBack(trail.EndColor.w, allocator);
				propertiesObj.AddMember("End Color", endColorArray, allocator);

				// Width parameters
				propertiesObj.AddMember("Start Width", trail.StartWidth, allocator);
				propertiesObj.AddMember("End Width", trail.EndWidth, allocator);

				// Last Position
				rapidjson::Value lastPosArray(kArrayType);
				lastPosArray.PushBack(trail.LastPosition.x, allocator);
				lastPosArray.PushBack(trail.LastPosition.y, allocator);
				lastPosArray.PushBack(trail.LastPosition.z, allocator);
				propertiesObj.AddMember("Last Position", lastPosArray, allocator);

				// Local Offset
				rapidjson::Value offsetArray(kArrayType);
				offsetArray.PushBack(trail.LocalOffset.x, allocator);
				offsetArray.PushBack(trail.LocalOffset.y, allocator);
				offsetArray.PushBack(trail.LocalOffset.z, allocator);
				propertiesObj.AddMember("Local Offset", offsetArray, allocator);

				// Boolean parameters
				propertiesObj.AddMember("Has Last Position", trail.HasLastPosition, allocator);
				propertiesObj.AddMember("Active", trail.Active, allocator);
				propertiesObj.AddMember("Emit Trail", trail.EmitTrail, allocator);

				componentObj.AddMember("Properties", propertiesObj, allocator);
				componentsArray.PushBack(componentObj, allocator);
			}

			entityObj.AddMember("Components", componentsArray, allocator);
			entitiesArray.PushBack(entityObj, allocator);
		}

		doc.AddMember("Entities", entitiesArray, allocator);

		// Update settings
		Value settingsArray(kArrayType);
		Value settingsObj(kObjectType);

		// TODO: replace these with wherever you actually store the values
		auto &sceneSettings = m_Scene->GetSceneSetting();
		settingsObj.AddMember("BloomToggle", sceneSettings.s_BloomToggle, allocator);
		settingsObj.AddMember("BloomStrength", sceneSettings.s_BloomStrength, allocator);
		settingsObj.AddMember("BloomFilterRadius", sceneSettings.s_BloomFilterRadius, allocator);
		settingsObj.AddMember("Exposure", sceneSettings.s_Exposure, allocator);
		settingsObj.AddMember("GlobalBias", sceneSettings.s_GlobalBias, allocator);

		settingsArray.PushBack(settingsObj, allocator);
		doc.AddMember("Settings", settingsArray, allocator);

		// Convert to string
		StringBuffer buffer;
		PrettyWriter<StringBuffer> writer(buffer);
		doc.Accept(writer);

		LOG_TRACE("Scene serialization complete");
		return buffer.GetString();
	}

	bool SceneSerializer::Deserialize(const std::string &filepath) {
		LOG_INFO("Deserializing scene from: ", filepath);

		// Read file
		std::ifstream file(filepath);
		if(!file.is_open()) {
			LOG_ERROR("Failed to open file for reading: ", filepath);
			return false;
		}

		std::string jsonString((std::istreambuf_iterator<char>(file)),
							   std::istreambuf_iterator<char>());
		file.close();

		return DeserializeFromString(jsonString);
	}

	bool SceneSerializer::DeserializeFromString(const std::string &jsonString) {
		using namespace rapidjson;

		//LOG_TRACE("Parsing JSON...");

		Document doc;
		doc.Parse(jsonString.c_str());

		if(doc.HasParseError()) {
			LOG_ERROR("JSON parse error at offset ", doc.GetErrorOffset());
			return false;
		}

		// Clear current scene
		auto &registry = m_Scene->GetRegistry();
		registry.clear();

		// Read scene name
		if(doc.HasMember("Scene")) {
			std::string sceneName = doc["Scene"].GetString();
			m_Scene->SetName(sceneName);  // Actually update the scene name
			LOG_INFO("Loading scene: ", doc["Scene"].GetString());
		}

		// Read entities
		if(!doc.HasMember("Entities") || !doc["Entities"].IsArray()) {
			LOG_ERROR("No entities array in scene file");
			return false;
		}

		const Value &entities = doc["Entities"];

		for(SizeType i = 0; i < entities.Size(); i++) {
			const Value &entityObj = entities[i];

			// Get entity name from TagComponent if available
			std::string entityName = "Entity";
			if(entityObj.HasMember("Components")) {
				const Value &components = entityObj["Components"];
				for(SizeType j = 0; j < components.Size(); j++) {
					if(components[j]["Type"].GetString() == std::string("TagComponent")) {
						const auto& props = components[j]["Properties"];
						if (props.HasMember("Name"))
							entityName = props["Name"].GetString();
						else if (props.HasMember("Tag"))
							entityName = props["Tag"].GetString();
						break;
					}
				}
			}

			// Read the saved entity ID
			entt::entity entityId = entt::null;
			if(entityObj.HasMember("ID")) {
				entityId = static_cast<entt::entity>(entityObj["ID"].GetUint());
				//LOG_TRACE("Restoring entity with ID: ", (uint32_t)entityId);
			}

			// Create entity
			Entity entity;
			if(entityId != entt::null) {
				// Create entity with specific ID to preserve it across saves
				entity = Entity(registry.create(entityId), &registry);
				//LOG_TRACE("Created entity '", entityName, "' with preserved ID: ", (uint32_t)entity);
			}
			else {
				// Fallback to auto-generated ID (for old scene files)
				entity = m_Scene->CreateEntity(entityName);
				LOG_WARNING("Entity ID not found in scene file, auto-generating new ID");
			}

			// Deserialize components
			if(entityObj.HasMember("Components")) {
				const Value &components = entityObj["Components"];

				for(SizeType j = 0; j < components.Size(); j++) {
					const Value &componentObj = components[j];
					std::string componentType = componentObj["Type"].GetString();
					const Value &properties = componentObj["Properties"];

					// Deserialize specific component types
					if(componentType == "TagComponent") {
						auto& tagComp = entity.AddComponent<TagComponent>();

						// Load the Name
						if (properties.HasMember("Name"))
							tagComp.Name = properties["Name"].GetString();
					
						// Load the Tag
						if (properties.HasMember("Tag"))
						{
							tagComp.Tag = properties["Tag"].GetString();
						}
					}
					else if(componentType == "PrefabComponent") {
						LOG_TRACE("  - Deserializing PrefabComponent");
						auto &prefabComp = entity.AddComponent<PrefabComponent>();

						// Basic prefab info
						if(properties.HasMember("ComponentGUID")) {
							prefabComp.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}

						if(properties.HasMember("PrefabAssetGuid")) {
							prefabComp.PrefabAssetGuid = xresource::instance_guid(
								std::stoull(properties["PrefabAssetGuid"].GetString())
							);
						}

						if(properties.HasMember("isPrefabRoot"))
							prefabComp.isPrefabRoot = properties["isPrefabRoot"].GetBool();

						if(properties.HasMember("isNestedPrefab"))
							prefabComp.isNestedPrefab = properties["isNestedPrefab"].GetBool();

						if(properties.HasMember("parentPrefabGuid")) {
							prefabComp.parentPrefabGuid = xresource::instance_guid(
								std::stoull(properties["parentPrefabGuid"].GetString())
							);
						}

						if(properties.HasMember("prefabName"))
							prefabComp.prefabName = properties["prefabName"].GetString();

						if(properties.HasMember("prefabVersion"))
							prefabComp.prefabVersion = properties["prefabVersion"].GetUint();

						//// NEW: Load prefab file path from scene if available
						//std::string prefabFilePath;
						//if (properties.HasMember("prefabFilePath")) {
						//	prefabFilePath = properties["prefabFilePath"].GetString();
						//	LOG_DEBUG("Loaded prefab path from scene: ", prefabFilePath);
						//}

						//// Auto-register prefab if it's a root and not already registered
						//if (prefabComp.isPrefabRoot && prefabComp.PrefabAssetGuid.m_Value != 0) {
						//	if (!PrefabRegistry::Get().IsPrefabRegistered(prefabComp.PrefabAssetGuid)) {
						//		// Use saved path if available, otherwise construct it
						//		if (prefabFilePath.empty()) {
						//			prefabFilePath = "Resources/Prefabs/" + prefabComp.prefabName + ".prefab";
						//		}

						//		LOG_INFO("Registering prefab: ", prefabComp.prefabName,
						//			" from path: ", prefabFilePath);

						//		PrefabRegistry::Get().RegisterPrefab(
						//			prefabComp.PrefabAssetGuid,
						//			prefabFilePath,
						//			prefabComp.prefabName
						//		);
						//		LOG_INFO("Auto-registered prefab from scene: ", prefabComp.prefabName);
						//	}
						//}
						// Deserialize component overrides
						if(properties.HasMember("componentOverrides") && properties["componentOverrides"].IsArray()) {
							const Value &overridesArray = properties["componentOverrides"];
							prefabComp.componentOverrides.clear();

							for(SizeType k = 0; k < overridesArray.Size(); k++) {
								const Value &overrideObj = overridesArray[k];
								ComponentOverride override;

								if(overrideObj.HasMember("componentType")) {
									override.componentType = static_cast<ComponentTypeID>(
										overrideObj["componentType"].GetUint()
										);
								}

								if(overrideObj.HasMember("isAddedComponent"))
									override.isAddedComponent = overrideObj["isAddedComponent"].GetBool();

								if(overrideObj.HasMember("isRemovedComponent"))
									override.isRemovedComponent = overrideObj["isRemovedComponent"].GetBool();

								if(overrideObj.HasMember("originalComponentJSON")) {
									override.originalComponentJSON = overrideObj["originalComponentJSON"].GetString();
								}

								if(overrideObj.HasMember("currentComponentJSON")) {
									override.currentComponentJSON = overrideObj["currentComponentJSON"].GetString();
								}

								// Deserialize modified property names
								if(overrideObj.HasMember("modifiedPropertyNames") &&
								   overrideObj["modifiedPropertyNames"].IsArray()) {
									const Value &propsArray = overrideObj["modifiedPropertyNames"];
									override.modifiedPropertyNames.clear();

									for(SizeType m = 0; m < propsArray.Size(); m++) {
										override.modifiedPropertyNames.push_back(
											propsArray[m].GetString()
										);
									}
								}

								prefabComp.componentOverrides.push_back(override);
							}
						}

						// Deserialize child entity IDs
						if(properties.HasMember("childEntityIDs") && properties["childEntityIDs"].IsArray()) {
							const Value &childrenArray = properties["childEntityIDs"];
							prefabComp.childEntityIDs.clear();

							for(SizeType k = 0; k < childrenArray.Size(); k++) {
								prefabComp.childEntityIDs.push_back(childrenArray[k].GetUint());
							}
						}

						// Deserialize deleted entities
						if(properties.HasMember("deletedEntities") && properties["deletedEntities"].IsArray()) {
							const Value &deletedArray = properties["deletedEntities"];
							prefabComp.deletedEntities.clear();

							for(SizeType k = 0; k < deletedArray.Size(); k++) {
								const Value &deletedObj = deletedArray[k];

								PrefabComponent::DeletedEntityData deleted;

								if(deletedObj.HasMember("prefabLocalID"))
									deleted.prefabLocalID = deletedObj["prefabLocalID"].GetUint64();

								if(deletedObj.HasMember("entityName"))
									deleted.entityName = deletedObj["entityName"].GetString();

								if(deletedObj.HasMember("serializedEntityData"))
									deleted.serializedEntityData = deletedObj["serializedEntityData"].GetString();

								prefabComp.deletedEntities.push_back(deleted);
							}

							LOG_INFO("Deserialized ", prefabComp.deletedEntities.size(), " deleted entities");
						}

					}
					else if(componentType == "TransformComponent") {
						auto &transform = entity.AddComponent<TransformComponent>();
						if(properties.HasMember("ComponentGUID")) {
							transform.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}

						// Position
						if(properties.HasMember("Position")) {
							const Value &posArray = properties["Position"];
							transform.Position = glm::vec3(
								posArray[0].GetFloat(),
								posArray[1].GetFloat(),
								posArray[2].GetFloat()
							);
						}

						// Rotation - Convert Euler angles to quaternion
						if(properties.HasMember("Rotation")) {
							const Value &rotArray = properties["Rotation"];
							glm::vec3 eulerRotation(
								rotArray[0].GetFloat(),
								rotArray[1].GetFloat(),
								rotArray[2].GetFloat()
							);
							transform.Rotation = glm::quat(glm::radians(eulerRotation));
						}

						// Scale
						if(properties.HasMember("Scale")) {
							const Value &scaleArray = properties["Scale"];
							transform.Scale = glm::vec3(
								scaleArray[0].GetFloat(),
								scaleArray[1].GetFloat(),
								scaleArray[2].GetFloat()
							);
						}

						if(properties.HasMember("Parent")) {
							transform.Parent = properties["Parent"].GetUint();
						}

						if(properties.HasMember("Children") && properties["Children"].IsArray()) {

							transform.Children.clear();
							const Value &childrenArray = properties["Children"];

							for(rapidjson::SizeType i = 0; i < childrenArray.Size(); ++i)
								transform.Children.push_back(childrenArray[i].GetUint());

						}
					}
					else if(componentType == "CameraComponent") {
						auto &camera = entity.AddComponent<CameraComponent>();

						if(properties.HasMember("Enabled"))
							camera.Enabled = properties["Enabled"].GetBool();
						if(properties.HasMember("Primary"))
							camera.Primary = properties["Primary"].GetBool();
						if(properties.HasMember("Projection"))
							camera.Projection = properties["Projection"].GetBool();
						if(properties.HasMember("autoAspect"))
							camera.autoAspect = properties["autoAspect"].GetBool();

						if(properties.HasMember("Size")) {
							const Value &size = properties["Size"];
							camera.Size = glm::vec2(
								size[0].GetFloat(),
								size[1].GetFloat()
							);
						}
						if(properties.HasMember("Depth"))
							camera.Depth = properties["Depth"].GetUint();
						if(properties.HasMember("Aspect"))
							camera.Aspect = properties["Aspect"].GetFloat();
						if(properties.HasMember("FOV"))
							camera.FOV = properties["FOV"].GetFloat();
						if(properties.HasMember("NearPlane"))
							camera.NearPlane = properties["NearPlane"].GetFloat();
						if(properties.HasMember("FarPlane"))
							camera.FarPlane = properties["FarPlane"].GetFloat();

						if(properties.HasMember("Target")) {
							const Value &target = properties["Target"];
							camera.Target = glm::vec3(
								target[0].GetFloat(),
								target[1].GetFloat(),
								target[2].GetFloat()
							);
						}
					}
					else if(componentType == "MeshRendererComponent") {
						auto &mesh = entity.AddComponent<MeshRendererComponent>();
						if(properties.HasMember("ComponentGUID")) {
							mesh.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}
						// Handle filename fields (strings)
						if(properties.HasMember("Mesh") && properties["Mesh"].IsString()) {
							std::string meshName = properties["Mesh"].GetString();
							mesh.MeshGuid = AM.getGuidFromName(meshName);
						}

						if(properties.HasMember("Material") && properties["Material"].IsString()) {
							std::string matName = properties["Material"].GetString();
							mesh.MaterialGuid = AM.getGuidFromName(matName);
						}

						if(properties.HasMember("Texture") && properties["Texture"].IsString()) {
							std::string texName = properties["Texture"].GetString();
							mesh.TextureGuid = AM.getGuidFromName(texName);
						}

						/*
						NOTE THIS FIELD IS DEPRECATED, AND IT'S ONLY USED FOR STABILITY & BACKWARD COMPATIBILITY, USED THE NEW
						FIELDS ABOVE WHEN REFERENCING THANK YOU!
						*/
						// Handle old GUID fields (for backward compatibility)
						if(properties.HasMember("MeshGuid")) {
							mesh.MeshGuid = xresource::instance_guid{ properties["MeshGuid"].GetUint64() };
						}

						if(properties.HasMember("MaterialGuid")) {
							mesh.MaterialGuid = xresource::instance_guid{ properties["MaterialGuid"].GetUint64() };
						}

						if(properties.HasMember("TextureGuid")) {
							mesh.TextureGuid = xresource::instance_guid{ properties["TextureGuid"].GetUint64() };
						}
						/*
						END NOTE
						*/

						// Handle other properties
						if(properties.HasMember("Visible")) mesh.Visible = properties["Visible"].GetBool();
						if(properties.HasMember("ShadowReceive")) mesh.ShadowReceive = properties["ShadowReceive"].GetBool();
						if(properties.HasMember("GlobalIlluminate")) mesh.GlobalIlluminate = properties["GlobalIlluminate"].GetBool();
						if(properties.HasMember("MeshType")) mesh.MeshType = properties["MeshType"].GetUint();

						// FIXED: Check if "Material" is a NUMBER before reading as integer
						if(properties.HasMember("Material") && properties["Material"].IsNumber()) {
							mesh.Material = properties["Material"].GetUint();
						}
						// Also check for the new name "MaterialIdx"
						else if(properties.HasMember("MaterialIdx")) {
							mesh.Material = properties["MaterialIdx"].GetUint();
						}

						// FIXED: Check if "Texture" is a NUMBER before reading as integer
						if(properties.HasMember("Texture") && properties["Texture"].IsNumber()) {
							mesh.Texture = properties["Texture"].GetUint();
						}
						// Also check for the new name "TextureIdx"
						else if(properties.HasMember("TextureIdx")) {
							mesh.Texture = properties["TextureIdx"].GetUint();
						}

						if(properties.HasMember("SubmeshIndex")) mesh.SubmeshIndex = properties["SubmeshIndex"].GetUint();

						if(properties.HasMember("CastType")) {
							mesh.CastType = static_cast<ShadowCastType>(properties["CastType"].GetUint());
						}
					}

					else if(componentType == "RigidbodyComponent") {
						auto &rb = entity.AddComponent<RigidbodyComponent>();

						if(properties.HasMember("ComponentGUID")) {
							rb.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}
						if(properties.HasMember("Mass"))
							rb.Mass = properties["Mass"].GetFloat();
						if(properties.HasMember("IsKinematic"))
							rb.IsKinematic = properties["IsKinematic"].GetBool();
						if(properties.HasMember("IsTrigger"))
							rb.IsTrigger = properties["IsTrigger"].GetBool();
						if(properties.HasMember("UseGravity"))
							rb.UseGravity = properties["UseGravity"].GetBool();

						if(properties.HasMember("Velocity")) {
							const Value &velArray = properties["Velocity"];
							rb.Velocity = glm::vec3(
								velArray[0].GetFloat(),
								velArray[1].GetFloat(),
								velArray[2].GetFloat()
							);
						}
						if(properties.HasMember("AngularVelocity")) {
							const Value &angVel = properties["AngularVelocity"];
							rb.AngularVelocity = glm::vec3(
								angVel[0].GetFloat(),
								angVel[1].GetFloat(),
								angVel[2].GetFloat()
							);
						}
						if(properties.HasMember("LinearDamping"))
							rb.LinearDamping = properties["LinearDamping"].GetFloat();
						if(properties.HasMember("AngularDamping"))
							rb.AngularDamping = properties["AngularDamping"].GetFloat();
						if(properties.HasMember("Restitution"))
							rb.Restitution = properties["Restitution"].GetFloat();
						if(properties.HasMember("CollideType"))
							rb.Shape = static_cast<ColliderType>(properties["CollideType"].GetInt());
						if(properties.HasMember("BoxHalfExtent")) {
							const Value &boxHalfExtent = properties["BoxHalfExtent"];
							rb.BoxHalfExtents = glm::vec3(
								boxHalfExtent[0].GetFloat(),
								boxHalfExtent[1].GetFloat(),
								boxHalfExtent[2].GetFloat()
							);
						}
						if(properties.HasMember("SphereRadius"))
							rb.SphereRadius = properties["SphereRadius"].GetFloat();
					}
					else if(componentType == "AudioComponent") {
						auto &audio = entity.AddComponent<AudioComponent>();
						if(properties.HasMember("ComponentGUID")) {
							audio.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}
						if(properties.HasMember("FilePath"))
							audio.AudioFilePath = properties["FilePath"].GetString();
						if(properties.HasMember("Type"))
							audio.Type = static_cast<AudioType>(properties["Type"].GetInt());
						if(properties.HasMember("State"))
							audio.State = static_cast<PlayState>(properties["State"].GetInt());
						if(properties.HasMember("Volume"))
							audio.Volume = properties["Volume"].GetFloat();
						if(properties.HasMember("Pitch"))
							audio.Pitch = properties["Pitch"].GetFloat();
						if(properties.HasMember("Loop"))
							audio.Loop = properties["Loop"].GetBool();
						if(properties.HasMember("Mute"))
							audio.Mute = properties["Mute"].GetBool();
						if(properties.HasMember("Reverb"))
							audio.ReverbProperties = properties["ReverbProperties"].GetFloat();
						if(properties.HasMember("Is3D"))
							audio.Is3D = properties["Is3D"].GetBool();
						if(properties.HasMember("MinDistance"))
							audio.MinDistance = properties["MinDistance"].GetFloat();
						if(properties.HasMember("MaxDistance"))
							audio.MaxDistance = properties["MaxDistance"].GetFloat();
						if(properties.HasMember("RolloffMode"))
							audio.RolloffMode = static_cast<AudioRolloffMode>(properties["RolloffMode"].GetInt());
						if(properties.HasMember("DopplerLevel"))
							audio.DopplerLevel = properties["DopplerLevel"].GetFloat();
						if(properties.HasMember("Pan2D"))
							audio.Pan2D = properties["Pan2D"].GetFloat();
					}
					else if(componentType == "ListenerComponent") {
						auto &listener = entity.AddComponent<ListenerComponent>();
						if(properties.HasMember("ComponentGUID")) {
							listener.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}
						if(properties.HasMember("Active"))
							listener.Active = properties["Active"].GetBool();
					}
					else if(componentType == "ReverbComponent") {
						auto &reverb = entity.AddComponent<ReverbZoneComponent>();
						if(properties.HasMember("ComponentGUID")) {
							reverb.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}
						if(properties.HasMember("Preset"))
							reverb.Preset = static_cast<ReverbPreset>(properties["Preset"].GetInt());
						if(properties.HasMember("MinDistance"))
							reverb.MinDistance = properties["MinDistance"].GetFloat();
						if(properties.HasMember("MaxDistance"))
							reverb.MaxDistance = properties["MaxDistance"].GetFloat();
						if(properties.HasMember("DecayTime"))
							reverb.DecayTime = properties["DecayTime"].GetFloat();
						if(properties.HasMember("HfDecayRatio"))
							reverb.HfDecayRatio = properties["HfDecayRatio"].GetFloat();
						if(properties.HasMember("Diffusion"))
							reverb.Diffusion = properties["Diffusion"].GetFloat();
						if(properties.HasMember("Density"))
							reverb.Density = properties["Density"].GetFloat();
						if(properties.HasMember("WetLevel"))
							reverb.WetLevel = properties["WetLevel"].GetFloat();
					}
					else if(componentType == "BehaviourTreeComponent") {
						auto &bt = entity.AddComponent<BehaviourTreeComponent>();
						if(properties.HasMember("Active"))
							bt.Active = properties["Active"].GetBool();
						if(properties.HasMember("ResetOnComplete"))
							bt.ResetOnComplete = properties["ResetOnComplete"].GetBool();
						if(properties.HasMember("TreeAssetPath")) {
							bt.TreeAssetPath = properties["TreeAssetPath"].GetString();
						}

						// Do NOT load the tree here only store the reference
						bt.TreeInstance = nullptr;

					}
					else if(componentType == "ParticleComponent") {
						auto &emitter = entity.AddComponent<ParticleComponent>();
						if(properties.HasMember("ComponentGUID")) {
							emitter.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}

						// Particle Type Advanced
						if(properties.HasMember("Particle Type Advanced") && properties["Particle Type Advanced"].IsString()) {
							std::string particleTypeAdvancedName = properties["Particle Type Advanced"].GetString();
							emitter.ParticleTypeAdvanced = AM.getGuidFromName(particleTypeAdvancedName);
						}

						// Material Type
						if(properties.HasMember("Material Type") && properties["Material Type"].IsString()) {
							std::string materialTypeName = properties["Material Type"].GetString();
							emitter.MaterialType = AM.getGuidFromName(materialTypeName);
						}

						// Initial Velocity
						if(properties.HasMember("Initial Velocity") && properties["Initial Velocity"].IsArray()) {
							const auto &velArray = properties["Initial Velocity"].GetArray();
							if(velArray.Size() >= 3) {
								emitter.InitialVelocity.x = velArray[0].GetFloat();
								emitter.InitialVelocity.y = velArray[1].GetFloat();
								emitter.InitialVelocity.z = velArray[2].GetFloat();
							}
						}

						// Start Size
						if(properties.HasMember("Start Size") && properties["Start Size"].IsArray()) {
							const auto &startSizeArray = properties["Start Size"].GetArray();
							if(startSizeArray.Size() >= 3) {
								emitter.StartSize.x = startSizeArray[0].GetFloat();
								emitter.StartSize.y = startSizeArray[1].GetFloat();
								emitter.StartSize.z = startSizeArray[2].GetFloat();
							}
						}
						// Default Size
						if(properties.HasMember("Default Size") && properties["Default Size"].IsArray()) {
							const auto &defaultSizeArray = properties["Default Size"].GetArray();
							if(defaultSizeArray.Size() >= 3) {
								emitter.DefaultSize.x = defaultSizeArray[0].GetFloat();
								emitter.DefaultSize.y = defaultSizeArray[1].GetFloat();
								emitter.DefaultSize.z = defaultSizeArray[2].GetFloat();
							}
						}
						// End Size
						if(properties.HasMember("End Size") && properties["End Size"].IsArray()) {
							const auto &endSizeArray = properties["End Size"].GetArray();
							if(endSizeArray.Size() >= 3) {
								emitter.EndSize.x = endSizeArray[0].GetFloat();
								emitter.EndSize.y = endSizeArray[1].GetFloat();
								emitter.EndSize.z = endSizeArray[2].GetFloat();
							}
						}

						// Emission Box Size
						if(properties.HasMember("Emission Box Size") && properties["Emission Box Size"].IsArray()) {
							const auto &emissionBoxSizeArray = properties["Emission Box Size"].GetArray();
							emitter.EmissionBoxSize.x = emissionBoxSizeArray[0].GetFloat();
							emitter.EmissionBoxSize.y = emissionBoxSizeArray[1].GetFloat();
							emitter.EmissionBoxSize.z = emissionBoxSizeArray[2].GetFloat();
						}

						// Color Min
						if(properties.HasMember("Color Min") && properties["Color Min"].IsArray()) {
							const auto &minColorArray = properties["Color Min"].GetArray();
							if(minColorArray.Size() >= 3) {
								emitter.ColorMin.x = minColorArray[0].GetFloat();
								emitter.ColorMin.y = minColorArray[1].GetFloat();
								emitter.ColorMin.z = minColorArray[2].GetFloat();
							}
						}

						// Color Max
						if(properties.HasMember("Color Max") && properties["Color Max"].IsArray()) {
							const auto &maxColorArray = properties["Color Max"].GetArray();
							if(maxColorArray.Size() >= 3) {
								emitter.ColorMax.x = maxColorArray[0].GetFloat();
								emitter.ColorMax.y = maxColorArray[1].GetFloat();
								emitter.ColorMax.z = maxColorArray[2].GetFloat();
							}
						}

						// Emitter Shape
						if(properties.HasMember("Emitter Shape"))
							emitter.Shape = static_cast<EmitterShape>(properties["Emitter Shape"].GetUint());

						// Max Particles
						if(properties.HasMember("Max Particles"))
							emitter.MaxParticles = properties["Max Particles"].GetUint();

						// Particle Type
						if(properties.HasMember("Particle Type"))
							emitter.ParticleType = properties["Particle Type"].GetUint();

						// Emission Rate
						if(properties.HasMember("Emission Rate"))
							emitter.EmissionRate = properties["Emission Rate"].GetFloat();

						// Particle Lifetime
						if(properties.HasMember("Particle Lifetime"))
							emitter.ParticleLifetime = properties["Particle Lifetime"].GetFloat();

						// Emission Accumulator
						if(properties.HasMember("Emission Accumulator"))
							emitter.EmissionAccumulator = properties["Emission Accumulator"].GetFloat();

						// Particle Size
						if(properties.HasMember("Particle Size"))
							emitter.ParticleSize = properties["Particle Size"].GetFloat();

						// Grow Phase End
						if(properties.HasMember("Grow Phase End"))
							emitter.GrowPhaseEnd = properties["Grow Phase End"].GetFloat();

						// Shrink Phase Start
						if(properties.HasMember("Shrink Phase Start"))
							emitter.ShrinkPhaseStart = properties["Shrink Phase Start"].GetFloat();

						// Emission Sphere Radius
						if(properties.HasMember("Emission Sphere Radius"))
							emitter.EmissionSphereRadius = properties["Emission Sphere Radius"].GetFloat();

						// Randomization parameters
						if(properties.HasMember("Velocity Randomness"))
							emitter.VelocityRandomness = properties["Velocity Randomness"].GetFloat();

						if(properties.HasMember("Lifetime Randomness"))
							emitter.LifetimeRandomness = properties["Lifetime Randomness"].GetFloat();

						if(properties.HasMember("Spread Angle"))
							emitter.SpreadAngle = properties["Spread Angle"].GetFloat();

						if(properties.HasMember("Min Speed"))
							emitter.MinSpeed = properties["Min Speed"].GetFloat();

						if(properties.HasMember("Max Speed"))
							emitter.MaxSpeed = properties["Max Speed"].GetFloat();

						if(properties.HasMember("Rotation Speed"))
							emitter.RotationSpeed = properties["Rotation Speed"].GetFloat();

						if(properties.HasMember("Play Delay"))
							emitter.PlayDelay = properties["Play Delay"].GetFloat();

						if(properties.HasMember("Delay Accumualator"))
							emitter.DelayAccumualator = properties["Delay Accumualator"].GetFloat();

						// Boolean parameters
						if(properties.HasMember("Randomize Rotation"))
							emitter.RandomizeRotation = properties["Randomize Rotation"].GetBool();

						if(properties.HasMember("Loop"))
							emitter.Loop = properties["Loop"].GetBool();

						if(properties.HasMember("Active"))
							emitter.Active = properties["Active"].GetBool();

						if(properties.HasMember("World Space"))
							emitter.WorldSpace = properties["World Space"].GetBool();

						if(properties.HasMember("Burst Mode"))
							emitter.BurstMode = properties["Burst Mode"].GetBool();
					}
					else if(componentType == "ScriptComponent") {
						LOG_TRACE("  - Deserializing ScriptComponent");

						auto &script = entity.AddComponent<ScriptComponent>();

						if(properties.HasMember("ComponentGUID")) {
							script.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}

						if(properties.HasMember("ScriptClassName")) {
							script.ScriptClassName = properties["ScriptClassName"].GetString();
							LOG_TRACE("    - Script class name: ", script.ScriptClassName);
						}

						// IMPORTANT:
						// Never create / restore managed instances during scene load.
						// Only restore cached editor-authored values.
						script.ScriptInstance = nullptr;
						script.GCHandle = 0;
						script.Started = false;
						script.SerializedFields.clear();

						if(properties.HasMember("Fields")) {
							const rapidjson::Value &fieldsObj = properties["Fields"];
							DeserializeScriptFieldsToComponentFromRapidJSON(script, fieldsObj);
							LOG_TRACE("    - Cached script fields restored (no managed instance created)");
						}
					}
					else if(componentType == "LightComponent") {
						auto &light = entity.AddComponent<LightComponent>();
						if(properties.HasMember("ComponentGUID")) {
							light.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}
						if(properties.HasMember("Enabled"))
							light.Enabled = properties["Enabled"].GetBool();
						if(properties.HasMember("Type"))
							light.Type = static_cast<LightType>(properties["Type"].GetInt()); // 0=Dir,1=Point,2=Spot
						// Optional: Mode is usually omitted in scene files (Realtime only), but handle if present
						//if (properties.HasMember("Mode"))
						//    light.Mode = static_cast<LightMode>(properties["Mode"].GetInt());
						if(properties.HasMember("Color") && properties["Color"].IsArray()) {
							const auto &col = properties["Color"];
							light.Color = glm::vec3(
								col[0].GetFloat(),
								col[1].GetFloat(),
								col[2].GetFloat()
							);
						}
						if(properties.HasMember("Intensity"))
							light.Intensity = properties["Intensity"].GetFloat();
						if(properties.HasMember("Range"))
							light.Range = properties["Range"].GetFloat();
						if(properties.HasMember("SpotAngleDeg"))
							light.SpotAngleDeg = properties["SpotAngleDeg"].GetFloat();
						if(properties.HasMember("IndirectMultiplier"))
							light.IndirectMultiplier = properties["IndirectMultiplier"].GetFloat();
						if(properties.HasMember("TypeShadow"))
							light.TypeShadow = static_cast<ShadowType>(properties["TypeShadow"].GetInt()); // 0 = No,1 = Hard ,2 = Soft
						if(properties.HasMember("Resolution"))
							light.Resolution = properties["Resolution"].GetUint();
						if(properties.HasMember("Strength"))
							light.Strength = properties["Strength"].GetFloat();
						if(properties.HasMember("Bias"))
							light.Bias = properties["Bias"].GetFloat();
						if(properties.HasMember("NearPlane"))
							light.NearPlane = properties["NearPlane"].GetFloat();

					}
					else if(componentType == "AnimatorComponent") {
						auto &animator = entity.AddComponent<AnimatorComponent>();
						if(properties.HasMember("ComponentGUID")) {
							animator.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}
						if(properties.HasMember("playing"))
							animator.playing = properties["playing"].GetBool();
						if(properties.HasMember("respectClipLoop"))
							animator.respectClipLoop = properties["respectClipLoop"].GetBool();
						if(properties.HasMember("controller"))
							animator.controller = properties["controller"].GetUint();
						if(properties.HasMember("currentClipIndex"))
							animator.currentClipIndex = properties["currentClipIndex"].GetUint();
						if(properties.HasMember("currentTime"))
							animator.currentTime = properties["currentTime"].GetFloat();
						if(properties.HasMember("playbackSpeed"))
							animator.playbackSpeed = properties["playbackSpeed"].GetFloat();
					}
					else if(componentType == "SpriteRendererComponent") {
						auto &spriterenderer = entity.AddComponent<SpriteRendererComponent>();

						if(properties.HasMember("Texture") && properties["Texture"].IsString()) {
							std::string texName = properties["Texture"].GetString();
							spriterenderer.TextureGuid = AM.getGuidFromName(texName);
						}

						if(properties.HasMember("Color") && properties["Color"].IsArray()) {
							const auto &colorArr = properties["Color"].GetArray();
							if(colorArr.Size() >= 4) {
								spriterenderer.Color.r = colorArr[0].GetFloat();
								spriterenderer.Color.g = colorArr[1].GetFloat();
								spriterenderer.Color.b = colorArr[2].GetFloat();
								spriterenderer.Color.a = colorArr[3].GetFloat();
							}
						}

						if(properties.HasMember("Quad")) {
							spriterenderer.Quad = properties["Quad"].GetUint();
						}

						if(properties.HasMember("Sprite Layer")) {
							spriterenderer.SpriteLayer = properties["Sprite Layer"].GetUint();
						}

						if(properties.HasMember("IsActive")) {
							spriterenderer.IsActive = properties["IsActive"].GetBool();
						}

						if(properties.HasMember("IsVisible")) {
							spriterenderer.IsVisible = properties["IsVisible"].GetBool();
						}

					}
					else if(componentType == "TextComponent") {
						auto &textComp = entity.AddComponent<TextComponent>();

						// ComponentGUID
						if(properties.HasMember("ComponentGUID") && properties["ComponentGUID"].IsString()) {
							textComp.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}

						// Text content
						if(properties.HasMember("text") && properties["text"].IsString()) {
							textComp.text = properties["text"].GetString();
						}

						//font name
						if (properties.HasMember("fontName") && properties["fontName"].IsString()) {
							textComp.fontName = properties["fontName"].GetString();
						}

						// Font size
						if(properties.HasMember("fontSize") && properties["fontSize"].IsFloat()) {
							textComp.fontSize = properties["fontSize"].GetFloat();
						}

						// Color
						if(properties.HasMember("color") && properties["color"].IsArray()) {
							const auto &colorArr = properties["color"].GetArray();
							if(colorArr.Size() >= 4) {
								textComp.color[0] = colorArr[0].GetFloat();
								textComp.color[1] = colorArr[1].GetFloat();
								textComp.color[2] = colorArr[2].GetFloat();
								textComp.color[3] = colorArr[3].GetFloat();
							}
						}

						//isVisible
						if (properties.HasMember("isVisible") && properties["isVisible"].IsBool()) {
							textComp.setVisible(properties["isVisible"].GetBool());
						}

						// Alignment
						if(properties.HasMember("align") && properties["align"].IsInt()) {
							textComp.align = static_cast<TextAlignment>(properties["align"].GetInt());
						}

						// Line spacing
						if(properties.HasMember("lineSpacing") && properties["lineSpacing"].IsFloat()) {
							textComp.lineSpacing = properties["lineSpacing"].GetFloat();
						}

						// Letter spacing
						if(properties.HasMember("letterSpacing") && properties["letterSpacing"].IsFloat()) {
							textComp.letterSpacing = properties["letterSpacing"].GetFloat();
						}

						// Max width
						if(properties.HasMember("maxWidth") && properties["maxWidth"].IsFloat()) {
							textComp.maxWidth = properties["maxWidth"].GetFloat();
						}

						// Mark as dirty since layout needs to be recalculated
						textComp.isDirty = true;
					}
					else if (componentType == "TrailComponent") {
						auto& trail = entity.AddComponent<TrailComponent>();

						if (properties.HasMember("ComponentGUID")) {
							trail.ComponentGUID = xresource::instance_guid(
								std::stoull(properties["ComponentGUID"].GetString())
							);
						}

						// Material GUID
						if (properties.HasMember("Material Type") && properties["Material Type"].IsString()) {
							std::string materialName = properties["Material Type"].GetString();
							trail.MaterialGuid = AM.getGuidFromName(materialName);
						}

						// Config parameters
						if (properties.HasMember("Max Segments"))
							trail.MaxSegments = properties["Max Segments"].GetUint();

						if (properties.HasMember("Segment Lifetime"))
							trail.SegmentLifetime = properties["Segment Lifetime"].GetFloat();

						if (properties.HasMember("Min Distance"))
							trail.MinDistance = properties["Min Distance"].GetFloat();

						if (properties.HasMember("Sample Interval"))
							trail.SampleInterval = properties["Sample Interval"].GetFloat();

						if (properties.HasMember("Sample Accumulator"))
							trail.SampleAccumulator = properties["Sample Accumulator"].GetFloat();

						// Color properties
						if (properties.HasMember("Start Color") && properties["Start Color"].IsArray()) {
							const auto& startColorArray = properties["Start Color"].GetArray();
							if (startColorArray.Size() >= 4) {
								trail.StartColor.x = startColorArray[0].GetFloat();
								trail.StartColor.y = startColorArray[1].GetFloat();
								trail.StartColor.z = startColorArray[2].GetFloat();
								trail.StartColor.w = startColorArray[3].GetFloat();
							}
						}

						if (properties.HasMember("End Color") && properties["End Color"].IsArray()) {
							const auto& endColorArray = properties["End Color"].GetArray();
							if (endColorArray.Size() >= 4) {
								trail.EndColor.x = endColorArray[0].GetFloat();
								trail.EndColor.y = endColorArray[1].GetFloat();
								trail.EndColor.z = endColorArray[2].GetFloat();
								trail.EndColor.w = endColorArray[3].GetFloat();
							}
						}

						// Width properties
						if (properties.HasMember("Start Width"))
							trail.StartWidth = properties["Start Width"].GetFloat();

						if (properties.HasMember("End Width"))
							trail.EndWidth = properties["End Width"].GetFloat();

						// Position tracking
						if (properties.HasMember("Last Position") && properties["Last Position"].IsArray()) {
							const auto& lastPosArray = properties["Last Position"].GetArray();
							if (lastPosArray.Size() >= 3) {
								trail.LastPosition.x = lastPosArray[0].GetFloat();
								trail.LastPosition.y = lastPosArray[1].GetFloat();
								trail.LastPosition.z = lastPosArray[2].GetFloat();
							}
						}

						if (properties.HasMember("Local Offset") && properties["Local Offset"].IsArray()) {
							const auto& offsetArray = properties["Local Offset"].GetArray();
							if (offsetArray.Size() >= 3) {
								trail.LocalOffset.x = offsetArray[0].GetFloat();
								trail.LocalOffset.y = offsetArray[1].GetFloat();
								trail.LocalOffset.z = offsetArray[2].GetFloat();
							}
						}

						// Boolean flags
						if (properties.HasMember("Has Last Position"))
							trail.HasLastPosition = properties["Has Last Position"].GetBool();

						if (properties.HasMember("Active"))
							trail.Active = properties["Active"].GetBool();

						if (properties.HasMember("Emit Trail"))
							trail.EmitTrail = properties["Emit Trail"].GetBool();
					}
				}
			}
		}

		// Read entities
		if(doc.HasMember("Settings")) {
			if(doc["Settings"].IsArray()) {
				// Update current settings
				auto &sceneSettings = m_Scene->GetSceneSetting();

				const auto &settingsArray = doc["Settings"].GetArray();
				if(!settingsArray.Empty() && settingsArray[0].IsObject()) {
					const auto &s = settingsArray[0];

					if(s.HasMember("BloomToggle") && s["BloomToggle"].IsBool())
						sceneSettings.s_BloomToggle = s["BloomToggle"].GetBool();

					if(s.HasMember("BloomStrength") && s["BloomStrength"].IsFloat())
						sceneSettings.s_BloomStrength = s["BloomStrength"].GetFloat();

					if(s.HasMember("BloomFilterRadius") && s["BloomFilterRadius"].IsFloat())
						sceneSettings.s_BloomFilterRadius = s["BloomFilterRadius"].GetFloat();

					if(s.HasMember("Exposure") && s["Exposure"].IsFloat())
						sceneSettings.s_Exposure = s["Exposure"].GetFloat();
				}
			}
		}
		else {
			LOG_ERROR("No settings array in scene file");
			return false;
			//LOG_WARNING("Settings exists but is not an array, using defaults");
		}

		LOG_INFO("Scene deserialized successfully");
		return true;
	}

} // namespace Engine
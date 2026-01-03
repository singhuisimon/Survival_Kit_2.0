#include "EditorPropertyPanel.h"
#include "../Utility/Logger.h"
#include "../Engine/Editor/Editor.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Asset/ResourceHelpers.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Texture.h"
#include  "../Serialization/MaterialSerializer.h"

namespace Engine
{
	void EditorPropertyPanel::PropertyPanel()
	{
		if (!m_Editor->GetPropertyWindowRef()) return;
		std::cout << "SelectedEntity: " << (uint32_t)m_SelectedEntity.GetHandle() << "\n";
		//Bug - Scene Switching
		m_Scene = m_Editor->GetActiveScene();
		m_SelectedEntity = m_Editor->GetSelectedEntity();
		m_ScenePath = m_Editor->GetScenePath();
		m_SceneName = m_Editor->GetSceneName();

		bool& open = m_Editor->GetPropertyWindowRef();
		if (ImGui::Begin("Properties", &open))
		{
			//ImGui::Text("m_Scene: %s", m_Scene->GetName().c_str());
			//ImGui::Text("m_ScenePath: %s", m_ScenePath.c_str());
			//ImGui::Text("m_SceneName: %s", m_SceneName.c_str());

			// Determine selected entity handle
			uint32_t entityHandle = static_cast<uint32_t>(m_SelectedEntity.GetHandle());
			ImGui::Text("SelectedEntity %u", entityHandle);
			//LOG_DEBUG("SelectedEntity handle: ", entityHandle);

			if (m_SelectedEntity) { // If entity is selected
				
				// Display Tag Component
				DisplayTagComponent();
				ImGui::Separator();

				// Calculate "..." button size
				ImVec2 dotTextSize = ImGui::CalcTextSize("...");
				ImVec2 dotButtonSize(dotTextSize.x + 8.0f, dotTextSize.y + 8.0f);

				// Display Prefab Component
				//DisplayPrefabComponent(dotButtonSize);

				// Display Transform Component
				DisplayTransformComponent(dotButtonSize);
				DisplayRigidBodyComponent(dotButtonSize);
				DisplayMeshRendererComponent(dotButtonSize);
				DisplayAudioComponent(dotButtonSize);
				DisplayReverbZoneComponent(dotButtonSize);
				DisplayListenerComponent(dotButtonSize);

				//DisplayBTComponent(ImVec2& buttonSize);

				DisplayParticleComponent(dotButtonSize);

				//DisplayScriptComponent(dotButtonSize);

				DisplayLightComponent(dotButtonSize);
				DisplayCameraComponent(dotButtonSize);

				//DisplayAnimatorComponent(dotButtonSize);

				AddComponent();

			}
			else { // If no entity is selected

				ImGui::Text("No entity selected");
			}
		}

		ImGui::End();

	}

	// Tag Component
	void EditorPropertyPanel::DisplayTagComponent() {
		// Display entity name (TagComponent)
		if (m_SelectedEntity.HasComponent<TagComponent>())
		{
			auto& tag = m_SelectedEntity.GetComponent<TagComponent>();
			char buffer[256];
			strncpy_s(buffer, sizeof(buffer), tag.Tag.c_str(), _TRUNCATE);
			if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newTag = buffer;
				newTag.erase(newTag.find_last_not_of(" \t\n\r\f\v") + 1);
				newTag.erase(0, newTag.find_first_not_of(" \t\n\r\f\v"));

				if (!newTag.empty())
				{
					tag.Tag = newTag;
				}

			}
		}
	}

	// Prefab Component
	void EditorPropertyPanel::DisplayPrefabComponent(ImVec2& buttonSize)
	{
		if (m_SelectedEntity.HasComponent<PrefabComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool openPrefabComp = ImGui::CollapsingHeader("Prefab Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removePrefabComp = false;
			bool hasParent = true;
			ImGui::NextColumn();

			if (ImGui::Button("...###PrefabBtn", buttonSize))
			{
				ImGui::OpenPopup("PrefabPopUp");
			}
			if (ImGui::BeginPopup("PrefabPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removePrefabComp = true;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (m_SelectedEntity.HasComponent<TransformComponent>())
			{
				auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
				hasParent = transform.Parent == u32_max ? false : true;
			}

			if (openPrefabComp)
			{
				auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

				ImGui::Text("Prefab GUID: %llu", prefabComp.PrefabGUID.m_Value);

				/*if (!isPrefabEditor)
				{
					if (hasParent)
					{
						ImGui::BeginDisabled();
					}
					if (ImGui::Button("Revert to Prefab"))
					{
						//LoadAllPrefabsIntoRegistry();
						RevertSelectedEntityToPrefab();
						//PrefabInstantiator::ApplyOverrides(m_SelectedEntity, m_Scene);
					}
					ImGui::SameLine();
					if (ImGui::Button("Apply Overrides"))
					{
						ApplyPrefabOverrides(m_SelectedEntity);
					}
					if (hasParent)
					{
						ImGui::EndDisabled();
					}
				}*/

			}
			if (removePrefabComp)
			{
				m_SelectedEntity.RemoveComponent<PrefabComponent>();
			}
		}
	}

	// Transform Component
	void EditorPropertyPanel::DisplayTransformComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<TransformComponent>())
		{

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();

				// Position
				glm::vec3 position = transform.Position;
				if (ImGui::DragFloat3("Position", &position.x, 0.1f))
				{
					transform.SetPosition(position);
				}

				// Rotation (in degrees)
				glm::vec3 rotation = glm::degrees(glm::eulerAngles(transform.Rotation));
				if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f))
				{
					// Convert back to quaternion
					transform.SetRotation(rotation);
				}

				// Scale
				glm::vec3 scale = transform.Scale;
				if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.001f))
				{
					transform.SetScale(scale);
				}

				u32 parent_id = transform.Parent;

				if (parent_id != u32_max)
				{
					if (ImGui::InputScalar("Parent", ImGuiDataType_U32, &parent_id))
					{
						TransformSystem::SetParent(m_Scene, m_SelectedEntity, static_cast<entt::entity>(parent_id));
					}
					ImGui::Text("Parent: %zu", parent_id);
				}
				else
				{
					ImGui::Text("Parent: None");
				}
			}
		}
	}

	void EditorPropertyPanel::DisplayRigidBodyComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<RigidbodyComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			// col 1: RigidBody component header
			bool openRigidBody = ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeRigidBody = false; // for remove part
			// col2: ...
			ImGui::NextColumn();
			if (ImGui::Button("...###RigidbodyBtn", buttonSize))
			{
				ImGui::OpenPopup("RigidBodyPopUp");
			}
			if (ImGui::BeginPopup("RigidBodyPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeRigidBody = true;
				}
				ImGui::EndPopup();
			}
			ImGui::Columns(1);
			if (openRigidBody)
			{
				auto& rigidBody = m_SelectedEntity.GetComponent<RigidbodyComponent>();

				// Mass
				float rigidMass = rigidBody.GetMass();
				if (ImGui::DragFloat("Mass", &rigidMass, 0.1f, 0.0f, 1000.0f))
				{
					rigidBody.SetMass(rigidMass);
				}
				ImGui::Separator();

				// Kinematic
				ImGui::Text("Boolean to check if body is moved by code (not Physics)");
				bool isKinematic = rigidBody.IsKinematic;
				if (ImGui::Checkbox("Is Kinematic", &isKinematic))
				{
					rigidBody.SetKinematic(isKinematic);
				}
				ImGui::Separator();

				// Trigger
				ImGui::Text("If true, acts as a sensor with no collision response");
				bool isTrigger = rigidBody.IsTrigger;
				if (ImGui::Checkbox("Is Trigger", &isTrigger))
				{
					rigidBody.IsTrigger = isTrigger;
				}
				ImGui::Separator();

				// Gravity
				ImGui::Text("Whether gravity affects this body");
				bool useGravity = rigidBody.UseGravity;
				if (ImGui::Checkbox("Use Gravity", &useGravity))
				{
					rigidBody.SetGravityEnabled(useGravity);
				}
				ImGui::Separator();

				// Velocity
				glm::vec3 vel = rigidBody.GetVelocity();
				if (ImGui::DragFloat3("Velocity", &vel.x, 0.1f))
				{
					rigidBody.SetVelocity(vel);
				}

				// Angular Velocity
				glm::vec3 angVel = rigidBody.AngularVelocity;
				if (ImGui::DragFloat3("Angular Velocity", &angVel.x, 0.1f))
				{
					rigidBody.AngularVelocity = angVel;
				}

				if (ImGui::Button("Stop"))
				{
					rigidBody.Stop();
				}
				ImGui::Separator();

				// Linear Damping
				float linearDamping = rigidBody.LinearDamping;
				if (ImGui::DragFloat("Linear Damping", &linearDamping, 0.01f, 0.0f, 1.0f))
				{
					rigidBody.LinearDamping = linearDamping;
				}

				// Angular Damping
				float angularDamping = rigidBody.AngularDamping;
				if (ImGui::DragFloat("Angular Damping", &angularDamping, 0.01f, 0.0f, 1.0f))
				{
					rigidBody.AngularDamping = angularDamping;
				}

				// Restitution
				float restitution = rigidBody.Restitution;
				if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f))
				{
					rigidBody.Restitution = restitution;
				}
				ImGui::Separator();

				// Collider Shape
				ColliderType colliderShape = rigidBody.Shape;
				if (ImGui::BeginCombo("Collider Shape", ColliderTypeToString(colliderShape)))
				{
					for (int i = 0; i < 4; ++i)
					{
						ColliderType type = (ColliderType)i;
						bool selected = (colliderShape == type);
						if (ImGui::Selectable(ColliderTypeToString(type), selected))
						{
							rigidBody.Shape = type;
						}
						if (selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// Collider-specific properties
				switch (rigidBody.Shape)
				{
				case ColliderType::AABB:
					ImGui::Text("AABB is automatically generated from the mesh.");
					break;
				case ColliderType::BOX:
					ImGui::Text("Box Properties");
					if (ImGui::DragFloat3("Box Half Extents", &rigidBody.BoxHalfExtents.x, 0.1f, 0.01f, 100.0f))
					{
						// Value updated directly
					}
					break;
				case ColliderType::SPHERE:
					ImGui::Text("Sphere Properties");
					ImGui::Text("Sphere radius is originally determined from the mesh.");
					if (ImGui::DragFloat("Sphere Radius", &rigidBody.SphereRadius, 0.1f, 0.01f, 100.0f))
					{
						// Value updated directly
					}
					break;
				case ColliderType::MESH:
					ImGui::Text("Mesh collider is generated directly from the mesh.");
					break;
				default:
					break;
				}
				ImGui::Separator();

				// Runtime Display Values (Read-only)
				ImGui::Text("Display Runtime Values:");
				ImGui::BeginDisabled();
				float speed = rigidBody.GetSpeed();
				ImGui::InputFloat("Speed (m/s)", &speed, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly);
				bool isMoving = rigidBody.IsMoving();
				ImGui::Checkbox("Is Moving", &isMoving);
				bool isStatic = rigidBody.IsStatic();
				ImGui::Checkbox("Is Static", &isStatic);
				ImGui::EndDisabled();
			}

			// Remove Rigid Body Component
			if (removeRigidBody)
			{
				m_SelectedEntity.RemoveComponent<RigidbodyComponent>();
			}
		}
	}

	void EditorPropertyPanel::DisplayMeshRendererComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<MeshRendererComponent>())
		{
			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openMeshComponent = ImGui::CollapsingHeader("Mesh Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeMesh = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###MeshBtn", buttonSize))
			{
				ImGui::OpenPopup("MeshPopUp");
			}
			if (ImGui::BeginPopup("MeshPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeMesh = true;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openMeshComponent)
			{
				auto& mesh = m_SelectedEntity.GetComponent<MeshRendererComponent>();

				// ======================= Asset Reference Section =======================
				ImGui::SeparatorText("Asset References");

				static bool showWrongType = false;

				// Helper lambda to display asset field with drag-drop support
				auto DisplayAssetField = [&](const char* label, xresource::instance_guid& guid, ResourceType expectedType)
					{
						// Get the filename from the GUID
						std::string displayName = AM.getNameFromGuid(guid);
						if (displayName.empty())
						{
							displayName = "<None>";
						}

						// Create a buffer for the input text (read-only display)
						char buffer[256];
						strncpy(buffer, displayName.c_str(), sizeof(buffer) - 1);
						buffer[sizeof(buffer) - 1] = '\0';

						ImGui::Text("%s", label);
						ImGui::SameLine();

						// Input text field (read-only)
						ImGui::PushID(label);
						ImGui::InputText("##AssetRef", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

						// Drag-drop target
						if (ImGui::BeginDragDropTarget())
						{
							// Accept payload from asset browser (assuming you use "ASSET_BROWSER_ITEM" as payload ID)
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
							{
								// Assuming payload contains xresource::instance_guid
								xresource::instance_guid droppedGuid = *(const xresource::instance_guid*)payload->Data;

								// Verify the asset type matches what's expected
								const AssetRecord* record = AM.getAssetRecord(droppedGuid);
								if (record && record->type == expectedType)
								{
									// Temporary fix for now: To be fully fixed by M3
									std::string fileName = std::filesystem::path(m_Editor->GetScenePath()).filename().string();
									std::string recordName = std::filesystem::path(record->sourcePath).filename().string();

									if ((fileName == "LoveLetterAnimation.json" || fileName == "lovelettertest.json")
										&& recordName != "E005_loveletter_v001.fbx")
									{

										showWrongType = true;
									}
									else
									{

										guid = droppedGuid;

									}
								}
								else
								{
									showWrongType = true;
								}
							}
							ImGui::EndDragDropTarget();
						}

						// Context menu to clear the reference
						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::MenuItem("Clear Reference"))
							{
								guid = xresource::instance_guid(); // Reset to invalid/default
							}
							ImGui::EndPopup();
						}

						ImGui::PopID();
					};

				// Display asset reference fields
				DisplayAssetField("Mesh", mesh.MeshGuid, ResourceType::MESH);
				DisplayAssetField("Material", mesh.MaterialGuid, ResourceType::MATERIAL);

				if (showWrongType)
				{

					ImGui::OpenPopup("Incompatible Asset Type");
					showWrongType = false;
				}

				// Popup for incompatible asset type
				if (ImGui::BeginPopup("Incompatible Asset Type"))
				{
					ImGui::Text("The dropped asset type does not match the expected type.");
					if (ImGui::Button("Close"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::Spacing();

				bool globalIlluminate = mesh.GlobalIlluminate;
				if (ImGui::Checkbox("Global Illuminate", &globalIlluminate))
				{
					mesh.GlobalIlluminate = globalIlluminate;
				}

				bool shadowCast = mesh.ShadowCast;
				if (ImGui::Checkbox("Shadow Cast", &shadowCast))
				{
					mesh.ShadowCast = shadowCast;
				}

				bool shadowReceive = mesh.ShadowReceive;
				if (ImGui::Checkbox("Shadow Receive", &shadowReceive))
				{
					mesh.ShadowReceive = shadowReceive;
				}

				bool visible = mesh.Visible;
				if (ImGui::Checkbox("Visible", &visible))
				{
					mesh.Visible = visible;
				}

				// Material Editor Section
				ImGui::SeparatorText("Material Properties");

				// Save Material Button
				static char materialSaveName[256] = "";
				ImGui::InputText("Material Name", materialSaveName, sizeof(materialSaveName));
				ImGui::SameLine();
				if (ImGui::Button("Save Material"))
				{
					if (strlen(materialSaveName) > 0)
					{
						MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(mesh.MaterialGuid));
						if (material)
						{
							std::string filename = std::string(materialSaveName);
							serializeMaterial(material, filename);

							// Refresh Asset Manager to recognize new material
							AM.scanAndProcess();

							// Optional: Clear the input field after saving
							memset(materialSaveName, 0, sizeof(materialSaveName));

							// Optional: Show confirmation message
							ImGui::OpenPopup("Material Saved");
						}
					}
					else
					{
						ImGui::OpenPopup("Invalid Name");
					}
				}

				// Popup for save confirmation
				if (ImGui::BeginPopupModal("Material Saved", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Material saved successfully!");
					if (ImGui::Button("OK"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				// Popup for invalid name
				if (ImGui::BeginPopupModal("Invalid Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Please enter a valid material name.");
					if (ImGui::Button("OK"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				// Get material reference
				MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(mesh.MaterialGuid));

				if (material)
				{
					// Shader Name (read-only for now)
					ImGui::Text("Shader: %s", material->shaderName.c_str());

					// Texture Maps (PBR Metallic/Roughness)
					if (ImGui::CollapsingHeader("Texture Maps"))
					{
						DisplayAssetField("Base Map (Albedo)", material->baseMap, ResourceType::TEXTURE);
						DisplayAssetField("Normal Map", material->normalMap, ResourceType::TEXTURE);
						DisplayAssetField("Metallic Map [NOT AVAILABLE]", material->metallicMap, ResourceType::TEXTURE);
						DisplayAssetField("Roughness Map [NOT AVAILABLE]", material->roughnessMap, ResourceType::TEXTURE);
						DisplayAssetField("Emission Map [NOT AVAILABLE]", material->emissionMap, ResourceType::TEXTURE);
						DisplayAssetField("Occlusion Map [NOT AVAILABLE]", material->occlusionMap, ResourceType::TEXTURE);
					}

					// Color Properties
					if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen))
					{

						// Base Color (RGB) - no alpha, as opacity is separate
						if (ImGui::ColorEdit3("Base Color", material->baseColor.data(),
							ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB))
						{
							// Material updated
						}

						// Emission Color
						if (ImGui::ColorEdit3("Emission Color", material->emissionColor.data(),
							ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB))
						{
							// Material updated (To do in M3)
						}
					}

					// Material Properties
					if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen))
					{
						// Metallic slider
						if (ImGui::SliderFloat("Metallic", &material->metallic, 0.0f, 1.0f, "%.2f"))
						{
							// Material updated
						}

						// Roughness slider
						if (ImGui::SliderFloat("Roughness", &material->roughness, 0.0f, 1.0f, "%.2f"))
						{
							// Material updated
						}

						// Opacity slider
						if (ImGui::SliderFloat("Opacity", &material->opacity, 0.0f, 1.0f, "%.2f"))
						{
							// Material updated
						}

						// Emission Strength
						if (ImGui::SliderFloat("Emission Strength", &material->emissionStrength, 0.0f, 100.0f, "%.2f"))
						{
							// TODO
							//material->emissionStrength = std::max(0.0f, material->emissionStrength);
						}

						// Alpha Threshold for alpha testing
						if (ImGui::SliderFloat("Alpha Threshold", &material->alphaThreshold, 0.0f, 1.0f, "%.3f"))
						{
							// TODO
							//material->alphaThreshold = std::max(0.0f, std::min(1.0f, material->alphaThreshold));
						}

						// Alpha Threshold for ambient occlusion
						if (ImGui::SliderFloat("Ambient Occlusion", &material->ambientOcclusion, 0.0f, 1.0f, "%.3f"))
						{
							// TODO
							//material->ambientOcclusion = std::max(0.0f, std::min(1.0f, material->ambientOcclusion));
						}
					}

					// UV Transform (Unchanged)
					if (ImGui::CollapsingHeader("UV Transform"))
					{
						// Tiling
						if (ImGui::DragFloat2("Tiling", material->tiling.data(), 0.1f, 0.1f, 10.0f, "%.2f"))
						{
							// TODO
							// Prevent zero or negative tiling
							//material->tiling[0] = std::max(0.1f, material->tiling[0]);
							//material->tiling[1] = std::max(0.1f, material->tiling[1]);
						}

						// Offset
						if (ImGui::DragFloat2("Offset", material->offset.data(), 0.01f, -10.0f, 10.0f, "%.3f"))
						{
							// No clamping needed for offset
						}
					}

					// Render Flags
					if (ImGui::CollapsingHeader("Render Flags"))
					{
						ImGui::Checkbox("Enable Emission", &material->enableEmission);
						ImGui::Checkbox("Alpha Test", &material->alphaTest);
						ImGui::Checkbox("Double Sided", &material->doubleSided);
						ImGui::Checkbox("Receive Shadows", &material->receiveShadows);
						ImGui::Checkbox("Cast Shadows", &material->castShadows);
					}
				}

				ImGui::SeparatorText("Values for Debugging:");

				ImGui::Text("Material: %u", mesh.Material);

				// For Ease of Gameplay Programmers to Use For the Time Being
				ImU32 meshType = mesh.MeshType;
				if (ImGui::InputScalar("Mesh Type", ImGuiDataType_U32, &meshType))
				{
					if (meshType == 0 || meshType == 1 || meshType == 2)
					{
						mesh.MeshType = meshType;
					}
					else
					{
						meshType = mesh.MeshType;
					}
				}


				ImGui::Text("Submesh Index: %u", mesh.SubmeshIndex);
				ImGui::Text("Texture: %u", mesh.Texture);

			}
			// ---------------------- Remove Mesh Component by ... -------------------------
			if (removeMesh)
			{
				m_SelectedEntity.RemoveComponent<MeshRendererComponent>();
			}
		}
	}

	void EditorPropertyPanel::DisplayAudioComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<AudioComponent>())
		{
			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openAudioComponent = ImGui::CollapsingHeader("Audio Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeAudio = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###AudioBtn", buttonSize))
			{
				ImGui::OpenPopup("AudioPopUp");
			}
			if (ImGui::BeginPopup("AudioPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeAudio = true;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openAudioComponent)
			{
				auto& audio = m_SelectedEntity.GetComponent<AudioComponent>();

				ImGui::Separator();

				if (audio.AudioFilePath.empty())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // Red
					ImGui::Text("No audio file loaded. Please select and audio file below.");
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); // Green
					ImGui::Text("Audio Filename: %s", audio.AudioFilePath.c_str());
					ImGui::PopStyleColor();
				}

				// Get from Asset Manager
				auto& db = AM.db();
				auto allAssets = db.AllMutable();

				std::vector<std::string> audioAssetNames;
				audioAssetNames.reserve(allAssets.size());

				for (const auto* record : allAssets)
				{
					if (!record || !record->valid)
					{
						continue;
					}

					if (record->type == ResourceType::AUDIO)
					{
						std::string filepath = record->sourcePath;
						size_t lastSlash = filepath.find_last_of("/\\");
						std::string filename = (lastSlash == std::string::npos) ? filepath : filepath.substr(lastSlash + 1);

						audioAssetNames.push_back(filename);
					}
				}

				// Const char* for dropdown
				std::vector<const char*> audioAssets;
				audioAssets.reserve(audioAssetNames.size());
				for (auto& name : audioAssetNames)
				{
					audioAssets.push_back(name.c_str());
				}

				int currentIndex = 0;
				for (size_t i = 0; i < audioAssetNames.size(); ++i)
				{
					if (audioAssetNames[i] == audio.AudioFilePath)
					{
						currentIndex = static_cast<int>(i);
						break;
					}
				}

				// Dropdown menu
				std::string label = "Filepath";
				if (ImGui::Combo(label.c_str(), &currentIndex, audioAssets.data(), static_cast<int>(audioAssets.size())))
				{
					audio.SetAudioFile(audioAssetNames[currentIndex]);
				}

				if (audio.AudioFilePath.empty())
				{
					ImGui::SameLine();
					if (ImGui::Button("Load File"))
					{
						audio.SetAudioFile(audioAssetNames[currentIndex]);
					}
				}

				if (audio.AudioFilePath.empty())
				{
					ImGui::BeginDisabled();
				}

				ImGui::Text("Audio Type:");
				AudioType type = audio.Type;

				if (ImGui::RadioButton("SFX", type == AudioType::SFX))
				{
					audio.SetAudioType(AudioType::SFX);
				}
				if (ImGui::RadioButton("BGM", type == AudioType::BGM))
				{
					audio.SetAudioType(AudioType::BGM);
				}
				if (ImGui::RadioButton("UI", type == AudioType::UI))
				{
					audio.SetAudioType(AudioType::UI);
				}

				ImGui::Separator();
				ImGui::Text("Play State:");
				PlayState playState = audio.State;
				if (ImGui::RadioButton("Play", playState == PlayState::PLAY))
				{
					audio.SetState(PlayState::PLAY);
				}
				if (ImGui::RadioButton("Pause", playState == PlayState::PAUSE))
				{
					audio.SetState(PlayState::PAUSE);
				}
				if (ImGui::RadioButton("Stop##Audio", playState == PlayState::STOP))
				{
					audio.SetState(PlayState::STOP);
				}

				ImGui::Separator();

				float volume = audio.Volume;
				if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.f))
				{
					audio.SetVolume(volume);
				}

				float pitch = audio.Pitch;
				if (ImGui::SliderFloat("Pitch", &pitch, 0.f, 1.f))
				{
					audio.SetPitch(pitch);
				}

				ImGui::Separator();
				bool looping = audio.Loop;
				if (ImGui::Checkbox("Looping", &looping))
				{
					audio.SetLoop(looping);
				}
				bool mute = audio.Mute;
				if (ImGui::Checkbox("Mute", &mute))
				{
					audio.SetMute(mute);
				}
				bool is_3d = audio.Is3D;
				if (ImGui::Checkbox("3D", &is_3d))
				{
					audio.Set3D(is_3d);
				}

				ImGui::Separator();
				float reverb = audio.ReverbProperties;
				if (ImGui::SliderFloat("Reverb", &reverb, 0.0f, 1.0f))
				{
					audio.SetReverbProperties(reverb);
				}

				ImGui::SeparatorText("Only for 3D");

				std::string advice = "Max Distance needs to be higher than Min Distance to have attenuation";
				ImGui::TextDisabled("(i)");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(advice.c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				// Disable only if not 3D
				ImGui::BeginDisabled(!is_3d);

				float min_distance = audio.MinDistance;
				if (ImGui::SliderFloat("MinDistance", &min_distance, 0.0f, audio.MaxDistance))
				{
					if (is_3d)
					{
						audio.SetMinDistance(min_distance);
					}
					else
					{
						audio.SetMinDistance(1.f);
					}
				}

				float max_distance = audio.MaxDistance;
				if (ImGui::SliderFloat("MaxDistance", &max_distance, audio.MinDistance, 1000.f))
				{
					if (is_3d)
					{
						audio.SetMaxDistance(max_distance);
					}
					else
					{
						audio.SetMaxDistance(10.f);
					}
				}

				ImGui::EndDisabled();

				ImGui::Separator();

				float dopplerLevel = audio.DopplerLevel;
				if (ImGui::SliderFloat("Doppler", &dopplerLevel, 0.f, 5.f))
				{
					audio.SetDopplerLevel(dopplerLevel);
				}

				ImGui::Text("RollOff Mode:");
				AudioRolloffMode mode = audio.RolloffMode;

				if (ImGui::RadioButton("INVERSE", mode == AudioRolloffMode::INVERSE))
				{
					audio.SetRolloffMode(AudioRolloffMode::INVERSE);
				}
				if (ImGui::RadioButton("LINEAR", mode == AudioRolloffMode::LINEAR))
				{
					audio.SetRolloffMode(AudioRolloffMode::LINEAR);
				}
				if (ImGui::RadioButton("LINEARSQUARE", mode == AudioRolloffMode::LINEARSQUARE))
				{
					audio.SetRolloffMode(AudioRolloffMode::LINEARSQUARE);
				}

				ImGui::SeparatorText("Only for 2D");

				std::string advice2D = "Only for 2D sounds (not 3D). Controls where 2D sound is being played (From the left, right or both speakers).";
				ImGui::TextDisabled("(i)");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(advice2D.c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				ImGui::BeginDisabled(is_3d);

				float pan = audio.Pan2D;
				if (ImGui::SliderFloat("Pan", &pan, -1.f, 1.f))
				{
					audio.SetPan(pan);
				}

				ImGui::EndDisabled();

				if (audio.AudioFilePath.empty())
				{
					ImGui::EndDisabled();
				}

			}
			// ---------------------- Remove Audio Component by ... -------------------------
			if (removeAudio)
			{
				m_SelectedEntity.RemoveComponent<AudioComponent>();
			}

		}
	}

	void EditorPropertyPanel::DisplayReverbZoneComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<ReverbZoneComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openReverbComponent = ImGui::CollapsingHeader("Reverb Zone Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeReverb = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###ReverbBtn", buttonSize))
			{
				ImGui::OpenPopup("ReverbPopUp");
			}
			if (ImGui::BeginPopup("ReverbPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeReverb = true;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openReverbComponent)
			{
				auto& reverbZone = m_SelectedEntity.GetComponent<ReverbZoneComponent>();

				const char* presets[] = { "Custom", "Generic", "Bathroom", "Room", "Cave", "Arena" };
				int currentIndex = static_cast<int>(reverbZone.Preset);

				ImGui::Text("Select an option:");

				// Dropdown menu
				if (ImGui::BeginCombo("Reverb Preset", presets[currentIndex]))
				{
					for (int i = 0; i < IM_ARRAYSIZE(presets); i++)
					{
						bool isSelected = (i == currentIndex);
						if (ImGui::Selectable(presets[i], isSelected))
						{
							// Update the enum when user picks a new item
							reverbZone.Preset = static_cast<ReverbPreset>(i);
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				if (reverbZone.Preset == ReverbPreset::Custom)
				{

					float& decayTime = reverbZone.DecayTime;
					if (ImGui::SliderFloat("Decay Time", &decayTime, 100.f, 20000.f))
					{
						reverbZone.SetDecayTime(decayTime);
					}

					float& hfDecayRatio = reverbZone.HfDecayRatio;
					if (ImGui::SliderFloat("High-Frequency Decay Ratio", &hfDecayRatio, 0.f, 100.f))
					{
						reverbZone.SetHfDecayRatio(hfDecayRatio);
					}

					float& diffusion = reverbZone.Diffusion;
					if (ImGui::SliderFloat("Diffusion", &diffusion, 0.f, 100.f))
					{
						reverbZone.SetDiffusion(diffusion);
					}

					float& density = reverbZone.Density;
					if (ImGui::SliderFloat("Density", &density, 0.f, 100.f))
					{
						reverbZone.SetDensity(density);
					}

					float& wetLevel = reverbZone.WetLevel;
					if (ImGui::SliderFloat("Wet Level", &wetLevel, -80.f, 20.f))
					{
						reverbZone.SetWetLevel(wetLevel);
					}
				}

				float& minDistanceReverb = reverbZone.MinDistance;
				if (ImGui::InputFloat("MinDistance###minreverb", &minDistanceReverb))
				{
					reverbZone.SetMinDistance(minDistanceReverb);
				}

				float& maxDistanceReverb = reverbZone.MaxDistance;
				if (ImGui::InputFloat("MaxDistance###maxreverb", &maxDistanceReverb))
				{
					reverbZone.SetMaxDistance(maxDistanceReverb);
				}
			}
			//---------------------- Remove ReverbZone Component by ... -------------------------
			if (removeReverb)
			{
				m_SelectedEntity.RemoveComponent<ReverbZoneComponent>();

			}
		}
	}

	void EditorPropertyPanel::DisplayListenerComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<ListenerComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openListenerComponent = ImGui::CollapsingHeader("Listener Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeListener = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###ListenBtn", buttonSize))
			{
				ImGui::OpenPopup("ListenPopUp");
			}
			if (ImGui::BeginPopup("ListenPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeListener = true;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openListenerComponent)
			{
				auto& listener = m_SelectedEntity.GetComponent<ListenerComponent>();
				bool& active = listener.Active;

				if (ImGui::Checkbox("Active###activeListener", &active))
				{
					listener.Active = active;
				}
			}
			// -------------------------- Remove ListernerComponent -------------------------
			if (removeListener)
			{
				m_SelectedEntity.RemoveComponent<ListenerComponent>();

			}
		}
	}

	//void EditorPropertyPanel::DisplayBTComponent(ImVec2& buttonSize)

	void EditorPropertyPanel::DisplayParticleComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<ParticleComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openParticleComp = ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeParticleComp = false;

			auto& particleComp = m_SelectedEntity.GetComponent<ParticleComponent>();

			ImGui::NextColumn();

			if (ImGui::Button("...###ParticleBtn", buttonSize))
			{
				ImGui::OpenPopup("ParticlePopUp");
			}
			if (ImGui::BeginPopup("ParticlePopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeParticleComp = true;
					//return;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openParticleComp)
			{
				// Playback Controls
				ImGui::Text("Playback");
				ImGui::Checkbox("Active###activeParticle", &particleComp.Active);
				ImGui::SameLine();
				ImGui::Checkbox("Loop", &particleComp.Loop);

				ImGui::Spacing();
				ImGui::Separator();

				// Emission Settings
				if (ImGui::TreeNodeEx("Emission", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::DragInt("Max Particles", (int*)&particleComp.MaxParticles, 1.0f, 1, 10000);
					ImGui::DragFloat("Emission Rate", &particleComp.EmissionRate, 0.1f, 0.0f, 1000.0f, "%.1f particles/sec");
					ImGui::DragFloat("Particle Lifetime", &particleComp.ParticleLifetime, 0.1f, 0.1f, 100.0f, "%.1f seconds");
					ImGui::TreePop();
				}

				// Particle Appearance
				if (ImGui::TreeNodeEx("Appearance", ImGuiTreeNodeFlags_DefaultOpen))
				{
					// Particle Type Dropdown
					const char* particleTypes[] = { "Cube", "Plane", "Sphere" };
					const char* currentType = particleTypes[particleComp.ParticleType];

					if (ImGui::BeginCombo("Particle Type", currentType))
					{
						for (int i = 0; i < 3; i++)
						{
							bool isSelected = (particleComp.ParticleType == static_cast<unsigned int>(i));
							if (ImGui::Selectable(particleTypes[i], isSelected))
							{
								particleComp.ParticleType = i;
							}
							if (isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}

					ImGui::DragFloat("Particle Size", &particleComp.ParticleSize, 0.01f, 0.01f, 10.0f, "%.2f");

					ImGui::Spacing();
					ImGui::Text("Color Range");
					ImGui::ColorEdit4("Color Min", &particleComp.ColorMin.x);
					ImGui::ColorEdit4("Color Max", &particleComp.ColorMax.x);

					ImGui::TreePop();
				}

				// Particle Behavior
				if (ImGui::TreeNodeEx("Behavior", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Text("Velocity");
					ImGui::DragFloat3("Initial Velocity", &particleComp.InitialVelocity.x, 0.1f);
					ImGui::DragFloat("Min Speed", &particleComp.MinSpeed, 0.01f, 0.0f, 10.0f, "%.2f");
					ImGui::DragFloat("Max Speed", &particleComp.MaxSpeed, 0.01f, 0.0f, 10.0f, "%.2f");
					ImGui::DragFloat("Spread Angle", &particleComp.SpreadAngle, 0.5f, 0.0f, 180.0f, "%.1f degrees");

					ImGui::Spacing();
					ImGui::Text("Rotation");
					ImGui::Checkbox("Randomize Rotation", &particleComp.RandomizeRotation);
					ImGui::DragFloat("Rotation Speed", &particleComp.RotationSpeed, 1.0f, -360.0f, 360.0f, "%.1f deg/sec");

					ImGui::TreePop();
				}

				// Randomization
				if (ImGui::TreeNodeEx("Randomization"))
				{
					ImGui::DragFloat("Velocity Randomness", &particleComp.VelocityRandomness, 0.01f, 0.0f, 1.0f, "%.2f");
					ImGui::DragFloat("Lifetime Randomness", &particleComp.LifetimeRandomness, 0.01f, 0.0f, 1.0f, "%.2f");

					// Optional: Add tooltips for clarity
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("0 = no variation, 1 = maximum variation");
					}

					ImGui::TreePop();
				}

				// Statistics
				if (ImGui::TreeNode("Statistics"))
				{
					int aliveCount = 0;
					for (const auto& particle : particleComp.Particles)
					{
						if (particle.Alive) aliveCount++;
					}

					ImGui::Text("Alive: %d / %u", aliveCount, particleComp.MaxParticles);
					ImGui::Text("Pool Size: %zu", particleComp.Particles.size());
					ImGui::Text("Accumulator: %.2f", particleComp.EmissionAccumulator);
					ImGui::ProgressBar((float)aliveCount / (float)particleComp.MaxParticles);
					ImGui::TreePop();
				}

				ImGui::Spacing();
				ImGui::Separator();

				// Controls
				if (ImGui::Button("Clear Particles", ImVec2(150, 0)))
				{
					for (auto& particle : particleComp.Particles)
					{
						particle.Alive = false;
					}
				}

				ImGui::SameLine();

				if (ImGui::Button("Reset System", ImVec2(150, 0)))
				{
					particleComp.Particles.clear();
					particleComp.EmissionAccumulator = 0.0f;
				}
			}
			// ----------------------------------------- Remove Particle Component -------------------------------
			if (removeParticleComp)
			{
				m_SelectedEntity.RemoveComponent<ParticleComponent>();
			}
		}
	}

	//void EditorPropertyPanel::DisplayScriptComponent(ImVec2& buttonSize)

	void EditorPropertyPanel::DisplayLightComponent(ImVec2& buttonSize){
	
		if (m_SelectedEntity.HasComponent<LightComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openLightComp = ImGui::CollapsingHeader("Light Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeLightComp = false;

			ImGui::NextColumn();

			if (ImGui::Button("...###LightBtn", buttonSize))
			{
				ImGui::OpenPopup("LightPopUp");
			}
			if (ImGui::BeginPopup("LightPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeLightComp = true;
					//return;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openLightComp)
			{
				auto& lightComp = m_SelectedEntity.GetComponent<LightComponent>();
				ImGui::Checkbox("Enabled", &lightComp.Enabled);

				// --- Light Type Dropdown ---
				const char* lightTypeNames[] = { "Directional", "Point", "Spot" };
				int currentType = static_cast<int>(lightComp.Type);

				if (ImGui::Combo("Type", &currentType, lightTypeNames, IM_ARRAYSIZE(lightTypeNames)))
				{
					lightComp.SetType(static_cast<LightType>(currentType));
				}

				// --- Color ---
				glm::vec3 color = lightComp.Color;
				if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
				{
					lightComp.SetColorLinear(color); // uses setter
				}


				// --- Intensity ---
				float intensity = lightComp.Intensity;
				if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f, "%.2f"))
				{
					lightComp.SetIntensity(intensity); //  uses setter
				}


				// --- Range  ---
				if (lightComp.Type != LightType::Directional)
				{
					float range = lightComp.Range;
					if (ImGui::DragFloat("Range", &range, 0.1f, 0.0f, 1000.0f, "%.2f"))
					{
						lightComp.SetRange(range); // uses setter
					}
				}

				if (lightComp.Type == LightType::Spot)
				{
					float spotAngle = lightComp.SpotAngleDeg;
					if (ImGui::DragFloat("Spot Angle", &spotAngle, 0.1f, 1.0f, 179.0f, "%.2f"))
					{
						lightComp.SetSpotAngleDeg(spotAngle); // uses setter
					}
				}
				// --- Indirect Multiplier ---
				float indirectMult = lightComp.IndirectMultiplier;
				if (ImGui::DragFloat("Indirect Multiplier", &indirectMult, 0.01f, 0.0f, 10.0f, "%.2f"))
				{
					lightComp.SetIndirectMultiplier(indirectMult); // uses setter
				}

			}
			// ---------------------------- Remove Light Comp --------------------------
			if (removeLightComp)
			{
				m_SelectedEntity.RemoveComponent<LightComponent>();
			}
		}
	}

	void EditorPropertyPanel::DisplayCameraComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<CameraComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openCameraComp = ImGui::CollapsingHeader("Camera Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeCameraComp = false;
			ImGui::NextColumn();

			if (ImGui::Button("...###CameraBtn", buttonSize))
			{
				ImGui::OpenPopup("CameraPopUp");
			}
			if (ImGui::BeginPopup("CameraPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeCameraComp = true;
					//return;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openCameraComp)
			{
				auto& camComp = m_SelectedEntity.GetComponent<CameraComponent>();

				// -------------------------------------------------
				// Enabled
				// -------------------------------------------------
				ImGui::Checkbox("Enabled", &camComp.Enabled);

				// Only show the rest when the camera is enabled
				if (camComp.Enabled)
				{
					ImGui::Separator();

					// -------------------------------------------------
					// Auto aspect
					// -------------------------------------------------
					ImGui::Checkbox("Auto Aspect", &camComp.autoAspect);

					if (!camComp.autoAspect)
					{
						float aspect = camComp.Aspect;
						if (ImGui::DragFloat("Aspect", &aspect, 0.01f, 0.1f, 10.0f))
						{
							camComp.SetAspect(aspect);   // rebuilds projection
						}
					}

					// -------------------------------------------------
					// Projection type (Perspective / Orthographic)
					// -------------------------------------------------
					int projIndex = camComp.Projection ? 1 : 0;     // 0 = Persp, 1 = Ortho
					const char* projItems[] = { "Perspective", "Orthographic" };
					if (ImGui::Combo("Projection", &projIndex, projItems, IM_ARRAYSIZE(projItems)))
					{
						bool isOrtho = (projIndex == 1);
						camComp.SetProjection(isOrtho);             // rebuilds projection
					}

					// -------------------------------------------------
					// FOV or Ortho Height (Size.y)
					// -------------------------------------------------
					if (!camComp.Projection)
					{
						// Perspective show FOV
						float fov = camComp.FOV;
						if (ImGui::DragFloat("FOV", &fov, 0.1f, 10.0f, 120.0f))
						{
							camComp.SetFOV(fov);                    // rebuilds projection
						}
					}
					else
					{
						// Orthographic edit height only (Size.y)
						float orthoHeight = camComp.Size.y;
						if (ImGui::DragFloat("Ortho Height", &orthoHeight, 0.1f, 0.1f, 10000.0f))
						{
							camComp.SetSize({ camComp.Size.x, orthoHeight }); // x ignored, y used
						}
					}

					// -------------------------------------------------
					// Near / Far planes
					// -------------------------------------------------
					float nearPlane = camComp.NearPlane;
					if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, camComp.FarPlane - 0.01f))
					{
						camComp.SetNearPlane(nearPlane);           // rebuilds projection
					}

					float farPlane = camComp.FarPlane;
					if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, camComp.NearPlane + 0.01f, 10000.0f))
					{
						camComp.SetFarPlane(farPlane);             // rebuilds projection
					}

					// -------------------------------------------------
					// Target
					// -------------------------------------------------
					glm::vec3 target = camComp.Target;
					if (ImGui::DragFloat3("Target", glm::value_ptr(target), 0.1f))
					{
						camComp.SetTarget(target);                 // only affects View
					}
				}
			}

			// ---------------------- Remove Camera Comp ---------------------------
			if (removeCameraComp)
			{
				m_SelectedEntity.RemoveComponent<CameraComponent>();
			}

		}
	}

	//void EditorPropertyPanel::DisplayAnimatorComponent(ImVec2& buttonSize){}

	void EditorPropertyPanel::AddComponent(){
		ImGui::Separator();
		ImVec2 windowSize = ImGui::GetWindowSize(); // get Properties window sizes
		ImVec2  addComponetbtnSize(140, 40); // set button size

		// Calculate centered position for x axis
		ImGui::SetCursorPosX((windowSize.x - addComponetbtnSize.x) * 0.5f);

		if (ImGui::Button("Add Component", addComponetbtnSize))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Add new component.");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{

			// -------------------- Add Transform Component -------------------------
			bool hasTransform = m_SelectedEntity.HasComponent<TransformComponent>();

			ImGui::BeginDisabled(hasTransform);

			if (ImGui::MenuItem("Transform3D Component"))
			{
				if (!hasTransform)
				{
					m_SelectedEntity.AddComponent<TransformComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasTransform)
				{
					ImGui::SetTooltip("Position, rotation, and scale of the object.");
				}
			}

			ImGui::EndDisabled();

			// ------------------------ Add RigidBody Component ----------------------------
			bool hasRigidBody = m_SelectedEntity.HasComponent<RigidbodyComponent>();
			ImGui::BeginDisabled(hasRigidBody);

			if (ImGui::MenuItem("RigidBody Component"))
			{
				if (!hasRigidBody)
				{
					m_SelectedEntity.AddComponent<RigidbodyComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasRigidBody)
				{
					ImGui::SetTooltip("Simulates physical: movement, rotation, and collisions.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Mesh Component ----------------------------
			bool hasMeshRenderComponent = m_SelectedEntity.HasComponent<MeshRendererComponent>();
			ImGui::BeginDisabled(hasMeshRenderComponent);

			if (ImGui::MenuItem("MeshRenderer Component"))
			{
				if (!hasMeshRenderComponent)
				{
					m_SelectedEntity.AddComponent<MeshRendererComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasMeshRenderComponent)
				{
					ImGui::SetTooltip("Defines the visual 3D model of the object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Audio Component ----------------------------
			bool hasAudioComponent = m_SelectedEntity.HasComponent<AudioComponent>();
			ImGui::BeginDisabled(hasAudioComponent);

			if (ImGui::MenuItem("Audio Component"))
			{
				if (!hasAudioComponent)
				{
					m_SelectedEntity.AddComponent<AudioComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasAudioComponent)
				{
					ImGui::SetTooltip("Adds sound playback to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Reverb Component ----------------------------
			bool hasReverbComponent = m_SelectedEntity.HasComponent<ReverbZoneComponent>();
			ImGui::BeginDisabled(hasReverbComponent);

			if (ImGui::MenuItem("Reverb Zone Component"))
			{
				if (!hasReverbComponent)
				{
					m_SelectedEntity.AddComponent<ReverbZoneComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasReverbComponent)
				{
					ImGui::SetTooltip("Adds reverb zone to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Listener Component ----------------------------
			bool hasListenerComponent = m_SelectedEntity.HasComponent<ListenerComponent>();
			ImGui::BeginDisabled(hasListenerComponent);

			if (ImGui::MenuItem("Listener Component"))
			{
				if (!hasListenerComponent)
				{
					m_SelectedEntity.AddComponent<ListenerComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasListenerComponent)
				{
					ImGui::SetTooltip("Sets object as listener.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Behavior Tree Component ----------------------------
			bool hasBehaviorTree = m_SelectedEntity.HasComponent<BehaviourTreeComponent>();
			ImGui::BeginDisabled(hasBehaviorTree);

			if (ImGui::MenuItem("Behaviour Tree Component"))
			{
				if (!hasBehaviorTree)
				{
					m_SelectedEntity.AddComponent<BehaviourTreeComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasBehaviorTree)
				{
					ImGui::SetTooltip("Adds behaviour tree to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Particle Component ----------------------------
			bool hasParticleSystem = m_SelectedEntity.HasComponent<ParticleComponent>();
			ImGui::BeginDisabled(hasParticleSystem);

			if (ImGui::MenuItem("Particle System Component"))
			{
				if (!hasParticleSystem)
				{
					m_SelectedEntity.AddComponent<ParticleComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasParticleSystem)
				{
					ImGui::SetTooltip("Adds particle system to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Script Component ----------------------------
			bool hasScriptComponent = m_SelectedEntity.HasComponent<ScriptComponent>();
			ImGui::BeginDisabled(hasScriptComponent);

			if (ImGui::MenuItem("Script Component"))
			{
				if (!hasScriptComponent)
				{
					m_SelectedEntity.AddComponent<ScriptComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasScriptComponent)
				{
					ImGui::SetTooltip("Add script to this object.");
				}
			}
			ImGui::EndDisabled();
			// ------------------------ Add Light Component ----------------------------
			bool hasLightComponent = m_SelectedEntity.HasComponent<LightComponent>();
			ImGui::BeginDisabled(hasLightComponent);

			if (ImGui::MenuItem("Light Component"))
			{
				if (!hasLightComponent)
				{
					m_SelectedEntity.AddComponent<LightComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasLightComponent)
				{
					ImGui::SetTooltip("Add light to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Canera Component ----------------------------
			bool hasCameraComponent = m_SelectedEntity.HasComponent<CameraComponent>();
			ImGui::BeginDisabled(hasCameraComponent);

			if (ImGui::MenuItem("Camera Component"))
			{
				if (!hasCameraComponent)
				{
					m_SelectedEntity.AddComponent<CameraComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasCameraComponent)
				{
					ImGui::SetTooltip("Add camera to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Animator Component ----------------------------
			bool hasAnimatorComponent = m_SelectedEntity.HasComponent<AnimatorComponent>();
			ImGui::BeginDisabled(hasAnimatorComponent);

			if (ImGui::MenuItem("Animator Component"))
			{
				if (!hasAnimatorComponent)
				{
					m_SelectedEntity.AddComponent<AnimatorComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasAnimatorComponent)
				{
					ImGui::SetTooltip("Adds animation playback data (controller, clip index, time) to this object.");
				}
			}
			ImGui::EndDisabled();
			ImGui::EndPopup(); // end pop up for Add Component  

		}
	}

	const char* EditorPropertyPanel::ColliderTypeToString(ColliderType& colliderType) {
		if (colliderType == ColliderType::AABB) {
			return "AABB";
		}
		else if (colliderType == ColliderType::BOX) {
			return "Box";
		}
		else if (colliderType == ColliderType::MESH) {
			return "Mesh";
		}
		else if (colliderType == ColliderType::SPHERE) {
			return "Sphere";
		}
		return "Unknown";
	}
}

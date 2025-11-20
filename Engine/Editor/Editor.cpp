/**
* @file Editor.cpp
* @brief Implementation of the functions of Editor class for running the IMGUI level editor.
* @author Liliana Hanawardani (47%), Saw Hui Shan (47%), Rio Shannon Yvon Leonardo (2%), Tan Jun Rui (2%), Wai Lwin Thit (2%)
* @date September 8, 2025
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/

// Include Header Files
#include "Editor.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Transform/TransformSystem.h"
#include "../Component/LightComponent.h"

#include "../Serialization/SceneSerializer.h"
#include "../Serialization/PrefabSerializer.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Texture.h"
#include "../Scripting/ScriptSerializer.h"
#include "../Editor/EditorPropertyPanel.h"
#include "../Editor/EditorHierarchyPanel.h"
#include "../Editor/EditorViewportPanel.h"

#include "../Asset/ResourceHelpers.h"

// Animation system / storage for clips & controllers
#include "ECS/Components.h"               // for AnimatorComponent
#include "Animation/AnimationStorage.h"   // AnimationClip / AnimatorController / storage
#include "Animation/AnimationSystem.h"    // optional, but handy for context


// Include other necessary headers
#include <GLFW/glfw3.h>
#include <cctype>

// Required for quaternion to Euler conversion
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "Serialization/MaterialSerializer.h"

namespace Engine
{
	void Editor::SetScene(Engine::Scene* scene)
	{
		m_Scene = scene;
	}

	void Editor::OnInit()
	{
		if (m_Initialized)
		{
			LOG_INFO("Editor: Editor Already initialized.");
			return;
		}
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = &ImGui::GetIO(); (void)io;

		// Set config flag
		io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();

		// Set WindowRounding and ImGuiCol_WindowBg when viewport is enabled
		if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Set default pickedID
		m_PickedID = 0xFFFFFFFFu;
		
		m_Initialized = true;
	}

	void Editor::OnUpdate(Timestep ts, GLuint texhandle)
	{
		if (!m_Initialized) return;


		//Start the ImGui frame
		StartImguiFrame();

		// Enable Docking Function
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		displayTopMenu();

		renderViewport(texhandle);

		// Panel Logic
		displayPropertiesPanel();

		displayHierarchyPanel();

		displayAnimatorPanel();

		displayAssetsBrowserPanel();

		displayPerformanceProfilePanel(ts);

		displayDescriptorEditorPanel();

		//Complete Imgui rendering for the frame
		CompleteFrame();
	}

	void Editor::displayTopMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{

			if (ImGui::BeginMenu("File"))
			{
				// --------------- New Scene -------------------
				if (ImGui::MenuItem("New Scene"))
				{
					if (m_Scene)
					{
						m_Scene->GetRegistry().clear();
						currScenePath = "";
						isNewScene = true;
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Create new scene.");

				// --------------- Open Scene -------------------
				if (ImGui::MenuItem("Open Scene..."))
				{
					openScenePanel = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Open scene from file.");

				// --------------- Save Scene -------------------
				if (ImGui::MenuItem("Save Scene"))
				{
					if (isPrefabEditor)
					{
						if (!currPrefabPath.empty())
						{
							// Save the current prefab directly from the scene
							auto view = m_Scene->GetRegistry().view<TagComponent>();
							if (view.begin() != view.end())
							{
								Entity entity(*view.begin(), &m_Scene->GetRegistry());

								// Create a new prefab from the current entity
								std::string entityName = entity.GetComponent<TagComponent>().Tag;
								auto updatedPrefab = PrefabSerializer::CreateEntityPrefab(entity, entityName);

								if (updatedPrefab && PrefabSerializer::SavePrefabToFile(*updatedPrefab, currPrefabPath))
								{
									LOG_INFO("Prefab saved: {}", currPrefabPath);

									// Update the registry with the new prefab
									PrefabRegistry::Get().RegisterPrefab(updatedPrefab);
									m_TemporaryPrefabPaths.erase(currPrefabPath);
								}
							}

						
						}
					}
					else
					{
						if (!currScenePath.empty())
						{
							m_Scene->SaveToFile(currScenePath);
							m_Scene->SaveToFile(convertAssetPathToRootResources(currScenePath));

						}
						else
						{
							saveAsPanel = true;
						}
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Save current scene.");

				// --------------- Save Scene As -------------------
				if (ImGui::MenuItem("Save Scene As..."))
				{
					saveAsPanel = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Save scene as a new file.");

				ImGui::Separator();

				// ====================== Script Section ==========================
				if (ImGui::MenuItem("Open Script"))
				{
					// open script logic
					openScript = true;

				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Open script from file.");

				if (ImGui::MenuItem("New Script"))
				{
					createScript = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Create a new script.");

				ImGui::Separator();

				// --------------- Exit -------------------
				if (ImGui::MenuItem("Exit", "Alt+F4"))
				{
					glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
				}

				ImGui::EndMenu();
			}

			// To Do for in M3
			//if (ImGui::BeginMenu("Edit"))
			//{
			//	if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}  // Disabled for now
			//	if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}  // Disabled for now
			//	ImGui::Separator();
			//	if (ImGui::MenuItem("Cut", "Ctrl+X", false, false)) {}
			//	if (ImGui::MenuItem("Copy", "Ctrl+C", false, false)) {}
			//	if (ImGui::MenuItem("Paste", "Ctrl+V", false, false)) {}
			//	ImGui::EndMenu();
			//}

			// to toggle show which panel
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Hierarchy", NULL, &hierachyWindow);
				ImGui::MenuItem("Properties", NULL, &inspectorWindow);
				ImGui::MenuItem("Animator", NULL, &animatorWindow);          // <--- NEW
				ImGui::MenuItem("Performance Profile", NULL, &performanceProfileWindow);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Compile"))
			{
	
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

				AM.CompileAllAsset(0); 
				}
				

				ImGui::EndMenu();
			}

			// ---------------- Display Current Scene Name ---------------------
			if (!currScenePath.empty())
			{
				std::filesystem::path filePath(currScenePath);
				std::string fileName = filePath.string();

				float textWidth = ImGui::CalcTextSize(fileName.c_str()).x;
				float menuBarWidth = ImGui::GetWindowSize().x;

				ImGui::SameLine(menuBarWidth - textWidth - 10.0f);
				ImGui::TextUnformatted(fileName.c_str());
			}

			ImGui::EndMainMenuBar();
		}

		//  =========================== Open Scene pop up panel =====================================
		if (openScenePanel)
		{
			sceneOpenPanel();
		}

		// ========================== Save as Scene panel ============================
		if (saveAsPanel)
		{
			saveAsScenePanel();
		}

		// ========================= Create Script Panel ===========================
		if (createScript)
		{
			CreateScriptPanel();
		}
		// ========================== Open Script Panel ============================
		if (openScript)
		{
			OpenScriptPanel();
		}
	}

	void Editor::displayPropertiesPanel()
	{
		if (!inspectorWindow)
			return;

		if (ImGui::Begin("Properties", &inspectorWindow))
		{
			if (m_SelectedEntity)
			{
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

						if (!newTag.empty()) {
							tag.Tag = newTag;
						}

					}
					
				}
				
				ImGui::Separator();

				// =========================== Display TransformComponent ===========================
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
				// calculate ... button size
				ImVec2 dotTextSize = ImGui::CalcTextSize("...");
				ImVec2 dotButtonSize(dotTextSize.x + 8.0f, dotTextSize.y + 8.0f);
				
				// =========================== Display Rigid Body components ===========================
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
					
					if (ImGui::Button("...###RigidbodyBtn", dotButtonSize))
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
						
						// mass
						float rigidMass = rigidBody.GetMass();
						if (ImGui::DragFloat("Mass", &rigidMass))
						{
							rigidBody.SetMass(rigidMass);
						}

						ImGui::Separator();

						// kinematic
						ImGui::Text("Boolean to check if body is moved by code (not Physics)");
						bool& isKinematic = rigidBody.IsKinematic;
						if (ImGui::Checkbox("Is Kinematic", &isKinematic)) {
							rigidBody.SetKinematic(isKinematic);
						}

						ImGui::Separator();

						// velocity
						glm::vec3 vel = rigidBody.GetVelocity();
						if (ImGui::DragFloat3("Velocity", &vel.x, 1.0f))
						{
							rigidBody.SetVelocity(vel);
						}

						if (ImGui::Button("Stop")) {
							rigidBody.Stop();
						}

						ImGui::Separator();

						ColliderType& colliderShape = rigidBody.Shape;

						if (ImGui::BeginCombo("Collider Shape", Engine::PropertyPanelHelper::ColliderTypeToString(colliderShape))) {
							for (int i = 0; i < 4; ++i) {
								ColliderType type = (ColliderType)i;
								bool selected = (colliderShape == type);

								if (ImGui::Selectable(Engine::PropertyPanelHelper::ColliderTypeToString(type), selected)) {
									colliderShape = type;
								}

								if (selected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						switch (colliderShape)
						{

						case ColliderType::AABB:
							ImGui::Text("AABB is automatically generated from the mesh.");
							break;

						case ColliderType::BOX:
							
							ImGui::Text("Box Properties");
							ImGui::DragFloat3("Box Half Extents", &rigidBody.BoxHalfExtents.x, 1.0f);
							break;

						case ColliderType::SPHERE:
							
							ImGui::Text("Sphere Properties");
							ImGui::Text("Sphere radius is originally determined from the mesh.");
							ImGui::DragFloat("Sphere Radius", &rigidBody.SphereRadius, 1.0f);
							break;

						case ColliderType::MESH:
							ImGui::Text("Mesh collider is generated directly from the mesh.");
							break;

						default:
							break;
						}

						ImGui::Separator();

						ImGui::Text("Display Runtime Value:");
						
						ImGui::BeginDisabled();
						
						float speed = rigidBody.GetSpeed();
						ImGui::InputFloat("Speed (m/s)", &speed, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly);

						bool isMoving = rigidBody.IsMoving();
						ImGui::Checkbox("Is Moving", &isMoving);

						bool isStatic = rigidBody.IsStatic();
						ImGui::Checkbox("Is Static", &isStatic);

						ImGui::EndDisabled();
					}
					// ---------------------- Remove Rigid Body Component by ... -------------------------
					if (removeRigidBody)
					{
						m_SelectedEntity.RemoveComponent<RigidbodyComponent>();
					}				
					
				}
				// =========================== Display Mesh Render Component ===========================
				if (m_SelectedEntity.HasComponent<MeshRendererComponent>())
				{
					ImGui::Separator();
					
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openMeshComponent = ImGui::CollapsingHeader("Mesh Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeMesh = false;

					// col2: ...
					ImGui::NextColumn();
					
					if (ImGui::Button("... ###MeshBtn", dotButtonSize))
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
						
#if 1 //start of AssetReference
						// ======================= Asset Reference Section =======================
						ImGui::SeparatorText("Asset References");

						static bool showWrongType = false;

						// Helper lambda to display asset field with drag-drop support
						auto DisplayAssetField = [&](const char* label, xresource::instance_guid& guid, ResourceType expectedType) {
							// Get the filename from the GUID
							std::string displayName = AM.getNameFromGuid(guid);
							if (displayName.empty()) {
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
										std::string fileName = std::filesystem::path(currScenePath).filename().string();
										std::string recordName = std::filesystem::path(record->sourcePath).filename().string();

										if ((fileName == "LoveLetterAnimation.json" || fileName == "lovelettertest.json")
											&& recordName != "E005_loveletter_v001.fbx") {
											
											showWrongType = true;
										}
										else {

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
						DisplayAssetField("Texture", mesh.TextureGuid, ResourceType::TEXTURE);

						if (showWrongType) {

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
#endif
						bool globalIlluminate = mesh.GlobalIlluminate;
						if (ImGui::Checkbox("Global Illuminate", &globalIlluminate)) {
							mesh.GlobalIlluminate = globalIlluminate;
						}

						bool shadowCast = mesh.ShadowCast;
						if (ImGui::Checkbox("Shadow Cast", &shadowCast)) {
							mesh.ShadowCast = shadowCast;
						}

						bool shadowReceive = mesh.ShadowReceive;
						if (ImGui::Checkbox("Shadow Receive", &shadowReceive)) {
							mesh.ShadowReceive = shadowReceive;
						}

						bool visible = mesh.Visible;
						if (ImGui::Checkbox("Visible", &visible)) {
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
									material->emissionStrength = std::max(0.0f, material->emissionStrength);
								}

								// Alpha Threshold for alpha testing
								if (ImGui::SliderFloat("Alpha Threshold", &material->alphaThreshold, 0.0f, 1.0f, "%.3f"))
								{
									material->alphaThreshold = std::max(0.0f, std::min(1.0f, material->alphaThreshold));
								}

								// Alpha Threshold for ambient occlusion
								if (ImGui::SliderFloat("Ambient Occlusion", &material->ambientOcclusion, 0.0f, 1.0f, "%.3f"))
								{
									material->ambientOcclusion = std::max(0.0f, std::min(1.0f, material->ambientOcclusion));
								}
							}

							// UV Transform (Unchanged)
							if (ImGui::CollapsingHeader("UV Transform"))
							{
								// Tiling
								if (ImGui::DragFloat2("Tiling", material->tiling.data(), 0.1f, 0.1f, 10.0f, "%.2f"))
								{
									// Prevent zero or negative tiling
									material->tiling[0] = std::max(0.1f, material->tiling[0]);
									material->tiling[1] = std::max(0.1f, material->tiling[1]);
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
							if (meshType == 0 || meshType  == 1 || meshType  == 2) {
								mesh.MeshType = meshType;
							}
							else {
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
				// =========================== Display Audio Component ===========================
				if (m_SelectedEntity.HasComponent<AudioComponent>())
				{
					ImGui::Separator();

					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openAudioComponent = ImGui::CollapsingHeader("Audio Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeAudio = false;

					// col2: ...
					ImGui::NextColumn();

					if (ImGui::Button("... ###AudioBtn", dotButtonSize))
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

						// Get from Asset Manager
						auto& db = AM.db();
						auto allAssets = db.AllMutable();

						std::vector<std::string> audioAssetNames;
						audioAssetNames.reserve(allAssets.size());

						for (const auto* record : allAssets) {
							if (!record || !record->valid) { 
								continue; 
							}

							if (record->type == ResourceType::AUDIO) {
								std::string filepath = record->sourcePath;
								size_t lastSlash = filepath.find_last_of("/\\");
								std::string filename = (lastSlash == std::string::npos) ? filepath : filepath.substr(lastSlash + 1);

								audioAssetNames.push_back(filename);
							}
						}

						// Const char* for dropdown
						std::vector<const char*> audioAssets;
						audioAssets.reserve(audioAssetNames.size());
						for (auto& name : audioAssetNames) {
							audioAssets.push_back(name.c_str());
						}

						int currentIndex = 0;
						for (size_t i = 0; i < audioAssetNames.size(); ++i) {
							if (audioAssetNames[i] == audio.AudioFilePath) {
								currentIndex = static_cast<int>(i);
								break;
							}
						}

						// Dropdown menu
						std::string label = "Filepath";
						if (ImGui::Combo(label.c_str(), &currentIndex, audioAssets.data(), static_cast<int>(audioAssets.size()))) {
							audio.SetAudioFile(audioAssetNames[currentIndex]);
						}

						ImGui::Text("Audio Type:");
						AudioType type = audio.Type;

						if (ImGui::RadioButton("SFX", type == AudioType::SFX)) {
							audio.SetAudioType(AudioType::SFX);
						}
						if (ImGui::RadioButton("BGM", type == AudioType::BGM)) {
							audio.SetAudioType(AudioType::BGM);
						}
						if (ImGui::RadioButton("UI", type == AudioType::UI)) {
							audio.SetAudioType(AudioType::UI);
						}

						ImGui::Separator();
						ImGui::Text("Play State:");
						PlayState playState = audio.State;
						if (ImGui::RadioButton("Play", playState == PlayState::PLAY)) {
							audio.SetState(PlayState::PLAY);
						}
						if (ImGui::RadioButton("Pause", playState == PlayState::PAUSE)) {
							audio.SetState(PlayState::PAUSE);
						}
						if (ImGui::RadioButton("Stop", playState == PlayState::STOP)) {
							audio.SetState(PlayState::STOP);
						}

						ImGui::Separator();

						float volume = audio.Volume;
						if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.f)) {
							audio.SetVolume(volume);
						}

						float pitch = audio.Pitch;
						if (ImGui::SliderFloat("Pitch", &pitch, 0.f, 1.f)) {
							audio.SetPitch(pitch);
						}

						ImGui::Separator();
						bool looping = audio.Loop;
						if (ImGui::Checkbox("Looping", &looping)) {
							audio.SetLoop(looping);
						}
						bool mute = audio.Mute;
						if (ImGui::Checkbox("Mute", &mute)) {
							audio.SetMute(mute);
						}
						bool is_3d = audio.Is3D;
						if (ImGui::Checkbox("3D", &is_3d)) {
							audio.Set3D(is_3d);
						}

						ImGui::Separator();
						float reverb = audio.ReverbProperties;
						if (ImGui::SliderFloat("Reverb", &reverb, 0.0f, 1.0f)) {
							audio.SetReverbProperties(reverb);
						}

						ImGui::Separator();

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
						if (ImGui::SliderFloat("MinDistance", &min_distance, 0.0f, audio.MaxDistance)) {
							if (is_3d) {
								audio.SetMinDistance(min_distance);
							}
							else {
								audio.SetMinDistance(1.f);
							}
						}

						float max_distance = audio.MaxDistance;
						if (ImGui::SliderFloat("MaxDistance", &max_distance, audio.MinDistance, 1000.f)) {
							if (is_3d) {
								audio.SetMaxDistance(max_distance);
							}
							else {
								audio.SetMaxDistance(10.f);
							}
						}

						ImGui::EndDisabled();

					}
					// ---------------------- Remove Audio Component by ... -------------------------
					if (removeAudio)
					{
						m_SelectedEntity.RemoveComponent<AudioComponent>();
					}
					
				}
				// ========================== Display ReverbZoneComponent =====================================
				if (m_SelectedEntity.HasComponent<ReverbZoneComponent>())
				{
					ImGui::Separator();
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openReverbComponent = ImGui::CollapsingHeader("Reverb Zone Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeReverb = false;

					// col2: ...
					ImGui::NextColumn();

					if (ImGui::Button("... ###ReverbBtn", dotButtonSize))
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

								if (isSelected){
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						if (reverbZone.Preset == ReverbPreset::Custom) {
							
							float& decayTime = reverbZone.DecayTime;
							if (ImGui::SliderFloat("Decay Time", &decayTime, 100.f, 20000.f)) {
								reverbZone.SetDecayTime(decayTime);
							}

							float& hfDecayRatio = reverbZone.HfDecayRatio;
							if (ImGui::SliderFloat("High-Frequency Decay Ratio", &hfDecayRatio, 0.f, 100.f)) {
								reverbZone.SetHfDecayRatio(hfDecayRatio);
							}

							float& diffusion = reverbZone.Diffusion;
							if (ImGui::SliderFloat("Diffusion", &diffusion, 0.f, 100.f)) {
								reverbZone.SetDiffusion(diffusion);
							}

							float& density = reverbZone.Density;
							if (ImGui::SliderFloat("Density", &density, 0.f, 100.f)) {
								reverbZone.SetDensity(density);
							}

							float& wetLevel = reverbZone.WetLevel;
							if (ImGui::SliderFloat("Wet Level", &wetLevel, -80.f, 20.f)) {
								reverbZone.SetWetLevel(wetLevel);
							}
						}

						float& minDistanceReverb = reverbZone.MinDistance;
						if (ImGui::InputFloat("MinDistance###minreverb", &minDistanceReverb)) {
							reverbZone.SetMinDistance(minDistanceReverb);
						}

						float& maxDistanceReverb = reverbZone.MaxDistance;
						if (ImGui::InputFloat("MaxDistance###maxreverb", &maxDistanceReverb)) {
							reverbZone.SetMaxDistance(maxDistanceReverb);
						}	
					}
					//---------------------- Remove ReverbZone Component by ... -------------------------
					if (removeReverb)
					{
						m_SelectedEntity.RemoveComponent<ReverbZoneComponent>();

					}
				}
				// ====================================== Display ListenerComponent ==================================
				if (m_SelectedEntity.HasComponent<ListenerComponent>())
				{
					ImGui::Separator();
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openListenerComponent = ImGui::CollapsingHeader("Listener Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeListener = false;

					// col2: ...
					ImGui::NextColumn();

					if (ImGui::Button("... ###ListenBtn", dotButtonSize))
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

						if (ImGui::Checkbox("Active###activeListener", &active)) {
							listener.Active = active;
						}
					}
					// -------------------------- Remove ListernerComponent -------------------------
					if (removeListener)
					{
						m_SelectedEntity.RemoveComponent<ListenerComponent>();

					}
				}
				// =============================== Display BT Component =========================
				if (m_SelectedEntity.HasComponent<BehaviourTreeComponent>())
				{
					ImGui::Separator();
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openBTComponent = ImGui::CollapsingHeader("Behaviour Tree Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeBTComponent = false;

					ImGui::NextColumn();

					if (ImGui::Button("...###BTBtn", dotButtonSize))
					{
						ImGui::OpenPopup("BTPopUp");
					}
					if (ImGui::BeginPopup("BTPopUp"))
					{
						if (ImGui::MenuItem("Remove Component"))
						{
							removeBTComponent = true;
						}
						ImGui::EndPopup();
					}

					ImGui::Columns(1);

					if (openBTComponent) 
					{
						auto& ai_bt = m_SelectedEntity.GetComponent<BehaviourTreeComponent>();

						// Getting BT itself
						BehaviourTree& treeInstance = *(ai_bt.TreeInstance);
						if (ai_bt.TreeInstance)
						{
							size_t stackDepth = treeInstance.GetStackDepth();
							auto root = treeInstance.GetRootNode();

							char treeBuffer[256];
							strncpy_s(treeBuffer, sizeof(treeBuffer), treeInstance.GetName().c_str(), _TRUNCATE);
							if (ImGui::InputText("Tree", treeBuffer, sizeof(treeBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
							{
								std::string newName = treeBuffer;
								newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);
								newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));

								if (!newName.empty()) {
									treeInstance.SetName(newName);
									ai_bt.TreeAssetPath = newName + ".json";
								}

							}

							ImGui::Text("Current Asset Path: %s", ai_bt.TreeAssetPath.c_str());
							ImGui::Text("Stack Depth: %zu", stackDepth);

							if (root)
							{
								ImGui::Text("Current Root: %s [%s]", root->GetName().c_str(), root->GetTypeName());
								Engine::PropertyPanelHelper::DrawBTNodeEditor(root);
							}
							else
							{
								ImGui::TextDisabled("No root node.");
								if (ImGui::Button("Create Root Node"))
								{
									auto rootNode = BehaviourTreeEditor::CreateNode("Selector");
									treeInstance.SetRootNode(rootNode);
								}
							}

							// --- Set Root Node ---
							ImGui::Separator();
							ImGui::Text("Root Node:");

							static int rootNodeTypeIndex = 0;
							auto allTypes = BehaviourTreeEditor::GetNodeTypesByCategory("Composite");
							ImGui::SetNextItemWidth(200.0f);
							if (ImGui::Combo("Node Type##Root", &rootNodeTypeIndex,
								[](void* data, int idx, const char** outText) -> bool {
									auto& types = *static_cast<std::vector<std::string>*>(data);
									*outText = types[idx].c_str();
									return true;
								},
								static_cast<void*>(&allTypes), (int)allTypes.size()))
							{
								;
							}

							// Button to create/set the root node
							if (ImGui::Button("Set Root Node"))
							{
								auto newRoot = BehaviourTreeEditor::CreateNode(allTypes[rootNodeTypeIndex]);
								treeInstance.SetRootNode(newRoot);
							}

							ImGui::Separator();

							// Reset the tree to initial state
							if (ImGui::Button("Reset")) {
								ai_bt.Reset();
							}

							ImGui::BeginDisabled();

							// Last execution status
							BTStatus& lastStatus = ai_bt.LastStatus;
							std::string lastStatusString{};
							if (lastStatus == BTStatus::Success) {
								lastStatusString = "Success";
							}
							else if (lastStatus == BTStatus::Failure) {
								lastStatusString = "Failure";
							}
							else {
								lastStatusString = "Running";
							}
							ImGui::Text("Execution Status: %s", lastStatusString.c_str());

							ImGui::EndDisabled();

							// Whether tree executes every frame
							bool& active = ai_bt.Active;
							if (ImGui::Checkbox("Active###activeBT", &active)) {
								ai_bt.Active = active;
							}

							// Reset the tree when it completes
							bool& resetComplete = ai_bt.ResetOnComplete;
							if (ImGui::Checkbox("Reset On Complete", &resetComplete)) {
								ai_bt.ResetOnComplete = resetComplete;
							}

							// Reference to current asset path
							std::string& treeAssetPath = ai_bt.TreeAssetPath;

							// Find BT folder

							std::filesystem::path repoRoot = getRepository();
							std::filesystem::path btPath = repoRoot / "Resources" / "Sources";

							auto folders = getAssetsInFolder(btPath.string());
							std::string btFolderPath;
							for (auto& folder : folders)
							{
								if (folder.name == "BT")
								{
									btFolderPath = folder.fullPath;
									break;
								}
							}

							// Collect all JSON assets in BT folder
							std::vector<AssetEntry> btAssets;
							if (!btFolderPath.empty())
							{
								auto files = getAssetsInFolder(btFolderPath);
								for (auto& f : files)
								{
									if (f.name.size() >= 5 && f.name.substr(f.name.size() - 5) == ".json")
										btAssets.push_back(f);
								}
							}

							// Determine current selection index
							int currentIndex = 0;
							for (size_t i = 0; i < btAssets.size(); ++i)
							{
								if (btAssets[i].name == treeAssetPath)
								{
									currentIndex = (int)i;
									break;
								}
							}

							// Draw the combo box
							ImGui::Text("Choose Tree Asset Path:");
							ImGui::SetNextItemWidth(400.0f);
							if (ImGui::Combo("##TreeAssetPath", &currentIndex,
								[](void* data, int idx, const char** outText) -> bool
								{
									auto& assets = *static_cast<std::vector<AssetEntry>*>(data);
									*outText = assets[idx].name.c_str();

									return true;
								},
								static_cast<void*>(&btAssets), (int)btAssets.size()))
							{	
								if (currentIndex >= 0 && currentIndex < (int)btAssets.size())
								{
									treeAssetPath = btAssets[currentIndex].name;
								}
							}

							if (ImGui::Button("Load Tree"))
							{
								if (currentIndex >= 0 && currentIndex < (int)btAssets.size())
								{
									std::string chosenPath = btAssets[currentIndex].name;
									ai_bt.TreeInstance = BehaviourTreeEditor::LoadTree(chosenPath);
								}
							}

							if (ImGui::Button("Save Tree")) {
								BehaviourTreeEditor::SaveTree(treeInstance, ai_bt.TreeAssetPath);
							}

							static char changeNewNameBuffer[256] = "";
							static char saveNewFileName[256] = "";  // Changed from saveNewTreePath - clearer naming
							static char saveNewTreeName[256] = "";

							if (ImGui::Button("Rename Tree File")) {
								strncpy_s(changeNewNameBuffer, sizeof(changeNewNameBuffer), ai_bt.TreeAssetPath.c_str(), _TRUNCATE);
								ImGui::OpenPopup("TreeRename Panel");
							}

							if (ImGui::Button("Save Tree File As")) {
								strncpy_s(saveNewFileName, sizeof(saveNewFileName), ai_bt.TreeAssetPath.c_str(), _TRUNCATE);
								strncpy_s(saveNewTreeName, sizeof(saveNewTreeName), treeInstance.GetName().c_str(), _TRUNCATE);
								ImGui::OpenPopup("SaveTreeRename Panel");
							}

							// Rename Tree Panel
							if (ImGui::BeginPopupModal("TreeRename Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
							{
								ImGui::Text("Current file: %s", ai_bt.TreeAssetPath.c_str());
								ImGui::Separator();

								ImGui::Text("Enter file name ('.json' will be added automatically):");
								ImGui::InputText("New Tree Filename", changeNewNameBuffer, sizeof(changeNewNameBuffer));

								if (ImGui::Button("Rename File", ImVec2(120, 0))) {
									std::string newFileName = changeNewNameBuffer;
									if (!newFileName.empty()) {
										std::string saveTreeName = newFileName + ".json";
										BehaviourTreeEditor::RenameFile(ai_bt.TreeAssetPath, saveTreeName, m_Scene);
										ImGui::CloseCurrentPopup();
									}
								}

								ImGui::SameLine();

								if (ImGui::Button("Cancel###RenameCancel", ImVec2(120, 0))) {  // Fixed ID
									ImGui::CloseCurrentPopup();
								}

								ImGui::EndPopup();
							}

							// Save As Panel
							if (ImGui::BeginPopupModal("SaveTreeRename Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
							{
								ImGui::Text("Save tree as new file");
								ImGui::Separator();

								ImGui::Text("Current Asset Path: %s", ai_bt.TreeAssetPath.c_str());
								ImGui::Text("Enter file name ('.json' will be added automatically):");
								ImGui::InputText("New Filename", saveNewFileName, sizeof(saveNewFileName));
								ImGui::InputText("New Tree Name", saveNewTreeName, sizeof(saveNewTreeName));

								if (ImGui::Button("Save As Tree File", ImVec2(120, 0))) {
									std::string newSaveFileName = saveNewFileName;
									std::string newSaveTreeName = saveNewTreeName;
									if (!newSaveFileName.empty() && !newSaveTreeName.empty()) {
										std::string saveFileName = newSaveFileName + ".json";
										BehaviourTreeEditor::SaveAs(ai_bt.TreeAssetPath, saveFileName, newSaveTreeName, true);
										ImGui::CloseCurrentPopup();
									}
								}

								ImGui::SameLine();

								if (ImGui::Button("Cancel###SaveAsCancel", ImVec2(120, 0))) {  // Fixed ID
									ImGui::CloseCurrentPopup();
								}

								ImGui::EndPopup();
							}

						}
						else {
							ai_bt.TreeInstance = BehaviourTreeEditor::CreateNewTree("PlaceholderTreeName");
						}
						
					}
					// ----------------------------------- Remove BT Component -----------------------
					if (removeBTComponent)
					{
						m_SelectedEntity.RemoveComponent<BehaviourTreeComponent>();
					}
				}

				// ======================== Display Particle System Component ===============================
				if (m_SelectedEntity.HasComponent<ParticleComponent>())
				{
					ImGui::Separator();
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openParticleComp = ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeParticleComp = false;

					auto& particleComp = m_SelectedEntity.GetComponent<ParticleComponent>();

					ImGui::NextColumn();

					if (ImGui::Button("...###ParticleBtn", dotButtonSize))
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
							for (const auto& particle : particleComp.Particles) {
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
							for (auto& particle : particleComp.Particles) {
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
				// ========================= Display Script Compoment ===============================
				if (m_SelectedEntity.HasComponent<ScriptComponent>())
				{
					ImGui::Separator();
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openScriptComp = ImGui::CollapsingHeader("Script Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeScriptComp = false;

					auto& scriptComp = m_SelectedEntity.GetComponent<ScriptComponent>();
					std::string scriptPath = getRepository();
					auto scriptFiles = getAssetsInFolder(scriptPath);

					ImGui::NextColumn();
					if (ImGui::Button("...##ScriptBtn", dotButtonSize))
						ImGui::OpenPopup("ScriptPopUp");

					if (ImGui::BeginPopup("ScriptPopUp"))
					{
						if (ImGui::MenuItem("Remove Component"))
							removeScriptComp = true;
						ImGui::EndPopup();
					}

					ImGui::Columns(1);

					if (openScriptComp)
					{
						ImGui::Text("Instance: %s", scriptComp.ScriptInstance ? "Active" : "None");
						ImGui::Text("Started: %s", scriptComp.Started ? "Yes" : "No");

						if (!scriptFiles.empty())
						{
							if (ImGui::BeginCombo("Select Script", scriptComp.ScriptClassName.empty() ? "None" : scriptComp.ScriptClassName.substr(scriptComp.ScriptClassName.find_last_of('.') + 1).c_str()))
							{
								for (const auto& asset : scriptFiles)
								{
									std::string className = asset.name;
									if (className.ends_with(".cs"))
										className = className.substr(0, className.size() - 3); // remove extension

									std::string selectedClassName = "Game." + className;
									bool isSelected = scriptComp.ScriptClassName == selectedClassName;

									if (ImGui::Selectable(className.c_str(), isSelected))
									{
										// Destroy previous script instance if exists
										if (scriptComp.ScriptInstance)
										{
											MonoScriptEngine::GetInstance().DestroyScriptInstance((MonoObject*)scriptComp.ScriptInstance);
											scriptComp.ScriptInstance = nullptr;
											scriptComp.Started = false;
										}

										// Assign the new script class
										scriptComp.ScriptClassName = selectedClassName;
										scriptComp.ScriptInstance = MonoScriptEngine::GetInstance().CreateScriptInstance(scriptComp.ScriptClassName);

										if (scriptComp.ScriptInstance)
										{
											//MonoScriptEngine::GetInstance().SetFieldValue((MonoObject*)scriptComp.ScriptInstance, "EntityID", m_SelectedEntity);
											MonoScriptEngine::GetInstance().CallMethod((MonoObject*)scriptComp.ScriptInstance, "OnStart");
											scriptComp.Started = true;
										}
									}

									if (isSelected)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}

							// ===== NEW: DISPLAY SERIALIZED FIELDS =====
							ImGui::Separator();
							ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Serialized Fields:");
							ImGui::Separator();

							// THIS IS THE KEY LINE - renders all [SerializeField] fields
							if (scriptComp.ScriptInstance)
							{
								RenderSerializedFieldsInImGui((MonoObject*)scriptComp.ScriptInstance);
							}
							else
							{
								ImGui::TextDisabled("(No script instance)");
							}



							if (ImGui::Button("Save Script Fields To JSON")) {
								if (scriptComp.ScriptInstance)
									SerializeScriptToDiskRapidJSON((MonoObject*)scriptComp.ScriptInstance, "SavedScriptFields.json");
							}

							// Similarly, add a load button to test deserialization:
							ImGui::SameLine();
							if (ImGui::Button("Load Script Fields From JSON")) {
								if (scriptComp.ScriptInstance)
									DeserializeScriptFromDiskRapidJSON((MonoObject*)scriptComp.ScriptInstance, "SavedScriptFields.json");
							}
							// ===== END NEW SERIALIZED FIELDS =====
						}
					}

					// Remove Script Component
					if (removeScriptComp)
						m_SelectedEntity.RemoveComponent<ScriptComponent>();
				}
				// ================================ Display Light Component ======================================
				if (m_SelectedEntity.HasComponent<LightComponent>())
				{
					ImGui::Separator();
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openLightComp = ImGui::CollapsingHeader("Light Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeLightComp = false;

					ImGui::NextColumn();

					if (ImGui::Button("...###LightBtn", dotButtonSize))
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

				// ============================ Display Camera Comp ===============================
				displayCameraComp(dotButtonSize);

				// =========================== Display Animator Component ===========================
				if (m_SelectedEntity.HasComponent<AnimatorComponent>())
				{
					ImGui::Separator();
					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnWidth(0, 200.0f);

					bool openAnimatorComponent = ImGui::CollapsingHeader("Animator Component", ImGuiTreeNodeFlags_DefaultOpen);
					bool removeAnimator = false;

					// Column 2: "..." button to remove component
					ImGui::NextColumn();

					if (ImGui::Button("... ###AnimatorBtn", dotButtonSize))
					{
						ImGui::OpenPopup("AnimatorPopUp");
					}
					if (ImGui::BeginPopup("AnimatorPopUp"))
					{
						if (ImGui::MenuItem("Remove Component"))
						{
							removeAnimator = true;
						}
						ImGui::EndPopup();
					}

					ImGui::Columns(1);

					if (openAnimatorComponent)
					{
						auto& animator = m_SelectedEntity.GetComponent<AnimatorComponent>();

						ImGui::Text("Controller Handle: %u", animator.controller);

						// Basic animator state controls
						ImGui::Checkbox("Playing", &animator.playing);
						ImGui::Checkbox("Respect Clip Loop", &animator.respectClipLoop);

						ImGui::DragFloat("Playback Speed", &animator.playbackSpeed, 0.01f, -5.0f, 5.0f);
						ImGui::DragInt("Current Clip Index", (int*)(&animator.currentClipIndex), 1.0f, 0, 100);
						ImGui::DragFloat("Current Time", &animator.currentTime, 0.01f, 0.0f, 1000.0f);

						if (ImGui::Button("Restart Clip"))
						{
							animator.currentTime = 0.0f;
						}
						ImGui::SameLine();
						if (ImGui::Button("Stop##AnimatorComp"))
						{
							animator.currentTime = 0.0f;
							animator.playing = false;
						}

						ImGui::Separator();
						ImGui::TextWrapped("Use the Animator window (View -> Animator) to edit "
							"controllers, clips, and keyframes.");

						// Convenience: open & focus Animator window from here
						if (ImGui::Button("Open Animator Window"))
						{
							animatorWindow = true;
							m_FocusAnimatorNextFrame = true;
						}
					}

					if (removeAnimator)
					{
						m_SelectedEntity.RemoveComponent<AnimatorComponent>();
					}
				}

				// ======================== Add Component Section ===============================
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
			else
			{
				ImGui::Text("No entity selected");
			}
		}
		ImGui::End();
	}

	void Editor::displayHierarchyPanel()
	{
		if (!hierachyWindow)
			return;

		if (ImGui::Begin("Hierarchy", &hierachyWindow))
		{
			if (!isPrefabEditor)
			{
				
				if (ImGui::Button("Create Entity"))
				{
					
					ImGui::OpenPopup("CreateEntityPopup");
				}
			}
			if (ImGui::BeginPopup("CreateEntityPopup"))
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					auto entity = m_Scene->CreateEntity("New Entity");
					entity.AddComponent<TagComponent>("New Entity");
					entity.AddComponent<TransformComponent>();
					ImGui::Separator();
				}
				auto prefabFiles = getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));
				ImGui::BeginDisabled(prefabFiles.empty());

				if (ImGui::MenuItem("Create Entity From Prefab"))
				{	
					ImGui::CloseCurrentPopup();
					createEttFromPrfab = true;
				}
				ImGui::EndDisabled();
				ImGui::EndPopup(); // end pop up of the CreateEntityPopup
			}
			
			// List all entities
			if (m_Scene)
			{
				auto view = m_Scene->GetRegistry().view<TagComponent>();

				for (auto entityHandle : view)
				{
					Entity entity(entityHandle, &m_Scene->GetRegistry());
					auto& transform = entity.GetComponent<TransformComponent>();
					if (transform.Parent == u32_max)
					{
						EditorHierarchyHelper::DrawEntityParentAndChildren(entity, m_Scene, m_SelectedEntity, m_PickedID,
							m_CurrentPrefab, m_TemporaryPrefabPaths, currPrefabPath, replacePrefabPending, selectedPrefabPath);
					}
					
				}

				if (EditorHierarchyHelper::openAttachEntityPopup)
				{
					ImGui::OpenPopup("Main Entity Selection");
					EditorHierarchyHelper::openAttachEntityPopup = false;
				}

				if (EditorHierarchyHelper::openSubEntityFromPrefabPopup)
				{
					ImGui::OpenPopup("Select Prefab For Sub-Entity");
					EditorHierarchyHelper::openSubEntityFromPrefabPopup = false;
				}

				//---------------------------------//
				if (ImGui::BeginPopupModal("Main Entity Selection", nullptr, ImGuiWindowFlags_NoDocking))
				{
					ImGui::SetWindowSize(ImVec2(500, 400), ImGuiCond_Once);

					if (EditorHierarchyHelper::entityToAttach && EditorHierarchyHelper::entityToAttach.HasComponent<TagComponent>())
					{
						ImGui::Text("Select a main entity to attach '%s' to:",
							EditorHierarchyHelper::entityToAttach.GetComponent<TagComponent>().Tag.c_str());
					}
					ImGui::Separator();

					auto view = m_Scene->GetRegistry().view<TagComponent>();
					for (auto entityHandle : view)
					{
						Entity entity(entityHandle, &m_Scene->GetRegistry());
						auto& transform = entity.GetComponent<TransformComponent>();

						// Only show main entities (no parent) that aren't the entity itself
						if (transform.Parent == u32_max && entity != EditorHierarchyHelper::entityToAttach)
						{
							auto& tag = entity.GetComponent<TagComponent>();
							if (ImGui::Selectable(tag.Tag.c_str()))
							{
								// Attach entityToAttach as child of selected entity
								auto& parentTransform = entity.GetComponent<TransformComponent>();
								parentTransform.Children.push_back((uint32_t)EditorHierarchyHelper::entityToAttach);

								auto& childTransform = EditorHierarchyHelper::entityToAttach.GetComponent<TransformComponent>();
								childTransform.SetParent(entity);

								ImGui::CloseCurrentPopup();
								break;
							}
						}
					}

					ImGui::Separator();
					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
				//-----------------------//

				if (ImGui::BeginPopupModal("Select Prefab For Sub-Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					auto prefabFiles = getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));
					for (auto& file : prefabFiles)
					{
						if (ImGui::Selectable(file.name.c_str()))
						{
							EditorHierarchyHelper::openSubEntityFromPrefabPopup = false;

							auto prefab = PrefabSerializer::LoadPrefabFromFile(file.fullPath);
							if (!prefab)
							{
								ImGui::CloseCurrentPopup();
								break;
							}

							PrefabRegistry::Get().RegisterPrefab(prefab);
							Entity newEntity = PrefabInstantiator::InstantiateEntityPrefab(
								m_Scene,
								prefab->GetGUID()
							);

							// Attach entityToAttach as child of selected entity
							auto& parentTransform = EditorHierarchyHelper::parentOfPrefabEntity.GetComponent<TransformComponent>();
							parentTransform.Children.push_back((uint32_t)newEntity);

							auto& childTransform = newEntity.GetComponent<TransformComponent>();
							childTransform.SetParent(EditorHierarchyHelper::parentOfPrefabEntity);

							m_SelectedEntity = newEntity;
							ImGui::CloseCurrentPopup();
							break;
						}
					}

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

			}

		}

		ImGui::End(); // End of the properties window

		// ================= Modal Popup for Replacing Prefab ===================================
		if (replacePrefabPending)
		{
			
			ImGui::OpenPopup("Select Prefab");
			replacePrefabPending = false;
		}
		
		if (ImGui::BeginPopupModal("Select Prefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			auto prefabFiles = getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));
			for (auto& file : prefabFiles)
			{
				if (ImGui::Selectable(file.name.c_str()))
				{
					selectedPrefabPath = file.fullPath;
					replacePrefabPending = false;

					auto prefab = PrefabSerializer::LoadPrefabFromFile(selectedPrefabPath);
					if (!prefab || !m_SelectedEntity)
					{
						ImGui::CloseCurrentPopup();
						return;
					}
					PrefabRegistry::Get().RegisterPrefab(prefab);

					m_Scene->DestroyEntity(m_SelectedEntity);

					Entity newEntity = PrefabInstantiator::InstantiateEntityPrefab(
						m_Scene,
						prefab->GetGUID()
					);

					m_SelectedEntity = newEntity;
				}
			}

			if (ImGui::Button("Cancel"))
			{
				replacePrefabPending = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// ================= Modal Popup for Create Entity from Prefab ===================================
		if (createEttFromPrfab)
		{
			ImGui::OpenPopup("createEttPrefab");
			createEttFromPrfab = false;
		}

		if (ImGui::BeginPopupModal("createEttPrefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			auto prefabFiles = getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));

			for (auto& file : prefabFiles)
			{
				if (ImGui::Selectable(file.name.c_str()))
				{
					createEttFromPrfab = false;
					auto prefab = PrefabSerializer::LoadPrefabFromFile(file.fullPath);
					if (!prefab)
					{
						ImGui::CloseCurrentPopup();
						break;
					}

					PrefabRegistry::Get().RegisterPrefab(prefab);
					Entity newEntity = PrefabInstantiator::InstantiateEntityPrefab(
						m_Scene,
						prefab->GetGUID()
					);
					
					m_SelectedEntity = newEntity;
					ImGui::CloseCurrentPopup();
					break;
				}
			}
		
			if (ImGui::Button("Cancel"))
			{
				createEttFromPrfab = false;
				ImGui::CloseCurrentPopup();
			}
			

			ImGui::EndPopup();
		}
	}

	void Editor::displayAnimatorPanel()
	{
		if (!animatorWindow)
			return;

		// If requested, focus this window on the next frame
		if (m_FocusAnimatorNextFrame)
		{
			ImGui::SetNextWindowFocus();
			m_FocusAnimatorNextFrame = false;
		}

		if (!ImGui::Begin("Animator", &animatorWindow))
		{
			ImGui::End();
			return;
		}

		if (!m_Scene)
		{
			ImGui::TextUnformatted("No scene loaded.");
			ImGui::End();
			return;
		}

		if (!m_SelectedEntity)
		{
			ImGui::TextUnformatted("No entity selected.");
			ImGui::End();
			return;
		}

		// Selected entity must have an AnimatorComponent
		if (!m_SelectedEntity.HasComponent<AnimatorComponent>())
		{
			ImGui::TextUnformatted("Selected entity has no AnimatorComponent.");
			ImGui::End();
			return;
		}

		auto& animator = m_SelectedEntity.GetComponent<AnimatorComponent>();

		// ---- Look up controller from global storage ----
		auto ctrlIt = m_AnimatorControllerStorage.find(animator.controller);
		if (ctrlIt == m_AnimatorControllerStorage.end())
		{
			ImGui::Text("AnimatorController handle %u is invalid.", animator.controller);
			ImGui::End();
			return;
		}

		AnimatorController& controller = ctrlIt->second;

		if (controller.clips.empty())
		{
			ImGui::TextUnformatted("AnimatorController has no clips.");
			ImGui::End();
			return;
		}

		// Clamp current clip index
		if (animator.currentClipIndex < 0 ||
			animator.currentClipIndex >= static_cast<int>(controller.clips.size()))
		{
			animator.currentClipIndex = 0;
		}

		u32 clipHandle = controller.clips[static_cast<size_t>(animator.currentClipIndex)];

		// Look up active clip
		AnimationClip* clipPtr = nullptr;
		{
			auto clipIt = m_AnimationClipStorage.find(clipHandle);
			if (clipIt != m_AnimationClipStorage.end())
				clipPtr = &clipIt->second;
		}

		if (!clipPtr)
		{
			ImGui::Text("Active clip handle %u is invalid.", clipHandle);
			ImGui::End();
			return;
		}

		AnimationClip& clip = *clipPtr;

		// -----------------------------------------------------------------
		// Header: entity + controller + clip selection
		// -----------------------------------------------------------------
		ImGui::Text("Entity: %s", m_SelectedEntity.GetComponent<TagComponent>().Tag.c_str());
		ImGui::Text("Controller: %s", controller.name.c_str());

		// Clip combo
		{
			std::string previewName = clip.name.empty() ? "Unnamed Clip" : clip.name;

			if (ImGui::BeginCombo("Clip", previewName.c_str()))
			{
				for (int i = 0; i < static_cast<int>(controller.clips.size()); ++i)
				{
					u32 h = controller.clips[static_cast<size_t>(i)];
					auto it = m_AnimationClipStorage.find(h);

					const char* name = "(missing)";
					if (it != m_AnimationClipStorage.end() && !it->second.name.empty())
						name = it->second.name.c_str();

					bool selected = (i == animator.currentClipIndex);
					if (ImGui::Selectable(name, selected))
					{
						animator.currentClipIndex = i;
						animator.currentTime = 0.0f;
						m_DopesheetSelectedTrack = DopesheetTrackType::None;
						m_DopesheetSelectedKey = -1;
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		//// -----------------------------------------------------------------
		//// Animator State (playback flags)
		//// -----------------------------------------------------------------
		//ImGui::SeparatorText("Animator State");

		//ImGui::Checkbox("Playing", &animator.playing);
		//ImGui::SameLine();
		//ImGui::Checkbox("Respect Clip Loop", &animator.respectClipLoop);

		//ImGui::DragFloat("Playback Speed", &animator.playbackSpeed, 0.01f, -5.0f, 5.0f);

		//if (ImGui::Button("Restart Clip##AnimPanel"))
		//{
		//	animator.currentTime = 0.0f;
		//}
		//ImGui::SameLine();
		//if (ImGui::Button("Stop##AnimPanel"))
		//{
		//	animator.currentTime = 0.0f;
		//	animator.playing = false;
		//}

		// -----------------------------------------------------------------
		// Clip settings + time controls
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Clip Settings");

		// Clip name edit
		{
			char nameBuffer[128];
			strncpy_s(nameBuffer, sizeof(nameBuffer), clip.name.c_str(), _TRUNCATE);

			if (ImGui::InputText("Clip Name", nameBuffer, sizeof(nameBuffer),
				ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newName = nameBuffer;
				newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);
				newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));

				if (!newName.empty())
				{
					clip.name = newName;
				}
			}
		}

		ImGui::InputFloat("Duration (s)", &clip.duration);
		if (clip.duration <= 0.0f) clip.duration = 0.001f;

		ImGui::Checkbox("Loop", &clip.loop);

		ImGui::SeparatorText("Time");

		// Clamp animator time
		animator.currentTime = glm::clamp(animator.currentTime, 0.0f, clip.duration);

		float scrubTime = animator.currentTime;
		if (ImGui::SliderFloat("Current Time", &scrubTime, 0.0f, clip.duration, "%.3f s"))
		{
			animator.currentTime = glm::clamp(scrubTime, 0.0f, clip.duration);
		}

		// -----------------------------------------------------------------
		// Dopesheet timeline (per-track colors, legend, key selection)
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Dopesheet");

		// Colors per track
		const ImU32 colPos = IM_COL32(80, 200, 120, 255);
		const ImU32 colRot = IM_COL32(220, 100, 100, 255);
		const ImU32 colScale = IM_COL32(100, 140, 230, 255);
		const ImU32 colPosSel = IM_COL32(130, 255, 170, 255);
		const ImU32 colRotSel = IM_COL32(255, 170, 170, 255);
		const ImU32 colScaleSel = IM_COL32(160, 190, 255, 255);
		const ImU32 colPlayhead = IM_COL32(255, 255, 50, 255);

		// Legend
		ImGui::Text("Legend:");
		ImDrawList* legendList = ImGui::GetWindowDrawList();

		auto drawLegendItem = [&](ImU32 col, const char* label)
			{
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 sz(12.0f, 12.0f);
				legendList->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), col);
				ImGui::Dummy(sz);
				ImGui::SameLine();
				ImGui::TextUnformatted(label);
			};

		drawLegendItem(colPos, "Position");
		drawLegendItem(colRot, "Rotation");
		drawLegendItem(colScale, "Scale");

		// Timeline canvas
		ImVec2 canvasPos = ImGui::GetCursorScreenPos();
		ImVec2 canvasSize = ImVec2(ImGui::GetContentRegionAvail().x, 80.0f);
		ImVec2 canvasEnd = ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);

		ImGui::InvisibleButton("##DopesheetTimeline", canvasSize);
		bool timelineHovered = ImGui::IsItemHovered();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		// Background
		drawList->AddRectFilled(canvasPos, canvasEnd, IM_COL32(30, 30, 30, 255));
		drawList->AddRect(canvasPos, canvasEnd, IM_COL32(80, 80, 80, 255));

		float midY = (canvasPos.y + canvasEnd.y) * 0.5f;

		// Horizontal mid line
		drawList->AddLine(ImVec2(canvasPos.x, midY),
			ImVec2(canvasEnd.x, midY),
			IM_COL32(100, 100, 100, 255));

		// Helper to map time in [0, duration] to X in [canvasPos.x, canvasEnd.x]
		auto timeToX = [&](float t)
			{
				float u = (clip.duration > 0.0f) ? (t / clip.duration) : 0.0f;
				u = glm::clamp(u, 0.0f, 1.0f);
				return canvasPos.x + u * canvasSize.x;
			};

		// --- First pass: handle click selection on keys ---
		const float keyRadius = 6.0f;
		const float keyRadiusSq = keyRadius * keyRadius;
		bool        clickedOnTimeline = timelineHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		ImVec2      mousePos = ImGui::GetIO().MousePos;

		if (clickedOnTimeline)
		{
			float bestDistSq = keyRadiusSq;
			DopesheetTrackType bestTrack = DopesheetTrackType::None;
			int bestIndex = -1;

			auto testKeys = [&](const auto& keys,
				DopesheetTrackType trackType,
				float yOffset)
				{
					for (int i = 0; i < static_cast<int>(keys.size()); ++i)
					{
						float x = timeToX(keys[static_cast<size_t>(i)].time);
						float y = midY + yOffset;

						float dx = mousePos.x - x;
						float dy = mousePos.y - y;
						float d2 = dx * dx + dy * dy;

						if (d2 < bestDistSq)
						{
							bestDistSq = d2;
							bestTrack = trackType;
							bestIndex = i;
						}
					}
				};

			// Position = slightly above line, Rotation on line, Scale below
			testKeys(clip.positionKeys, DopesheetTrackType::Position, -12.0f);
			testKeys(clip.rotationKeys, DopesheetTrackType::Rotation, 0.0f);
			testKeys(clip.scaleKeys, DopesheetTrackType::Scale, +12.0f);

			if (bestTrack != DopesheetTrackType::None)
			{
				// Select the nearest key and move playhead there
				m_DopesheetSelectedTrack = bestTrack;
				m_DopesheetSelectedKey = bestIndex;

				float keyTime = 0.0f;
				switch (bestTrack)
				{
				case DopesheetTrackType::Position:
					keyTime = clip.positionKeys[static_cast<size_t>(bestIndex)].time;
					break;
				case DopesheetTrackType::Rotation:
					keyTime = clip.rotationKeys[static_cast<size_t>(bestIndex)].time;
					break;
				case DopesheetTrackType::Scale:
					keyTime = clip.scaleKeys[static_cast<size_t>(bestIndex)].time;
					break;
				default: break;
				}
				animator.currentTime = glm::clamp(keyTime, 0.0f, clip.duration);
			}
			else
			{
				// Clicked empty region: scrub time
				float tNorm = (mousePos.x - canvasPos.x) / canvasSize.x;
				tNorm = glm::clamp(tNorm, 0.0f, 1.0f);
				animator.currentTime = tNorm * clip.duration;
				m_DopesheetSelectedTrack = DopesheetTrackType::None;
				m_DopesheetSelectedKey = -1;
			}
		}

		// --- Second pass: draw keys with per-track colors and selection highlight ---
		auto drawTrackKeys = [&](const auto& keys,
			DopesheetTrackType trackType,
			ImU32 col, ImU32 colSelected,
			float yOffset)
			{
				for (int i = 0; i < static_cast<int>(keys.size()); ++i)
				{
					float x = timeToX(keys[static_cast<size_t>(i)].time);
					float y = midY + yOffset;

					bool selected = (m_DopesheetSelectedTrack == trackType &&
						m_DopesheetSelectedKey == i);

					ImU32 useCol = selected ? colSelected : col;
					drawList->AddCircleFilled(ImVec2(x, y), keyRadius, useCol);
				}
			};

		drawTrackKeys(clip.positionKeys, DopesheetTrackType::Position,
			colPos, colPosSel, -12.0f);
		drawTrackKeys(clip.rotationKeys, DopesheetTrackType::Rotation,
			colRot, colRotSel, 0.0f);
		drawTrackKeys(clip.scaleKeys, DopesheetTrackType::Scale,
			colScale, colScaleSel, +12.0f);

		// Playhead line
		{
			float x = timeToX(animator.currentTime);
			drawList->AddLine(ImVec2(x, canvasPos.y),
				ImVec2(x, canvasEnd.y),
				colPlayhead, 2.0f);
		}

		// -----------------------------------------------------------------
		// Simple curves view layered under the dopesheet
		// -----------------------------------------------------------------
		bool showCurves = true;
		if (showCurves)
		{
			ImGui::SeparatorText("Curves (X component)");

			ImVec2 cPos = ImGui::GetCursorScreenPos();
			ImVec2 cSize = ImVec2(ImGui::GetContentRegionAvail().x, 120.0f);
			ImVec2 cEnd = ImVec2(cPos.x + cSize.x, cPos.y + cSize.y);

			ImGui::InvisibleButton("##AnimCurves", cSize);
			ImDrawList* cDraw = ImGui::GetWindowDrawList();

			cDraw->AddRectFilled(cPos, cEnd, IM_COL32(20, 20, 20, 255));
			cDraw->AddRect(cPos, cEnd, IM_COL32(80, 80, 80, 255));

			auto drawCurveForTrack = [&](const auto& keys,
				auto getValueX,
				ImU32 color)
				{
					if (keys.size() < 2) return;

					// Compute min/max of X component
					float minVal = getValueX(keys[0]);
					float maxVal = minVal;
					for (size_t i = 1; i < keys.size(); ++i)
					{
						float v = getValueX(keys[i]);
						if (v < minVal) minVal = v;
						if (v > maxVal) maxVal = v;
					}
					if (minVal == maxVal)
					{
						// Avoid div-by-zero; pad a bit
						minVal -= 1.0f;
						maxVal += 1.0f;
					}

					auto valToY = [&](float v)
						{
							float u = (v - minVal) / (maxVal - minVal); // 0..1
							// 0 at bottom, 1 at top:
							return cEnd.y - u * cSize.y;
						};

					ImVec2 prev;
					bool hasPrev = false;

					for (size_t i = 0; i < keys.size(); ++i)
					{
						float tNorm = (clip.duration > 0.0f) ? (keys[i].time / clip.duration) : 0.0f;
						tNorm = glm::clamp(tNorm, 0.0f, 1.0f);

						float x = cPos.x + tNorm * cSize.x;
						float y = valToY(getValueX(keys[i]));

						ImVec2 p(x, y);
						if (hasPrev)
						{
							cDraw->AddLine(prev, p, color, 2.0f);
						}
						prev = p;
						hasPrev = true;

						// Small point at key
						cDraw->AddCircleFilled(p, 3.0f, color);
					}
				};

			// Position.X curve
			drawCurveForTrack(
				clip.positionKeys,
				[](const PositionKeyframe& k) { return k.position.x; },
				colPos);

			// Rotation.X curve (Euler degrees)
			drawCurveForTrack(
				clip.rotationKeys,
				[](const RotationKeyframe& k)
				{
					glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(k.rotation));
					return eulerDeg.x;
				},
				colRot);

			// Scale.X curve
			drawCurveForTrack(
				clip.scaleKeys,
				[](const ScaleKeyframe& k) { return k.scale.x; },
				colScale);
		}

		// -----------------------------------------------------------------
		// Track editors (tables) - now synced with timeline selection
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Tracks");

		// Helper: sort by time after edits
		auto sortByTime = [](auto& keys)
			{
				std::sort(keys.begin(), keys.end(),
					[](const auto& a, const auto& b) { return a.time < b.time; });
			};

		// Position track
		if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Add Key##Pos"))
			{
				glm::vec3 defaultPos(0.0f);
				if (m_SelectedEntity.HasComponent<TransformComponent>())
					defaultPos = m_SelectedEntity.GetComponent<TransformComponent>().Position;

				PositionKeyframe k;
				k.time = animator.currentTime;
				k.position = defaultPos;
				clip.positionKeys.push_back(k);

				sortByTime(clip.positionKeys);
			}

			if (ImGui::BeginTable("PosKeysTable", 6,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn("Index");
				ImGui::TableSetupColumn("Time");
				ImGui::TableSetupColumn("X##Pos");
				ImGui::TableSetupColumn("Y##Pos");
				ImGui::TableSetupColumn("Z##Pos");
				ImGui::TableSetupColumn("Remove");
				ImGui::TableHeadersRow();

				for (int i = 0; i < static_cast<int>(clip.positionKeys.size()); ++i)
				{
					auto& k = clip.positionKeys[static_cast<size_t>(i)];
					ImGui::PushID(i);

					ImGui::TableNextRow();

					bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::Position &&
						m_DopesheetSelectedKey == i);

					ImGui::TableSetColumnIndex(0);
					if (isSelected)
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
					ImGui::Text("%d", i);
					if (isSelected)
						ImGui::PopStyleColor();

					// Click index cell to select and jump playhead
					if (ImGui::IsItemClicked())
					{
						m_DopesheetSelectedTrack = DopesheetTrackType::Position;
						m_DopesheetSelectedKey = i;
						animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
					}

					ImGui::TableSetColumnIndex(1);
					ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
					if (ImGui::IsItemClicked())
					{
						m_DopesheetSelectedTrack = DopesheetTrackType::Position;
						m_DopesheetSelectedKey = i;
					}

					ImGui::TableSetColumnIndex(2);
					ImGui::DragFloat("##X", &k.position.x, 0.1f);

					ImGui::TableSetColumnIndex(3);
					ImGui::DragFloat("##Y", &k.position.y, 0.1f);

					ImGui::TableSetColumnIndex(4);
					ImGui::DragFloat("##Z", &k.position.z, 0.1f);

					ImGui::TableSetColumnIndex(5);
					if (ImGui::SmallButton("X"))
					{
						clip.positionKeys.erase(clip.positionKeys.begin() + i);
						if (m_DopesheetSelectedTrack == DopesheetTrackType::Position &&
							m_DopesheetSelectedKey == i)
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::None;
							m_DopesheetSelectedKey = -1;
						}
						ImGui::PopID();
						--i;
						continue;
					}

					ImGui::PopID();
				}

				sortByTime(clip.positionKeys);
				ImGui::EndTable();
			}
		}

		// Rotation track
		if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Add Key##Rot"))
			{
				glm::quat defaultRot = glm::quat(1, 0, 0, 0);
				if (m_SelectedEntity.HasComponent<TransformComponent>())
					defaultRot = m_SelectedEntity.GetComponent<TransformComponent>().Rotation;

				Engine::RotationKeyframe k;
				k.time = animator.currentTime;
				k.rotation = defaultRot;
				clip.rotationKeys.push_back(k);
				sortByTime(clip.rotationKeys);
			}

			if (ImGui::BeginTable("RotKeysTable", 6,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn("Index");
				ImGui::TableSetupColumn("Time");
				ImGui::TableSetupColumn("Pitch");
				ImGui::TableSetupColumn("Yaw");
				ImGui::TableSetupColumn("Roll");
				ImGui::TableSetupColumn("Remove");
				ImGui::TableHeadersRow();

				for (int i = 0; i < static_cast<int>(clip.rotationKeys.size()); ++i)
				{
					auto& k = clip.rotationKeys[static_cast<size_t>(i)];
					ImGui::PushID(1000 + i);

					ImGui::TableNextRow();

					bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::Rotation &&
						m_DopesheetSelectedKey == i);

					ImGui::TableSetColumnIndex(0);
					if (isSelected)
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
					ImGui::Text("%d", i);
					if (isSelected)
						ImGui::PopStyleColor();

					if (ImGui::IsItemClicked())
					{
						m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
						m_DopesheetSelectedKey = i;
						animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
					}

					// Time
					ImGui::TableSetColumnIndex(1);
					ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
					if (ImGui::IsItemClicked())
					{
						m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
						m_DopesheetSelectedKey = i;
					}

					// --- quat -> Euler (deg) for UI ---
					glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(k.rotation));

					bool changed = false;

					ImGui::TableSetColumnIndex(2);
					changed |= ImGui::DragFloat("##Pitch", &eulerDeg.x, 1.0f);

					ImGui::TableSetColumnIndex(3);
					changed |= ImGui::DragFloat("##Yaw", &eulerDeg.y, 1.0f);

					ImGui::TableSetColumnIndex(4);
					changed |= ImGui::DragFloat("##Roll", &eulerDeg.z, 1.0f);

					// --- Only write back if something actually changed ---
					if (changed)
					{
						k.rotation = glm::quat(glm::radians(eulerDeg));
					}

					// Remove button
					ImGui::TableSetColumnIndex(5);
					if (ImGui::SmallButton("X"))
					{
						clip.rotationKeys.erase(clip.rotationKeys.begin() + i);
						if (m_DopesheetSelectedTrack == DopesheetTrackType::Rotation &&
							m_DopesheetSelectedKey == i)
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::None;
							m_DopesheetSelectedKey = -1;
						}
						ImGui::PopID();
						--i;
						continue;
					}

					ImGui::PopID();
				}

				sortByTime(clip.rotationKeys);
				ImGui::EndTable();
			}
		}

		// Scale track
		if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Add Key##Scale"))
			{
				glm::vec3 defaultScale(1.0f);
				if (m_SelectedEntity.HasComponent<TransformComponent>())
					defaultScale = m_SelectedEntity.GetComponent<TransformComponent>().Scale;

				ScaleKeyframe k;
				k.time = animator.currentTime;
				k.scale = defaultScale;
				clip.scaleKeys.push_back(k);

				sortByTime(clip.scaleKeys);
			}

			if (ImGui::BeginTable("ScaleKeysTable", 6,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn("Index");
				ImGui::TableSetupColumn("Time");
				ImGui::TableSetupColumn("X##Scale");
				ImGui::TableSetupColumn("Y##Scale");
				ImGui::TableSetupColumn("Z##Scale");
				ImGui::TableSetupColumn("Remove");
				ImGui::TableHeadersRow();

				for (int i = 0; i < static_cast<int>(clip.scaleKeys.size()); ++i)
				{
					auto& k = clip.scaleKeys[static_cast<size_t>(i)];
					ImGui::PushID(2000 + i);

					ImGui::TableNextRow();

					bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::Scale &&
						m_DopesheetSelectedKey == i);

					ImGui::TableSetColumnIndex(0);
					if (isSelected)
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
					ImGui::Text("%d", i);
					if (isSelected)
						ImGui::PopStyleColor();

					if (ImGui::IsItemClicked())
					{
						m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
						m_DopesheetSelectedKey = i;
						animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
					}

					ImGui::TableSetColumnIndex(1);
					ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
					if (ImGui::IsItemClicked())
					{
						m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
						m_DopesheetSelectedKey = i;
					}

					ImGui::TableSetColumnIndex(2);
					ImGui::DragFloat("##X", &k.scale.x, 0.1f);

					ImGui::TableSetColumnIndex(3);
					ImGui::DragFloat("##Y", &k.scale.y, 0.1f);

					ImGui::TableSetColumnIndex(4);
					ImGui::DragFloat("##Z", &k.scale.z, 0.1f);

					ImGui::TableSetColumnIndex(5);
					if (ImGui::SmallButton("X"))
					{
						clip.scaleKeys.erase(clip.scaleKeys.begin() + i);
						if (m_DopesheetSelectedTrack == DopesheetTrackType::Scale &&
							m_DopesheetSelectedKey == i)
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::None;
							m_DopesheetSelectedKey = -1;
						}
						ImGui::PopID();
						--i;
						continue;
					}

					ImGui::PopID();
				}

				sortByTime(clip.scaleKeys);
				ImGui::EndTable();
			}
		}

		ImGui::End();
	}



	void Editor::displayAssetsBrowserPanel()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Assets Browser", &assetsWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			ImGui::Columns(2, nullptr, true);
			static std::string selectedFolder = "";
			static ResourceType selectedType = ResourceType::UNKNOWN;

			// ================= Left column panel display all the resources folder ========================
			ImGui::BeginChild("Project List", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Text("Projects:");

			// For resources handled by Asset Browser
			if (ImGui::CollapsingHeader("Raw Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& db = AM.db();
				auto allAssets = db.AllMutable();

				std::set<ResourceType> availableTypes;
				for (const auto* record : allAssets)
				{
					if (record && record->valid && record->type != ResourceType::UNKNOWN)
					{
						availableTypes.insert(record->type);
					}
				}

				for (const auto& type : availableTypes) {
					std::string typeName = resourceTypeToString(type);
					bool isSelected = (selectedType == type);

					if (ImGui::Selectable(typeName.c_str(), isSelected)) {

						raw_asset = true;
						selectedType = type;
						selectedFolder = typeName;
						selectedResourcesIndex = -1;
					}
				}
			}

			// For resources handled by filepath (Prefabs and Scenes)
			if (ImGui::CollapsingHeader("Composed Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto folders = getAssetsInFolder(getAssetFilePath("Sources/"));

				for (auto& folder : folders)
				{
					if (folder.name != "Audio" && folder.name != "Meshes" && folder.name != "Shaders" && folder.name != "Textures" && folder.name != "Material") {
						bool isSelected = (selectedFolder == folder.fullPath);
						if (ImGui::Selectable(folder.name.c_str(), isSelected))
						{
							selectedType = ResourceType::UNKNOWN;
							raw_asset = false;
							selectedFolder = folder.fullPath;
							selectedResourcesIndex = -1; // reset asset selection
						}
					}
				}
			}

			ImGui::EndChild();

			// ================= Right column panel - display assets of selected type ========================

			auto& db = AM.db();
			auto allAssets = db.AllMutable();

			std::vector<const AssetRecord*> filteredAssets;
			filteredAssets.reserve(allAssets.size());

			for (const auto* record : allAssets) {
				if (!record || !record->valid) continue;
				if (record->type == selectedType) {
					filteredAssets.push_back(record);
				}
			}

			// To get the files in the selected folder
			auto assetsList = getAssetsInFolder(selectedFolder);

			ImGui::NextColumn();
			ImGui::BeginChild("Asset List", ImVec2(0, 0), true);


			if (raw_asset && selectedResourcesIndex != -1) {
				ImGui::Text("Asset Selected: %s", filteredAssets[selectedResourcesIndex]->sourcePath.c_str());
			} else if(!raw_asset && selectedResourcesIndex != -1) {
				ImGui::Text("Asset Selected: %s", assetsList[selectedResourcesIndex].fullPath.c_str());
			}

			// For resources handled by Asset Browser
			if (!selectedFolder.empty() && raw_asset) {

				// Display filtered assets
				ImGui::Text(("Resources > " + resourceTypeToString(selectedType)).c_str());
				ImGui::Separator();

				const float padding = 10.0f;
				const float thumbnailSize = 64.0f;
				const float cellSize = thumbnailSize + padding;
				float panelWidth = ImGui::GetContentRegionAvail().x;
				int itemsPerRow = std::max(1, static_cast<int>(panelWidth / cellSize));

				if (ImGui::BeginTable("AssetGrid", itemsPerRow)) {
					for (size_t i = 0; i < filteredAssets.size(); ++i) {
						
						const auto* record = filteredAssets[i];

						std::filesystem::path assetPath(record->sourcePath);
						std::string filename = assetPath.filename().string();
						std::string extension = record->ext;
						std::string hash = record->contentHash;
						std::time_t writeTime = record->lastWriteTime;

						ImGui::TableNextColumn();

						bool isSelected = (selectedResourcesIndex == static_cast<int>(i));

						// Optional background color for selected
						if (isSelected) {
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.65f, 0.20f, 1.0f)); // selected color
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.75f, 0.30f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.55f, 0.15f, 1.0f));
						}

						// Unique ID per button
						ImGui::PushID(static_cast<int>(i));

						if (ImGui::Button(filename.c_str(), ImVec2(thumbnailSize, thumbnailSize))) {
							selectedResourcesIndex = static_cast<int>(i);
							ImGui::OpenPopup("AssetContextMenu");
						}

						// ======================= DRAG-DROP SOURCE  =======================
						// Enables dragging assets from the browser to component fields
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
						{
							// Package the GUID as the payload
							xresource::instance_guid draggedGuid = record->guid;
							ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &draggedGuid, sizeof(xresource::instance_guid));

							// Visual feedback while dragging
							ImGui::Text("Dragging: %s", filename.c_str());
							ImGui::Text("Type: %s", resourceTypeToString(record->type).c_str());

							ImGui::EndDragDropSource();
						}

						if (ImGui::BeginPopupContextItem("AssetContextMenu")) 
						{
							ImGui::Text("%s", filename.c_str());
							
							// Only Texture and Meshes for now
							if (record->type == ResourceType::TEXTURE || record->type == ResourceType::MESH) {
								ImGui::Separator();

								if (ImGui::MenuItem("Edit"))
								{
									// open asset editor or show rename dialog
									LOG_INFO("Edit asset: ", filename);

									showDescriptorEditorPanel = true;
									currentEditingGuid = record->guid;
									editedAsset = filename;
								}

							}
							ImGui::EndPopup();
						}

						if (isSelected) {
							ImGui::PopStyleColor(3);
						}

						// ==================== Display info detail ==========================
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::Text("Name: %s", filename.c_str());
							ImGui::Text("Type: %s", extension.c_str());
							ImGui::Text("Content Hash: %s", hash.c_str());
							
							char timeBuf[64];
							std::tm* tm_local = std::localtime(&writeTime);
							std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_local);
							ImGui::Text("Last Write Time: %s", timeBuf);

							ImGui::EndTooltip();
						}

						// ==================== To center text under thumbnail ================
						ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());
						float textX = (thumbnailSize - textSize.x) * 0.5f;
						if (textX < 0) textX = 0;
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
						ImGui::TextWrapped("%s", filename.c_str());

						ImGui::PopID();
						ImGui::NextColumn();
					}
					ImGui::EndTable();
				}
			}

			// For resources handled by filepath
			if (!selectedFolder.empty() && !raw_asset)
			{
				// display the selected folder name
				std::filesystem::path folderPath(selectedFolder);
				std::string folderName = folderPath.filename().string();
				ImGui::Text(("Resources > " + folderName).c_str());

				ImGui::Separator();

				const float padding = 10.0f;
				const float thumbnailSize = 64.0f;
				const float cellSize = thumbnailSize + padding;
				float panelWidth = ImGui::GetContentRegionAvail().x;
				int itemsPerRow = std::max(1, (int)(panelWidth / cellSize));

				// int textureCount = -1;
				ImGui::Columns(itemsPerRow, nullptr, false);

				// loop through files in selected folder
				for (size_t i = 0; i < assetsList.size(); i++)
				{
					const auto& asset = assetsList[i];
					std::string fileName = asset.name;
					std::string filePath = asset.fullPath;

					ImGui::PushID(fileName.c_str());

					bool isSelected = (selectedResourcesIndex == static_cast<int>(i));

					if (isSelected)
					{
						// Change the button background color
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.65f, 0.20f, 1.0f)); // selected color
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.75f, 0.30f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.55f, 0.15f, 1.0f));
					}

					if (ImGui::Button(fileName.c_str(), ImVec2(thumbnailSize, thumbnailSize)))
					{
						selectedResourcesIndex = static_cast<int>(i);

						std::string extension = asset.name.substr(asset.name.find_last_of('.'));
						if (extension == ".json" && folderName != "BT") // For scene, not BT
						{
							if (isPrefabEditor)
							{
								if (m_SelectedEntity && m_SelectedEntity.HasComponent<PrefabComponent>())
								{
									auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

									std::string prefabPath = currPrefabPath;

									if (!prefabPath.empty())
									{
										// Create updated prefab from current entity state
										std::string entityName = m_SelectedEntity.GetComponent<TagComponent>().Tag;
										auto updatedPrefab = PrefabSerializer::CreateEntityPrefab(m_SelectedEntity, entityName);

										if (updatedPrefab && PrefabSerializer::SavePrefabToFile(*updatedPrefab, prefabPath))
										{
											PrefabRegistry::Get().RegisterPrefab(updatedPrefab);
											prefabComp.ClearModifications(); // Reset overrides 
											LOG_INFO("Prefab updated: {}", prefabPath);
											isPrefabEditor = false;
										}
									}
								}

							}

							currScenePath = filePath; // update curr file path
							currFileName = fileName; // store file name
							m_Scene->SetName(fileName);
							if (m_Scene)
							{

								m_Scene->GetRegistry().clear();
								m_Scene->LoadFromFile(filePath);
								m_SelectedEntity = Entity{}; // resets
								m_PickedID = 0xFFFFFFFFu;
								m_Operation = static_cast<ImGuizmo::OPERATION>(-1);
								isPrefabEditor = false;
							}

						}
						else if (extension == ".prefab" && folderName != "BT") // FOr Prefab, not BT (To be fixed in M3)
						{
							if (!isPrefabEditor)
							{
								if (!currScenePath.empty())
								{
									m_Scene->SaveToFile(currScenePath);
									m_Scene->SaveToFile(convertAssetPathToRootResources(currScenePath));
									LOG_INFO("Scene auto-saved before switching to prefab:", currScenePath);
								}
							}
							currPrefabPath = filePath;
							m_Scene->SetName("Prefab");
							auto prefab = PrefabSerializer::LoadPrefabFromFile(currPrefabPath);
							if (prefab)
							{
								m_Scene->GetRegistry().clear();
								PrefabRegistry::Get().RegisterPrefab(prefab);
								Entity entity = PrefabInstantiator::InstantiateEntityPrefab(m_Scene, prefab->GetGUID());

								m_SelectedEntity = Entity{};
								m_PickedID = 0xFFFFFFFFu;

								if (!currScenePath.empty())
								{
									currScenePath.clear();
								}
								m_SelectedEntity = Entity(); //reset entity
								m_PickedID = 0xFFFFFFFFu;
								isPrefabEditor = true;


								LOG_INFO("Now editing prefab:", currPrefabPath);
							}
						}
					}
					// to change the color of the selected
					if (isSelected)
					{
						ImGui::PopStyleColor(3);
					}

					// ==================== Display info detail ==========================
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("Name: %s", fileName.c_str());
						std::string extension = fileName.substr(fileName.find_last_of('.') + 1);
						ImGui::Text("Type: %s", extension.c_str());
						ImGui::EndTooltip();
					}

					// ==================== To center text under thumbnail ================
					ImVec2 textSize = ImGui::CalcTextSize(fileName.c_str());
					float textX = (thumbnailSize - textSize.x) * 0.5f;
					if (textX < 0) textX = 0;
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
					ImGui::TextWrapped("%s", fileName.c_str());

					ImGui::PopID();
					ImGui::NextColumn();

				}

			}

			ImGui::EndChild();
			ImGui::Columns(1);

		}

		ImGui::End();
	}

	void Editor::displayDescriptorEditorPanel() {
		
		if (!showDescriptorEditorPanel) {
			descriptorEditor.Clear();
			return;
		}

		if (ImGui::Begin("Descriptor Editor Panel", &showDescriptorEditorPanel, ImGuiWindowFlags_NoDocking)) {
			LOG_DEBUG("displayDescriptorEditorPanel OPEN");

			if (!descriptorEditor.IsLoaded() || currentEditingGuid != descriptorEditor.GetGuid()) {
				if (!descriptorEditor.Load(currentEditingGuid)) {
					ImGui::Text("Failed to load descriptor for %s", editedAsset.c_str());
				}
			}
			else {
				ImGui::Columns(2, nullptr, true);

				// Drawing asset in descriptor editor if it is a texture
				if (descriptorEditor.GetType() == ResourceType::TEXTURE) {
					auto* texture = RM.loadResource<TextureResource>(Engine::convertToTextureGuid(currentEditingGuid));
					if (texture != nullptr) {
						float tex_w = static_cast<float>(texture->width);
						float tex_h = static_cast<float>(texture->height);


						ImVec2 window_size = ImGui::GetWindowSize();
						float win_w = window_size.x * 3 / 4;
						float win_h = window_size.y * 3 / 4;

						float aspect = tex_w / tex_h;

						ImVec2 viewportSize;
						if (win_w / win_h > aspect) {
							viewportSize.x = win_h * aspect;
							viewportSize.y = win_h;
						}
						else {
							viewportSize.x = win_w;
							viewportSize.y = win_w / aspect;
						}

						ImGui::Image(
							(ImTextureID)(intptr_t)((GLuint)texture->textureID),
							viewportSize,
							ImVec2(0, 0), ImVec2(1, 1)
						);
					}
				}

				ImGui::NextColumn();

				ImGui::SeparatorText("Asset Information");
				ImGui::Text("Asset Name: %s", descriptorEditor.GetDisplayName().c_str());
				ImGui::Text("Source: %s", descriptorEditor.GetSourcePath().c_str());

				std::string assetType{};
				switch (descriptorEditor.GetType()) {
				case ResourceType::TEXTURE:
					assetType = "Texture";
					break;
				case ResourceType::MESH:
					assetType = "Mesh";
					break;
				default:
					break;
				}
				ImGui::Text("Asset Type: %s", assetType.c_str());

				ImGui::Spacing();
				ImGui::SeparatorText("Editable Properties");

				// Check type
				if (descriptorEditor.GetType() == ResourceType::TEXTURE) {

					TextureSettings* settings = descriptorEditor.GetTextureSettings();

					auto quality = settings->quality;
					if (ImGui::SliderFloat("Quality", &quality, 0.0f, 1.0f)) {
						settings->quality = quality;
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Minimaps", &settings->generateMipmaps)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("sRGB", &settings->srgb)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::BeginCombo("Compression", settings->compression.c_str())) {
						for (auto& option : descriptorEditor.GetCompressionOptions()) {
							if (ImGui::Selectable(option.c_str())) {
								settings->compression = option;
								descriptorEditor.MarkModified();
							}
						}
						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo("Usage", settings->usageType.c_str())) {
						for (auto& option : descriptorEditor.GetUsageTypeOptions()) {
							if (ImGui::Selectable(option.c_str())) {
								settings->usageType = option;
								descriptorEditor.MarkModified();
							}
						}
						ImGui::EndCombo();
					}
				}
				else if (descriptorEditor.GetType() == ResourceType::MESH) {

					MeshSettings* settings = descriptorEditor.GetMeshSettings();

					if (ImGui::Checkbox("Generate Normals", &settings->generateNormals)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Colors", &settings->includeColors)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Normals", &settings->includeNormals)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Position", &settings->includePos)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Texture Coordinates", &settings->includeTexCoords)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::BeginCombo("Index Type", settings->indexType.c_str())) {
						for (auto& option : descriptorEditor.GetIndexTypeOptions()) {
							if (ImGui::Selectable(option.c_str())) {
								settings->indexType = option;
								descriptorEditor.MarkModified();
							}
						}
						ImGui::EndCombo();
					}
					
					if (ImGui::Checkbox("Optimize Vertices", &settings->optimizeVertices)) {
						descriptorEditor.MarkModified();
					}
					
					char formatBuffer[256];
					strncpy_s(formatBuffer, sizeof(formatBuffer), settings->outputFormat.c_str(), _TRUNCATE);
					if (ImGui::InputText("Output Format", formatBuffer, sizeof(formatBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						settings->outputFormat = std::string(formatBuffer);
						descriptorEditor.MarkModified();
					}

					float meshScale = settings->scale;
					if (ImGui::DragFloat("Scale", &meshScale))
					{
						settings->scale = meshScale;
						descriptorEditor.MarkModified();
					}
					
				}

				if (!descriptorEditor.GetTags().empty()) {
					ImGui::SeparatorText("Tags");
					for (auto& tag : descriptorEditor.GetTags()) {
						ImGui::Text("%s", tag.c_str());
					}
				}

				ImGui::SeparatorText("Last Imported");
				std::time_t writeTime = descriptorEditor.GetLastImported();
				char timeBuf[64];
				std::tm* tm_local = std::localtime(&writeTime);
				std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_local);
				ImGui::Text("Last Write Time: %s", timeBuf);

				static std::string notifMsg{};
				static ImVec4 notifColour(0.0f, 0.0f, 0.0f, 0.0f);

				if (ImGui::Button("Validate Descriptor")) {
					if (descriptorEditor.Validate()) {
						notifMsg = "Descriptor is Valid";
						notifColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
					}
					else {
						notifMsg = "Descriptor is NOT Valid";
						notifColour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
					}
				}

				if (!notifMsg.empty()) {

					ImGui::TextColored(notifColour, "%s", notifMsg.c_str());

					static float notifTimer = 2.0f;
					notifTimer -= ImGui::GetIO().DeltaTime;

					if (notifTimer <= 0.0f) {
						notifTimer = 2.0f;
						notifMsg.clear();
					}
				}

				// Save button
				if (descriptorEditor.IsModified()) {
					if (ImGui::Button("Save & Compile")) {
						if (descriptorEditor.Save()) {
							notifMsg = "Descriptor is Saved";
							notifColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

							//compile
							if (AM.CompileSingleAsset(currentEditingGuid, true)) {
								notifMsg = "Saved and Compiled successfully!"; 
								notifColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);//green 
							}
							else {
								notifMsg = "Saved but compilation FAILED";
								notifColour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red for error
							}
						}
						else {
							notifMsg = "Descriptor is NOT Saved";
							notifColour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						}
					}


				}
			}

			ImGui::End();
		}
	}

	void Editor::displayPerformanceProfilePanel(Timestep ts)
	{
		if (!performanceProfileWindow)
			return;
		
		ImGui::SetNextWindowSize(ImVec2(500, 300));
		if (ImGui::Begin("Performance Profile", &performanceProfileWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("Tracy Window:");

			// Launches Tracy.exe when copied to correct folder
			if (ImGui::Button("Launch Tracy Window"))
			{
#ifdef TRACY_ENABLE
				if (auto profiler = m_Profiler.lock()) {
					profiler->LaunchTracy();
					LOG_INFO("  -> Tracy profiler launched successfully");
				}
				else {
					LOG_WARNING("  -> Tracy profiler reference expired.");
				}
#else
				LOG_WARNING("  -> TRACY_ENABLE not defined. Skipping profiler launch.");
#endif
			}

			ImGui::Separator();
			// ========================= ImGui Graph Section =============================
			float deltaTime = ts.GetSeconds();
			float currFPS = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;
			float currFrameTime = ts.GetMilliseconds();

			// ======================= history statistics variables ============================
			static const int FPS_HISTORY_SIZE = 90;  //store up to 90 frames
			static float fpsHistory[FPS_HISTORY_SIZE] = {};
			static float frameTimeHistory[FPS_HISTORY_SIZE] = {}; //for frame time
			static int fpsHistoryOffset = 0;
			static int frameCount = 0;

			fpsHistory[fpsHistoryOffset] = currFPS;
			frameTimeHistory[fpsHistoryOffset] = currFrameTime;
			fpsHistoryOffset = (fpsHistoryOffset + 1) % FPS_HISTORY_SIZE;
			frameCount = std::min(frameCount + 1, FPS_HISTORY_SIZE);

			//========================== update min/max statistics =============================
			static float minFPS = FLT_MAX;
			static float maxFPS = 0.0f;
			static float minFrameTime = FLT_MAX;
			static float maxFrameTime = 0.0f;

			minFPS = std::min(minFPS, currFPS);
			maxFPS = std::max(maxFPS, currFPS);
			minFrameTime = std::min(minFrameTime, currFrameTime);
			maxFrameTime = std::max(maxFrameTime, currFrameTime);

			// ======================== Cal average ===================================
			float avgFPS = 0.0f;
			float avgFrameTime = 0.0f;

			for (int i = 0; i < FPS_HISTORY_SIZE; i++)
			{
				avgFPS += fpsHistory[i];
				avgFrameTime += frameTimeHistory[i];
			}

			avgFPS /= frameCount;
			avgFrameTime /= frameCount;

			// ======================= Showcase statistics ==============================
			ImGui::Text("Frame Statistics:");
			ImGui::Spacing();
			//create a table to display statistics better
			if (ImGui::BeginTable("StatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_WidthFixed, 40.0f);
				ImGui::TableHeadersRow();

				// Average FPS
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Average FPS:");
				ImGui::TableNextColumn();
				ImGui::Text("%.1f", avgFPS);
				ImGui::TableNextColumn();
				ImGui::Text("fps");

				// Average Frame Time
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Avg Frame Time:");
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", avgFrameTime);
				ImGui::TableNextColumn();
				ImGui::Text("ms");

				// Min Frame Time
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Min Frame Time:");
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", minFrameTime);
				ImGui::TableNextColumn();
				ImGui::Text("ms");

				// Max Frame Time
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Max Frame Time:");
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", maxFrameTime);
				ImGui::TableNextColumn();
				ImGui::Text("ms");

				ImGui::EndTable();
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// ============================= showcase performance graphs section ======================
			ImGui::Text("Performance Graphs:");
			ImGui::Spacing();
			float graphWidth = ImGui::GetContentRegionAvail().x;

			//----------------- FPS graph ---------------------
			char fpsOverlay[64];
			sprintf_s(fpsOverlay, sizeof(fpsOverlay), "FPS - avg %.1f", avgFPS);

			float fpsMinScale = (avgFPS - 30.0f > 0.0f) ? (avgFPS - 30.0f) : 0.0f;
			float fpsMaxScale = avgFPS + 30.0f;

			ImGui::PlotLines(
				"##FPS",
				fpsHistory,
				FPS_HISTORY_SIZE,
				fpsHistoryOffset,
				fpsOverlay,
				fpsMinScale,
				fpsMaxScale,
				ImVec2(graphWidth, 100.0f),
				sizeof(float)
			);
			// ----------- frame time graph -------------
			char frameTimeOverlay[64];
			sprintf_s(frameTimeOverlay, sizeof(frameTimeOverlay), "Frame Time (ms) - avg %.2f", avgFrameTime);

			// ---------- dynamic scaling ------------  
			float ftMinScale = std::max(avgFrameTime - 5.0f, 0.0f);
			float ftMaxScale = avgFrameTime + 5.0f;

			ImGui::PlotLines(
				"##FrameTime",
				frameTimeHistory,
				FPS_HISTORY_SIZE,
				fpsHistoryOffset,
				frameTimeOverlay,
				ftMinScale,
				ftMaxScale,
				ImVec2(graphWidth, 100.0f),
				sizeof(float)
			);

			ImGui::Spacing();
			ImGui::Separator();

			// ========= different coloring to indicate performance status ======= 
			ImGui::Spacing();
			if (currFPS >= 60.0f)
			{
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Performance: Excellent");
			}
			else if (currFPS >= 30.0f)
			{
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance: Good");
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Performance: Poor");
			}

			ImGui::Spacing();

			if (ImGui::Begin("Performance Profile", &performanceProfileWindow))
			{
				ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
				ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
			}
			ImGui::End();
		}

		ImGui::End();
	}

	void Editor::StartImguiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}


#if 1
	void Editor::renderViewport(GLuint texhandle)
	{
		ImVec2 texture_pos = ImGui::GetCursorScreenPos();

		// viewport size calculation...
		ImVec2 viewportSize = { 600, 600 };
		if (m_Window) {
			int width = 0;
			int height = 0;
			glfwGetWindowSize(m_Window, &width, &height);
			viewportSize = {
				static_cast<float>(width) / 2.0f,
				static_cast<float>(height) / 2.0f
			};
		}

		ImGui::Begin("Viewport");

		ViewportPanelHelper::ViewportButtons(isPlaying, m_Scene, m_SelectedEntity, 
											 currScenePath, currFileName, m_PickedID);
		if (texhandle) {
			ImVec2 imagePos = ImGui::GetCursorScreenPos();
			ImGui::Image((ImTextureID)(intptr_t)texhandle, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

			ImVec2 tl_screen = ImGui::GetItemRectMin();    // Top left of image wrt SCREEN space
			ImVec2 actualSize = ImGui::GetItemRectSize();  // Get ACTUAL rendered size

			ImGuiViewport* vp = ImGui::GetWindowViewport();

			// Convert to CLIENT-WINDOW coords (origin = top-left of that GLFW window's content area)
			ImVec2 tl_client;
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
				tl_client = { tl_screen.x - vp->Pos.x, tl_screen.y - vp->Pos.y }; // subtract OS window's top-left in screen coords
			}
			else {
				// Single viewport: ImGui "screen" origin coincides with your main client window
				tl_client = tl_screen;
			}

			// Save editor viewport data for OBJECT PICKING (client coordinates)
			editorViewportData.tl = tl_client;
			editorViewportData.size = viewportSize;

			// Sync with renderer using the existing getEditorViewport() method
			if (m_Renderer) {
				m_Renderer->getEditorViewport() = editorViewportData;
			}

			bool currentCamToggle = m_Renderer->getEditorCamToggle();

			if (m_PreviousEditorCamToggle != currentCamToggle)
			{
				// toggled camera
				m_Operation = static_cast<ImGuizmo::OPERATION>(-1);
				m_SelectedEntity = Entity{};
				std::cout << "*** [GIZMO] Reset operation after camera toggle ***" << std::endl;
				m_PreviousEditorCamToggle = currentCamToggle;
			}

			if (m_Renderer->getEditorCamToggle()) {
				// Store screen coordinates separately for ImGuizmo - use ACTUAL size
				m_ImGuizmoViewportData.tl = tl_screen;
				m_ImGuizmoViewportData.size = actualSize;  // Use actual rendered size

				// Track current frame gizmo state
				bool isUsingGizmoThisFrame = false;
				bool isOverGizmoThisFrame = false;

				// FIRST: Handle ImGuizmo manipulation if we have a selected entity
				if (m_SelectedEntity) {
					ManipulateEntityTransform(m_SelectedEntity);
					isUsingGizmoThisFrame = ImGuizmo::IsUsing();
					isOverGizmoThisFrame = ImGuizmo::IsOver();
				}

				if (ImGui::BeginPopupContextWindow("GizmoContextMenu", ImGuiPopupFlags_MouseButtonRight))
				{
					LOG_INFO("[DEBUG] Right-click popup opened!");

					
					if (ImGui::MenuItem("Move", "W"))  
					{
						m_Operation = ImGuizmo::TRANSLATE;
						std::cout << "*** [GIZMO] Switched to MOVE mode ***" << std::endl;
					}

					if (ImGui::MenuItem("Rotate", "E"))
					{
						m_Operation = ImGuizmo::ROTATE;
						std::cout << "*** [GIZMO] Switched to ROTATE mode ***" << std::endl;
					}

					if (ImGui::MenuItem("Scale", "R"))
					{
						m_Operation = ImGuizmo::SCALE;
						std::cout << "*** [GIZMO] Switched to SCALE mode ***" << std::endl;
					}

					ImGui::Separator();
					ImGui::EndPopup();
				}

				// Only handle keyboard shortcuts when viewport is focused
				if (ImGui::IsWindowFocused()) {
					if (ImGui::IsKeyPressed(ImGuiKey_W)) {
						m_Operation = ImGuizmo::TRANSLATE;
						std::cout << "*** [GIZMO] Switched to MOVE mode (Keyboard W) ***" << std::endl;
					}
					if (ImGui::IsKeyPressed(ImGuiKey_E)) {
						m_Operation = ImGuizmo::ROTATE;
						std::cout << "*** [GIZMO] Switched to ROTATE mode (Keyboard E) ***" << std::endl;
					}
					if (ImGui::IsKeyPressed(ImGuiKey_R)) {
						m_Operation = ImGuizmo::SCALE;
						std::cout << "*** [GIZMO] Switched to SCALE mode (Keyboard R) ***" << std::endl;
					}
					if (ImGui::IsKeyPressed(ImGuiKey_Q)) {
						m_Operation = static_cast<ImGuizmo::OPERATION>(-1);
						std::cout << "*** [GIZMO] Disabled manipulation (Keyboard Q) ***" << std::endl;
					}
				}

				// SECOND: Handle object selection
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					// Only allow selection if we weren't using or over the gizmo in the PREVIOUS frame
					if (!m_WasUsingGizmoLastFrame && !m_WasOverGizmoLastFrame)
					{
						if (m_PickedID != 0xFFFFFFFFu && m_Scene)
						{
							LOG_INFO("[DEBUG] m_PickedID = {}", m_PickedID);
							m_SelectedEntity = Entity{ (entt::entity)m_PickedID, &m_Scene->GetRegistry() };
						}
						else
						{
							m_SelectedEntity = Entity{};
							LOG_INFO("Deselected entity.");
						}
					}
					else
					{
						LOG_INFO("Selection blocked - was interacting with gizmo last frame");
					}
				}

				// Update gizmo state for next frame
				m_WasUsingGizmoLastFrame = isUsingGizmoThisFrame;
				m_WasOverGizmoLastFrame = isOverGizmoThisFrame;
			}
		}

		ImGui::End();
	}

#endif
	// Helper function for top menu 
	void Editor::sceneOpenPanel()
	{
		// get all files inside scene
		auto sceneFiles = getAssetsInFolder(getAssetFilePath("Sources/Scenes"));
		if (openScenePanel)
		{
			ImGui::OpenPopup("Scene Level Selection");
		}

		// pop up panel to open scene file
		if (ImGui::BeginPopupModal("Scene Level Selection", nullptr, ImGuiWindowFlags_NoDocking))
		{
			ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);

			// list all scene files
			for (auto& scenesAsset : sceneFiles)
			{

				if (ImGui::Selectable(scenesAsset.name.c_str()))
				{
					if (!m_Scene)
					{
						LOG_ERROR("No active scene exists to load into!");
						continue;
					}
					
					// clear current scene
					m_Scene->GetRegistry().clear();
					m_SelectedEntity = Entity();
					m_PickedID = 0xFFFFFFFFu;
				
					// load the selected scene file
					if (m_Scene->LoadFromFile(scenesAsset.fullPath))
					{
						//LOG_ERROR("Failed to load scene %s", sceneFiles);
						currScenePath = scenesAsset.fullPath;
						LOG_INFO("Scene loaded successfully: ", currScenePath);
						openScenePanel = false; //  reset after select scene
						currFileName = m_Scene->GetName();
						LOG_DEBUG("//// m_Scene->GetName() in open file is ", currFileName);
						ImGui::CloseCurrentPopup();
					}
				}
			}
			// --------------- Cancel Selection for Open Scene -----------------------
			if (ImGui::Button("Cancel"))
			{
				openScenePanel = false; //  reset after click cancel button
				ImGui::CloseCurrentPopup();
			}


			ImGui::EndPopup(); // end pop up panel for scene level selection
		}
		
	}

	void Editor::saveAsScenePanel()
	{
		if (saveAsPanel)
		{
			ImGui::OpenPopup("Save As Panel");
		}
		if (ImGui::BeginPopupModal("Save As Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{

			ImGui::InputText("File name", saveAsDefaultSceneName, IM_ARRAYSIZE(saveAsDefaultSceneName));

			// ------------------ Select save button to save new scene --------------------
			if (ImGui::Button("Save", ImVec2(120, 0)))
			{
				if (strlen(saveAsDefaultSceneName) == 0) // validate that file name is not empty
				{
					ImGui::OpenPopup("Empty Filename");
				}
				else
				{
					// default new scene path 
					std::string defaultNewScenePath = getAssetFilePath("Sources/Scenes/") + saveAsDefaultSceneName;

					if (!std::filesystem::path(defaultNewScenePath).has_extension()) {

						defaultNewScenePath += ".json"; // ensure .json extension
					}

					if (std::filesystem::exists(defaultNewScenePath))
					{
						ImGui::OpenPopup("Confirm Overwrite"); // if save as name repeat, open confirmation panel for overwrite it
					}
					else
					{

						m_Scene->SaveToFile(defaultNewScenePath); // save scene file
						m_Scene->SaveToFile(convertAssetPathToRootResources(defaultNewScenePath));
						// currScenePath = defaultNewScenePath; // update current scene path
						m_Scene->SetName(saveAsDefaultSceneName);
						saveAsPanel = false; // to close pop up
						isNewScene = false;
						ImGui::CloseCurrentPopup();

					}
				}
			}

			// ------------------- Cancel save as button ---------------------
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				saveAsPanel = false; // reset to close pop up
				ImGui::CloseCurrentPopup();
			}

			// ----------------------- Overwrite Existing Save as Scene File -------------------
			if (ImGui::BeginPopupModal("Confirm Overwrite", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("File %s already exists.\nDo you want to replace it?", saveAsDefaultSceneName);

				ImGui::Separator();

				if (ImGui::Button("Yes", ImVec2(120, 0)))
				{
					// default new scene path 
					std::string defaultNewScenePath = getAssetFilePath("Sources/Scenes/") + saveAsDefaultSceneName;
					if (!std::filesystem::path(defaultNewScenePath).has_extension()) {
						defaultNewScenePath += ".json"; // ensure .json extension
					}
					m_Scene->SaveToFile(defaultNewScenePath);
					m_Scene->SaveToFile(convertAssetPathToRootResources(defaultNewScenePath));
					currScenePath = defaultNewScenePath;

					saveAsPanel = false;
					isNewScene = false;
					ImGui::CloseCurrentPopup(); // close save as panel
				}

				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup(); // end pop up confirm overwrite panel
			}

			// ---------------- If is Emty Filename Warning -------------------
			if (ImGui::BeginPopupModal("Empty Filename", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Please enter a file name.");

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::EndPopup(); // end pop up for save as scene panel
		}
	}

	void Editor::CompleteFrame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

	}

	std::vector<Editor::AssetEntry> Editor::getAssetsInFolder(const std::string& folderPath)
	{
		std::vector<AssetEntry> entries;

		if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
			return entries;

		for (const auto& entry : std::filesystem::directory_iterator(folderPath))
		{
			entries.push_back({
				entry.path().filename().string(),
				entry.path().generic_string(),
				});
		}

		return entries;
	}

	void Editor::CleanupTemporaryPrefabs()
	{
		for (const auto& prefabPath : m_TemporaryPrefabPaths)
		{
			if (std::filesystem::exists(prefabPath))
			{
				std::filesystem::remove(prefabPath);
				LOG_INFO("Deleted unsaved prefab: {}", prefabPath);
			}
		}
		m_TemporaryPrefabPaths.clear();
	}

	void Editor::ManipulateEntityTransform(Entity& entity)
	{
		
		if(!entity || !m_Scene || !entity.HasComponent<TransformComponent>())
			return;

		Camera3D& camera = m_Renderer->getEditorCamera();

		auto& tc = entity.GetComponent<TransformComponent>();
		//glm::mat4 transform = BuildTransformMatrix(tc);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Position);
		transform = transform * glm::mat4_cast(tc.Rotation); // Use quaternion directly
		transform = glm::scale(transform, tc.Scale);

		// Set up ImGuizmo with SCREEN coordinates
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

		// Use screen coordinates for ImGuizmo
		float x = m_ImGuizmoViewportData.tl.x;
		float y = m_ImGuizmoViewportData.tl.y;
		float width = m_ImGuizmoViewportData.size.x;
		float height = m_ImGuizmoViewportData.size.y;

		ImGuizmo::SetRect(x, y, width, height);

		// Calculate aspect ratio from actual viewport size
		float aspect_ratio = (height > 0) ? (width / height) : 1.0f;
		glm::mat4 view = camera.getLookAt();
		glm::mat4 proj = camera.getPerspective(aspect_ratio);

		if (m_Operation != (ImGuizmo::OPERATION)-1) {

			
			//ImGuizmo::MODE mode = (m_Operation == ImGuizmo::ROTATE) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

			ImGuizmo::Manipulate(
				glm::value_ptr(view),
				glm::value_ptr(proj),
				m_Operation,
				ImGuizmo::WORLD,
				glm::value_ptr(transform)
			);

			if (ImGuizmo::IsUsing()) {
				if (m_Operation == ImGuizmo::TRANSLATE) {
					// update position
					glm::vec3 newPosition = glm::vec3(transform[3]);
					tc.SetPosition(newPosition);
				}
				else if (m_Operation == ImGuizmo::ROTATE) {
					glm::mat3 rotationMatrix;
					rotationMatrix[0] = glm::normalize(glm::vec3(transform[0]));
					rotationMatrix[1] = glm::normalize(glm::vec3(transform[1]));
					rotationMatrix[2] = glm::normalize(glm::vec3(transform[2]));

					// Convert to quaternion and set directly
					glm::quat newRotation = glm::quat_cast(rotationMatrix);
					tc.Rotation = newRotation;
					tc.IsDirty = true; 

				}
				else if (m_Operation == ImGuizmo::SCALE) {
					
					glm::vec3 newScale;
					newScale.x = glm::length(glm::vec3(transform[0]));
					newScale.y = glm::length(glm::vec3(transform[1]));
					newScale.z = glm::length(glm::vec3(transform[2]));

					tc.SetScale(newScale);
				}
			}
		}
	}

	void Editor::CreateScriptPanel()
	{
		if (createScript)
		{
			ImGui::OpenPopup("Create Script Panel");
		}

		if (ImGui::BeginPopupModal("Create Script Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char scriptNewBuffer[128] = "";
			static std::string newScriptName{};

			ImGui::Text("Enter new script name:");
			if (ImGui::InputText("##NewScriptName", scriptNewBuffer, sizeof(scriptNewBuffer)))
			{
				newScriptName = scriptNewBuffer;
			}

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				if (!newScriptName.empty())
				{
					if (newScriptName.ends_with(".cs"))
						newScriptName = newScriptName.substr(0, newScriptName.size() - 3);

					std::string scriptPath = getRepository() + "\\Scripts\\Game\\" + newScriptName + ".cs"; // get the saving path

					// writing script template 
					std::string scriptTemplate =
						"using Engine;\n"
						"using System;\n\n"
						"namespace Game\n"
						"{\n"
						"    public class " + newScriptName + "\n"
						"    {\n"
						"        public override void OnStart()\n"
						"        {\n"
						//"            Log.Info(\"" + newScriptName + " started!\");\n"
						"			 Engine.InternalCalls.Log(\"" + newScriptName + " started!\");\n"
						"        }\n\n"
						"        public override void OnUpdate(float deltaTime)\n"
						"        {\n"
						"\n"
						"        }\n"
						"    }\n"
						"}\n";

					std::ofstream outFile(scriptPath);
					if (outFile.is_open())
					{
						outFile << scriptTemplate;
						outFile.close();
						OpenScriptInEditor(newScriptName); // to open script after created

						//std::cout << "[Editor] Created new script: " << scriptPath << "\n";
					}
					

					scriptNewBuffer[0] = '\0';
					newScriptName.clear();
					createScript = false;
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				createScript = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void Editor::OpenScriptPanel()
	{
		
		if (openScript)
		{
			ImGui::OpenPopup("Open Script");
		}

		// pop up panel to open scene file
		if (ImGui::BeginPopupModal("Open Script", nullptr, ImGuiWindowFlags_NoDocking))
		{
			ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);

			std::string scriptPath = getRepository() + "\\Scripts\\Game";
			auto getScriptFiles = getAssetsInFolder(scriptPath);

			ImGui::Text("Select a script to open:");
			ImGui::Separator();

			if (getScriptFiles.empty())
			{
				ImGui::TextDisabled("No script files found in:");
				ImGui::TextWrapped("%s", scriptPath.c_str());
			}
			else
			{
				for (const auto& scriptFile : getScriptFiles)
				{
					if (ImGui::Selectable(scriptFile.name.c_str()))
					{
						OpenScriptInEditor(scriptFile.name.c_str());
						openScript = false;
					}
				}
			}
			
			ImGui::Separator();
			if (ImGui::Button("Cancel"))
			{
				openScript = false; //  reset after click cancel button
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup(); 
		}
		
	}

	bool Editor::OpenScriptInEditor(const std::string& scriptName)
	{
		std::string sanitizedName = scriptName;
		if (sanitizedName.ends_with(".cs"))
		{
			sanitizedName = sanitizedName.substr(0, sanitizedName.size() - 3);
		}

		std::string scriptPath = getRepository() + "\\Scripts\\Game\\" + sanitizedName + ".cs";

		if (!std::filesystem::exists(scriptPath))
		{
			return false;
		}
		try
		{
			std::string windowsPath = scriptPath;
			std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');

			std::string command = "start \"\" \"" + windowsPath + "\"";

			int result = system(command.c_str());
			return result == 0;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}
	void Editor::displayCameraComp(ImVec2& buttonSize)
	{
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
				bool changed = false;

				changed |= ImGui::Checkbox("Enabled", &camComp.Enabled);
		/*		changed |= ImGui::Checkbox("Primary", &camComp.Primary);*/
				changed |= ImGui::Checkbox("Auto Aspect", &camComp.autoAspect);

				if (!camComp.autoAspect)
				{
					changed |= ImGui::DragFloat("Aspect", &camComp.Aspect, 0.01f, 0.1f, 10.0f);
				}

				changed |= ImGui::DragFloat("FOV", &camComp.FOV, 0.1f, 10.0f, 120.0f);
				changed |= ImGui::DragFloat("Near Plane", &camComp.NearPlane, 0.01f, 0.01f, camComp.FarPlane - 0.01f);
				changed |= ImGui::DragFloat("Far Plane", &camComp.FarPlane, 1.0f, camComp.NearPlane + 0.01f, 10000.0f);

				// For M3
			/*	int depth = static_cast<int>(camComp.Depth);
				if (ImGui::DragInt("Depth", &depth, 1, 0, 100))
				{
					camComp.Depth = static_cast<u32>(depth);
					changed = true;
				}*/

				/*changed |= ImGui::DragFloat3("Target", glm::value_ptr(camComp.Target), 0.1f);*/

				if (changed)
				{
					camComp.isDirty = true;

				}
			}
			// ---------------------- Remove Camera Comp ---------------------------
			if (removeCameraComp)
			{
				m_SelectedEntity.RemoveComponent<CameraComponent>();
			}

		}
	}
} // end of namespace Engine

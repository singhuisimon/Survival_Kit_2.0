/**
* @file Editor.cpp
* @brief Implementation of the functions of IMGUI_Manager class for running the IMGUI level editor.
* @author Liliana Hanawardani (45%), Saw Hui Shan (45%), Rio Shannon Yvon Leonardo (10%)
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

#include "../Serialization/SceneSerializer.h"
#include "../Serialization/PrefabSerializer.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Texture.h"

#include "../Asset/ResourceHelpers.h"
// Include other necessary headers
#include <GLFW/glfw3.h>

// Required for quaternion to Euler conversion
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

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

		// Set default pickedID (entt::null (0xFFFFFFFFu) is no hit)
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

		displayAssetsBrowserPanel();

		displayPerformanceProfilePanel(ts);

		displayDescriptorEditorPanel();
		//DrawPrefabInspector();

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
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
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
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
				{
					openScenePanel = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Open scene from file.");

				// --------------- Save Scene -------------------
				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				{
					
					//if (isPrefabEditor && !currPrefabPath.empty() && m_Scene->GetName() == "Prefab")
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

							//if (m_SelectedEntity && m_SelectedEntity.HasComponent<PrefabComponent>())
							//{
							//	auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

							//	std::string prefabPath = currPrefabPath;

							//	if (!prefabPath.empty())
							//	{
							//		// Create updated prefab from current entity state
							//		std::string entityName = m_SelectedEntity.GetComponent<TagComponent>().Tag;
							//		auto updatedPrefab = PrefabSerializer::CreateEntityPrefab(m_SelectedEntity, entityName);

							//		if (updatedPrefab && PrefabSerializer::SavePrefabToFile(*updatedPrefab, prefabPath))
							//		{
							//			PrefabRegistry::Get().RegisterPrefab(updatedPrefab);
							//			m_TemporaryPrefabPaths.erase(currPrefabPath);
							//			prefabComp.ClearModifications(); // Reset overrides 
							//			LOG_INFO("Prefab updated: {}", prefabPath);
							//		}
							//	}
							//}
						}
					}
					/*else if (!currScenePath.empty())
					{
						//SceneSerializer serializer(m_Scene);

						//if (serializer.Serialize(currScenePath))
						//{
						//	m_TemporaryPrefabPaths.clear(); // remove from temporary list
						//	//PrefabInstantiator::InstantiateScenePrefab()
						//
						//}
						m_Scene->SaveToFile(currScenePath);
						LOG_DEBUG("current scene is ", currScenePath);
					}*/
					else
					{
						if (!currScenePath.empty())
						{
							m_Scene->SaveToFile(currScenePath);
							//LOG_INFO("Scene saved:", currScenePath);
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
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
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

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}  // Disabled for now
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}  // Disabled for now
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "Ctrl+X", false, false)) {}
				if (ImGui::MenuItem("Copy", "Ctrl+C", false, false)) {}
				if (ImGui::MenuItem("Paste", "Ctrl+V", false, false)) {}
				ImGui::EndMenu();
			}

			// to toggle show which panel
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Hierarchy", NULL, &hierachyWindow);
				ImGui::MenuItem("Properties", NULL, &inspectorWindow);
				ImGui::MenuItem("Performance Profile", NULL, &performanceProfileWindow);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Compile"))
			{
				// Compile fucntion goes here
				ImGui::EndMenu();
			}

			// ---------------- Display Current Scene Name ---------------------
			if (!currScenePath.empty())
			{
				std::filesystem::path filePath(currScenePath);
				std::string fileName = filePath.filename().string();

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
	}

	static void ReplaceChildNode(std::shared_ptr<BTNode> parent,
		std::shared_ptr<BTNode> oldChild,
		std::shared_ptr<BTNode> newChild)
	{
		auto& children = parent->GetChildren();
		for (size_t i = 0; i < children.size(); ++i)
		{
			if (children[i] == oldChild)
			{
				children[i] = newChild;
				break;
			}
		}
	}

	void DrawBTNodeEditor(std::shared_ptr<BTNode> node, std::shared_ptr<BTNode> parent = nullptr)
	{
		if (!node) return;

		// Unique label for ImGui to avoid ID collisions
		std::string labelID = node->GetName() + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.get()));

		// Determine flags for leaf/composite
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (!node->GetChildren().empty())
			flags |= ImGuiTreeNodeFlags_DefaultOpen;

		bool nodeOpen = ImGui::TreeNodeEx(labelID.c_str(), flags, "%s [%s]", node->GetName().c_str(), node->GetTypeName());

		// --- Right-click context menu ---
		if (ImGui::BeginPopupContextItem())
		{
			static int selectedType = 0;
			static bool addChildPending = false;

			// Add Child - TODO: Leaf node that's child of decorator isn't deleting properly
			if (node->CanHaveChildren()) {
				// Show Add Child submenu
				if (ImGui::BeginMenu("Add Child"))
				{
					auto allTypes = BehaviourTreeEditor::GetAllNodeTypes();

					ImGui::Text("Select Node Type:");
					ImGui::Separator();

					for (int i = 0; i < (int)allTypes.size(); ++i)
					{
						bool isSelected = (selectedType == i);
						if (ImGui::Selectable(allTypes[i].c_str(), isSelected))
						{
							selectedType = i;
							addChildPending = true;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndMenu();
				}

				if (addChildPending)
				{
					addChildPending = false;
					auto allTypes = BehaviourTreeEditor::GetAllNodeTypes();
					auto newChild = BehaviourTreeEditor::CreateNode(allTypes[selectedType]);
					BehaviourTreeEditor::AddChildNode(node, newChild);
				}
			}

			if (parent)
			{
				if (ImGui::MenuItem("Remove Node"))
				{
					BehaviourTreeEditor::RemoveChildNode(parent, node);
					ImGui::EndPopup();

					// Pop the tree node BEFORE returning if it was opened
					if (nodeOpen)
						ImGui::TreePop();

					return;
				}
			}

			ImGui::EndPopup();
		}

		// --- Inline node editor ---
		if (nodeOpen)
		{
			// Node name
			char nameBuffer[128];
			strncpy_s(nameBuffer, node->GetName().c_str(), _TRUNCATE);
			if (ImGui::InputText(("Name##" + labelID).c_str(), nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newName = nameBuffer;
				newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);
				newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));

				if (!newName.empty()) {
					node->SetName(nameBuffer);
				}

				/*if(nameBuffer[0] != '\0')
				{
					node->SetName(nameBuffer);
				}*/
			}

			// Node type selector
			std::vector<std::string> allTypes = BehaviourTreeEditor::GetAllNodeTypes();
			int currentTypeIndex = 0;
			for (size_t i = 0; i < allTypes.size(); ++i)
			{
				if (allTypes[i] == node->GetTypeName())
				{
					currentTypeIndex = (int)i;
					break;
				}
			}

			if (ImGui::Combo(("Type##" + labelID).c_str(), &currentTypeIndex,
				[](void* data, int idx, const char** outText) {
					auto& types = *static_cast<std::vector<std::string>*>(data);
					*outText = types[idx].c_str();
					return true;
				},
				static_cast<void*>(&allTypes), (int)allTypes.size()))
			{
				if (parent)
				{
					auto newNode = BehaviourTreeEditor::CreateNode(allTypes[currentTypeIndex]);
					newNode->SetName(node->GetName()); // Preserve name

					// Transfer children
					for (auto& child : node->GetChildren())
						newNode->AddChild(child);

					// Replace in parent's child list
					ReplaceChildNode(parent, node, newNode);
				}
			}

			// --- Node properties ---
			std::vector<std::pair<std::string, std::string>> properties;
			node->GetProperties(properties);

			for (auto& [name, value] : properties)
			{
				char valueBuffer[256];

				snprintf(valueBuffer, sizeof(valueBuffer), "%s", value.c_str());

				float fullWidth = ImGui::GetContentRegionAvail().x;
				ImGui::PushItemWidth(fullWidth * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f);

				// value box (left)
				if (ImGui::InputText(name.c_str(), valueBuffer, sizeof(valueBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (nameBuffer[0] != '\0')
					{
						node->SetProperty(name, valueBuffer);
					}
				}

				ImGui::PopItemWidth();
			}

			// --- Draw children recursively ---
			auto children = node->GetChildren(); // copy to avoid iterator invalidation if removed
			for (auto& child : children)
			{
				DrawBTNodeEditor(child, node);
			}
			ImGui::TreePop();
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

						//tag.Tag = std::string(buffer);
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

						if (ImGui::InputScalar("Parent", ImGuiDataType_U32, &parent_id))
						{
							TransformSystem::SetParent(m_Scene, m_SelectedEntity,  static_cast<entt::entity>(parent_id));
						}

						if (parent_id != u32_max)
						{
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
					//ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 30);
					
					if (ImGui::Button("...###RigidbodyBtn", dotButtonSize))
					{
						ImGui::OpenPopup("RigidBodyPopUp");
					}
					if (ImGui::BeginPopup("RigidBodyPopUp"))
					{
						if (ImGui::MenuItem("Remove Component"))
						{
							removeRigidBody = true;
							//return;
						}
						ImGui::EndPopup();
					}
					
					ImGui::Columns(1);
					//ImGui::Separator();
					
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

						//// gravity
						//ImGui::Text("Boolean to check if gravity affects the body");
						//bool isGravity = rigidBody.IsGravityEnabled();
						//if (ImGui::Checkbox("Use Gravity", &isGravity)) {
						//	rigidBody.SetGravityEnabled(isGravity);
						//}

						//ImGui::Separator();

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

						ImGui::Text("Display Runtime Value:");
						
						ImGui::BeginDisabled();

						/*glm::vec3 vel = rigidBody.GetVelocity();
						float velocity[3]{ vel.x, vel.y, vel.z };
						ImGui::InputFloat3("Velocity", velocity, "%.3f", ImGuiInputTextFlags_ReadOnly);*/
						
						float speed = rigidBody.GetSpeed();
						ImGui::InputFloat("Speed (m/s)", &speed, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly);

						bool isMoving = rigidBody.IsMoving();
						ImGui::Checkbox("Is Moving", &isMoving);

						bool isStatic = rigidBody.IsStatic();
						ImGui::Checkbox("Is Static", &isStatic);

						ImGui::EndDisabled();
								
						//ImGui::InputFloat3("Velocity", velocity, "%.3f", ImGuiInputTextFlags_ReadOnly);
						/*ImGui::Checkbox("Is Kinematic", &rigidBody.IsKinematic);
						ImGui::Checkbox("Use Gravity", &rigidBody.UseGravity);

						//bool isKinematic = rigidBody.
						ImGui::Separator();
						ImGui::Text("Display Runtime Value:");
						glm::vec3 vel = rigidBody.GetVelocity();
						float velocity[3]{ vel.x, vel.y, vel.z };
						ImGui::InputFloat3("Velocity", velocity, "%.3f", ImGuiInputTextFlags_ReadOnly);
						float speed = rigidBody.GetSpeed();
						ImGui::InputFloat("Speed (m/s)", &speed, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly);*/
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
							//return;
						}
						ImGui::EndPopup();
					}

					ImGui::Columns(1);
					//ImGui::Separator();


					if (openMeshComponent)
					{
						auto& mesh = m_SelectedEntity.GetComponent<MeshRendererComponent>();
						
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

						ImGui::SeparatorText("Values for Debugging:");

						ImGui::Text("Material: %u", mesh.Material);
						ImGui::Text("Mesh Type: %u", mesh.MeshType);
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
							//return;
						}
						ImGui::EndPopup();
					}

					ImGui::Columns(1);
					//ImGui::Separator();

					if (openAudioComponent)
					{
						//ImGui::Separator();
						auto& audio = m_SelectedEntity.GetComponent<AudioComponent>();

						ImGui::Separator();

						auto& db = AM.db();
						auto allAssets = db.AllMutable();

						std::vector<std::string> audioAssetNames;
						audioAssetNames.reserve(allAssets.size());

						for (const auto* record : allAssets) {
							if (!record || !record->valid) continue;

							if (record->type == ResourceType::AUDIO) {
								std::string filepath = record->sourcePath;
								size_t lastSlash = filepath.find_last_of("/\\");
								std::string filename = (lastSlash == std::string::npos)
									? filepath
									: filepath.substr(lastSlash + 1);

								LOG_DEBUG("Filepath: ", filepath);
								LOG_DEBUG("Filename: ", filename);

								audioAssetNames.push_back(filename);
							}
						}

						std::vector<const char*> audioAssets;
						audioAssets.reserve(audioAssetNames.size());
						for (auto& name : audioAssetNames)
							audioAssets.push_back(name.c_str());

						int currentIndex = 0;
						for (size_t i = 0; i < audioAssetNames.size(); ++i) {
							if (audioAssetNames[i] == audio.AudioFilePath) {
								currentIndex = static_cast<int>(i);
								break;
							}
						}

						std::string label = "Filepath"; // Label for the dropdown
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
						if (ImGui::SliderFloat("MinDistance", &min_distance, 0.1f, 0.f)) {
							if (is_3d) {
								audio.SetMinDistance(min_distance);
							}
							else {
								audio.SetMinDistance(1.f);
							}
						}

						float max_distance = audio.MaxDistance;
						if (ImGui::SliderFloat("MaxDistance", &max_distance, 0.1f, 0.f)) {
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
				if (m_SelectedEntity.HasComponent<ReverbZoneComponent>())
				{
					ImGui::Separator();

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
							//return;
						}
						ImGui::EndPopup();
					}

					ImGui::Columns(1);

					if (removeReverb)
					{
						m_SelectedEntity.RemoveComponent<ReverbZoneComponent>();
					
					} else if (openReverbComponent) {
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
									ImGui::SetItemDefaultFocus();
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
				}
				if (m_SelectedEntity.HasComponent<ListenerComponent>())
				{
					ImGui::Separator();

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

					if (removeListener)
					{
						m_SelectedEntity.RemoveComponent<ListenerComponent>();
					
					} else if (openListenerComponent) {
						auto& listener = m_SelectedEntity.GetComponent<ListenerComponent>();
						bool& active = listener.Active;

						if (ImGui::Checkbox("Active###activeListener", &active)) {
							listener.Active = active;
						}
					}
				}
				if (m_SelectedEntity.HasComponent<BehaviourTreeComponent>())
				{
					ImGui::Separator();

					bool openBTComponent = ImGui::CollapsingHeader("Behaviour Tree Component", ImGuiTreeNodeFlags_DefaultOpen);

					if (openBTComponent) {

						auto& ai_bt = m_SelectedEntity.GetComponent<BehaviourTreeComponent>();

						// For actual BT
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

							ImGui::Text("Stack Depth: %zu", stackDepth);

							if (root)
							{
								ImGui::Text("Current Root: %s [%s]", root->GetName().c_str(), root->GetTypeName());
								DrawBTNodeEditor(root);
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

							// Dropdown to pick node type for new root
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
								// Optional: nothing here, selection changes root only when button clicked
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

							// Last execution status (for debugging)
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
								auto files = getAssetsInFolder(btFolderPath); // returns AssetEntry
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
								if (btAssets[i].fullPath == treeAssetPath) // store fullPath in treeAssetPath
								{
									currentIndex = (int)i;
									break;
								}
							}

							// Draw the combo box
							ImGui::Text("Tree Asset Path:");
							ImGui::SetNextItemWidth(400.0f);
							if (ImGui::Combo("##TreeAssetPath", &currentIndex,
								[](void* data, int idx, const char** outText) -> bool
								{
									auto& assets = *static_cast<std::vector<AssetEntry>*>(data);
									*outText = assets[idx].name.c_str(); // display name only

									return true;
								},
								static_cast<void*>(&btAssets), (int)btAssets.size()))
							{
								// Update treeAssetPath when selected
								ai_bt.TreeAssetPath = btAssets[currentIndex].fullPath; // store full path
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
						}
						else {
							ai_bt.TreeInstance = BehaviourTreeEditor::CreateNewTree("NewTree");
						}
						
						// BehaviourTreeEditor:
						/*CreateNewTree //IN ASSET BROWSER
						SetNodeProperty //
						GetAllNodeTypes
						GetAllCategories
						ConvertToPrefab //LATER
						LoadFromPrefab //LATER
						ValidateTree - Button -> 
						CloneTree
						*/
					}

					bool removeBT = false;

					ImGui::NextColumn();

					if (ImGui::Button("... ###BehaviorbBtn", dotButtonSize))
					{
						ImGui::OpenPopup("BehaviorPopUp");
					}
					if (ImGui::BeginPopup("BehaviorPopUp"))
					{
						if (ImGui::MenuItem("Remove Component"))
						{
							removeBT = true;
						}
						ImGui::EndPopup();
					}

					ImGui::Columns(1);

					if (removeBT)
					{
						m_SelectedEntity.RemoveComponent<BehaviourTreeComponent>();
					}
				}

				// ======================== Display Particle System Component ===============================
				if (m_SelectedEntity.HasComponent<ParticleComponent>())
				{
					ImGui::Separator();
					auto& particleComp = m_SelectedEntity.GetComponent<ParticleComponent>();

					if (ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen))
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
					//ImGui::SetWindowFontScale(1.3f);
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

					//ImGui::SetWindowFontScale(1.0f); // Reset

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

	void Editor::DrawEntityRecursive(Entity entity, entt::registry& registry)
	{
		auto& tag = entity.GetComponent<TagComponent>();
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		// Check for selection
		if (m_SelectedEntity == entity)
			flags |= ImGuiTreeNodeFlags_Selected;

		// Check if entity has children
		bool hasChildren = false;
		auto view = registry.view<TransformComponent>();
		for (auto childHandle : view)
		{
			auto& childTransform = view.get<TransformComponent>(childHandle);
			if (childTransform.Parent == (uint32_t)entity)
			{
				hasChildren = true;
				break;
			}
		}

		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());

		if (ImGui::IsItemClicked())
			m_SelectedEntity = entity;

		// Your existing popup code goes here (unchanged)
		// ----------------------------------------------------

		if (opened && hasChildren)
		{
			for (auto childHandle : view)
			{
				auto& childTransform = view.get<TransformComponent>(childHandle);
				if (childTransform.Parent == (uint32_t)entity)
				{
					Entity child(childHandle, &registry);
					DrawEntityRecursive(child, registry);
				}
			}
			ImGui::TreePop();
		}
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
					auto& tag = entity.GetComponent<TagComponent>();
					/*auto& transform = entity.GetComponent<TransformComponent>();

					if (transform.Parent == u32_max)
					{
						DrawEntityRecursive(entity, m_Scene->GetRegistry());
					}*/

					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					if (m_SelectedEntity == entity)
					{
						flags |= ImGuiTreeNodeFlags_Selected;
					}

					ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());

					if (ImGui::IsItemClicked())
					{
						m_SelectedEntity = entity;

						uint32_t newID = (uint32_t)entity; // uses your operator uint32_t()

						LOG_DEBUG("Clicked entity ID = ", newID, " | old m_PickedID = ", m_PickedID);

						m_PickedID = newID;

						LOG_DEBUG("Updated m_PickedID to ", m_PickedID);
					}

					// Right-click context menu
					if (ImGui::BeginPopupContextItem())
					{
						// ==================== Selected Entity Section =======================
						if (ImGui::MenuItem("Delete Entity"))
						{
							// If this entity has a parent, unparent it first
							if (entity.HasComponent<TransformComponent>()) {
								TransformSystem::UnParent(m_Scene, entity);
							}

							m_Scene->DestroyEntity(entity);
							if (m_SelectedEntity == entity)
							{
								m_SelectedEntity = Entity();
							}
						}

						// ===================== Prefab Section ==========================
						if (ImGui::BeginMenu("Prefabs"))
						{
							if (ImGui::MenuItem("Create Prefab"))
							{
								if (m_SelectedEntity)
								{
									std::string entityName = m_SelectedEntity.GetComponent<TagComponent>().Tag;
									auto prefab = PrefabSerializer::CreateEntityPrefab(m_SelectedEntity, entityName);

									if (!prefab)
									{
										//LOG_ERROR("Failed to create prefab from entity: {}", entityName);
										return;
									}

									auto prefabFolder = getAssetFilePath("Sources/Prefabs/") + entityName + ".prefab";

									if (PrefabSerializer::SavePrefabToFile(*prefab, prefabFolder))
									{
										//LOG_INFO("Prefab saved to {}", prefabFolder);
										PrefabRegistry::Get().RegisterPrefab(prefab);
										m_CurrentPrefab = prefab.get();
										currPrefabPath = prefabFolder;
										//isPrefabEditor = true;
										//m_CurrentPrefab = PrefabSerializer::LoadPrefabFromFile(path);
										m_TemporaryPrefabPaths.insert(prefabFolder);

									}
									//m_SelectedEntity = entity;
								}
							}
							if (ImGui::MenuItem("Replace Prefab"))
							{
								//if (m_SelectedEntity)
								//{
									
								replacePrefabPending = true;
								selectedPrefabPath = "";
									//ImGui::OpenPopup("Select Prefab");
								//}
							}

							ImGui::EndMenu(); // end prefab menu
						}
						
						ImGui::EndPopup(); // end of the pop up context item

					}
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
			//LOG_DEBUG("TEST POP up replace is called ?");
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
					//newEntity.AddComponent<PrefabComponent>({ prefabFilePath, prefab->GetGUID() });

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
			//LOG_DEBUG("TEST POP up replace is called ?");
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
					if (folder.name != "Audio" && folder.name != "Meshes" && folder.name != "Shaders" && folder.name != "Textures") {
						bool isSelected = (selectedFolder == folder.fullPath);
						if (ImGui::Selectable(folder.name.c_str(), isSelected))
						{
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

			// to get the files in the selected folder
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

						// Unique ID per button so ImGui doesn�t confuse them
						ImGui::PushID(static_cast<int>(i));

						if (ImGui::Button(filename.c_str(), ImVec2(thumbnailSize, thumbnailSize))) {
							selectedResourcesIndex = static_cast<int>(i);
							ImGui::OpenPopup("AssetContextMenu");
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

								ImGui::EndPopup();
							}
						}

						if (isSelected)
							ImGui::PopStyleColor(3);

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
						if (extension == ".json") // if it is scene
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
								//LOG_DEBUG("////m_Scene->GetName() in json ", currFileName);
							}

							// LOG_DEBUG("Check isPrefabEditor is ", isPrefabEditor);

						}
						else if (extension == ".prefab")
						{
							//auto prefab = PrefabSerializer::LoadPrefabFromFile(filePath);
							if (!isPrefabEditor)
							{
								if (!currScenePath.empty())
								{
									m_Scene->SaveToFile(currScenePath);
									LOG_INFO("Scene auto-saved before switching to prefab:", currScenePath);
								}
							}
							//if (!saveAsPanel)
							//{
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
							//}

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
						//ImGui::Text("Type: %s", filePath.c_str();
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

		if (ImGui::Begin("Descriptor Editor Panel", &showDescriptorEditorPanel)) {
			LOG_DEBUG("displayDescriptorEditorPanel OPEN");

			if (!descriptorEditor.IsLoaded() || currentEditingGuid != descriptorEditor.GetGuid()) {
				if (!descriptorEditor.Load(currentEditingGuid)) {
					ImGui::Text("Failed to load descriptor for %s", editedAsset.c_str());
				}
			}
			else {
				ImGui::Columns(2, nullptr, true);

				if (descriptorEditor.GetType() == ResourceType::TEXTURE) {
					auto* texture = RM.loadResource<TextureResource>(Engine::convertToTextureGuid(currentEditingGuid));
					if (texture != nullptr) {
						float tex_w = texture->width;
						float tex_h = texture->height;

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

				// Check type and get appropriate settings
				if (descriptorEditor.GetType() == ResourceType::TEXTURE) {

					TextureSettings* settings = descriptorEditor.GetTextureSettings();

					// Modify settings directly
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

					if (ImGui::BeginCombo("Usage", settings->compression.c_str())) {
						for (auto& option : descriptorEditor.GetCompressionOptions()) {
							if (ImGui::Selectable(option.c_str())) {
								settings->compression = option;
								descriptorEditor.MarkModified();
							}
						}
						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo("Compression", settings->usageType.c_str())) {
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
					}

					float meshScale = settings->scale;
					if (ImGui::DragFloat("Scale", &meshScale))
					{
						settings->scale = meshScale;
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
					if (ImGui::Button("Save Descriptor")) {
						if (descriptorEditor.Save()) {
							notifMsg = "Descriptor is Saved";
							notifColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
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
					//LOG_DEBUG("This is in", fullPath);
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
						//LOG_DEBUG("Scene save as: ", defaultNewScenePath);
						currScenePath = defaultNewScenePath; // update current scene path
						//std::string currJsonName = m_Scene->GetName();
						//LOG_DEBUG("////Bfr::m_Scene->GetName() in save as panel ", m_Scene->GetName());
						//LOG_DEBUG("////saveAsDefaultSceneName in save as panel ", saveAsDefaultSceneName);
						m_Scene->SetName(saveAsDefaultSceneName);
						//LOG_DEBUG("////Afr::m_Scene->GetName() in save as panel ", m_Scene->GetName());
						//m_Scene->SetName(saveAsDefaultSceneName);
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
					//std::cout << defaultNewScenePath << "json file test\n";
					m_Scene->SaveToFile(defaultNewScenePath);
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

		/*if (!saveAsPanel)
			return;

		ImGui::OpenPopup("Save Scene As");

		if (ImGui::BeginPopupModal("Save Scene As", &saveAsPanel))
		{
			static char scenePath[256] = "Resources/Scenes/";

			ImGui::Text("Enter scene file name:");
			ImGui::InputText("##scenepath", scenePath, sizeof(scenePath));

			// Add .json extension if not present
			std::string pathStr(scenePath);
			if (pathStr.find(".json") == std::string::npos)
			{
				pathStr += ".json";
			}

			if (ImGui::Button("Save"))
			{
				SceneSerializer serializer(m_Scene);
				if (serializer.Serialize(pathStr))
				{
					currScenePath = pathStr;
					LOG_INFO("Scene saved successfully to: " + pathStr);
				}
				else
				{
					LOG_ERROR("Failed to save scene to: " + pathStr);
				}
				saveAsPanel = false;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				saveAsPanel = false;
			}

			ImGui::EndPopup();
		}*/
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
		//if (!entity) return;
		/*if (!entity || !m_Scene) {
			m_SelectedEntity = Entity{};
			m_PickedID = 0xFFFFFFFFu;
		if (!entity || !m_Scene || !entity.HasComponent<TransformComponent>()) {
			return;
		}*/

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

		//// Use screen coordinates for the mode label too
		//ImVec2 modeLabelPos = { m_ImGuizmoViewportData.tl.x + 10.0f, m_ImGuizmoViewportData.tl.y + 10.0f };
		//ImGui::GetForegroundDrawList()->AddText(
		//	modeLabelPos,
		//	IM_COL32(255, 230, 100, 255),
		//	m_Operation == ImGuizmo::TRANSLATE ? "Mode: Translate" :
		//	m_Operation == ImGuizmo::ROTATE ? "Mode: Rotate" :
		//	m_Operation == ImGuizmo::SCALE ? "Mode: Scale" : "Mode: None"
		//);
	}

} // end of namespace Engine

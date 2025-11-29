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
		//LoadAllPrefabsIntoRegistry();
		m_Initialized = true;
	}

	void Editor::OnUpdate(Timestep ts, GLuint texhandle)
	{
		if (!m_Initialized) return;

		/*if (!isPrefabEditor)
		{

			CheckAndUpdatePrefabInstances();
		}*/
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

		displayHDRSettingsPanel();

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
						if (isPrefabEditor)
						{
							isPrefabEditor = false;
							currPrefabPath.clear();
							//LOG_INFO("Exited prefab editor mode for new scene");
						}
						m_SelectedEntity = Entity();
						m_PickedID = 0xFFFFFFFFu;
						m_Scene->GetRegistry().clear();
						currScenePath = "";
						isNewScene = true;

						//m_Scene->SetName("New Scene");
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
							LOG_DEBUG("======= Start Prefab Save Scene =====");

							auto existingPrefab = PrefabSerializer::LoadPrefabFromFile(currPrefabPath);
							if (!existingPrefab)
							{
								LOG_ERROR("Cannot update - failed to load existing prefab: ", currPrefabPath);
								return;
							}

							if (existingPrefab->GetType() == PrefabType::Entity)
							{
								LOG_DEBUG("Updating Entity Prefab");
								if (m_SelectedEntity)
								{
									auto updatedPrefab = PrefabSerializer::CreateEntityPrefab(
										m_SelectedEntity,
										existingPrefab->GetName()
									);

									updatedPrefab->SetGUID(existingPrefab->GetGUID());
									if (PrefabSerializer::SavePrefabToFile(*updatedPrefab, currPrefabPath))
									{
										//auto reloadedPrefab = PrefabSerializer::LoadPrefabFromFile(currPrefabPath);
										/*if (reloadedPrefab)
										{

										}*/
										PrefabRegistry::Get().UpdatePrefab(updatedPrefab);
										MarkPrefabAsUpdated(updatedPrefab->GetGUID());
										PrefabRegistry::Get().RegisterPrefab(updatedPrefab);
										LOG_DEBUG("Entity Prefab updated successfully: ", currPrefabPath);
									}
								}
							}
							else
							{
								LOG_DEBUG("=== Start Scene Entity Prefab ===");


								std::vector<Entity> allEntities;
								auto view = m_Scene->GetRegistry().view<TagComponent>();
								for (auto entityHandle : view)
								{
									Entity entity(entityHandle, &m_Scene->GetRegistry());
									allEntities.push_back(entity);
								}
								auto updatedPrefab = PrefabSerializer::CreateScenePrefab(
									m_Scene,
									allEntities,
									existingPrefab->GetName()
								);

								if (!updatedPrefab)
								{
									LOG_ERROR("Failed to create updated scene prefab");
									return;
								}

								updatedPrefab->SetGUID(existingPrefab->GetGUID());

								if (PrefabSerializer::SavePrefabToFile(*updatedPrefab, currPrefabPath))
								{
									PrefabRegistry::Get().UpdatePrefab(updatedPrefab);
									MarkPrefabAsUpdated(updatedPrefab->GetGUID());
									PrefabRegistry::Get().RegisterPrefab(updatedPrefab);
									LOG_DEBUG("Scene Prefab updated successfully: ", currPrefabPath);
									//ClearPrefabInstances(existingPrefab->GetGUID());
								}
								else
								{
									LOG_ERROR("Failed to save scene prefab to file");
								}
								LOG_DEBUG("=== End Scene Entity Prefab ===");
							}

							LOG_DEBUG("======= End Save Prefab Scene =====");

						} // end of !currPrefabPath
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
				// =========================== Display PrefabComponent ==============================

				displayPrefabComp();

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
				displayRigidBodyComp(dotButtonSize);
				// =========================== Display Mesh Render Component ===========================
				displayMeshRendererComp(dotButtonSize);
				// =========================== Display Audio Component =================================
				displayAudioComp(dotButtonSize);
				// ========================== Display ReverbZoneComponent =====================================
				displayReverbZoneComp(dotButtonSize);
				// ====================================== Display ListenerComponent ==================================
				displayListenerComp(dotButtonSize);
				// =============================== Display BT Component =========================
				displayBTComp(dotButtonSize);
				// ======================== Display Particle System Component ===============================
				displayParticleComp(dotButtonSize);
				// ========================= Display Script Compoment ===============================
				displayScriptComp(dotButtonSize);
				// ================================ Display Light Component ======================================
				displayLightComp(dotButtonSize);

				// ============================ Display Camera Comp ===============================
				displayCameraComp(dotButtonSize);

				// =========================== Display Animator Component ===========================
				displayAnimatorComp(dotButtonSize);

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

					addComponents();
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
				//std::vector<entt::entity> rootEntities;

				auto view = m_Scene->GetRegistry().view<TagComponent>();

				for (auto entityHandle : view)
				{

					Entity entity(entityHandle, &m_Scene->GetRegistry());
					auto& transform = entity.GetComponent<TransformComponent>();
					if (transform.Parent == u32_max)
					{
						EditorHierarchyHelper::DrawEntityParentAndChildren(entity, m_Scene, m_SelectedEntity, m_PickedID,
							m_CurrentPrefab, m_TemporaryPrefabPaths, currPrefabPath, replacePrefabPending, selectedPrefabPath, m_Scene);
					}
				}

				EditorHierarchyHelper::DeleteEntityParentAndChildren(m_Scene);

				EditorHierarchyHelper::CheckForParentlessChildren(m_Scene);
				EditorHierarchyHelper::ClearParentlessChildren(m_Scene);

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

					//auto view = m_Scene->GetRegistry().view<TagComponent>();
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

					auto& registry = PrefabRegistry::Get();
					std::shared_ptr<Prefab> prefab = nullptr;

					for (auto& [guid, regPrefab] : registry.GetAllPrefabs())
					{
						if (regPrefab && regPrefab->GetSourcePath() == selectedPrefabPath)
						{
							prefab = regPrefab;
							break;
						}
					}
					if (!prefab)
					{
						prefab = PrefabSerializer::LoadPrefabFromFile(selectedPrefabPath);
						if (!prefab)
						{
							ImGui::CloseCurrentPopup();
							return;
						}
						registry.RegisterPrefab(prefab);
					}

					Entity entityToReplace = m_SelectedEntity;
					if (entityToReplace && !isPrefabEditor)
					{

						if (prefab->GetType() == PrefabType::Entity)
						{

							m_Scene->DestroyEntity(entityToReplace);
							Entity newEntity = PrefabInstantiator::InstantiateEntityPrefab(
								m_Scene,
								prefab->GetGUID()
							);
							m_SelectedEntity = newEntity;
						}
						else // prefab with parent and child
						{
							LOG_DEBUG("===== Replacing Scene Prefab ======");
							auto& registry = m_Scene->GetRegistry();
							entt::entity selectedEntt = (entt::entity)entityToReplace;

							// Save the parent before deleting
							entt::entity oldParent = entt::null;
							if (registry.all_of<TransformComponent>(selectedEntt))
							{
								auto& t = registry.get<TransformComponent>(selectedEntt);
								if (t.Parent != u32_max)
									oldParent = (entt::entity)t.Parent;
							}

							m_Scene->DestroyEntity(entityToReplace);

							Entity newRoot = PrefabInstantiator::InstantiateScenePrefab(
								m_Scene, prefab->GetGUID()
							);

							if (oldParent != entt::null)
							{
								Entity parentEntity(oldParent, &registry);
								auto& parentTransform = parentEntity.GetComponent<TransformComponent>();
								parentTransform.Children.push_back((uint32_t)newRoot);

								auto& newRootTransform = newRoot.GetComponent<TransformComponent>();
								newRootTransform.Parent = (uint32_t)oldParent;
							}

							m_SelectedEntity = newRoot;
							LOG_DEBUG("===== End of Replacing Scene Prefab ======");
						}
					}


					ImGui::CloseCurrentPopup();
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
#if 0 // original code bfr modified
					auto existingInRegistry = PrefabRegistry::Get().GetPrefab(prefab->GetGUID());

					PrefabRegistry::Get().RegisterPrefab(prefab);

					Entity newEntity = PrefabInstantiator::InstantiateEntityPrefab(
						m_Scene,
						prefab->GetGUID()
					);
#endif 

#if 1
					//PrefabRegistry::Get().RegisterPrefab(prefab);
					Entity newEntity;
					if (prefab->GetType() == PrefabType::Scene)
					{
						newEntity = PrefabInstantiator::InstantiateScenePrefab(m_Scene, prefab->GetGUID());
					}
					else
					{
						newEntity = PrefabInstantiator::InstantiateEntityPrefab(m_Scene, prefab->GetGUID());
					}
#endif
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

		// First-time size
		ImGui::SetNextWindowSize(ImVec2(1000.0f, 450.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Animator", &animatorWindow))
		{
			ImGui::End();
			return;
		}

		// If requested, focus this window on the next frame
		if (m_FocusAnimatorNextFrame)
		{
			ImGui::SetWindowFocus();
			m_FocusAnimatorNextFrame = false;
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

		// ---------------------------------------------------------------------
		// Look up controller from storage (or allow user to create a new one)
		// ---------------------------------------------------------------------
		auto ctrlIt = m_AnimatorControllerStorage.find(animator.controller);
		if (ctrlIt == m_AnimatorControllerStorage.end())
		{
			ImGui::Text("Animator has no valid controller (handle %u).", animator.controller);
			ImGui::Spacing();

			if (ImGui::Button("Create New Controller"))
			{
				AnimatorController newCtrl{};
				newCtrl.name = "NewController";
				newCtrl.defaultClipIndex = 0;

				u32 newHandle = static_cast<u32>(m_AnimatorControllerStorage.size());
				newCtrl.id = newHandle;
				m_AnimatorControllerStorage[newHandle] = newCtrl;

				animator.controller = newHandle;
				animator.currentClipIndex = 0;
				animator.currentTime = 0.0f;
				m_DopesheetSelectedTrack = DopesheetTrackType::None;
				m_DopesheetSelectedKey = -1;
			}

			ImGui::End();
			return;
		}

		AnimatorController& controller = ctrlIt->second;

		// Static state for Controller "Save As" popup
		static bool s_OpenCtrlSaveAsPopup = false;
		static char s_CtrlFileNameBuf[128] = "NewController";

		// NEW: static state for Add/Remove clip popups
		static bool s_OpenAddClipPopup = false;
		static int  s_AddClipSelectedIndex = 0;
		static bool s_AddClipDuplicateWarning = false;

		static bool s_OpenRemoveClipPopup = false;
		static int  s_RemoveClipSelectedIndex = 0;

		// Helper: controller file toolbar (New / Save / Save As)
		bool newControllerCreated = false;
		auto DrawControllerFileToolbar = [&](AnimatorController& controllerRef, AnimatorComponent& animatorRef)
			{
				ImGui::SeparatorText("Controller");

				// Controller name edit
				{
					char ctrlNameBuf[128];
					strncpy_s(ctrlNameBuf, sizeof(ctrlNameBuf), controllerRef.name.c_str(), _TRUNCATE);

					if (ImGui::InputText("Controller Name", ctrlNameBuf, sizeof(ctrlNameBuf),
						ImGuiInputTextFlags_EnterReturnsTrue))
					{
						std::string newName = ctrlNameBuf;

						// Trim
						newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));
						if (!newName.empty())
							newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);

						if (!newName.empty())
						{
							controllerRef.name = newName;
						}
					}
				}

				// --- New Controller ---
				if (ImGui::Button("New Controller"))
				{
					AnimatorController newCtrl{};
					newCtrl.name = "NewController";
					newCtrl.defaultClipIndex = 0;

					u32 newHandle = static_cast<u32>(m_AnimatorControllerStorage.size());
					newCtrl.id = newHandle;
					m_AnimatorControllerStorage[newHandle] = newCtrl;

					animatorRef.controller = newHandle;
					animatorRef.currentClipIndex = 0;
					animatorRef.currentTime = 0.0f;
					m_DopesheetSelectedTrack = DopesheetTrackType::None;
					m_DopesheetSelectedKey = -1;

					newControllerCreated = true;
				}

				ImGui::SameLine();

				// --- Save Controller ---
				if (ImGui::Button("Save Controller"))
				{
					Engine::SaveAnimatorControllerAsset(controller);
				}

				ImGui::SameLine();

				// --- Save Controller As ---
				if (ImGui::Button("Save Controller As"))
				{
					s_OpenCtrlSaveAsPopup = true;
					ImGui::OpenPopup("Save Controller As");
				}

				if (ImGui::BeginPopupModal("Save Controller As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					if (s_OpenCtrlSaveAsPopup)
					{
						std::string initial = controllerRef.name.empty() ? "NewController" : controllerRef.name;
						std::snprintf(s_CtrlFileNameBuf, sizeof(s_CtrlFileNameBuf), "%s", initial.c_str());
						s_OpenCtrlSaveAsPopup = false;
					}

					ImGui::Text("File name (without extension):");
					ImGui::InputText("##CtrlFileName", s_CtrlFileNameBuf, IM_ARRAYSIZE(s_CtrlFileNameBuf));

					if (ImGui::Button("OK"))
					{
						std::string fileName = s_CtrlFileNameBuf;
						if (fileName.empty())
							fileName = "NewController";

						controllerRef.name = fileName;

						std::string path1 = "../../Resources/Sources/AnimationControllers/" + fileName + ".animcontroller";
						std::string path2 = "../bin/Debug/Resources/Sources/AnimationControllers/" + fileName + ".animcontroller";

						SerializeAnimationController(controllerRef, path1);
						SerializeAnimationController(controllerRef, path2);

						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			};

		// =====================================================================
		// CASE 1: Controller exists but has NO clips yet.
		// Show a simpler UI that lets you save the controller and create first clip.
		// =====================================================================
		if (controller.clips.empty()) {
			ImGui::Text("Entity: %s", m_SelectedEntity.GetComponent<TagComponent>().Tag.c_str());
			ImGui::Text("Controller: %s (handle %u)", controller.name.c_str(), controller.id);

			DrawControllerFileToolbar(controller, animator);
			if (newControllerCreated)
			{
				ImGui::End();
				return;
			}

			ImGui::SeparatorText("Clips");
			ImGui::TextUnformatted("This controller has no clips yet.");
			ImGui::Spacing();

			if (ImGui::Button("New Clip"))
			{
				AnimationClip newClip{};
				newClip.name = "NewClip";
				newClip.duration = 1.0f;
				newClip.loop = true;

				u32 newHandle = static_cast<u32>(m_AnimationClipStorage.size());
				newClip.id = newHandle;
				m_AnimationClipStorage[newHandle] = newClip;

				controller.clips.push_back(newHandle);
				controller.defaultClipIndex = 0;
				animator.currentClipIndex = 0;
				animator.currentTime = 0.0f;
				m_DopesheetSelectedTrack = DopesheetTrackType::None;
				m_DopesheetSelectedKey = -1;
			}

			ImGui::End();
			return;
		}

		// =====================================================================
		// CASE 2: Normal path ? controller has at least one clip
		// =====================================================================
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

		// Compute left/right sizes
		ImVec2 avail = ImGui::GetContentRegionAvail();
		float leftWidth = glm::clamp(avail.x * 0.32f, 260.0f, avail.x * 0.5f);

		// =====================================================================
		// Split into LEFT (meta + tracks) and RIGHT (dopesheet/curves)
		// =====================================================================
		ImGui::BeginChild(
			"Animator_LeftPanel",
			ImVec2(leftWidth, 0.0f),          // full height, fixed width
			true,
			ImGuiWindowFlags_HorizontalScrollbar
		);

		// =============================== LEFT PANE ============================
		// -----------------------------------------------------------------
		// Header: entity + controller + clip selection
		// -----------------------------------------------------------------
		ImGui::Text("Entity: %s", m_SelectedEntity.GetComponent<TagComponent>().Tag.c_str());
		ImGui::Text("Controller: %s", controller.name.c_str());

		// Controller toolbar (New / Save / Save As)
		DrawControllerFileToolbar(controller, animator);
		if (newControllerCreated)
		{
			ImGui::EndChild();
			ImGui::End();
			return;
		}

		// -----------------------------------------------------------------
		// Clip selection
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Clip");

		// Clip dropdown
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

		// -----------------------------------------------------------------
		// Add / Remove Clip popups
		// -----------------------------------------------------------------
		ImGui::Spacing();

		if (ImGui::Button("Add Clip"))
		{
			s_OpenAddClipPopup = true;
			s_AddClipSelectedIndex = 0;
			s_AddClipDuplicateWarning = false;
			ImGui::OpenPopup("Add Clip to Controller");
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove Clip"))
		{
			s_OpenRemoveClipPopup = true;
			if (!controller.clips.empty())
			{
				s_RemoveClipSelectedIndex = glm::clamp((int)animator.currentClipIndex, 0, (int)controller.clips.size() - 1);
			}
			ImGui::OpenPopup("Remove Clip from Controller");
		}

		// --- Add Clip popup ---
		if (ImGui::BeginPopupModal("Add Clip to Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// Build list of all clips in storage
			std::vector<std::pair<u32, AnimationClip*>> allClips;
			allClips.reserve(m_AnimationClipStorage.size());
			for (auto& kv : m_AnimationClipStorage)
			{
				allClips.emplace_back(kv.first, &kv.second);
			}

			if (allClips.empty())
			{
				ImGui::TextUnformatted("No clips available in storage.");
			}
			else
			{
				if (s_AddClipSelectedIndex < 0 || s_AddClipSelectedIndex >= (int)allClips.size())
					s_AddClipSelectedIndex = 0;

				if (ImGui::BeginListBox("##AddClipList", ImVec2(350, 200)))
				{
					for (int i = 0; i < (int)allClips.size(); ++i)
					{
						u32 h = allClips[i].first;
						AnimationClip* c = allClips[i].second;

						std::string label = std::to_string(h) + " - " +
							(c->name.empty() ? "Unnamed Clip" : c->name);

						bool selected = (i == s_AddClipSelectedIndex);
						if (ImGui::Selectable(label.c_str(), selected))
							s_AddClipSelectedIndex = i;

						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndListBox();
				}
			}

			if (s_AddClipDuplicateWarning)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
					"This clip is already in the controller.");
			}

			if (ImGui::Button("Add"))
			{
				if (!allClips.empty())
				{
					u32 handleToAdd = allClips[s_AddClipSelectedIndex].first;
					bool already = false;
					for (u32 existing : controller.clips)
					{
						if (existing == handleToAdd)
						{
							already = true;
							break;
						}
					}

					if (already)
					{
						s_AddClipDuplicateWarning = true;
					}
					else
					{
						controller.clips.push_back(handleToAdd);
						animator.currentClipIndex = (int)controller.clips.size() - 1;
						animator.currentTime = 0.0f;
						controller.defaultClipIndex = animator.currentClipIndex;
						m_DopesheetSelectedTrack = DopesheetTrackType::None;
						m_DopesheetSelectedKey = -1;

						s_AddClipDuplicateWarning = false;
						ImGui::CloseCurrentPopup();
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// --- Remove Clip popup ---
		if (ImGui::BeginPopupModal("Remove Clip from Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// Build list of clips currently in controller
			std::vector<std::pair<u32, AnimationClip*>> controllerClipList;
			controllerClipList.reserve(controller.clips.size());
			for (u32 h : controller.clips)
			{
				auto it = m_AnimationClipStorage.find(h);
				AnimationClip* c = (it != m_AnimationClipStorage.end()) ? &it->second : nullptr;
				controllerClipList.emplace_back(h, c);
			}

			if (controllerClipList.empty())
			{
				ImGui::TextUnformatted("Controller has no clips.");
			}
			else
			{
				if (s_RemoveClipSelectedIndex < 0 ||
					s_RemoveClipSelectedIndex >= (int)controllerClipList.size())
				{
					s_RemoveClipSelectedIndex = 0;
				}

				if (ImGui::BeginListBox("##RemoveClipList", ImVec2(350, 200)))
				{
					for (int i = 0; i < (int)controllerClipList.size(); ++i)
					{
						u32 h = controllerClipList[i].first;
						AnimationClip* c = controllerClipList[i].second;

						std::string label = std::to_string(h) + " - ";
						if (c)
							label += (c->name.empty() ? "Unnamed Clip" : c->name);
						else
							label += "(missing)";

						bool selected = (i == s_RemoveClipSelectedIndex);
						if (ImGui::Selectable(label.c_str(), selected))
							s_RemoveClipSelectedIndex = i;
						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndListBox();
				}
			}

			if (ImGui::Button("Remove"))
			{
				if (!controllerClipList.empty())
				{
					int removeIndex = s_RemoveClipSelectedIndex;
					if (removeIndex >= 0 && removeIndex < (int)controller.clips.size())
					{
						controller.clips.erase(controller.clips.begin() + removeIndex);

						if (controller.clips.empty())
						{
							animator.currentClipIndex = 0;
							animator.currentTime = 0.0f;
							m_DopesheetSelectedTrack = DopesheetTrackType::None;
							m_DopesheetSelectedKey = -1;
						}
						else
						{
							if (animator.currentClipIndex >= (int)controller.clips.size())
								animator.currentClipIndex = (int)controller.clips.size() - 1;
							if (controller.defaultClipIndex >= (int)controller.clips.size())
								controller.defaultClipIndex = (int)controller.clips.size() - 1;
						}
					}
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

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

				// Trimming
				newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));
				newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);

				if (!newName.empty())
				{
					clip.name = newName;
				}
			}
		}

		ImGui::InputFloat("Duration (s)", &clip.duration);
		if (clip.duration <= 0.0f) clip.duration = 0.001f; // Ensure sane duration

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
		// Track editors (tables) - now synced with timeline selection
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Component");

		const char* componentItems[] = { "Transform", "UV Transform" };
		int componentIndex =
			(m_SelectedComponentTrack == AnimatorComponentTrack::Transform) ? 0 : 1;

		if (ImGui::Combo("##AnimatorComponent",
			&componentIndex,
			componentItems,
			IM_ARRAYSIZE(componentItems)))
		{
			m_SelectedComponentTrack = (componentIndex == 0)
				? AnimatorComponentTrack::Transform
				: AnimatorComponentTrack::UVTransform;
		}

		ImGui::Spacing();
		ImGui::SeparatorText("Tracks");

		if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
		{
			ImGui::Text("Transform");

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
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(3);
						ImGui::DragFloat("##Y", &k.position.y, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(4);
						ImGui::DragFloat("##Z", &k.position.z, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
						}

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

					RotationKeyframe k;
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
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(3);
						changed |= ImGui::DragFloat("##Yaw", &eulerDeg.y, 1.0f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(4);
						changed |= ImGui::DragFloat("##Roll", &eulerDeg.z, 1.0f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
						}

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
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(3);
						ImGui::DragFloat("##Y", &k.scale.y, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(4);
						ImGui::DragFloat("##Z", &k.scale.z, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
						}


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
		}
		else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
		{
			ImGui::Text("UV Transform");

			// Helper: sort by time after edits
			auto sortByTime = [](auto& keys)
				{
					std::sort(keys.begin(), keys.end(),
						[](const auto& a, const auto& b) { return a.time < b.time; });
				};

			// --------------------- Tiling track ---------------------
			if (ImGui::CollapsingHeader("Tiling", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("Add Key##UVTiling"))
				{
					Engine::UVKeyframe k;
					k.time = animator.currentTime;

					// Default tiling 1,1 or copy last key
					if (!clip.uvTilingKeys.empty())
						k.value = clip.uvTilingKeys.back().value;
					else
						k.value = { 1.0f, 1.0f };

					clip.uvTilingKeys.push_back(k);
					sortByTime(clip.uvTilingKeys);
				}

				if (ImGui::BeginTable("UVTilingKeysTable", 5,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("U##UVTiling");
					ImGui::TableSetupColumn("V##UVTiling");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(clip.uvTilingKeys.size()); ++i)
					{
						auto& k = clip.uvTilingKeys[static_cast<size_t>(i)];
						ImGui::PushID(3000 + i);

						ImGui::TableNextRow();

						bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::UVTiling &&
							m_DopesheetSelectedKey == i);

						// Index
						ImGui::TableSetColumnIndex(0);
						if (isSelected)
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						ImGui::Text("%d", i);
						if (isSelected)
							ImGui::PopStyleColor();

						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
							animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
						}

						// Time
						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
						}

						// U
						ImGui::TableSetColumnIndex(2);
						float u = k.value[0];
						if (ImGui::DragFloat("##U", &u, 0.01f))
							k.value[0] = u;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
						}

						// V
						ImGui::TableSetColumnIndex(3);
						float v = k.value[1];
						if (ImGui::DragFloat("##V", &v, 0.01f))
							k.value[1] = v;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
						}

						// Remove
						ImGui::TableSetColumnIndex(4);
						if (ImGui::SmallButton("X"))
						{
							clip.uvTilingKeys.erase(clip.uvTilingKeys.begin() + i);
							if (m_DopesheetSelectedTrack == DopesheetTrackType::UVTiling &&
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


					sortByTime(clip.uvTilingKeys);
					ImGui::EndTable();
				}
			}

			// --------------------- Offset track ---------------------
			if (ImGui::CollapsingHeader("Offset", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("Add Key##UVOffset"))
				{
					Engine::UVKeyframe k;
					k.time = animator.currentTime;

					// Default offset 0,0 or copy last key
					if (!clip.uvOffsetKeys.empty())
						k.value = clip.uvOffsetKeys.back().value;
					else
						k.value = { 0.0f, 0.0f };

					clip.uvOffsetKeys.push_back(k);
					sortByTime(clip.uvOffsetKeys);
				}

				if (ImGui::BeginTable("UVOffsetKeysTable", 5,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("U##UVOffset");
					ImGui::TableSetupColumn("V##UVOffset");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(clip.uvOffsetKeys.size()); ++i)
					{
						auto& k = clip.uvOffsetKeys[static_cast<size_t>(i)];
						ImGui::PushID(4000 + i);

						ImGui::TableNextRow();

						bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::UVOffset &&
							m_DopesheetSelectedKey == i);

						// Index
						ImGui::TableSetColumnIndex(0);
						if (isSelected)
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						ImGui::Text("%d", i);
						if (isSelected)
							ImGui::PopStyleColor();

						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
							animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
						}

						// Time
						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
						}

						// U
						ImGui::TableSetColumnIndex(2);
						float u = k.value[0];
						if (ImGui::DragFloat("##U", &u, 0.01f))
							k.value[0] = u;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
						}

						// V
						ImGui::TableSetColumnIndex(3);
						float v = k.value[1];
						if (ImGui::DragFloat("##V", &v, 0.01f))
							k.value[1] = v;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
						}

						// Remove
						ImGui::TableSetColumnIndex(4);
						if (ImGui::SmallButton("X"))
						{
							clip.uvOffsetKeys.erase(clip.uvOffsetKeys.begin() + i);
							if (m_DopesheetSelectedTrack == DopesheetTrackType::UVOffset &&
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


					sortByTime(clip.uvOffsetKeys);
					ImGui::EndTable();
				}
			}
		}

		// ---------------------------------------------------------------------
		// File toolbar: New / Save / Save As (for the current clip)
		// ---------------------------------------------------------------------
		if (clipPtr)
		{
			ImGui::SeparatorText("File");

			// ---- New Clip: create a brand-new asset and attach it ----
			if (ImGui::Button("New Clip"))
			{
				AnimationClip newClip{};
				newClip.name = "NewClip";
				newClip.duration = 1.0f;
				newClip.loop = true;

				// Allocate new handle = current storage size (simple allocator)
				u32 newHandle = static_cast<u32>(m_AnimationClipStorage.size());
				newClip.id = newHandle;

				m_AnimationClipStorage[newHandle] = newClip;

				// Attach to controller and select it
				controller.clips.push_back(newHandle);
				animator.currentClipIndex = static_cast<int>(controller.clips.size() - 1);

				// Reset selection
				m_DopesheetSelectedTrack = DopesheetTrackType::None;
				m_DopesheetSelectedKey = -1;
			}

			ImGui::SameLine();

			// ---- Save: overwrite current clip file (same id/handle) ----
			if (ImGui::Button("Save"))
			{
				Engine::SaveAnimationClipAsset(clip);
			}

			ImGui::SameLine();

			// ---- Save As: clone current clip into a new asset + new id ----
			static bool openSaveAsPopup = false;
			if (ImGui::Button("Save As"))
			{
				openSaveAsPopup = true;
				ImGui::OpenPopup("Save Clip As");
			}

			if (ImGui::BeginPopupModal("Save Clip As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				static char fileNameBuf[128] = "NewClip";

				// Pre-fill with current clip name when popup opens
				if (openSaveAsPopup)
				{
					std::string initial = clip.name.empty() ? "NewClip" : clip.name;
					std::snprintf(fileNameBuf, sizeof(fileNameBuf), "%s", initial.c_str());
					openSaveAsPopup = false;
				}

				ImGui::Text("File name (without extension):");
				ImGui::InputText("##ClipFileName", fileNameBuf, IM_ARRAYSIZE(fileNameBuf));

				if (ImGui::Button("OK"))
				{
					std::string fileName = fileNameBuf;
					if (fileName.empty())
						fileName = "NewClip";

					// Clone current clip into a new asset
					AnimationClip newClip = clip;
					newClip.name = fileName;

					u32 newHandle = static_cast<u32>(m_AnimationClipStorage.size());
					newClip.id = newHandle;

					m_AnimationClipStorage[newHandle] = newClip;
					controller.clips.push_back(newHandle);
					animator.currentClipIndex = static_cast<int>(controller.clips.size() - 1);

					std::string path1 = "../../Resources/Sources/AnimationClips/" + fileName + ".animclip";
					std::string path2 = "../bin/Debug/Resources/Sources/AnimationClips/" + fileName + ".animclip";

					SerializeAnimationClip(newClip, path1);
					SerializeAnimationClip(newClip, path2);

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::EndChild(); // End of left pane

		// =============================== RIGHT PANE ===========================
		ImGui::SameLine();

		ImGui::BeginChild("Animator_RightPanel", ImVec2(0.0f, 0.0f), true);

		// View mode buttons
		ImGui::SeparatorText("View");
		if (ImGui::RadioButton("Dopesheet", m_AnimatorViewMode == AnimatorViewMode::Dopesheet))
			m_AnimatorViewMode = AnimatorViewMode::Dopesheet;
		ImGui::SameLine();
		if (ImGui::RadioButton("Curves", m_AnimatorViewMode == AnimatorViewMode::Curves))
			m_AnimatorViewMode = AnimatorViewMode::Curves;

		ImGui::Separator();

		//// Colors per track
		//const ImU32 colPos = IM_COL32(80, 200, 120, 255);
		//const ImU32 colRot = IM_COL32(220, 100, 100, 255);
		//const ImU32 colScale = IM_COL32(100, 140, 230, 255);
		//const ImU32 colPosSel = IM_COL32(130, 255, 170, 255);
		//const ImU32 colRotSel = IM_COL32(255, 170, 170, 255);
		//const ImU32 colScaleSel = IM_COL32(160, 190, 255, 255);
		//const ImU32 colPlayhead = IM_COL32(255, 255, 50, 255);

		if (m_AnimatorViewMode == AnimatorViewMode::Dopesheet) {

			// Colors per track
			const ImU32 colPos = IM_COL32(80, 200, 120, 255);
			const ImU32 colRot = IM_COL32(220, 100, 100, 255);
			const ImU32 colScale = IM_COL32(100, 140, 230, 255);
			const ImU32 colPosSel = IM_COL32(130, 255, 170, 255);
			const ImU32 colRotSel = IM_COL32(255, 170, 170, 255);
			const ImU32 colScaleSel = IM_COL32(160, 190, 255, 255);

			const ImU32 colUVTiling = IM_COL32(220, 200, 80, 255);
			const ImU32 colUVOffset = IM_COL32(120, 180, 220, 255);
			const ImU32 colUVTilingSel = IM_COL32(255, 240, 120, 255);
			const ImU32 colUVOffsetSel = IM_COL32(170, 220, 255, 255);

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

			if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
			{
				drawLegendItem(colPos, "Position");
				drawLegendItem(colRot, "Rotation");
				drawLegendItem(colScale, "Scale");
			}
			else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
			{
				drawLegendItem(colUVTiling, "UV Tiling");
				drawLegendItem(colUVOffset, "UV Offset");
			}

			ImGui::NewLine();

			// Timeline canvas
			ImVec2 canvasPos = ImGui::GetCursorScreenPos();
			ImVec2 canvasSize = ImGui::GetContentRegionAvail();
			if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
			if (canvasSize.y < 60.0f) canvasSize.y = 60.0f;
			ImVec2 canvasEnd = ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);

			ImGui::InvisibleButton("##AnimDopesheet", canvasSize);
			bool  timelineHovered = ImGui::IsItemHovered();
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// Background
			drawList->AddRectFilled(canvasPos, canvasEnd, IM_COL32(20, 20, 20, 255));
			drawList->AddRect(canvasPos, canvasEnd, IM_COL32(80, 80, 80, 255));

			float midY = 0.5f * (canvasPos.y + canvasEnd.y);
			float keyRadius = 4.0f;
			float keyRadiusSq = keyRadius * keyRadius;

			// Time -> X mapping
			auto timeToX = [&](float t)
				{
					float u = (clip.duration > 0.0f) ? (t / clip.duration) : 0.0f;
					if (u < 0.0f) u = 0.0f;
					if (u > 1.0f) u = 1.0f;
					return canvasPos.x + u * canvasSize.x;
				};

			// ---------------- CLICK HANDLING (select nearest key or scrub) ----------------
			if (timelineHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				ImVec2 mousePos = ImGui::GetIO().MousePos;
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

				if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
				{
					testKeys(clip.positionKeys, DopesheetTrackType::Position, -12.0f);
					testKeys(clip.rotationKeys, DopesheetTrackType::Rotation, 0.0f);
					testKeys(clip.scaleKeys, DopesheetTrackType::Scale, +12.0f);
				}
				else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
				{
					testKeys(clip.uvTilingKeys, DopesheetTrackType::UVTiling, -6.0f);
					testKeys(clip.uvOffsetKeys, DopesheetTrackType::UVOffset, +6.0f);
				}

				if (bestTrack != DopesheetTrackType::None)
				{
					// Select nearest key and jump playhead
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
					case DopesheetTrackType::UVTiling:
						keyTime = clip.uvTilingKeys[static_cast<size_t>(bestIndex)].time;
						break;
					case DopesheetTrackType::UVOffset:
						keyTime = clip.uvOffsetKeys[static_cast<size_t>(bestIndex)].time;
						break;
					default:
						break;
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

			// ---------------- DRAW KEYS (with selection highlight) ----------------
			auto drawTrackKeys = [&](auto& keys,
				DopesheetTrackType trackType,
				ImU32 col,
				ImU32 colSelected,
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

			if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
			{
				drawTrackKeys(clip.positionKeys, DopesheetTrackType::Position,
					colPos, colPosSel, -12.0f);
				drawTrackKeys(clip.rotationKeys, DopesheetTrackType::Rotation,
					colRot, colRotSel, 0.0f);
				drawTrackKeys(clip.scaleKeys, DopesheetTrackType::Scale,
					colScale, colScaleSel, +12.0f);
			}
			else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
			{
				drawTrackKeys(clip.uvTilingKeys, DopesheetTrackType::UVTiling,
					colUVTiling, colUVTilingSel, -6.0f);
				drawTrackKeys(clip.uvOffsetKeys, DopesheetTrackType::UVOffset,
					colUVOffset, colUVOffsetSel, +6.0f);
			}

			// Playhead line
			{
				float x = timeToX(animator.currentTime);
				drawList->AddLine(ImVec2(x, canvasPos.y),
					ImVec2(x, canvasEnd.y),
					colPlayhead, 2.0f);
			}
		}
		else // ============================== CURVES VIEW ====================== 
		{
			// Choose which track to display curves for
			DopesheetTrackType track = m_DopesheetSelectedTrack;

			// If nothing explicitly selected, pick something sensible
			if (track == DopesheetTrackType::None)
			{
				if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
				{
					if (!clip.positionKeys.empty())      track = DopesheetTrackType::Position;
					else if (!clip.rotationKeys.empty()) track = DopesheetTrackType::Rotation;
					else if (!clip.scaleKeys.empty())    track = DopesheetTrackType::Scale;
				}
				else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
				{
					if (!clip.uvTilingKeys.empty())      track = DopesheetTrackType::UVTiling;
					else if (!clip.uvOffsetKeys.empty()) track = DopesheetTrackType::UVOffset;
				}
			}

			if (track == DopesheetTrackType::None)
			{
				ImGui::TextUnformatted("No keys to display curves for.");
				ImGui::EndChild();
				ImGui::End();
				return;
			}

			const ImU32 COL_X = IM_COL32(230, 90, 90, 255);   // red
			const ImU32 COL_Y = IM_COL32(100, 220, 120, 255);   // green
			const ImU32 COL_Z = IM_COL32(110, 160, 255, 255);   // blue

			// Legend
			switch (track)
			{
			case DopesheetTrackType::Position:
				DrawCurveLegendRow("Position", "X", COL_X, "Y", COL_Y, "Z", COL_Z);
				break;
			case DopesheetTrackType::Rotation:
				DrawCurveLegendRow("Rotation", "Pitch", COL_X, "Yaw", COL_Y, "Roll", COL_Z);
				break;
			case DopesheetTrackType::Scale:
				DrawCurveLegendRow("Scale", "X", COL_X, "Y", COL_Y, "Z", COL_Z);
				break;
			case DopesheetTrackType::UVTiling:
				// We only really use X/Y; Z legend is dummy
				DrawCurveLegendRow("UV Tiling", "U", COL_X, "V", COL_Y, "-", IM_COL32(80, 80, 80, 255));
				break;
			case DopesheetTrackType::UVOffset:
				DrawCurveLegendRow("UV Offset", "U", COL_X, "V", COL_Y, "-", IM_COL32(80, 80, 80, 255));
				break;
			default:
				break;
			}

			ImGui::Separator();

			ImVec2 cPos = ImGui::GetCursorScreenPos();
			ImVec2 cSize = ImGui::GetContentRegionAvail();
			if (cSize.x < 50.0f) cSize.x = 50.0f;
			if (cSize.y < 80.0f) cSize.y = 80.0f;
			ImVec2 cEnd = ImVec2(cPos.x + cSize.x, cPos.y + cSize.y);

			ImGui::InvisibleButton("##AnimCurves", cSize);
			ImDrawList* cDraw = ImGui::GetWindowDrawList();

			cDraw->AddRectFilled(cPos, cEnd, IM_COL32(20, 20, 20, 255));
			cDraw->AddRect(cPos, cEnd, IM_COL32(80, 80, 80, 255));

			auto timeToXCurve = [&](float t)
				{
					float u = (clip.duration > 0.0f) ? (t / clip.duration) : 0.0f;
					if (u < 0.0f) u = 0.0f;
					if (u > 1.0f) u = 1.0f;
					return cPos.x + u * cSize.x;
				};

			// Generic vec3 curve drawer (Position / Rotation / Scale)
			auto drawVec3Curves = [&](const auto& keys, auto getVec)
				{
					if (keys.size() < 2)
						return;

					std::vector<glm::vec3> values;
					values.reserve(keys.size());

					float minVal = 0.0f, maxVal = 0.0f;
					bool  first = true;

					for (size_t i = 0; i < keys.size(); ++i)
					{
						glm::vec3 v = getVec(keys[i]);
						values.push_back(v);

						float localMin = std::min(v.x, std::min(v.y, v.z));
						float localMax = std::max(v.x, std::max(v.y, v.z));

						if (first)
						{
							minVal = localMin;
							maxVal = localMax;
							first = false;
						}
						else
						{
							if (localMin < minVal) minVal = localMin;
							if (localMax > maxVal) maxVal = localMax;
						}
					}

					if (maxVal - minVal < 1e-3f)
					{
						maxVal += 0.5f;
						minVal -= 0.5f;
					}

					auto valToY = [&](float v)
						{
							float u = (v - minVal) / (maxVal - minVal);
							if (u < 0.0f) u = 0.0f;
							if (u > 1.0f) u = 1.0f;
							return cEnd.y - u * cSize.y;
						};

					auto drawAxis = [&](int axis, ImU32 color)
						{
							ImVec2 prev;
							bool   hasPrev = false;
							for (size_t i = 0; i < keys.size(); ++i)
							{
								float t = keys[i].time;
								float x = timeToXCurve(t);
								float v = (axis == 0) ? values[i].x :
									(axis == 1) ? values[i].y : values[i].z;
								float y = valToY(v);

								ImVec2 cur(x, y);
								if (hasPrev)
									cDraw->AddLine(prev, cur, color, 2.0f);
								prev = cur;
								hasPrev = true;
							}
						};

					// X/Y/Z curves
					drawAxis(0, COL_X);
					drawAxis(1, COL_Y);
					drawAxis(2, COL_Z);
				};

			// Vec2 curve drawer (UV Tiling / Offset)
			auto drawVec2Curves = [&](const auto& keys, auto getVec)
				{
					if (keys.size() < 2)
						return;

					std::vector<glm::vec2> values;
					values.reserve(keys.size());

					float minVal = 0.0f, maxVal = 0.0f;
					bool  first = true;

					for (size_t i = 0; i < keys.size(); ++i)
					{
						glm::vec2 v = getVec(keys[i]);
						values.push_back(v);

						float localMin = std::min(v.x, v.y);
						float localMax = std::max(v.x, v.y);

						if (first)
						{
							minVal = localMin;
							maxVal = localMax;
							first = false;
						}
						else
						{
							if (localMin < minVal) minVal = localMin;
							if (localMax > maxVal) maxVal = localMax;
						}
					}

					if (maxVal - minVal < 1e-3f)
					{
						maxVal += 0.5f;
						minVal -= 0.5f;
					}

					auto valToY = [&](float v)
						{
							float u = (v - minVal) / (maxVal - minVal);
							if (u < 0.0f) u = 0.0f;
							if (u > 1.0f) u = 1.0f;
							return cEnd.y - u * cSize.y;
						};

					auto drawAxis = [&](int axis, ImU32 color)
						{
							ImVec2 prev;
							bool   hasPrev = false;
							for (size_t i = 0; i < keys.size(); ++i)
							{
								float t = keys[i].time;
								float x = timeToXCurve(t);
								float v = (axis == 0) ? values[i].x : values[i].y;
								float y = valToY(v);

								ImVec2 cur(x, y);
								if (hasPrev)
									cDraw->AddLine(prev, cur, color, 2.0f);
								prev = cur;
								hasPrev = true;
							}
						};

					// U/V curves
					drawAxis(0, COL_X);
					drawAxis(1, COL_Y);
				};

			// Dispatch to correct curve drawer
			switch (track)
			{
			case DopesheetTrackType::Position:
				drawVec3Curves(clip.positionKeys,
					[](const PositionKeyframe& k) { return k.position; });
				break;
			case DopesheetTrackType::Rotation:
				drawVec3Curves(clip.rotationKeys,
					[](const RotationKeyframe& k)
					{
						glm::vec3 euler = glm::degrees(glm::eulerAngles(k.rotation));
						return euler;
					});
				break;
			case DopesheetTrackType::Scale:
				drawVec3Curves(clip.scaleKeys,
					[](const ScaleKeyframe& k) { return k.scale; });
				break;
			case DopesheetTrackType::UVTiling:
				drawVec2Curves(clip.uvTilingKeys,
					[](const UVKeyframe& k)
					{
						return glm::vec2(k.value[0], k.value[1]);
					});
				break;
			case DopesheetTrackType::UVOffset:
				drawVec2Curves(clip.uvOffsetKeys,
					[](const UVKeyframe& k)
					{
						return glm::vec2(k.value[0], k.value[1]);
					});
				break;
			default:
				break;
			}

		}
		ImGui::EndChild(); // End of right pane

		ImGui::End();
	}

	void Editor::displayAssetsBrowserPanel()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Assets Browser", &assetsWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			ImGui::Columns(2, nullptr, true);
			//static std::string selectedFolder = "";
			//static ResourceType selectedType = ResourceType::UNKNOWN;

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
			}
			else if (!raw_asset && selectedResourcesIndex != -1) {
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

							currScenePath = filePath; // update curr file path
							currFileName = fileName; // store file name

							//LoadAllPrefabsIntoRegistry();
							//m_Scene->SetName(fileName);
							LOG_DEBUG("m_Scene->SetName(fileName)", fileName);
							if (m_Scene)
							{
								m_SelectedEntity = Entity{};
								//auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
								isPrefabEditor = false;
								LoadAllPrefabsIntoRegistry();
								m_Scene->GetRegistry().clear();
								m_Scene->LoadFromFile(filePath);
								// check for update 
								//LOG_DEBUG("Before CheckAndUpdatePrefabInstances - isPrefabEditor: ", isPrefabEditor);

								CheckAndUpdatePrefabInstances();

								if (!m_UpdatedPrefabsThisSession.empty())
								{
									m_Scene->SaveToFile(filePath);
									m_Scene->SaveToFile(convertAssetPathToRootResources(filePath));
									LOG_DEBUG("Scene auto-saved after prefab updates");
								}

								//m_Scene->SaveToFile(filePath);
								m_SelectedEntity = Entity{}; // resets
								m_PickedID = 0xFFFFFFFFu;
								m_Operation = static_cast<ImGuizmo::OPERATION>(-1);
								//LOG_DEBUG("After setting isPrefabEditor: ", isPrefabEditor);
							}
						}
						else if (extension == ".prefab" && folderName != "BT") // FOr Prefab, not BT (To be fixed in M3)
						{
							LOG_DEBUG("=====Start Load Prefab File=========");
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

							//m_Scene->SetName("Prefab");
							auto prefab = PrefabSerializer::LoadPrefabFromFile(currPrefabPath);
							auto& registry = PrefabRegistry::Get();
							PrefabRegistry::Get().RegisterPrefab(prefab);

							if (prefab)
							{
								m_Scene->GetRegistry().clear();
								//PrefabRegistry::Get().RegisterPrefab(prefab);

#if 1 // added code for the parent child prefab file to work
								Entity entity;
								if (prefab->GetType() == PrefabType::Scene) {

									LOG_INFO("Loading Scene prefab with hierarchy");
									entity = PrefabInstantiator::InstantiateScenePrefab(m_Scene, prefab->GetGUID());
								}
								else {
									LOG_INFO("Loading single Entity prefab");
									entity = PrefabInstantiator::InstantiateEntityPrefab(m_Scene, prefab->GetGUID());
								}


								if (!currScenePath.empty())
								{
									currScenePath.clear();
								}

								m_SelectedEntity = Entity(); //reset entity
								m_PickedID = 0xFFFFFFFFu;
								isPrefabEditor = true;


								LOG_INFO("Now editing prefab:", currPrefabPath);
								LOG_DEBUG("=====End Load Prefab File=========");

#endif
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

					// ========== TRANSFORM SECTION ==========
					ImGui::SeparatorText("Transform");

					// Scale
					float meshScale = settings->scale;
					if (ImGui::DragFloat("Scale", &meshScale, 0.001f, 0.0001f, 1000.0f, "%.4f")) {
						settings->scale = meshScale;
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Uniform scale factor (e.g., 0.001 for mm to m)");
					}

					ImGui::Spacing();

					// Position
					ImGui::Text("Position Offset:");
					float position[3] = { settings->positionX, settings->positionY, settings->positionZ };
					if (ImGui::DragFloat3("Position", position, 0.1f)) {
						settings->positionX = position[0];
						settings->positionY = position[1];
						settings->positionZ = position[2];
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Position offset in mesh units (X, Y, Z)");
					}

					ImGui::Spacing();

					// Rotation
					ImGui::Text("Rotation (Degrees):");
					float rotation[3] = { settings->rotationX, settings->rotationY, settings->rotationZ };
					if (ImGui::DragFloat3("Rotation", rotation, 1.0f, -180.0f, 180.0f)) {
						settings->rotationX = rotation[0];
						settings->rotationY = rotation[1];
						settings->rotationZ = rotation[2];
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Rotation in degrees (X=Pitch, Y=Yaw, Z=Roll)");
					}



					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// ========== VERTEX DATA SECTION ==========
					ImGui::SeparatorText("Vertex Data");

					if (ImGui::Checkbox("Include Position", &settings->includePos)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Normals", &settings->includeNormals)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Colors", &settings->includeColors)) {
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Texture Coordinates", &settings->includeTexCoords)) {
						descriptorEditor.MarkModified();
					}

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// ========== OUTPUT SETTINGS SECTION ==========
					ImGui::SeparatorText("Output Settings");

					char formatBuffer[256];
					strncpy_s(formatBuffer, sizeof(formatBuffer), settings->outputFormat.c_str(), _TRUNCATE);
					if (ImGui::InputText("Output Format", formatBuffer, sizeof(formatBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
						settings->outputFormat = std::string(formatBuffer);
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

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// ========== OPTIMIZATION SECTION ==========
					ImGui::SeparatorText("Optimization");

					if (ImGui::Checkbox("Optimize Vertices", &settings->optimizeVertices)) {
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Remove duplicate vertices and optimize for cache");
					}

					if (ImGui::Checkbox("Generate Normals", &settings->generateNormals)) {
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Generate normals if missing");
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
				//std::cout << "*** [GIZMO] Reset operation after camera toggle ***" << std::endl;
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
						//std::cout << "*** [GIZMO] Switched to MOVE mode ***" << std::endl;
					}

					if (ImGui::MenuItem("Rotate", "E"))
					{
						m_Operation = ImGuizmo::ROTATE;
						//std::cout << "*** [GIZMO] Switched to ROTATE mode ***" << std::endl;
					}

					if (ImGui::MenuItem("Scale", "R"))
					{
						m_Operation = ImGuizmo::SCALE;
						//std::cout << "*** [GIZMO] Switched to SCALE mode ***" << std::endl;
					}

					ImGui::Separator();
					ImGui::EndPopup();
				}

				// Only handle keyboard shortcuts when viewport is focused
				if (ImGui::IsWindowFocused()) {
					if (ImGui::IsKeyPressed(ImGuiKey_W)) {
						m_Operation = ImGuizmo::TRANSLATE;
						//std::cout << "*** [GIZMO] Switched to MOVE mode (Keyboard W) ***" << std::endl;
					}
					if (ImGui::IsKeyPressed(ImGuiKey_E)) {
						m_Operation = ImGuizmo::ROTATE;
						//std::cout << "*** [GIZMO] Switched to ROTATE mode (Keyboard E) ***" << std::endl;
					}
					if (ImGui::IsKeyPressed(ImGuiKey_R)) {
						m_Operation = ImGuizmo::SCALE;
						//std::cout << "*** [GIZMO] Switched to SCALE mode (Keyboard R) ***" << std::endl;
					}
					if (ImGui::IsKeyPressed(ImGuiKey_Q)) {
						m_Operation = static_cast<ImGuizmo::OPERATION>(-1);
						//std::cout << "*** [GIZMO] Disabled manipulation (Keyboard Q) ***" << std::endl;
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
							//LOG_INFO("[DEBUG] m_PickedID = {}", m_PickedID);
							m_SelectedEntity = Entity{ (entt::entity)m_PickedID, &m_Scene->GetRegistry() };
							ViewportPanelHelper::ViewPortClickAndTeleport(m_PickedID, m_LastClickedID, m_LastClickedTime, m_DoublePickedTime, m_SelectedEntity, m_Renderer);
						}
						else
						{
							m_SelectedEntity = Entity{};
							//LOG_INFO("Deselected entity.");
							m_LastClickedTime = 0.0;
							m_LastClickedID = 0xFFFFFFFFu;
						}
					}
					/*else
					{
						LOG_INFO("Selection blocked - was interacting with gizmo last frame");
					}*/
				}

				// Update gizmo state for next frame
				m_WasUsingGizmoLastFrame = isUsingGizmoThisFrame;
				m_WasOverGizmoLastFrame = isOverGizmoThisFrame;
			}
		}

		ViewportPanelHelper::CameraControl(m_Renderer);

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
						//LOG_DEBUG("//// m_Scene->GetName() in open file is ", currFileName);
						isPrefabEditor = false;

						selectedFolder = getAssetFilePath("Sources/Scenes");
						selectedResourcesIndex = -1;

						auto assetsList = getAssetsInFolder(selectedFolder);
						for (size_t i = 0; i < assetsList.size(); ++i)
						{
							if (assetsList[i].fullPath == currScenePath)
							{
								selectedResourcesIndex = static_cast<int>(i);
								break;
							}
						}
						LOG_DEBUG("Asset browser state:");
						LOG_DEBUG("  selectedFolder: {}", selectedFolder);
						LOG_DEBUG("  raw_asset: {}", raw_asset);
						LOG_DEBUG("  selectedType: {}", static_cast<int>(selectedType));
						LOG_DEBUG("  selectedResourcesIndex: {}", selectedResourcesIndex);
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
						m_Scene->SetName(saveAsDefaultSceneName);
						m_Scene->SaveToFile(convertAssetPathToRootResources(defaultNewScenePath));
						currScenePath = defaultNewScenePath; // update current scene path
						LOG_DEBUG("m_Scene->SetName(saveAsDefaultSceneName): ", saveAsDefaultSceneName);

						if (isPrefabEditor)
						{
							isPrefabEditor = false;
							currPrefabPath.clear();
							//LOG_INFO("Exited prefab editor mode after saving scene");
						}

						//LoadAllPrefabsIntoRegistry();

						m_Scene->GetRegistry().clear();
						m_Scene->LoadFromFile(defaultNewScenePath);
						currFileName = m_Scene->GetName();
						// update asset browser

						selectedFolder = getAssetFilePath("Sources/Scenes");

						selectedResourcesIndex = -1;
						auto assetsList = getAssetsInFolder(selectedFolder);
						for (size_t i = 0; i < assetsList.size(); ++i)
						{
							if (assetsList[i].fullPath == currScenePath)
							{
								selectedResourcesIndex = static_cast<int>(i);
								break;
							}
						}
						raw_asset = false;
						selectedType = ResourceType::UNKNOWN;

						// Clear current selection
						m_SelectedEntity = Entity{};
						m_PickedID = 0xFFFFFFFFu;
						m_Operation = static_cast<ImGuizmo::OPERATION>(-1);

						memset(saveAsDefaultSceneName, 0, sizeof(saveAsDefaultSceneName));
						saveAsPanel = false; // to close pop up
						isNewScene = false;
						ImGui::CloseCurrentPopup();

						LOG_DEBUG("Setting asset browser selection:");
						LOG_DEBUG("  selectedFolder: {}", selectedFolder);
						LOG_DEBUG("  raw_asset: {}", raw_asset);
						LOG_DEBUG("  selectedType: {}", static_cast<int>(selectedType));
						LOG_DEBUG("  selectedResourcesIndex: {}", selectedResourcesIndex);

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

		if (!entity || !m_Scene || !entity.HasComponent<TransformComponent>())
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
			auto getScriptFiles = getAssetsInFolder("Scripts\\Game");

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
	void Editor::displayPrefabComp()
	{
		if (m_SelectedEntity.HasComponent<PrefabComponent>())
		{
			bool hasParent = true;
			if (m_SelectedEntity.HasComponent<TransformComponent>()) {
				auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
				hasParent = transform.Parent == u32_max ? false : true;
			}

			if (ImGui::CollapsingHeader("Prefab", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

				ImGui::Text("Prefab GUID: %llu", prefabComp.PrefabGUID.m_Value);

				if (!isPrefabEditor)
				{
					if (hasParent) {
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

						//ApplyPrefabOverrides(m_SelectedEntity);

					}

					if (hasParent) {
						ImGui::EndDisabled();
					}

				}
				/*else
				{
					CheckAndUpdatePrefabInstances();
				}*/

			}
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
						// Perspective � show FOV
						float fov = camComp.FOV;
						if (ImGui::DragFloat("FOV", &fov, 0.1f, 10.0f, 120.0f))
						{
							camComp.SetFOV(fov);                    // rebuilds projection
						}
					}
					else
					{
						// Orthographic � edit height only (Size.y)
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

					// For M3
					/*
					int depth = static_cast<int>(camComp.Depth);
					if (ImGui::DragInt("Depth", &depth, 1, 0, 100))
					{
						camComp.Depth = static_cast<u32>(depth);
					}
					*/
				}
			}

			// ---------------------- Remove Camera Comp ---------------------------
			if (removeCameraComp)
			{
				m_SelectedEntity.RemoveComponent<CameraComponent>();
			}

		}
	}

	void Editor::displayRigidBodyComp(ImVec2& buttonSize)
	{
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

				float linearDamping = rigidBody.LinearDamping;
				if (ImGui::DragFloat("LinearDamping", &linearDamping))
				{
					rigidBody.LinearDamping = linearDamping;
				}

				float restitution = rigidBody.Restitution;
				if (ImGui::DragFloat("Restitution", &restitution))
				{
					rigidBody.Restitution = restitution;
				}


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

	}

	void Editor::displayMeshRendererComp(ImVec2& buttonSize)
	{
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
					if (meshType == 0 || meshType == 1 || meshType == 2) {
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
	}

	void Editor::displayAudioComp(ImVec2& buttonSize)
	{
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

				if (audio.AudioFilePath.empty()) {
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // Red
					ImGui::Text("No audio file loaded. Please select and audio file below.");
					ImGui::PopStyleColor();
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); // Green
					ImGui::Text("Audio Filename: %s", audio.AudioFilePath.c_str());
					ImGui::PopStyleColor();
				}

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

				if (audio.AudioFilePath.empty()) {
					ImGui::SameLine();
					if (ImGui::Button("Load File")) {
						audio.SetAudioFile(audioAssetNames[currentIndex]);
					}
				}

				if (audio.AudioFilePath.empty()) {
					ImGui::BeginDisabled();
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

				ImGui::Separator();

				float dopplerLevel = audio.DopplerLevel;
				if (ImGui::SliderFloat("Doppler", &dopplerLevel, 0.f, 5.f)) {
					audio.SetDopplerLevel(dopplerLevel);
				}

				ImGui::Text("RollOff Mode:");
				AudioRolloffMode mode = audio.RolloffMode;

				if (ImGui::RadioButton("INVERSE", mode == AudioRolloffMode::INVERSE)) {
					audio.SetRolloffMode(AudioRolloffMode::INVERSE);
				}
				if (ImGui::RadioButton("LINEAR", mode == AudioRolloffMode::LINEAR)) {
					audio.SetRolloffMode(AudioRolloffMode::LINEAR);
				}
				if (ImGui::RadioButton("LINEARSQUARE", mode == AudioRolloffMode::LINEARSQUARE)) {
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
				if (ImGui::SliderFloat("Pan", &pan, -1.f, 1.f)) {
					audio.SetPan(pan);
				}

				ImGui::EndDisabled();

				if (audio.AudioFilePath.empty()) {
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

	void Editor::displayReverbZoneComp(ImVec2& buttonSize)
	{
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

						if (isSelected) {
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
	}

	void Editor::displayListenerComp(ImVec2& buttonSize)
	{
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
	}

	void Editor::displayBTComp(ImVec2& buttonSize)
	{
		if (m_SelectedEntity.HasComponent<BehaviourTreeComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openBTComponent = ImGui::CollapsingHeader("Behaviour Tree Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeBTComponent = false;

			ImGui::NextColumn();

			if (ImGui::Button("...###BTBtn", buttonSize))
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
	}

	void Editor::displayParticleComp(ImVec2& buttonSize)
	{
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
	}

	void Editor::displayScriptComp(ImVec2 &buttonSize) {
		if(m_SelectedEntity.HasComponent<ScriptComponent>()) {
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool openScriptComp = ImGui::CollapsingHeader("Script Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeScriptComp = false;
			auto &scriptComp = m_SelectedEntity.GetComponent<ScriptComponent>();
			std::string scriptPath = getRepository() + "\\Scripts\\Game";
			auto scriptFiles = getAssetsInFolder(scriptPath);
			ImGui::NextColumn();
			if(ImGui::Button("...##ScriptBtn", buttonSize))
				ImGui::OpenPopup("ScriptPopUp");
			if(ImGui::BeginPopup("ScriptPopUp")) {
				if(ImGui::MenuItem("Remove Component"))
					removeScriptComp = true;
				ImGui::EndPopup();
			}
			ImGui::Columns(1);
			if(openScriptComp) {
				ImGui::Text("Instance: %s", scriptComp.ScriptInstance ? "Active" : "None");
				ImGui::Text("Started: %s", scriptComp.Started ? "Yes" : "No");

				if(!scriptFiles.empty()) {
					if(ImGui::BeginCombo("Select Script", scriptComp.ScriptClassName.empty() ? "None" : scriptComp.ScriptClassName.substr(scriptComp.ScriptClassName.find_last_of('.') + 1).c_str())) {
						for(const auto &asset : scriptFiles) {
							std::string className = asset.name;
							if(className.ends_with(".cs"))
								className = className.substr(0, className.size() - 3); // remove extension
							std::string selectedClassName = "Game." + className;
							bool isSelected = scriptComp.ScriptClassName == selectedClassName;
							if(ImGui::Selectable(className.c_str(), isSelected)) {
								// Destroy previous script instance if exists
								if(scriptComp.ScriptInstance) {
									MonoScriptEngine::GetInstance().DestroyScriptInstance((MonoObject *)scriptComp.ScriptInstance);
									scriptComp.ScriptInstance = nullptr;
									scriptComp.Started = false;
								}

								// Assign the new script class name
								scriptComp.ScriptClassName = selectedClassName;

								// DON'T create instance in editor - let ScriptSystem handle it!
								// Just setting the class name is enough
								// The instance will be created and EntityID will be bound when you play
							}
							if(isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					// ===== NEW: DISPLAY SERIALIZED FIELDS =====
					ImGui::Separator();
					ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Serialized Fields:");
					ImGui::Separator();
					// THIS IS THE KEY LINE - renders all [SerializeField] fields
					if(scriptComp.ScriptInstance) {
						RenderSerializedFieldsInImGui((MonoObject *)scriptComp.ScriptInstance);
					}
					else {
						ImGui::TextDisabled("(No script instance - will be created when playing)");
					}
					if(ImGui::Button("Save Script Fields To JSON")) {
						if(scriptComp.ScriptInstance)
							SerializeScriptToDiskRapidJSON((MonoObject *)scriptComp.ScriptInstance, "SavedScriptFields.json");
					}
					// Similarly, add a load button to test deserialization:
					ImGui::SameLine();
					if(ImGui::Button("Load Script Fields From JSON")) {
						if(scriptComp.ScriptInstance)
							DeserializeScriptFromDiskRapidJSON((MonoObject *)scriptComp.ScriptInstance, "SavedScriptFields.json");
					}
					// ===== END NEW SERIALIZED FIELDS =====
				}
			}
			// Remove Script Component
			if(removeScriptComp)
				m_SelectedEntity.RemoveComponent<ScriptComponent>();
		}
	}

	void Editor::displayAnimatorComp(ImVec2& buttonSize)
	{
		if (m_SelectedEntity.HasComponent<AnimatorComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openAnimatorComponent = ImGui::CollapsingHeader("Animator Component", ImGuiTreeNodeFlags_DefaultOpen);
			bool removeAnimator = false;

			// Column 2: "..." button to remove component
			ImGui::NextColumn();

			if (ImGui::Button("... ###AnimatorBtn", buttonSize))
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

				// -----------------------------------------------------------------
				// Controller selection combo (from m_AnimatorControllerStorage)
				// -----------------------------------------------------------------
				std::vector<u32> controllerHandles;
				controllerHandles.reserve(m_AnimatorControllerStorage.size());

				std::vector<std::string> controllerLabels;
				controllerLabels.reserve(m_AnimatorControllerStorage.size());

				// Build a simple list of (handle, "Name (id)") pairs
				for (const auto& kv : m_AnimatorControllerStorage)
				{
					u32 handle = kv.first;
					const AnimatorController& ctrl = kv.second;

					controllerHandles.push_back(handle);

					std::string label;
					if (!ctrl.name.empty())
						label = ctrl.name + " (" + std::to_string(handle) + ")";
					else
						label = "Controller " + std::to_string(handle);

					controllerLabels.push_back(label);
				}

				// Find the currently assigned controller in the list
				int currentIndex = -1;
				for (int i = 0; i < static_cast<int>(controllerHandles.size()); ++i)
				{
					if (controllerHandles[i] == animator.controller)
					{
						currentIndex = i;
						break;
					}
				}

				const char* previewLabel = "(None)";
				if (currentIndex >= 0 && currentIndex < static_cast<int>(controllerLabels.size()))
					previewLabel = controllerLabels[currentIndex].c_str();

				ImGui::Text("Controller:");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200.0f);
				if (ImGui::BeginCombo("##AnimatorControllerCombo", previewLabel))
				{
					for (int i = 0; i < static_cast<int>(controllerHandles.size()); ++i)
					{
						bool isSelected = (i == currentIndex);
						const char* itemLabel = controllerLabels[i].c_str();

						if (ImGui::Selectable(itemLabel, isSelected))
						{
							// Assign new controller to this AnimatorComponent
							animator.controller = controllerHandles[i];
							animator.currentClipIndex = 0;
							animator.currentTime = 0.0f;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				// Debug/info line for handle
				ImGui::Text("Controller Handle: %u", animator.controller);

				ImGui::SeparatorText("Playback");

				// Basic animator state controls
				ImGui::Checkbox("Playing", &animator.playing);
				ImGui::SameLine();
				ImGui::Checkbox("Respect Clip Loop", &animator.respectClipLoop);

				ImGui::DragFloat("Playback Speed", &animator.playbackSpeed, 0.01f, -5.0f, 5.0f);

				// We keep these for debugging / manual scrubbing
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

	}

	void Editor::addComponents()
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
	}

	void Editor::RevertSelectedEntityToPrefab()
	{
		if (!m_SelectedEntity ||
			!m_SelectedEntity.HasComponent<PrefabComponent>() ||
			m_SelectedEntity.GetComponent<TransformComponent>().Parent != u32_max) // If entity is a sub-entity; DO NOT REVERT BY ITSELF
			return;

		auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

		// Load prefab data
		auto prefab = PrefabRegistry::Get().GetPrefab(prefabComp.PrefabGUID);

		if (!prefab)
		{
			LOG_ERROR("Revert failed: Prefab not found!");
			return;
		}

		auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
		if (transform.Children.empty()) // For main entities with no children
		{
			// Remove the modified instance
			Entity old = m_SelectedEntity;
			m_Scene->DestroyEntity(old);

			// Create fresh prefab instance no overrides applied
			Entity newEntity = PrefabInstantiator::InstantiateEntityPrefab(
				m_Scene,
				prefab->GetGUID()
			);


			// Clear modifications on the new PrefabComponent (optional safety)
			if (newEntity.HasComponent<PrefabComponent>())
				newEntity.GetComponent<PrefabComponent>().ClearModifications();

			// Select it in the editor
			m_SelectedEntity = newEntity;
		}
		else { // For main entities with children

			Entity old = m_SelectedEntity;
			EditorHierarchyHelper::FillEntitiesWithChildrenToDelete(old, m_Scene, m_SelectedEntity, m_PickedID);
			EditorHierarchyHelper::DeleteEntityParentAndChildren(m_Scene);

			// Create fresh prefab instance no overrides applied
			Entity newEntity = PrefabInstantiator::InstantiateScenePrefab(
				m_Scene,
				prefab->GetGUID()
			);

			// Clear modifications on the new PrefabComponent (optional safety)
			if (newEntity.HasComponent<PrefabComponent>())
				newEntity.GetComponent<PrefabComponent>().ClearModifications();

			// Select it in the editor
			m_SelectedEntity = newEntity;
		}


		LOG_INFO("Prefab instance reverted to original prefab state.");
	}

	void Editor::displayLightComp(ImVec2& buttonSize)
	{
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

#if 1 // no use for now
	void Editor::LoadAllPrefabsIntoRegistry()
	{
		auto& registry = Engine::PrefabRegistry::Get();
		std::string prefabDir = getAssetFilePath("Sources/Prefabs/");

		if (!std::filesystem::exists(prefabDir))
		{
			LOG_WARNING("Prefab directory does not exist: {}", prefabDir);
			return;
		}

		for (auto& entry : std::filesystem::directory_iterator(prefabDir))
		{
			if (entry.path().extension() == ".prefab")
			{
				auto prefab = Engine::PrefabSerializer::LoadPrefabFromFile(entry.path().string());
				if (prefab)
				{
					registry.RegisterPrefab(prefab);
				}
				else
				{
					LOG_WARNING("Failed to load prefab: {}", entry.path().string());
				}
			}
		}
	}

#endif
	void Editor::UpdateAllInstancesOfPrefab(xresource::instance_guid prefabGUID, Entity modifiedEntity)
	{
		auto it = m_PrefabEntities.find(prefabGUID);
		if (it == m_PrefabEntities.end()) return;

		std::vector<Entity> updatedEntities;

#if 1 // original code
		for (Entity e : it->second)
		{
			// Skip the entity that was just modified
			if (e == modifiedEntity)
			{
				updatedEntities.push_back(e);
				continue;
			}

			// Destroy old entity
			m_Scene->DestroyEntity(e);

			// Reload prefab from registry
			auto prefab = PrefabRegistry::Get().GetPrefab(prefabGUID);
			if (!prefab) continue;

			Entity newEntity;
			// Instantiate a fresh prefab entity
			if (prefab->GetType() == PrefabType::Entity)
			{
				newEntity = PrefabInstantiator::InstantiateEntityPrefab(m_Scene, prefab->GetGUID());

			}
			else
			{
				newEntity = PrefabInstantiator::InstantiateScenePrefab(m_Scene, prefab->GetGUID());
			}

			if (newEntity.HasComponent<PrefabComponent>())
				newEntity.GetComponent<PrefabComponent>().ClearModifications();

			updatedEntities.push_back(newEntity);
		}

		// Replace the entity list in the map with updated entities
		m_PrefabEntities[prefabGUID] = updatedEntities;

#endif


	}

	void Editor::CheckAndUpdatePrefabInstances()
	{
		if (!m_Scene || m_UpdatedPrefabsThisSession.empty()) return;

		LOG_DEBUG("===== Start of CheckAndUpdatePrefabInstances ====");

		LOG_DEBUG("Auto-updating instances for", m_UpdatedPrefabsThisSession.size(), "modified prefabs");

		std::vector<xresource::instance_guid> prefabsToUpdate;

		for (auto prefabGUID : m_UpdatedPrefabsThisSession)
		{
			if (m_SceneUpdateHistory[currScenePath].count(prefabGUID) == 0)
			{
				prefabsToUpdate.push_back(prefabGUID);
			}
		}

		for (auto prefabGUID : prefabsToUpdate)
		{
			auto prefab = PrefabRegistry::Get().GetPrefab(prefabGUID);
			if (!prefab) continue;

			LOG_DEBUG("Updating instances for prefab: ", prefab->GetName());

			if (prefab->GetType() == PrefabType::Scene)
			{
				UpdateScenePrefabInstances(prefabGUID, prefab);
			}
			else
			{
				UpdateEntityPrefabInstances(prefabGUID, prefab);
			}

			// Mark this prefab as updated for current scene
			m_SceneUpdateHistory[currScenePath].insert(prefabGUID);
		}
		//std::string prefabDir = getAssetFilePath("Sources/Prefabs/");
		//if (!std::filesystem::exists(prefabDir)) return;

		//bool anyPrefabUpdated = false;
		//std::vector<std::pair<xresource::instance_guid, PrefabType>> updatedPrefabs;


		//// Check all prefab files for modifications
		//for (auto& entry : std::filesystem::directory_iterator(prefabDir))
		//{
		//	if (entry.path().extension() != ".prefab") continue;

		//	auto lastWriteTime = std::filesystem::last_write_time(entry.path());
		//	auto fileTime = std::chrono::duration_cast<std::chrono::seconds>(
		//		lastWriteTime.time_since_epoch()).count();

		//	// Load prefab to get its GUID
		//	auto prefab = PrefabSerializer::LoadPrefabFromFile(entry.path().string());
		//	if (!prefab) continue;

		//	xresource::instance_guid guid = prefab->GetGUID();
		//	PrefabType type = prefab->GetType();


		//	LOG_DEBUG("Checking prefab: ", entry.path().filename().string());
		//	LOG_DEBUG("  - GUID: ", guid.m_Value);
		//	LOG_DEBUG("  - Type: ", (type == PrefabType::Scene ? "Scene" : "Entity"));


		//	// Check if this is first time seeing this prefab or if it's been modified
		//	auto it = m_PrefabLastModifiedTimes.find(guid);
		//	if (it == m_PrefabLastModifiedTimes.end())
		//	{
		//		// First time - just record the time (silent)
		//		m_PrefabLastModifiedTimes[guid] = fileTime;
		//		LOG_DEBUG("  - First time seeing this prefab");
		//	}
		//	else if (it->second < fileTime)
		//	{
		//		// Prefab was modified! - ONLY SHOW DEBUG WHEN MODIFIED
		//		m_PrefabLastModifiedTimes[guid] = fileTime;
		//		updatedPrefabs.push_back({ guid, type });
		//		anyPrefabUpdated = true;

		//		auto freshPrefab = PrefabSerializer::LoadPrefabFromFile(entry.path().string());
		//		if (freshPrefab)
		//		{
		//			PrefabRegistry::Get().UpdatePrefab(freshPrefab);
		//			LOG_INFO("Reloaded and updated prefab: ", entry.path().filename().string());
		//		}
		//	}
		//	else
		//	{
		//		LOG_DEBUG("  - No modification detected");
		//	}
		//}

		//if (m_Scene)
		//{
		//	auto view = m_Scene->GetRegistry().view<PrefabComponent>();
		//	LOG_DEBUG("Scene contains {} entities with PrefabComponent", view.size());

		//	for (auto entityHandle : view)
		//	{
		//		Entity entity(entityHandle, &m_Scene->GetRegistry());
		//		if (entity.HasComponent<PrefabComponent>())
		//		{
		//			auto& prefabComp = entity.GetComponent<PrefabComponent>();
		//			LOG_DEBUG("Entity ", static_cast<uint32_t>(entityHandle), " has PrefabGUID: ", prefabComp.PrefabGUID.m_Value);
		//		}
		//	}
		//}
		//LOG_DEBUG("anyPrefabUpdated: ", anyPrefabUpdated, " | isPrefabEditor: ", isPrefabEditor);


		//// Update all instances of modified prefabs
		//if (anyPrefabUpdated && !isPrefabEditor)
		//{
		//	LOG_DEBUG("is not PrefabEditor");
		//	for (auto& [prefabGUID, prefabType] : updatedPrefabs)
		//	{
		//		
		//		LOG_DEBUG("Updating prefab instances for scene: ", currScenePath);
		//		if (prefabType == PrefabType::Scene)
		//		{
		//			LOG_DEBUG("PrefabType::Scene");
		//			UpdateScenePrefabInstances(prefabGUID, PrefabRegistry::Get().GetPrefab(prefabGUID));
		//		}
		//		else
		//		{
		//			LOG_DEBUG("PrefabType::Entity");
		//			UpdateEntityPrefabInstances(prefabGUID, PrefabRegistry::Get().GetPrefab(prefabGUID));
		//		}
		//	}
		//}
		//else
		//{
		//	if (!anyPrefabUpdated)
		//		LOG_DEBUG(" Condition failed - anyPrefabUpdated is false");
		//	if (isPrefabEditor)
		//		LOG_DEBUG(" Condition failed - isPrefabEditor is true");
		//}
		//LOG_DEBUG("===== End of CheckAndUpdatePrefabInstances ====");
	}

	//	void Editor::UpdateAllPrefabInstancesInScene(xresource::instance_guid prefabGUID)
	//	{
	//		if (!m_Scene) return;
	//
	//		auto prefab = PrefabRegistry::Get().GetPrefab(prefabGUID);
	//		if (!prefab)
	//		{
	//			return;
	//		}
	//
	//#if 1
	//		// Determine if it's a scene prefab or entity prefab
	//		bool isScenePrefab = (prefab->GetType() == PrefabType::Scene);
	//
	//		if (isScenePrefab)
	//		{
	//			UpdateScenePrefabInstances(prefabGUID, prefab);
	//		}
	//		else
	//		{
	//			UpdateEntityPrefabInstances(prefabGUID, prefab);
	//		}
	//#endif
	//
	//
	//	}

	void Editor::DrawCurveLegendRow(const char* label,
		const char* c0Label, ImU32 c0,
		const char* c1Label, ImU32 c1,
		const char* c2Label, ImU32 c2)
	{
		ImGui::TextUnformatted(label);

		auto drawEntry = [](const char* lbl, ImU32 col)
			{
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(10.0f, 0.0f)); // small gap
				ImGui::SameLine();

				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 size(12.0f, 12.0f);
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col, 2.0f);
				dl->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(0, 0, 0, 255), 2.0f);

				ImGui::Dummy(size);
				ImGui::SameLine();
				ImGui::TextUnformatted(lbl);
			};

		drawEntry(c0Label, c0);
		drawEntry(c1Label, c1);
		drawEntry(c2Label, c2);
	}

	void Editor::displayHDRSettingsPanel()
	{
		ImGui::Begin("HDR Settings");

		// Exposure control
		if (ImGui::SliderFloat("Exposure", &m_Renderer->m_exposure, 0.1f, 5.0f, "%.2f"))
		{
			// Exposure value changed
		}

		// Optional: Add a reset button
		if (ImGui::Button("Reset to Default"))
		{
			m_Renderer->m_exposure = 1.0f;
		}

		// Optional: Add tooltip
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Reset exposure to default value (1.0)");
		}


		ImGui::Separator();

		// ======================
		// Bloom toggle
		// ======================
		auto& bloomToggle = m_Renderer->getBloomToggle();
		auto& bloomStrength = m_Renderer->getBloomStrength();
		auto& bloomFilter = m_Renderer->getBloomFilterRadius();
		ImGui::Checkbox("Enable Bloom", &bloomToggle);

		// ======================
		// Bloom strength
		// ======================
		// Slider
		if (ImGui::SliderFloat("Bloom Strength",
			&bloomStrength,
			0.0f, 1.0f, "%.3f"))
		{
			// Bloom strength changed
		}

		// Input box on the same line
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::InputFloat("##BloomStrengthInput",
			&bloomStrength,
			0.0f, 0.0f, "%.3f");

		// Clamp manually if you want to enforce range:
		bloomStrength = std::clamp(bloomStrength, 0.0f, 1.0f);

		// ======================
		// Bloom filter radius
		// ======================
		// Slider (typical useful range is small around 0.001 to 0.02)
		if (ImGui::SliderFloat("Filter Radius",
			&bloomFilter,
			0.001f, 0.02f, "%.4f"))
		{
			// Filter radius changed
		}

		// Input box on the same line
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::InputFloat("##FilterRadiusInput",
			&bloomFilter,
			0.0f, 0.0f, "%.4f");

		// Clamp to avoid nonsense values 
		bloomFilter = std::clamp(bloomFilter, 0.0001f, 0.05f);

		// Reset button for bloom settings
		if (ImGui::Button("Reset to Default##Bloom"))
		{
			bloomStrength = 0.04f;
			bloomFilter = 0.005f;
		}

		// Tooltip for bloom settings
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Reset bloom strength (0.04) and filter radius (0.005) to default values");
		}

		ImGui::End();
	}

	void Editor::ApplyPrefabOverrides(Entity entity)
	{
		//ApplyOverrideButtonTriggle = true;
		LOG_DEBUG(" ========== Start Apply Override =========");
		auto& prefabComp = entity.GetComponent<PrefabComponent>();
		auto prefab = PrefabRegistry::Get().GetPrefab(prefabComp.PrefabGUID);

		if (!prefab)
		{
			LOG_ERROR("Prefab not found");
			return;
		}

#if 1 // to try to override with parent and children


		bool hasChildren = false;
		if (entity.HasComponent<TransformComponent>())
		{
			const auto& transform = entity.GetComponent<TransformComponent>();

			if (!transform.Children.empty()) {
				hasChildren = true;
			}
		}

		if (hasChildren)
		{
			std::vector<Entity> allEntities;
			allEntities.reserve(64); // Reserve reasonable size
			allEntities.push_back(entity);

			// OPTIMIZATION 3: Use iterative collection instead of recursive
			CollectChildEntitiesIterative(entity, allEntities);

			LOG_DEBUG("Collecting ", allEntities.size(), " entities for scene prefab");

			// OPTIMIZATION 4: Serialize only once
			std::string sceneJson = PrefabSerializer::SerializeEntities(allEntities, m_Scene->GetRegistry());

			if (sceneJson.empty())
			{
				LOG_ERROR("Failed to serialize entities to JSON");
				return;
			}

			// OPTIMIZATION 5: Update prefab in single operation
			prefab->SetSceneData(sceneJson);
			prefab->SetType(PrefabType::Scene);

			// OPTIMIZATION 6: Save to file
			if (!PrefabSerializer::SavePrefabToFile(*prefab, prefab->GetSourcePath()))
			{
				LOG_ERROR("Failed to save prefab file: ", prefab->GetSourcePath());
				return;
			}

			// OPTIMIZATION 7: Clear modifications in batch
			for (Entity ent : allEntities)
			{
				if (ent.HasComponent<PrefabComponent>())
				{
					ent.GetComponent<PrefabComponent>().ClearModifications();
				}
			}

			CheckAndUpdatePrefabInstances();

			LOG_INFO("Successfully applied overrides to scene prefab: ", prefab->GetSourcePath());

		}
		else
		{
			// Handle Entity Prefab (single entity)
			// Handle Scene Prefab (with hierarchy)
			LOG_DEBUG(" ========== Start Apply Override =========");
			std::string updatedJson = PrefabSerializer::SerializeEntity(entity, {});
			prefab->SetEntityData(updatedJson);
			PrefabSerializer::SavePrefabToFile(*prefab, prefab->GetSourcePath());

			entity.GetComponent<PrefabComponent>().ClearModifications();
			CheckAndUpdatePrefabInstances();
			LOG_INFO("Applied overrides to entity prefab: ", prefab->GetSourcePath());

		}

#endif
#if 0// original code working for prefab entity without children only
		std::string updatedJson = PrefabSerializer::SerializeEntity(entity, {});
		prefab->SetEntityData(updatedJson);
		PrefabSerializer::SavePrefabToFile(*prefab, prefab->GetSourcePath());

		prefabComp.ClearModifications();
		//UpdateAllInstancesOfPrefab(prefabComp.PrefabGUID, entity);

		LOG_INFO("Applied overrides to prefab: ", prefab->GetSourcePath());
#endif
	}


	/*void Editor::CollectChildEntities(Entity parentEntity, std::vector<Entity>& outEntities)
	{
		if (!parentEntity.HasComponent<TransformComponent>())
			return;

		const auto& transform = parentEntity.GetComponent<TransformComponent>();
		auto& registry = m_Scene->GetRegistry();

		for (uint32_t childID : transform.Children)
		{
			entt::entity childHandle = static_cast<entt::entity>(childID);
			if (registry.valid(childHandle))
			{
				Entity childEntity(childHandle, &registry);
				outEntities.push_back(childEntity);
				CollectChildEntities(childEntity, outEntities);
			}
		}
	}*/

	void Editor::CollectChildEntitiesIterative(Entity parentEntity, std::vector<Entity>& outEntities)
	{
		if (!parentEntity.HasComponent<TransformComponent>())
			return;

		auto& registry = m_Scene->GetRegistry();
		std::queue<Entity> toProcess;
		std::unordered_set<uint32_t> processedIDs;

		toProcess.push(parentEntity);
		processedIDs.insert(static_cast<uint32_t>(parentEntity));

		while (!toProcess.empty())
		{
			Entity current = toProcess.front();
			toProcess.pop();

			if (!current.HasComponent<TransformComponent>())
				continue;

			const auto& transform = current.GetComponent<TransformComponent>();

			for (uint32_t childID : transform.Children)
			{
				// Avoid processing same entity twice
				if (processedIDs.count(childID) > 0)
					continue;

				entt::entity childHandle = static_cast<entt::entity>(childID);

				if (registry.valid(childHandle))
				{
					Entity childEntity(childHandle, &registry);
					outEntities.push_back(childEntity);
					processedIDs.insert(childID);
					toProcess.push(childEntity);
				}
			}
		}
	}

	void Editor::UpdateEntityPrefabInstances(xresource::instance_guid prefabGUID, std::shared_ptr<Prefab> prefab)
	{
		struct InstanceInfo {
			entt::entity handle;
			uint32_t parentID;
		};

		std::vector<InstanceInfo> instancesToUpdate;

		{
			auto view = m_Scene->GetRegistry().view<PrefabComponent>();
			for (auto entityHandle : view)
			{
				auto& prefabComp = m_Scene->GetRegistry().get<PrefabComponent>(entityHandle);

				if (prefabComp.PrefabGUID.m_Value == prefabGUID.m_Value)
				{
					if (m_SelectedEntity == Entity(entityHandle, &m_Scene->GetRegistry()) && isPrefabEditor)
						continue;

					InstanceInfo info;
					info.handle = entityHandle;
					info.parentID = u32_max;

					if (m_Scene->GetRegistry().all_of<TransformComponent>(entityHandle))
					{
						info.parentID = m_Scene->GetRegistry().get<TransformComponent>(entityHandle).Parent;
					}

					instancesToUpdate.push_back(info);
				}
			}
		}

		// Update each instance
		for (const auto& info : instancesToUpdate)
		{
			auto& registry = m_Scene->GetRegistry();

			if (!registry.valid(info.handle))
				continue;

			entt::entity handle = info.handle;
			uint32_t parentID = info.parentID;

			// Destroy and recreate
			registry.destroy(handle);

			Entity newEntity = PrefabInstantiator::InstantiateEntityPrefab(
				m_Scene,
				prefab->GetGUID(),
				handle
			);

			auto& freshRegistry = m_Scene->GetRegistry();

			if (freshRegistry.valid(handle))
			{
				// Restore parent relationship
				if (parentID != u32_max && freshRegistry.all_of<TransformComponent>(handle))
				{
					freshRegistry.get<TransformComponent>(handle).Parent = parentID;
				}

				if (freshRegistry.all_of<PrefabComponent>(handle))
				{
					freshRegistry.get<PrefabComponent>(handle).ClearModifications();
				}

				LOG_INFO("Updated entity prefab instance: {}", static_cast<uint32_t>(handle));
			}
		}
	}

	void Editor::UpdateScenePrefabInstances(xresource::instance_guid prefabGUID, std::shared_ptr<Prefab> prefab)
	{
		struct InstanceInfo {
			entt::entity rootHandle;
			std::vector<entt::entity> allHandles;
			uint32_t parentID;
			entt::entity parentHandle;
		};

		std::vector<InstanceInfo> instancesToUpdate;

		{
			auto view = m_Scene->GetRegistry().view<PrefabComponent>();
			for (auto entityHandle : view)
			{
				auto& prefabComp = m_Scene->GetRegistry().get<PrefabComponent>(entityHandle);

				if (prefabComp.PrefabGUID.m_Value == prefabGUID.m_Value)
				{
					if (m_SelectedEntity == Entity(entityHandle, &m_Scene->GetRegistry()) && isPrefabEditor)
						continue;

					InstanceInfo info;
					info.rootHandle = entityHandle;
					info.parentID = u32_max;
					info.parentHandle = entt::null;

					// Collect all entities in this hierarchy
					Entity rootEntity(entityHandle, &m_Scene->GetRegistry());
					info.allHandles.push_back(entityHandle);

					if (rootEntity.HasComponent<TransformComponent>())
					{
						auto& transform = rootEntity.GetComponent<TransformComponent>();

						info.parentID = transform.Parent;
						if (transform.Parent != u32_max)
						{
							info.parentHandle = static_cast<entt::entity>(transform.Parent);
						}
						CollectChildHandles(rootEntity, info.allHandles);
					}

					instancesToUpdate.push_back(info);
				}
			}
		}

		// Update each scene prefab instance
		for (const auto& info : instancesToUpdate)
		{
			auto& registry = m_Scene->GetRegistry();

			if (!registry.valid(info.rootHandle))
				continue;
			bool parentExists = registry.valid(info.parentHandle);


			//uint32_t rootParentID = info.parentID;

			// Destroy all entities in this instance
			for (entt::entity handle : info.allHandles)
			{
				if (registry.valid(handle))
				{
					registry.destroy(handle);
				}
			}


			// Recreate the entire scene prefab instance
			Entity newRootEntity = PrefabInstantiator::InstantiateScenePrefab(
				m_Scene,
				prefab->GetGUID()
			);



			auto& freshRegistry = m_Scene->GetRegistry();
			entt::entity newRootHandle = static_cast<entt::entity>(newRootEntity);

			if (freshRegistry.valid(newRootHandle))
			{
				// Restore parent relationship if the root was a child of something outside the prefab
				if (info.parentID != u32_max && freshRegistry.all_of<TransformComponent>(newRootHandle))
				{
					auto& newRootTransform = freshRegistry.get<TransformComponent>(newRootHandle);
					newRootTransform.Parent = info.parentID;
					Entity parentEntity(info.parentHandle, &freshRegistry);
					//freshRegistry.get<TransformComponent>(newRootHandle).Parent = rootParentID;
					if (parentEntity && parentEntity.HasComponent<TransformComponent>())
					{
						auto& parentTransform = parentEntity.GetComponent<TransformComponent>();
						// Remove old child reference (if any)
						parentTransform.Children.erase(
							std::remove(parentTransform.Children.begin(), parentTransform.Children.end(),
								static_cast<uint32_t>(info.rootHandle)),
							parentTransform.Children.end()
						);
						// Add new child reference
						parentTransform.Children.push_back(static_cast<uint32_t>(newRootHandle));
					}
				}

				LOG_INFO("Updated scene prefab instance");
			}
		}
	}

	void Editor::CollectChildHandles(Entity parentEntity, std::vector<entt::entity>& outHandles)
	{
		if (!parentEntity.HasComponent<TransformComponent>())
			return;

		const auto& transform = parentEntity.GetComponent<TransformComponent>();
		auto& registry = m_Scene->GetRegistry();

		for (uint32_t childID : transform.Children)
		{
			entt::entity childHandle = static_cast<entt::entity>(childID);
			if (registry.valid(childHandle))
			{
				outHandles.push_back(childHandle);
				Entity childEntity(childHandle, &registry);
				CollectChildHandles(childEntity, outHandles);
			}
		}
	}


} // end of namespace Engine

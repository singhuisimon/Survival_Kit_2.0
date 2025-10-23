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
#include "../Serialization/SceneSerializer.h"

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

		m_Initialized = true;

	}

	void Editor::OnUpdate(Timestep ts)
	{
		//Start the ImGui frame
		if (!m_Initialized) return;

		// Start ImGui Frame
		StartImguiFrame();

		// Create the dockspace
		/*ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", nullptr, window_flags);
		ImGui::PopStyleVar();

		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);*/

		// Alternate Docking Function
		//ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		displayTopMenu();

		// Panel Logic
		displayPropertiesPanel();

		displayHierarchyPanel();

		displayAssetsBrowserPanel();

		displayPerformanceProfilePanel();

		RenderEditor();

		static float elapsedTime = 0.0f;
		elapsedTime += ts;
	}

	void Editor::displayTopMenu()
	{
		// To start top menu
		if (ImGui::BeginMainMenuBar())
		{
			ImGui::Separator();
			// First item in top menu
			if (ImGui::BeginMenu("File"))
			{
				// ======================== Scene Section ===========================
				// Under file menu list
				// ------------- Create New Scene -------------
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{
					// Clear current scene
					m_Scene->GetRegistry().clear();
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Create new scene.");
				}
				// --------------- Open Scene -------------------
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
				{
					openScenePanel = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Open Scene from file.");
				}
				// ------------------ Save as Scene -----------------------
				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				{
					if (!currScenePath.empty())
					{
						// Save to current path
						SceneSerializer serializer(m_Scene);
						if (serializer.Serialize(currScenePath))
						{
							LOG_INFO("Scene saved successfully to: ", currScenePath);
						}
						else
						{
							LOG_ERROR("Failed to save scene to: ", currScenePath);
						}
					}
					else
					{
						// No current path, open save dialog
						saveAsPanel = true;
					}
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Save scene as new file.");
				}

				ImGui::Separator();

				// ====================== Script Section ==========================
				// ---------------------- Open Script -------------------------
				if (ImGui::MenuItem("Open Script"))
				{

				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Open Script from file.");
				}
				// ---------------------- Create new script -------------------
				if (ImGui::MenuItem("New Script"))
				{
					createScript = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Create new script.");
				}

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				{
					saveAsPanel = true;
				}

				ImGui::Separator();

				// close File menu
				if (ImGui::MenuItem("Exit", "Alt+F4"))
				{
					// Signal the application to close
					glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
				}

				ImGui::EndMenu();
				ImGui::Separator();
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

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Hierarchy", NULL, &hierachyWindow);
				ImGui::MenuItem("Properties", NULL, &inspectorWindow);
				ImGui::MenuItem("Performance Profile", NULL, &performanceProfileWindow);
				ImGui::EndMenu();
			}

			// close main menu bar
			ImGui::EndMainMenuBar();
		} //  end of begin main menu bar

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

	void Editor::displayPropertiesPanel()
	{
		if (!inspectorWindow)
			return;

		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Properties/ Inspector", &inspectorWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			if (m_SelectedEntity) {

				if (m_SelectedEntity.HasComponent<TagComponent>())
				{
					auto& tag = m_SelectedEntity.GetComponent<TagComponent>();
					char buffer[256];
					strncpy_s(buffer, sizeof(buffer), tag.Tag.c_str(), _TRUNCATE);
					if (ImGui::InputText("Name", buffer, sizeof(buffer)))
					{
						tag.Tag = std::string(buffer);
					}

					//Backup Code
					//Display and Edit Entity Name
					/*auto& tag = m_SelectedEntity.GetComponent<TagComponent>().Tag;
					char entityNameBuffer[256];
					strcpy_s(entityNameBuffer, sizeof(entityNameBuffer), tag.c_str());

					// Add ImGuiInputTextFlags_EnterReturnsTrue to ensure only change name after user press enter
					// Game crash if delete the last alphabet since it keep updating the frame and cause a empty ID 
					// TODO: Check the above
					if (ImGui::InputText("Entity Name", entityNameBuffer, sizeof(entityNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {

						std::string newName = entityNameBuffer;
						if (newName.empty()) {
							newName = m_SelectedEntity.GetComponent<TagComponent>().Tag;
						}
						tag = newName;
					}*/
				}

				// Display entity ID
				ImGui::Text("Entity ID: %u", static_cast<uint32_t>(m_SelectedEntity));

				// Display component information
				ImGui::Separator();
				ImGui::Text("Components:");

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
					}

				}
				
				// Display other components...
			}
			else
			{
				ImGui::Text("No entity selected");
			}
		}

		ImGui::End(); // End of the properties window
	}

	void Editor::displayHierarchyPanel()
	{
		if (!hierachyWindow)
			return;

		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Hierarchy", &hierachyWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			// Button to create new entity
			if (ImGui::Button("Create Entity"))
			{
				auto entity = m_Scene->CreateEntity("New Entity");
				entity.AddComponent<TagComponent>("New Entity");
				entity.AddComponent<TransformComponent>();
			}

			ImGui::Separator();

			if (m_Scene) {

				// Get name of Scene
				// TODO: Check when Serialization is fixed
				auto& sceneName = m_Scene->GetName();
				char sceneNameBuffer[256];
				strcpy_s(sceneNameBuffer, sizeof(sceneNameBuffer), sceneName.c_str());

				// Add ImGuiInputTextFlags_EnterReturnsTrue to ensure only change name after user press enter
				if (ImGui::InputText("Scene Name", sceneNameBuffer, sizeof(sceneNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {

					std::string newSceneName = sceneNameBuffer;
					if (newSceneName.empty()) {
						newSceneName = m_Scene->GetName();
					}
					m_Scene->SetName(newSceneName);
				}

				// List of entities
				auto view = m_Scene->GetRegistry().view<TagComponent>();

				for (auto entityHandle : view)
				{
					Entity entity(entityHandle, &m_Scene->GetRegistry());
					auto& tag = entity.GetComponent<TagComponent>();

					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					if (m_SelectedEntity == entity)
					{
						flags |= ImGuiTreeNodeFlags_Selected;
					}

					ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());

					if (ImGui::IsItemClicked())
					{
						m_SelectedEntity = entity;
					}

					// Right-click context menu
					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::MenuItem("Delete Entity"))
						{
							m_Scene->DestroyEntity(entity);
							if (m_SelectedEntity == entity)
							{
								m_SelectedEntity = Entity();
							}
						}
						ImGui::EndPopup();
					}
				}
			}
		}

		ImGui::End(); // End of the properties window
	}

	void Editor::displayAssetsBrowserPanel()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Assets Browser", &assetsWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{

		}

		ImGui::End(); // End of the assets browser window
	}

	void Editor::displayPerformanceProfilePanel()
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100));
		if (ImGui::Begin("Performance Profile", &performanceProfileWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{

		}
		ImGui::End(); // end of performance profile panel
	}

	void Editor::StartImguiFrame() {

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

	}

	void Editor::renderViewport()
	{
		// Logic here 

	}

	// Render after Render System
	void Editor::RenderEditor() {

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

	// Helper function for top menu 
	void Editor::sceneOpenPanel()
	{
		if (!openScenePanel)
			return;

		ImGui::OpenPopup("Open Scene");

		if (ImGui::BeginPopupModal("Open Scene", &openScenePanel))
		{
			static char scenePath[256] = "Resources/Scenes/";

			ImGui::Text("Enter scene file path:");
			ImGui::InputText("##scenepath", scenePath, sizeof(scenePath));

			if (ImGui::Button("Open"))
			{
				SceneSerializer serializer(m_Scene);
				if (serializer.Deserialize(scenePath))
				{
					currScenePath = scenePath;
					LOG_INFO("Scene loaded successfully from: " + std::string(scenePath));
				}
				else
				{
					LOG_ERROR("Failed to load scene from: " + std::string(scenePath));
				}
				openScenePanel = false;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				openScenePanel = false;
			}

			ImGui::EndPopup();
		}

		//Backup Code
		// get all files inside scene
		/*auto sceneFiles = getFilesInFolder("Scenes");

		if (openScenePanel)
		{
			ImGui::OpenPopup("Scene Level Selection");
		}

		// pop up panel to open scene file
		if (ImGui::BeginPopupModal("Scene Level Selection", nullptr, ImGuiWindowFlags_NoDocking))
		{
			ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);

			// list all scene files
			for (auto& [fileName, fullPath] : sceneFiles)
			{
				if (ImGui::Selectable(fileName.c_str()))
				{

					if (!m_Scene)
					{
						LOG_ERROR("No active scene exists to load into!");
						continue;
					}

					// clear current scene
					auto& registry = m_Scene->GetRegistry();
					registry.clear();

					// load the selected scene file
					if (!m_Scene->LoadFromFile(fullPath))
					{
						//LOG_ERROR("Failed to load scene %s", sceneFiles);
					}
					openScenePanel = false; //  reset after select scene
					ImGui::CloseCurrentPopup();

				}
			}
			// --------------- Cancel Selection for Open Scene -----------------------
			if (ImGui::Button("Cancel"))
			{
				openScenePanel = false; //  reset after click cancel button
				ImGui::CloseCurrentPopup();
			}


			ImGui::EndPopup(); // end pop up panel for scene level selection
		}*/
	}

	std::vector<std::pair<std::string, std::string>> Editor::getFilesInFolder(const std::string& folderName)
	{
		std::string folderPath = Engine::getAssetFilePath(folderName);

		assert(!folderPath.empty() && "Folder path is empty!");
		assert(std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath) && "Folder does not exist!");

		std::vector<std::pair<std::string, std::string>> files;

		for (const auto& entry : std::filesystem::directory_iterator(folderPath))
		{
			if (std::filesystem::is_regular_file(entry.path()))
			{
				// .first = filename, .second = full path
				files.emplace_back(entry.path().filename().string(), entry.path().generic_string());
			}
		}

		return files;
	}
	void Editor::saveAsScenePanel()
	{
		if (!saveAsPanel)
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
		}

		// Backup Code
		/*if (saveAsPanel)
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
					std::string defaultNewScenePath = getAssetFilePath("Scene/") + saveAsDefaultSceneName;
					if (!std::filesystem::path(defaultNewScenePath).has_extension()) {

						defaultNewScenePath += ".json"; // ensure .json extension
					}

					if (std::filesystem::exists(defaultNewScenePath))
					{
						ImGui::OpenPopup("Confirm Overwrite"); // if save as name repeat, open confirmation panel for overwrite it
					}
					else
					{
						//// Ensure the Scene directory exists
						//std::string sceneDir = getAssetFilePath("Scene/");
						//if (!std::filesystem::exists(sceneDir))
						//	std::filesystem::create_directories(sceneDir);

						//std::string defaultNewScenePath = sceneDir + saveAsDefaultSceneName;
						//if (!std::filesystem::path(defaultNewScenePath).has_extension())
						//	defaultNewScenePath += ".json"; // ensure .json extension
						//if (std::filesystem::exists(defaultNewScenePath))
						//{
						//	ImGui::OpenPopup("Confirm Overwrite");
						//}
						//else
						//{
						//	// Try saving and check if it succeeds
						//	try
						//	{
						//		m_Scene->SaveToFile(defaultNewScenePath);
						//		LOG_DEBUG("Scene save as: ", defaultNewScenePath);
						//		currScenePath = defaultNewScenePath;
						//		saveAsPanel = false;
						//		ImGui::CloseCurrentPopup();
						//	}
						//	catch (const std::exception& e)
						//	{
						//		LOG_ERROR("Failed to save scene: ", e.what());
						//		ImGui::OpenPopup("Save Error");
						//	}
						//}
						m_Scene->SaveToFile(defaultNewScenePath); // save scene file
						LOG_DEBUG("Scene save as: ", defaultNewScenePath);
						currScenePath = defaultNewScenePath; // update current scene path
						saveAsPanel = false; // to close pop up
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
					std::string defaultNewScenePath = getAssetFilePath("Scene/") + saveAsDefaultSceneName;
					if (!std::filesystem::path(defaultNewScenePath).has_extension()) {
						defaultNewScenePath += ".json"; // ensure .json extension
					}
					std::cout << defaultNewScenePath << "json file test\n";
					m_Scene->SaveToFile(defaultNewScenePath);
					currScenePath = defaultNewScenePath;

					saveAsPanel = false;
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
		}*/
	}

} // end of namespace Engine

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

// Include other necessary headers
#include <GLFW/glfw3.h>

namespace Engine
{
	void Editor::SetScene(Engine::Scene* scene)
	{
		m_Scene = scene;
	}

	void Editor::OnInit(GLuint texhandle)
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
		ImGui_ImplOpenGL3_Init();

		// Save created FBO texture handle
		m_FBOTextureHandle = texhandle;

		m_Initialized = true;

	}

	void Editor::OnUpdate(Timestep ts)
	{
		if (!m_Initialized) return;

		// Start ImGui Frame
		StartImguiFrame();

		displayTopMenu();

		//// Enable Docking Function
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		renderViewport();

		// Panel Logic
		displayPropertiesPanel();

		displayHierarchyPanel();

		displayAssetsBrowserPanel();

		displayPerformanceProfilePanel();
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
				if (ImGui::MenuItem("New"))
				{

				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Create new scene.");
				}
				// --------------- Open Scene -------------------
				if (ImGui::MenuItem("Open Scene"))
				{
					openScenePanel = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Open Scene from file.");
				}
				// ------------------ Save as Scene -----------------------
				if (ImGui::MenuItem("Save as"))
				{
					saveAsPanel = true;
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
				// close File menu
				ImGui::EndMenu();
				ImGui::Separator();
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
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Properties/ Inspector", &inspectorWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			if (m_SelectedEntity) {

				if (m_SelectedEntity.HasComponent<TagComponent>())
				{
					//Display and Edit Entity Name
					auto& tag = m_SelectedEntity.GetComponent<TagComponent>().Tag;
					char entityNameBuffer[128];
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
					}
				}
				
				// Display entity ID
				ImGui::Text("Entity ID: %u", static_cast<uint32_t>(m_SelectedEntity));
				
				// Display component information
				ImGui::Separator();
				ImGui::Text("Components:");

				if (m_SelectedEntity.HasComponent<TransformComponent>())
				{
					// TODO: Make the functions for collapsing headers
					if (ImGui::CollapsingHeader("Transform")) {
						auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
						ImGui::DragFloat3("Position", &transform.Position.x, 0.1f);
						ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.1f);
						ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
					}

				}
			}
		}

		ImGui::End(); // End of the properties window
	}

	void Editor::displayHierarchyPanel()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Hierarchy", &hierachyWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			if (m_Scene) {

				// Get name of Scene
				// TODO: Check when Serialization is fixed
				auto& sceneName = m_Scene->GetName();
				char sceneNameBuffer[128];
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
				auto& registry = m_Scene->GetRegistry();
				auto view = registry.view<TagComponent>();

				for (auto entityHandle : view) {

					Entity entity(entityHandle, &registry);

					// Get entity name
					std::string name = "Unnamed Entity";
					if (entity.HasComponent<TagComponent>())
					{
						name = entity.GetComponent<TagComponent>().Tag;
					}

					if (ImGui::Selectable(name.c_str(), (m_SelectedEntity == entity)))
					{
						m_SelectedEntity = entity;
						
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

		ImVec2 texture_pos = ImGui::GetCursorScreenPos();

		// Hard-coded values just in case (Will change values later)
		ImVec2 viewportSize =
		{
			 600,
			 600
		};

		if (m_Window) {

			int width = 0.f;
			int height = 0.f;
			glfwGetWindowSize(m_Window, &width, &height);

			viewportSize =
			{
				 static_cast<float>(static_cast<float>(width)) / 2.0f,
				 static_cast<float>(static_cast<float>(height)) / 2.0f
			};

		}

		ImGui::Begin("Viewport");

		// Uncomment this once the viewport texture has been obtained
		if (m_FBOTextureHandle) {
			ImVec2 imagePos = ImGui::GetCursorScreenPos();

			ImGui::Image((ImTextureID)(intptr_t)m_FBOTextureHandle,
				viewportSize,
				ImVec2(0, 1), ImVec2(1, 0));

			// TODO: Handle mouse in viewport
			//handleViewPortClick(imagePos, viewportSize);
		}

		ImGui::End();
		
		
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
		// get all files inside scene
		auto sceneFiles = getFilesInFolder("Scenes");

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
		}
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
		}
	}

} // end of namespace Engine

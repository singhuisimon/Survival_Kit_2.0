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
		ImGui_ImplOpenGL3_Init();

		m_Initialized = true;

	}

	void Editor::OnUpdate(Timestep ts)
	{
		if (!m_Initialized) return;

		// Start ImGui Frame
		StartImguiFrame();

		displayTopMenu();

		// Enable Docking Function
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

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

		}
	}

	void Editor::displayPropertiesPanel()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Properties/ Inspector", &inspectorWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			
		}

		ImGui::End(); // End of the properties window
	}

	void Editor::displayHierarchyPanel()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Hierarchy", &hierachyWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{

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
		// get all files inside scene
		auto sceneFiles = getFilesInFolder("Scene");

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
					
				}
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
} // end of namespace Engine

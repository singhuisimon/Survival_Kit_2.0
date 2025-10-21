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
				// Under file menu list
				if(ImGui::MenuItem("New"))
				{

				}
				if (ImGui::IsAnyItemHovered())
				{
					ImGui::SetTooltip("Create new scene.");
				}
				// close File menu
				ImGui::EndMenu();
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
} // end of namespace Engine

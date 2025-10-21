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
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Set config flag
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();

		// Set WindowRounding and ImGuiCol_WindowBg when viewport is enabled
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		ImGui_ImplOpenGL3_Init();

		m_Initialized = true;
		
	}
} // end of namespace Engine

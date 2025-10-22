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

		//// Enable Docking Function
		//ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		renderViewport();

		// Panel Logic
		displayPropertiesPanel();

		displayHierarchyPanel();

		displayAssetsBrowserPanel();

		displayPerformanceProfilePanel(ts);
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
					if (m_Scene)
					{
						m_Scene->GetRegistry().clear();
						currScenePath = "";
						isNewScene = true;
					}
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
				// --------------- Save Scene -------------------
				if (ImGui::MenuItem("Save"))
				{
					if (!currScenePath.empty())
					{
						m_Scene->SaveToFile(currScenePath);
						LOG_INFO("Current scene path: ", currScenePath);
					}
					else
					{
						saveAsPanel = true; // redirect to Save as if the current scene is empty
						if (isNewScene)
						{
							saveAsPanel = true;
						}
						//LOG_INFO("Current scene has not been saved yet (no file path).");
					}
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
			// ---------------- Display Current Scene Name ---------------------
			if (!currScenePath.empty())
			{
				std::filesystem::path filePath(currScenePath);
				std::string fileName = filePath.filename().string();

				ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80.0f);
				ImGui::TextUnformatted(fileName.c_str());
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
			
			ImGui::Columns(2, nullptr, true);
			static std::string selectedFolder = "";
			// ================= Left column panel display all the resources folder ========================
			ImGui::BeginChild("Project List", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Text("Projects:");
			if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto folders = getAssetsInFolder(getAssetFilePath("Sources/"));

				for (auto& folder : folders)
				{
					bool isSelected = (selectedFolder == folder.fullPath);
					if (ImGui::Selectable(folder.name.c_str(), isSelected))
					{
						selectedFolder = folder.fullPath;
						selectedResourcesIndex = -1; // reset asset selection
					}
				}
			}
			ImGui::EndChild(); // end the left column 

			// ===================== Right column panel display the resources files =======================

			ImGui::NextColumn();
			ImGui::BeginChild("Assets Panel", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			// if folder is selected, display the files
			if (!selectedFolder.empty())
			{
				// to get the files in the selected folder
				auto assetsList = getAssetsInFolder(selectedFolder); 
				// display the selected folder name
				ImGui::Text(("Assets > " + selectedFolder).c_str());
				ImGui::Separator();

				const float padding = 10.0f;
				const float thumbnailSize = 64.0f;
				const float cellSize = thumbnailSize + padding;
				float panelWidth = ImGui::GetContentRegionAvail().x;
				int itemsPerRow = std::max(1, (int)(panelWidth / cellSize));

				int textureCount = -1;
				ImGui::Columns(itemsPerRow, nullptr, false);

				// loop through files in selected folder
				for (size_t i = 0; i < assetsList.size(); i++)
				{
					const auto& asset = assetsList[i];
					std::string fileName = asset.name;
					std::string filePath = asset.fullPath;

					/*if (asset.name.ends_with(".png") || asset.name.ends_with(".jpeg"))
					{
						++textureCount;
					}*/

					ImGui::PushID(fileName.c_str());

					if (ImGui::Button(fileName.c_str(), ImVec2(thumbnailSize, thumbnailSize)))
					{
						selectedResourcesIndex = static_cast<int>(i);
						//currScenePath = filePath;

						std::string extension = asset.name.substr(asset.name.find_last_of('.'));
						if (extension == ".json") // if it is scene
						{
							if (m_Scene)
							{
								m_Scene->GetRegistry().clear();
								m_Scene->LoadFromFile(filePath);
								currScenePath = filePath; // update curr file path
							}
						}
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

			ImGui::EndChild(); // end of the right column
			ImGui::Columns(1);
		}
		ImGui::End(); // End of the assets browser window
	}

	void Editor::displayPerformanceProfilePanel(Timestep ts)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 300));
		if (ImGui::Begin("Performance Profile", &performanceProfileWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("Tracy Window:");
			if (ImGui::Button("Launch Tracy Window"))
			{
				// TO DO: Tracy Running 
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
		// TODO: Get Texture from Graphics
		// auto texture = GFXM.getImguiTex();

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
		/*if (texture) {

			ImVec2 imagePos = ImGui::GetCursorScreenPos();

			ImGui::Image((ImTextureID)(intptr_t)GFXM.getImguiTex(),
				viewportSize,
				ImVec2(0, 1), ImVec2(1, 0));

			// TODO: Handle mouse in viewport
			//handleViewPortClick(imagePos, viewportSize);
		}*/

		ImGui::End();
		
		
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
			for (auto& scenesAsset: sceneFiles)
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
					auto& registry = m_Scene->GetRegistry();
					registry.clear();
					
					// load the selected scene file
					if (m_Scene->LoadFromFile(scenesAsset.fullPath))
					{
						//LOG_ERROR("Failed to load scene %s", sceneFiles);
						currScenePath = scenesAsset.fullPath;
						LOG_INFO("Scene loaded successfully: ", currScenePath);
						openScenePanel = false; //  reset after select scene
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
					std::cout << defaultNewScenePath << "json file test\n";
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

	


} // end of namespace Engine
